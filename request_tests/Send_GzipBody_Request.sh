#!/bin/bash

# Configuration
SERVER_HOST="localhost"
SERVER_PORT="8004"
ENDPOINT="/uploads"
JSON_DATA='{"test": "data", "number": 12345}'

# Create temporary file for compressed data
TEMP_FILE="/tmp/gzip_request_$$.bin"

echo "Sending GZIP compressed request to ${SERVER_HOST}:${SERVER_PORT}${ENDPOINT}"

# Compress the JSON data using gzip
echo "$JSON_DATA" | node -e "
const zlib = require('zlib');
process.stdin.pipe(zlib.createGzip()).pipe(process.stdout);
" > "$TEMP_FILE"

# Get the content length
CONTENT_LENGTH=$(wc -c < "$TEMP_FILE")

echo "Original data: $JSON_DATA"
echo "Compressed size: $CONTENT_LENGTH bytes"

# Send the HTTP request
(printf "POST %s HTTP/1.1\r\n" "$ENDPOINT"; \
 printf "Host: %s:%s\r\n" "$SERVER_HOST" "$SERVER_PORT"; \
 printf "Content-Type: application/json\r\n"; \
 printf "Content-Encoding: gzip\r\n"; \
 printf "Content-Length: %s\r\n\r\n" "$CONTENT_LENGTH"; \
 cat "$TEMP_FILE") | nc "$SERVER_HOST" "$SERVER_PORT"

# Clean up
rm -f "$TEMP_FILE"

echo -e "\nRequest completed."