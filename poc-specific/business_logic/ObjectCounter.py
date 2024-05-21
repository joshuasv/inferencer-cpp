import time

class ObjectCounter:

  def __init__(self, expected_classes: list):
    self.classes_counter = {
      str(c): {}
      for c in expected_classes
    }
    self.classes = [str(cls) for cls in expected_classes]

  def update(self, class_id, track_id, missing_thr_ms):
    # Check that class_id is in the expected classes
    if class_id in self.classes:
      curr_time = time.time_ns()
      # Update
      if not track_id in self.classes_counter[class_id]:
        self.classes_counter[class_id][track_id] = {}
        self.classes_counter[class_id][track_id]['first'] = curr_time
      self.classes_counter[class_id][track_id]['last'] = curr_time

      self._check_missing(missing_thr_ms)

  def _check_missing(self, milliseconds):
    current_items = list(self.classes_counter.items())
    for class_id, tracks in current_items:
      current_tracks = list(tracks.items())
      for track_id, timestamp in current_tracks:
        elapsed = (time.time_ns() - timestamp['last']) * 1.e-6
        if elapsed > milliseconds:
          print(f"Deleted: {class_id=} {track_id=}")
          del self.classes_counter[class_id][track_id]
          break # Stop checking track was deleted
  
  def count(self, active_thr_ms):
    counts = {}
    for class_id, tracks in self.classes_counter.items():
      counts[class_id] = 0
      for timestamp in tracks.values():
        # Check if it has been detected for active_thr_ms
        elapsed = (time.time_ns() - timestamp['first']) * 1.e-6
        if elapsed >= active_thr_ms:
          counts[class_id] += 1

    return counts