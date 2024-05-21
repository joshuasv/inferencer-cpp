import threading

class QueueFeeder:
    
    def __init__(self, queue, redis_handler):
        self.queue = queue
        self.redis_handler = redis_handler
        self.stop_event = threading.Event()
        self.thread = threading.Thread(target=self._start_feeding_queue)
        self.thread.start()

    def _start_feeding_queue(self):
        while True:
            if self.stop_event.is_set():
                continue
            messages = self.redis_handler.read_stream(block=1000)  # Block for 1 second
            for message in messages:
                if self.queue.queue.full():
                    self.queue.queue.get()
                self.queue.queue.put(message)

    def start(self):
        self.stop_event.clear()

    def stop(self):
        self.stop_event.set()
        self.queue.reset()

    def kill(self):
        if self.thread.is_alive():
            self.stop_event.set()
            self.thread.join()
            print("Feeding thread stopped.")
            self.queue.reset()