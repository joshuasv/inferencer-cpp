import json

import redis

from Order import Order


class RedisHandler:

  def __init__(self, hostname="localhost", port=6379, db=0, stream_name="test"):
    self.client = redis.Redis(
      host=hostname,
      port=port,
      db=db,
      decode_responses=True
    )
    self.stream_name = stream_name
    self.last_id = "$"

  def read_stream(self, block=None, count=None):
    try:
      messages = self.client.xread(
        {self.stream_name: self.last_id},
        block=block,
        count=count
      )
      data_batch = []
      if messages:
        for _, message_list in messages:
          for message in message_list:
            message_id, data = message
            data_batch.append(data)
        self.last_id = message_id
      return data_batch
    
    except redis.RedisError as e:
      print(f"REDIS ERROR. Failed to read from stream {stream_name}: {e}")
      return []

  def notify_new_order(self, order: Order = None):
    if order is None:
      order_message = ""
    else:
      order_info = {
        "id": order.id,
        "timestamp": order.timestamp,
        "fulfilled": order.fulfilled,
        "objects": [
          o.__dict__
          for o in order.objects
        ]
      }
      order_message = json.dumps(order_info)
    self.client.publish("order_updates", order_message)

  def close_connection(self):
    self.client.close()

if __name__ == "__main__":
  import redis
  import time

  r = redis.Redis(host='localhost', port=6379, db=0)
  stream_name = 'test'
  last_id = '$'  # Starting at the most recent position of the stream

  def process_message(message):
    print(f"Processing: {message}")

  try:
    while True:
      # Read messages from the stream
      messages = r.xread({stream_name: last_id}, count=10, block=1000)
      if messages:
        for stream, message_list in messages:
          for message in message_list:
            message_id, data = message
            process_message(data)
            last_id = message_id  # Update last_id to the latest message processed
                  
      time.sleep(1)  # Prevent tight loop if no messages are available
  except KeyboardInterrupt:
    print("Stopping stream processing")