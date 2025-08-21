#!/bin/bash

# Configuration
SERVER_HOST="localhost"
SERVER_PORT="8004"
ENDPOINT="/uploads"
JSON_DATA='{"test": "data", "number": 12345}'

echo "Sending uncompressed (identity) request to ${SERVER_HOST}:${SERVER_PORT}${ENDPOINT}"

# Get the content length of the raw JSON data
CONTENT_LENGTH=${#JSON_DATA}

echo "Original data: $JSON_DATA"
echo "Content length: $CONTENT_LENGTH bytes"

# Send the HTTP request with no compression (identity encoding)
(printf "POST %s HTTP/1.1\r\n" "$ENDPOINT"; \
 printf "Host: %s:%s\r\n" "$SERVER_HOST" "$SERVER_PORT"; \
 printf "Content-Type: application/json\r\n"; \
 printf "Content-Encoding: identity\r\n"; \
 printf "Content-Length: %s\r\n\r\n" "$CONTENT_LENGTH"; \
 printf "%s" "$JSON_DATA") | nc "$SERVER_HOST" "$SERVER_PORT"

echo -e "\nRequest completed."