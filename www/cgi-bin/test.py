#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print("<html>")
print("<head><title>Python CGI Test</title></head>")
print("<body>")
print("<h1>Python CGI Test</h1>")
print("<p>Request Method: " + os.environ.get('REQUEST_METHOD', 'N/A') + "</p>")
print("<p>Query String: " + os.environ.get('QUERY_STRING', 'N/A') + "</p>")
print("<p>Script Name: " + os.environ.get('SCRIPT_NAME', 'N/A') + "</p>")
print("<p>Content Length: " + os.environ.get('CONTENT_LENGTH', 'N/A') + "</p>")
if os.environ.get('REQUEST_METHOD', 'N/A') == 'POST':
    if os.environ.get('CONTENT_LENGTH', 'N/A'):
        # Read POST data from stdin
        post_data = sys.stdin.read(int(os.environ.get('CONTENT_LENGTH', 'N/A')))
        print(f"Post data: {post_data}")
print("<p>Current Working Directory: " + os.getcwd() + "</p>")
print("<p>Script Path: " + sys.argv[0] if len(sys.argv) > 0 else "No script path" + "</p>")
print("<p>Python Path: " + sys.executable + "</p>")
print("</body>")
print("</html>")
