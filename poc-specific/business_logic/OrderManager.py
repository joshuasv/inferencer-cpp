from Order import Order
from ClientAPI import ClientAPI
from SharedQueue import SharedQueue
from QueueFeeder import QueueFeeder
from RedisHandler import RedisHandler
from ObjectCounter import ObjectCounter

class OrderManager:

  def __init__(self, db_host, db_port, redis_host, redis_port, redis_db_id, redis_stream):
    self.redis = RedisHandler(redis_host, redis_port, redis_db_id, redis_stream)
    self.api = ClientAPI(base_url=f"http://{db_host}:{db_port}")
    self.shared_queue = SharedQueue()
    self.queue_feeder = QueueFeeder(self.shared_queue, self.redis)


  def fill_order(self, order_id=None):
    if order_id is None:
      order = self.api.get_unfilled_order()
      self._start_filling_order(order)
    else:
      print("Not implemented yet.")

  def _start_filling_order(self, order: Order):
    print(f"Start filling order: {order.id}")
    # Send message through Redis
    self.redis.notify_new_order(order)

    # Reset SharedQueue 
    self.shared_queue.reset()

    # Launch thread that fills SharedQueue with Redis messages
    self.queue_feeder.start()

    # Consume the SharedQueue until order is filled and count objects
    order_objects = {str(obj.class_id): obj.quantity for obj in order.objects}
    object_counter = ObjectCounter(expected_classes=list(order_objects.keys()))
    while not self._is_order_filled(order_objects, object_counter.count(active_thr_ms=1000)):
      print("Needed:")
      print(order_objects)
      print("Current:")
      print(object_counter.classes_counter)
      msg = self.shared_queue.get_next_item()
      object_counter.update(
        class_id=msg["classId"], track_id=msg["trackId"], missing_thr_ms=1000)
    
    # Order was filled
    self.api.set_fulfill_order(order.id)
    self.queue_feeder.stop()
    self.redis.notify_new_order() # clean status
    print("Order filled!")

  def _is_order_filled(self, objects1, objects2):
    assert objects1.keys() == objects2.keys(), f"Orders should have the same object ids. Got {objects1=} and {objects2=}"
    keys = objects1.keys()
    for k in keys:
      if objects1[k] != objects2[k]:
        return False
    return True