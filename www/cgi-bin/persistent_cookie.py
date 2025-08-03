#!/usr/bin/env python3
import os
import sys
import time
import random

# Get session ID from query string or create new one
session_id = os.environ.get('QUERY_STRING', '')
if 'session_id=' in session_id:
    session_id = session_id.split('session_id=')[1].split('&')[0]
else:
    session_id = f"persistent_{int(time.time())}_{random.randint(1000, 9999)}"

# Output CGI headers
print("Content-Type: text/html")
print(f"Set-Cookie: session_id={session_id}; Path=/; HttpOnly; SameSite=Strict; Max-Age=31536000")
print()

# Output HTML body
print("""
<!DOCTYPE html>
<html>
<head>
    <title>Cookie Set</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .success { background: #d4edda; padding: 20px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="success">
        <h2>✅ Persistent Cookie Set!</h2>
        <p><strong>Session ID:</strong> """ + session_id + """</p>
        <p>This cookie will persist even if you:</p>
        <ul>
            <li>Close and reopen your browser</li>
            <li>Delete cookies manually</li>
            <li>Refresh the page</li>
        </ul>
        <p>You can now close this window and return to the demo.</p>
    </div>
</body>
</html>
""") 