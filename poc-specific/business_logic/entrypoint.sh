#!/bin/sh
set -e

# Run the application
exec python main.py --db_host "$SQLITE_HOST" --db_port "$SQLITE_PORT" \
  --redis_host "$REDIS_HOST" --redis_port "$REDIS_PORT" \
  --redis_db_id "$REDIS_DB_ID" --redis_stream "$REDIS_STREAM"
