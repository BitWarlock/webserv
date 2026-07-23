#!/usr/bin/env bash
set -u

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
PORT="${WEBSERV_TEST_PORT:-18080}"
CONF_FILE="$(mktemp /tmp/webserv-regression.XXXXXX.conf)"
LOG_FILE="$(mktemp /tmp/webserv-regression.XXXXXX.log)"
BODY_FILE="$(mktemp /tmp/webserv-regression.XXXXXX.body)"
SESSION_SNAPSHOT="$(mktemp /tmp/webserv-regression.XXXXXX.sessions)"
SESSION_DIR="/tmp/webserv_sessions"
SERVER_PID=""
failures=0

cleanup() {
	if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
		kill "$SERVER_PID" 2>/dev/null
		wait "$SERVER_PID" 2>/dev/null
	fi
	if [ -d "$SESSION_DIR" ]; then
		find "$SESSION_DIR" -maxdepth 1 -type f -name 'ws_*.json' -print | while IFS= read -r session_file; do
			session_name="$(basename "$session_file")"
			if ! grep -Fxq "$session_name" "$SESSION_SNAPSHOT" 2>/dev/null; then
				rm -f "$session_file"
			fi
		done
	fi
	rm -f "$CONF_FILE" "$LOG_FILE" "$BODY_FILE" "$SESSION_SNAPSHOT"
}
trap cleanup EXIT

fail() {
	printf 'FAIL: %s\n' "$1" >&2
	failures=$((failures + 1))
}

cat > "$CONF_FILE" <<EOF
server {
    listen 127.0.0.1:${PORT};
    root ./;
    allowed_methods GET POST DELETE;
    client_max_body_size 10485760;
    client_timeout 6;
    client_header_timeout 5;
    autoindex on;

    location / {
        root ./www/html;
        allowed_methods GET POST DELETE;
        index index.html;
    }

    location /uploads {
        root ./uploads;
        upload_store ./uploads;
        allowed_methods GET POST DELETE;
    }

    location /cgi-bin {
        root ./www/cgi-bin/;
        allowed_methods GET POST;
        cgi_info .py /usr/bin/python3;
        cgi_info .sh /usr/bin/bash;
    }
}
EOF

cd "$ROOT_DIR" || exit 1
if [ -d "$SESSION_DIR" ]; then
	find "$SESSION_DIR" -maxdepth 1 -type f -name 'ws_*.json' -exec basename {} \; > "$SESSION_SNAPSHOT"
else
	: > "$SESSION_SNAPSHOT"
fi

./webserv "$CONF_FILE" >"$LOG_FILE" 2>&1 &
SERVER_PID="$!"

ready=0
for _ in $(seq 1 50); do
	if curl -sS --max-time 1 "http://127.0.0.1:${PORT}/" >/dev/null 2>&1; then
		ready=1
		break
	fi
	if ! kill -0 "$SERVER_PID" 2>/dev/null; then
		printf 'Server exited while starting:\n' >&2
		cat "$LOG_FILE" >&2
		exit 1
	fi
	sleep 0.1
done

if [ "$ready" -ne 1 ]; then
	printf 'Server did not become ready on port %s\n' "$PORT" >&2
	cat "$LOG_FILE" >&2
	exit 1
fi

code="$(curl --path-as-is -sS --max-time 5 -o "$BODY_FILE" -w '%{http_code}' \
	"http://127.0.0.1:${PORT}/uploads/../configs/default.conf")"
if [ "$code" = "200" ] && grep -q 'server {' "$BODY_FILE"; then
	fail "path traversal leaked configs/default.conf"
fi

chunked_response="$(
	printf 'POST /uploads/chunk-regression.txt HTTP/1.1\r\nHost: 127.0.0.1:%s\r\nTransfer-Encoding: chunked\r\nContent-Encoding: br\r\n\r\n5\r\nhello\r\n0\r\n\r\n' "$PORT" \
	| timeout 5 nc 127.0.0.1 "$PORT" 2>/dev/null
)"
if ! printf '%s' "$chunked_response" | grep -q '^HTTP/1.1 415 '; then
	fail "chunked POST with unsupported Content-Encoding was not rejected with 415"
fi
rm -f uploads/chunk-regression.txt

listing="$(curl -sS --max-time 5 "http://127.0.0.1:${PORT}/uploads")"
if printf '%s' "$listing" | grep -q "/uploadscorn.jpg"; then
	fail "directory listing generated /uploadscorn.jpg without a slash"
fi
if ! printf '%s' "$listing" | grep -q "/uploads/corn.jpg"; then
	fail "directory listing did not generate /uploads/corn.jpg"
fi

sids=""
for _ in $(seq 1 4); do
	sid="$(curl -sS -D - -o /dev/null --max-time 5 "http://127.0.0.1:${PORT}/" \
		| awk -F'[=;]' 'BEGIN{IGNORECASE=1} /^Set-Cookie: session_id=/ {print $2; exit}')"
	if [ -z "$sid" ]; then
		fail "response did not set a session_id cookie"
	else
		sids="${sids}${sid}
"
	fi
done
if [ -n "$sids" ]; then
	total_sids="$(printf '%s' "$sids" | sed '/^$/d' | wc -l | tr -d ' ')"
	unique_sids="$(printf '%s' "$sids" | sed '/^$/d' | sort -u | wc -l | tr -d ' ')"
	if [ "$total_sids" != "$unique_sids" ]; then
		fail "session IDs collided across new clients"
	fi
fi

malformed_response="$(
	printf 'GET / HTTP/1.1\r\n: bad\r\nHost: 127.0.0.1:%s\r\n\r\n' "$PORT" \
	| timeout 5 nc 127.0.0.1 "$PORT" 2>/dev/null
)"
if ! printf '%s' "$malformed_response" | grep -q '^HTTP/1.1 400 '; then
	fail "header with empty field-name did not produce 400"
fi
sleep 0.2
if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	fail "server crashed after malformed header"
fi

if [ "$failures" -ne 0 ]; then
	printf '\nServer log:\n' >&2
	cat "$LOG_FILE" >&2
	exit 1
fi

printf 'All regression checks passed\n'
