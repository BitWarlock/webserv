#!/usr/bin/env python3
import os
import sys
import time
import json
from urllib.parse import parse_qs

# === Helper Functions ===
def get_cookie(name):
    """Safely get cookie value"""
    cookies = os.environ.get('HTTP_COOKIE', '')
    if not cookies:
        return None
    for part in cookies.split(';'):
        part = part.strip()
        if part.startswith(f"{name}="):
            return part.split('=', 1)[1].strip()
    return None

def set_cookie(name, value, max_age=31536000):
    """Set a cookie"""
    value = value.replace(';', '%3B')  # Basic escaping
    print(f"Set-Cookie: {name}={value}; Path=/; HttpOnly; SameSite=Strict; Max-Age={max_age}")

def get_session_data(session_id):
    """Load session data from file"""
    session_file = f"/tmp/webserv_sessions/{session_id}.json"
    try:
        with open(session_file, 'r') as f:
            data = json.load(f)
            data.setdefault("visit_count", 0)
            data.setdefault("username", "")
            data.setdefault("login_status", False)
            return data
    except Exception:
        return {"username": "", "login_status": False, "visit_count": 0}

def save_session_data(session_id, data):
    """Save session data to file"""
    session_file = f"/tmp/webserv_sessions/{session_id}.json"
    try:
        os.makedirs("/tmp/webserv_sessions", exist_ok=True)
        with open(session_file, 'w') as f:
            json.dump(data, f)
    except Exception:
        pass

def create_session_id():
    """Create a unique session ID"""
    return f"persistent_{int(time.time())}_{hash(str(time.time())) % 99999}"

# === Main Logic ===
query_string = os.environ.get('QUERY_STRING', '')
params = parse_qs(query_string)

# JSON mode for AJAX integration
is_json = params.get('format', [''])[0] == 'json'

# Get or create session ID
session_id = get_cookie('session_id')
if not session_id:
    session_id = create_session_id()

# Load session data
session_data = get_session_data(session_id)
session_data["visit_count"] += 1

# Get current theme (from cookie or default)
theme = get_cookie('theme') or 'light'
theme_source = "cookie" if get_cookie('theme') else "default ('light')"

# Flag to detect if we redirect
redirected = False

# Handle actions
action = params.get('action', [''])[0]

if action == 'login':
    username = params.get('username', ['User'])[0]
    session_data["username"] = username
    session_data["login_status"] = True
    save_session_data(session_id, session_data)

elif action == 'logout':
    session_data["login_status"] = False
    session_data["username"] = ""
    save_session_data(session_id, session_data)

elif action == 'theme':
    toggle = params.get('toggle', [''])[0]
    if toggle == '1':
        # Toggle theme
        new_theme = 'dark' if theme == 'light' else 'light'
        
        # Save session and set theme cookie
        save_session_data(session_id, session_data)
        set_cookie('theme', new_theme)
        theme = new_theme
        
        # In JSON mode, return JSON instead of redirecting
        if is_json:
            print("Content-Type: application/json")
            set_cookie('session_id', session_id)
            set_cookie('theme', theme)
            print()
            print(json.dumps({
                "login_status": session_data["login_status"],
                "username": session_data["username"],
                "visit_count": session_data["visit_count"],
                "theme": theme
            }))
            sys.exit(0)
        
        # 303 Redirect to prevent refresh replay for HTML mode
        print("Status: 303 See Other")
        print("Location: /cgi-bin/simple_login.py")
        print()
        sys.exit(0)

# If no redirect, save session normally (login/logout already saved)
if action not in ['login', 'logout', 'theme']:
    save_session_data(session_id, session_data)

# === Output HTTP Headers ===
if is_json:
    print("Content-Type: application/json")
    set_cookie('session_id', session_id)
    set_cookie('theme', theme)  # Ensure theme cookie is always set
    print()
    print(json.dumps({
        "login_status": session_data["login_status"],
        "username": session_data["username"],
        "visit_count": session_data["visit_count"],
        "theme": theme
    }))
    sys.exit(0)
