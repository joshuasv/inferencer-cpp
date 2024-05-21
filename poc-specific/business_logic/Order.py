class Order:
  def __init__(self, order_data):
    """Initialize an Order object with data from the JSON response."""
    self.id = order_data.get('id')
    self.timestamp = order_data.get('timestamp')
    self.fulfilled = order_data.get('fulfilled')
    self.objects = [OrderObject(obj) for obj in order_data.get('objects', [])]

  def __str__(self):
    object_details = ', '.join(str(obj) for obj in self.objects)
    return f"Order ID: {self.id}, Timestamp: {self.timestamp}, Fulfilled: {self.fulfilled}, Objects: [{object_details}]"
    
class OrderObject:
  def __init__(self, object_data):
    """Initialize an OrderObject with individual object data."""
    self.id = object_data.get('id')
    self.class_id = int(object_data.get('class_id'))
    self.quantity = int(object_data.get('quantity'))
    self.display_name = object_data.get('display_name')

  def __str__(self):
    return f"Object ID: {self.id}, Name: {self.display_name}, Quantity: {self.quantity}"