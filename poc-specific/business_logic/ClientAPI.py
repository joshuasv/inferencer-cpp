import time
import logging
import requests

from Order import Order

def retry_with_exponengial_backoff(fn, max_retries=5, backoff_factor=0.5):
  retries = 0
  while retries < max_retries:
    try:
      result = fn()
      return result
    except requests.RequestException as e:
      logging.error(f"Request error: {e}, retrying...")
      time.sleep(2 ** retries * backoff_factor)
      retries += 1
  logging.error("Max retries reached, function execution failed.")
  return None
  
class ClientAPI:

  def __init__(self, base_url):
    self.base_url = base_url
  
  def get_unfilled_order(self):
    def fetch_order():
      response = requests.get(f"{self.base_url}/orders/unfilled")
      if response.status_code == 200:
        return Order(response.json())
      elif response.status_code == 400:
        return "No unfilled orders found."
      response.raise_for_status()

    return retry_with_exponengial_backoff(fetch_order)

  def set_fulfill_order(self, order_id):
    def set_order():
      response = requests.post(f"{self.base_url}/orders/{order_id}/fulfill")
      if response.status_code == 200:
        return True
      response.raise_for_status()
    
    return retry_with_exponengial_backoff(set_order)

  
if __name__ == "__main__":

  client = ClientAPI("http://127.0.0.1:5000")
  print(client.get_unfilled_order())