else:
    print("Content-Type: text/html")
    set_cookie('session_id', session_id)
    set_cookie('theme', theme)  # Ensure theme cookie is always set
    print()

# Generate page based on login status
if session_data["login_status"]:
    # === Dashboard (Logged In) ===
    print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Dashboard</title>
    <style>
        body {{ 
            font-family: Arial, sans-serif; 
            margin: 40px; 
            background: {'#1a1a1a' if theme == 'dark' else '#ffffff'};
            color: {'#ffffff' if theme == 'dark' else '#000000'};
        }}
        .container {{ 
            max-width: 600px; 
            margin: 0 auto; 
            padding: 20px;
            background: {'#2d2d2d' if theme == 'dark' else '#f5f5f5'};
            border-radius: 10px;
        }}
        .button {{ 
            background: #007bff; 
            color: white; 
            border: none; 
            padding: 10px 20px; 
            border-radius: 5px; 
            cursor: pointer; 
            margin: 5px;
            text-decoration: none;
            display: inline-block;
        }}
        .warning {{ background: #ffc107; color: black; }}
    </style>
</head>
<body>
    <div class=\"container\">
        <h1>Welcome, {session_data['username']}!</h1>
        <p><strong>Visit Count:</strong> {session_data['visit_count']}</p>
        <p><strong>Theme:</strong> {theme}</p>
        
        <div>
            <a href=\"/cgi-bin/simple_login.py?action=theme&toggle=1\" class=\"button\">
                Switch to {'Light' if theme == 'dark' else 'Dark'} Mode
            </a>
            <a href=\"/cgi-bin/simple_login.py?action=logout\" class=\"button warning\">
                Logout
            </a>
        </div>
        
        <p><small>Session ID: {session_id}</small></p>

    </div>
</body>
</html>
""")
else:
    # === Login Page ===
    print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
    <style>
        body {{ 
            font-family: Arial, sans-serif; 
            margin: 40px; 
            background: {'#1a1a1a' if theme == 'dark' else '#ffffff'};
            color: {'#ffffff' if theme == 'dark' else '#000000'};
        }}
        .container {{ 
            max-width: 400px; 
            margin: 0 auto; 
            padding: 20px;
            background: {'#2d2d2d' if theme == 'dark' else '#f5f5f5'};
            border-radius: 10px;
        }}
        input {{ 
            width: 100%; 
            padding: 10px; 
            margin: 10px 0; 
            border: 1px solid #ddd; 
            border-radius: 5px;
            background: {'#3d3d3d' if theme == 'dark' else '#ffffff'};
            color: {'#ffffff' if theme == 'dark' else '#000000'};
        }}
        .button {{ 
            background: #007bff; 
            color: white; 
            border: none; 
            padding: 10px 20px; 
            border-radius: 5px; 
            cursor: pointer; 
            width: 100%;
        }}
        .theme-toggle {{ 
            background: #6c757d; 
            color: white; 
            border: none; 
            padding: 5px 10px; 
            border-radius: 3px; 
            cursor: pointer; 
            margin: 10px 0;
            text-decoration: none;
            display: inline-block;
        }}
    </style>
</head>
<body>
    <div class=\"container\">
        <h1>Login</h1>
        <p><strong>Visit Count:</strong> {session_data['visit_count']}</p>
        <p><strong>Theme:</strong> {theme}</p>
        
        <a href=\"/cgi-bin/simple_login.py?action=theme&toggle=1\" class=\"theme-toggle\">
            Switch to {'Light' if theme == 'dark' else 'Dark'} Mode
        </a>
        
        <form action=\"/cgi-bin/simple_login.py\" method=\"get\">
            <input type=\"text\" name=\"username\" placeholder=\"Enter username\" required>
            <input type=\"hidden\" name=\"action\" value=\"login\">
            <button type=\"submit\" class=\"button\">Login</button>
        </form>
        
        <p><small>Session ID: {session_id}</small></p>

    </div>
</body>
</html>
""")
