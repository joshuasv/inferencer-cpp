from flask import Flask, request, jsonify, render_template
from flask_sqlalchemy import SQLAlchemy
from flask_migrate import Migrate
import json
import uuid
from datetime import datetime
import random
import argparse

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///mydatabase.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)
migrate = Migrate(app, db)

class Object(db.Model):
    id = db.Column(db.String(36), primary_key=True)
    displayName = db.Column(db.String(120), unique=True, nullable=False)
    classId = db.Column(db.Integer, unique=True, nullable=False)

class Order(db.Model):
    id = db.Column(db.String(36), primary_key=True)
    timestamp = db.Column(db.DateTime, default=datetime.now)
    objects = db.Column(db.Text)  # This will store a JSON string of objects
    fulfilled = db.Column(db.Boolean, default=False, nullable=False)

# Create the database and tables
with app.app_context():
  db.create_all()

@app.route('/')
def index():
    return render_template('index.html')

# Add an object
@app.route('/objects', methods=['POST'])
def add_object():
    data = request.json
    if 'displayName' not in data or 'classId' not in data:
        return jsonify({'error': 'Missing displayName or classId'}), 400
    if Object.query.filter_by(displayName=data['displayName']).first() is not None:
        return jsonify({'error': 'Display name must be unique'}), 400
    if Object.query.filter_by(classId=data['classId']).first() is not None:
        return jsonify({'error': 'Class ID must be unique'}), 400
    new_object = Object(id=str(uuid.uuid4()), displayName=data['displayName'], classId=data['classId'])
    db.session.add(new_object)
    db.session.commit()
    return jsonify({'id': new_object.id, 'displayName': new_object.displayName, 'classId': new_object.classId}), 201

# Get an object
@app.route('/objects/<object_id>', methods=['GET'])
def get_object_by_id(object_id):
    obj = Object.query.get(object_id)
    if obj:
        return jsonify({'id': obj.id, 'displayName': obj.displayName, 'classId': obj.classId}), 200
    else:
        return jsonify({'error': 'Object not found'}), 404

# Delete an object
@app.route('/objects/<object_id>', methods=['DELETE'])
def delete_object(object_id):
    obj = Object.query.get(object_id)
    if obj:
        db.session.delete(obj)
        db.session.commit()
        return jsonify({'message': 'Object deleted successfully'}), 200
    return jsonify({'error': 'Object not found'}), 404

# Edit an object
@app.route('/objects/<object_id>', methods=['PUT'])
def edit_object(object_id):
    data = request.json
    obj = Object.query.get(object_id)
    if obj is None:
        return jsonify({'error': 'Object not found'}), 404
    if 'displayName' in data and data['displayName'] and data['displayName'] != obj.displayName:
        if Object.query.filter_by(displayName=data['displayName']).first() is not None:
            return jsonify({'error': 'Display name must be unique'}), 400
        obj.displayName = data['displayName']
    if 'classId' in data and data['classId'] and data['classId'] != obj.classId:
        if Object.query.filter_by(classId=data['classId']).first() is not None:
            return jsonify({'error': 'Class ID must be unique'}), 400
        obj.classId = data['classId']
    db.session.commit()
    return jsonify({'id': obj.id, 'displayName': obj.displayName, 'classId': obj.classId}), 200

@app.route('/upload-objects', methods=['POST'])
def upload_objects():
    file = request.files['file']
    if not file:
        return jsonify({'error': 'No file provided'}), 400

    for line in file:
        classId, displayName = line.decode('utf-8').strip().split(',')
        if not Object.query.filter_by(classId=classId).first():
            new_object = Object(id=str(uuid.uuid4()), displayName=displayName, classId=classId)
            db.session.add(new_object)

    db.session.commit()
    return jsonify({'message': 'Objects created successfully'}), 201

# List all objects
@app.route('/objects', methods=['GET'])
def get_objects():
    all_objects = Object.query.all()
    objects_list = [{'id': obj.id, 'displayName': obj.displayName, 'classId': obj.classId} for obj in all_objects]
    return jsonify(objects_list), 200


# Generate a random order
@app.route('/orders', methods=['GET'])
def generate_order():
    order_id = str(uuid.uuid4())
    complex = request.args.get('complex', 'false').lower() == 'true'
    max_number = int(request.args.get('max', 1))
    all_objects = Object.query.all()
    selected_objects = random.sample(all_objects, random.randint(1, len(all_objects)))
    order_objects = []
    for obj in selected_objects:
        quantity = random.randint(1, max_number) if complex else 1
        order_objects.append({'id': obj.id, 'class_id': obj.classId, 'quantity': quantity, 'display_name': obj.displayName})
    # The fulfilled attribute is set to False by default and not modified
    new_order = Order(id=order_id, objects=json.dumps(order_objects), fulfilled=False)
    db.session.add(new_order)
    db.session.commit()
    return jsonify({'id': new_order.id, 'timestamp': new_order.timestamp, 'objects': order_objects, 'fulfilled': new_order.fulfilled}), 200

# List all orders
@app.route('/orders/list', methods=['GET'])  # Changed endpoint to avoid conflict with generate_order
def get_orders():
    all_orders = Order.query.all()
    orders_list = [{'id': order.id, 'timestamp': order.timestamp.isoformat(), 'objects': order.objects, 'fulfilled': order.fulfilled} for order in all_orders]
    return jsonify(orders_list), 200

@app.route('/orders/<order_id>/fulfill', methods=['POST'])
def fulfill_order(order_id):
    order = Order.query.get(order_id)
    if not order:
        return jsonify({'error': 'Order not found'}), 404
    order.fulfilled = True
    db.session.commit()
    return jsonify({'message': 'Order marked as fulfilled', 'id': order.id}), 200

@app.route('/orders/<order_id>', methods=['DELETE'])
def delete_order(order_id):
    order = Order.query.get(order_id)
    if order:
        db.session.delete(order)
        db.session.commit()
        return jsonify({'message': 'Order deleted successfully'}), 200
    else:
        return jsonify({'error': 'Order not found'}), 404

@app.route('/orders/<order_id>', methods=['GET'])
def get_order_by_id(order_id):
    order = Order.query.get(order_id)
    if order:
        return jsonify({}), 200
        # return jsonify({'id': obj.id, 'displayName': obj.displayName, 'classId': obj.classId}), 200
    else:
        return jsonify({'error': 'Order not found'}), 404

@app.route('/orders/unfilled', methods=['GET'])
def get_unfilled_order():
    # Query for the first unfilled order
    unfilled_order = Order.query.filter_by(fulfilled=False).first()
    if unfilled_order:
        # If an unfilled order exists, serialize and return its data
        order_data = {
            'id': unfilled_order.id,
            'timestamp': unfilled_order.timestamp.isoformat(),
            'objects': json.loads(unfilled_order.objects),
            'fulfilled': unfilled_order.fulfilled
        }
        return jsonify(order_data), 200
    else:
        # If no unfilled order is found, return an error message
        return jsonify({'error': 'No unfilled orders found'}), 404

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run the Order API app.')
    parser.add_argument('--host', type=str, required=True, help='The to bind to.')
    parser.add_argument('--port', type=int, required=True, help='The port to bind to.')
    args = parser.parse_args()

    app.run(host=args.host, port=args.port, debug=True)