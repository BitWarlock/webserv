#!/usr/bin/env python3
import os
import sys
import time
import random
import json
import hashlib

def get_session_id():
    """Get session ID from cookies or create new one"""
    cookies = os.environ.get('HTTP_COOKIE', '')
    if 'session_id=' in cookies:
        for cookie in cookies.split(';'):
            if 'session_id=' in cookie:
                return cookie.split('=')[1].strip()
    return None

def create_session_id():
    """Create a new session ID"""
    timestamp = int(time.time())
    random_num = random.randint(1000, 9999)
    return f"session_{timestamp}_{random_num}"

def get_session_data(session_id):
    """Get session data from file"""
    session_file = f"/tmp/webserv_sessions/{session_id}.json"
    try:
        with open(session_file, 'r') as f:
            return json.load(f)
    except:
        return {"created": time.time(), "last_access": time.time(), "visits": 0}

def save_session_data(session_id, data):
    """Save session data to file"""
    session_file = f"/tmp/webserv_sessions/{session_id}.json"
    try:
        with open(session_file, 'w') as f:
            json.dump(data, f)
    except:
        pass

def ensure_session_directory():
    """Ensure session directory exists"""
    import os
    session_dir = "/tmp/webserv_sessions"
    if not os.path.exists(session_dir):
        os.makedirs(session_dir, exist_ok=True)

# Ensure session directory exists
ensure_session_directory()

# Get or create session ID
session_id = get_session_id()
if not session_id:
    session_id = create_session_id()

# Get session data
session_data = get_session_data(session_id)
session_data["last_access"] = time.time()
session_data["visits"] += 1
save_session_data(session_id, session_data)

# Output CGI headers
print("Content-Type: text/html")
print(f"Set-Cookie: session_id={session_id}; Path=/; HttpOnly; SameSite=Strict; Max-Age=31536000")
print()

# Output HTML
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Session Manager</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 40px; }}
        .info {{ background: #e3f2fd; padding: 20px; border-radius: 5px; margin: 20px 0; }}
        .success {{ background: #d4edda; padding: 20px; border-radius: 5px; margin: 20px 0; }}
        .warning {{ background: #fff3cd; padding: 20px; border-radius: 5px; margin: 20px 0; }}
        .data {{ background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 10px 0; font-family: monospace; }}
    </style>
</head>
<body>
    <div class="success">
        <h2>✅ Session Active!</h2>
        <p><strong>Session ID:</strong> {session_id}</p>
        <p><strong>Created:</strong> {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(session_data['created']))}</p>
        <p><strong>Last Access:</strong> {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(session_data['last_access']))}</p>
        <p><strong>Visits:</strong> {session_data['visits']}</p>
    </div>
    
    <div class="info">
        <h3>🔧 How This Works:</h3>
        <ul>
            <li>Server automatically creates session when you visit</li>
            <li>Session persists even if you delete cookies</li>
            <li>Server re-establishes cookie on every visit</li>
            <li>Session data stored server-side in /tmp/webserv_sessions/</li>
        </ul>
    </div>
    
    <div class="warning">
        <h3>🧪 Test Instructions:</h3>
        <ol>
            <li>Check F12 → Application → Cookies (you'll see session_id)</li>
            <li>Delete the cookie in F12 → Application → Cookies</li>
            <li>Refresh this page - cookie will reappear!</li>
            <li>Close browser completely and reopen</li>
            <li>Visit this page again - session continues!</li>
        </ol>
    </div>
    
    <div class="data">
        <strong>Session Data:</strong><br>
        {json.dumps(session_data, indent=2)}
    </div>
    
    <p><a href="/www/cookie_demo.html">← Back to Cookie Demo</a></p>
</body>
</html>
""") 