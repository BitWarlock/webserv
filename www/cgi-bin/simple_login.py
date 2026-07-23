#!/usr/bin/env python3
import os
import sys
import json
from urllib.parse import parse_qs


def get_cookie(name):
    cookies = os.environ.get('HTTP_COOKIE', '')
    if not cookies:
        return None
    for part in cookies.split(';'):
        part = part.strip()
        if part.startswith(f"{name}="):
            return part.split('=', 1)[1].strip()
    return None


def set_cookie(name, value, max_age=31536000):
    value = value.replace(';', '%3B')
    print(f"Set-Cookie: {name}={value}; Path=/; HttpOnly; SameSite=Strict; Max-Age={max_age}")


def valid_session_id(session_id):
    if not session_id or len(session_id) > 128:
        return False
    return all(ch.isalnum() or ch in "_-" for ch in session_id)


def load_session(session_id):
    try:
        if not valid_session_id(session_id):
            return {"username": "", "login_status": False, "visit_count": 0}
        path = f"/tmp/webserv_sessions/{session_id}.json"
        with open(path, 'r') as f:
            return json.load(f)
    except Exception:
        return {"username": "", "login_status": False, "visit_count": 0}


def save_session(session_id, data):
    try:
        if not valid_session_id(session_id):
            return
        path = f"/tmp/webserv_sessions/{session_id}.json"
        with open(path, 'w') as f:
            json.dump(data, f)
    except Exception:
        pass


query_string = os.environ.get('QUERY_STRING', '')
params = parse_qs(query_string)

is_json = params.get('format', [''])[0] == 'json'

session_id = get_cookie('session_id') or ''

session = load_session(session_id) if session_id else {"username": "", "login_status": False, "visit_count": 0}

vc_cookie = get_cookie('vc')
try:
    visit_count = int(vc_cookie) + 1 if vc_cookie is not None else 1
except ValueError:
    visit_count = 1
session["visit_count"] = visit_count

if session_id:
    save_session(session_id, session)

theme = get_cookie('theme') or 'light'

action = params.get('action', [''])[0]

if action == 'login' and session_id:
    username = params.get('username', ['User'])[0]
    session["username"] = username
    session["login_status"] = True
    save_session(session_id, session)
elif action == 'logout' and session_id:
    session["login_status"] = False
    session["username"] = ""
    save_session(session_id, session)
elif action == 'theme':
    toggle = params.get('toggle', [''])[0]
    if toggle == '1':
        new_theme = 'dark' if theme == 'light' else 'light'
        set_cookie('theme', new_theme)
        set_cookie('vc', str(visit_count))
        theme = new_theme
        if is_json:
            print("Content-Type: application/json")
            print()
            print(json.dumps({
                "login_status": session.get("login_status", False),
                "username": session.get("username", ""),
                "visit_count": visit_count,
                "theme": theme
            }))
            sys.exit(0)
        print("Status: 303 See Other")
        print("Location: /cgi-bin/simple_login.py")
        print()
        sys.exit(0)

print("Content-Type: application/json" if is_json else "Content-Type: text/html")
set_cookie('vc', str(visit_count))
if theme:
    set_cookie('theme', theme)
print()

if is_json:
    print(json.dumps({
        "login_status": session.get("login_status", False),
        "username": session.get("username", ""),
        "visit_count": visit_count,
        "theme": theme
    }))
    sys.exit(0)

print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 40px; background: {'#1a1a1a' if theme == 'dark' else '#ffffff'}; color: {'#ffffff' if theme == 'dark' else '#000000'}; }}
        .container {{ max-width: 400px; margin: 0 auto; padding: 20px; background: {'#2d2d2d' if theme == 'dark' else '#f5f5f5'}; border-radius: 10px; }}
        input {{ width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; background: {'#3d3d3d' if theme == 'dark' else '#ffffff'}; color: {'#ffffff' if theme == 'dark' else '#000000'}; }}
        .button {{ background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; width: 100%; }}
        .theme-toggle {{ background: #6c757d; color: white; border: none; padding: 5px 10px; border-radius: 3px; cursor: pointer; margin: 10px 0; text-decoration: none; display: inline-block; }}
    </style>
</head>
<body>
    <div class=\"container\">\n        <h1>Login</h1>\n        <p><strong>Visit Count:</strong> {visit_count}</p>\n        <p><strong>Theme:</strong> {theme}</p>\n        <a href=\"/cgi-bin/simple_login.py?action=theme&toggle=1\" class=\"theme-toggle\">Switch to {'Light' if theme == 'dark' else 'Dark'} Mode</a>\n        <form action=\"/cgi-bin/simple_login.py\" method=\"get\">\n            <input type=\"text\" name=\"username\" placeholder=\"Enter username\" required>\n            <input type=\"hidden\" name=\"action\" value=\"login\">\n            <button type=\"submit\" class=\"button\">Login</button>\n        </form>\n        <p><small>Session ID: {session_id or 'None (server should set)'}</small></p>\n    </div>
</body>
</html>
""")
