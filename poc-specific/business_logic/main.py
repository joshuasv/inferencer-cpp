import argparse
from OrderManager import OrderManager

def menu():
  print("Menu:")
  print("1. Fill next order")
  print("2. Fill order by ID")
  print("3. Exit")


def main(args):
  om = OrderManager(args.db_host, args.db_port, args.redis_host, args.redis_port, args.redis_db_id, args.redis_stream)

  while True:
    menu()
    user_option = input("Select an option: ").strip().lower()
    if user_option == "1":
      om.fill_order()
    elif user_option == "2":
      print("Not implemented yet.")
    elif user_option == "3":
      break
    else:
      print("Option not considered. Try again.")

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='Run the business logic.')
  parser.add_argument('--db_host', type=str, help='Database host to bind to.')
  parser.add_argument('--db_port', type=int, help='Database port to bind to.')
  parser.add_argument('--redis_host', type=str, help='Redis host to bind to.')
  parser.add_argument('--redis_port', type=int, help='Redis port to bind to.')
  parser.add_argument('--redis_db_id', type=int, help='Redis database id to bind to.')
  parser.add_argument('--redis_stream', type=str, help='Redis stream name to bind to.')

  args = parser.parse_args()
  main(args)
  