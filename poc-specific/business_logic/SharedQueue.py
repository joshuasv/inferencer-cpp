import queue

class SharedQueue:

  def __init__(self, max_cap = 50):
    self.max_cap = max_cap
    self.queue = queue.LifoQueue(max_cap)

  def reset(self):
    self.queue = queue.LifoQueue(self.max_cap)

  def get_next_item(self):
    return self.queue.get()