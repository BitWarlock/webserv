# Webserv

> A fully functional HTTP/1.1 web server written in C++98 — built from scratch with non-blocking I/O via `epoll`, CGI support, virtual hosts, session management, cookie handling, and a custom configuration parser. No external libraries. No nginx. No shortcuts.

---

## Table of Contents

- [Overview](#overview)
- [Team & Responsibilities](#team--responsibilities)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [How It Works — Request Lifecycle](#how-it-works--request-lifecycle)
- [Module 1 — HTTP Request Parser](#module-1--http-request-parser)
- [Module 2 — Server, epoll & HTTP Response](#module-2--server-epoll--http-response)
- [Module 3 — CGI, Config Parser & Cookies](#module-3--cgi-config-parser--cookies)
- [Configuration File](#configuration-file)
- [HTTP/1.1 Concepts](#http11-concepts)
- [epoll Deep Dive](#epoll-deep-dive)
- [CGI Deep Dive](#cgi-deep-dive)
- [Cookies & Session Management](#cookies--session-management)
- [Error Handling](#error-handling)
- [Compilation & Usage](#compilation--usage)
- [Mandatory Rules](#mandatory-rules)

---

## Overview

**Webserv** is a 42 School project. The objective is to build an HTTP/1.1 compliant web server in C++98 that:

- Handles multiple simultaneous clients without blocking, using **`epoll`**
- Serves **static files** from `www/html/` and `www/gallery/`
- Executes **CGI scripts** from `www/cgi-bin/`
- Supports **virtual hosts** and per-location routing via `configs/`
- Parses a custom **NGINX-inspired configuration file**
- Handles **cookies** and **sessions** via `SessionManager`
- Stores uploaded files in `uploads/`
- Serves fully custom error pages from `error_pages/`
- Responds correctly to `GET`, `POST`, and `DELETE`

The server must **never block** and must **never crash** under any circumstances.

---

## Team & Responsibilities

| Member | Module | Files owned |
|--------|--------|-------------|
| **Member 1** | HTTP Request Parser | `ParseRequest.cpp/hpp`, `Lexer.cpp/hpp`, `ParseDirective.cpp/hpp` |
| **Member 2** | Server + epoll + Response | `Server.cpp/hpp`, `SetupServers.cpp/hpp`, `Response.cpp/hpp`, `DirectoryListing.cpp`, `webserv.cpp` |
| **Member 3** | CGI + Config + Cookies | `Cgi.cpp/hpp`, `Config.cpp/hpp`, `Location.cpp/hpp`, `SessionManager.cpp/hpp`, `WebServ.hpp` |

---

## Project Structure

```
webserv/
│
├── configs/
│   ├── default.conf              # minimal default server config
│   └── webserv.conf              # full config with virtual hosts, locations, CGI
│
├── error_pages/                  # custom HTML error pages served on HTTP errors
│   ├── Bad_Gateway.html
│   ├── Bad_Request.html
│   ├── Conflict.html
│   ├── Content_Too_Large.html
│   ├── Forbidden.html
│   ├── Gateway_Timeout.html
│   ├── HTTP_Version_Not_Supported.html
│   ├── Internal_Server_Error.html
│   ├── Length_Required.html
│   ├── Method_Not_Allowed.html
│   ├── Moved_Permanently.html
│   ├── Not_Found.html
│   ├── Not_Implemented.html
│   ├── Request_Header_Fields_Too_Large.html
│   ├── Request_Timeout.html
│   ├── URI_Too_Long.html
│   ├── Unsupported_Media_Type.html
│   ├── listening_page.html
│   └── template.html
│
├── inc/                          # all header files
│   ├── Cgi.hpp                   # CGI execution interface
│   ├── Config.hpp                # parsed server/location config structures
│   ├── Lexer.hpp                 # config file tokenizer
│   ├── Location.hpp              # per-location routing rules
│   ├── ParseDirective.hpp        # config directive parser
│   ├── ParseRequest.hpp          # HTTP request parser
│   ├── Response.hpp              # HTTP response builder
│   ├── Server.hpp                # per-server socket + client management
│   ├── SessionManager.hpp        # cookie-based session store
│   ├── SetupServers.hpp          # multi-server initialization
│   └── WebServ.hpp               # global types, constants, shared utilities
│
├── request_tests/                # raw HTTP request scripts for manual testing
│   ├── Send_DeflateBody_Request.sh
│   ├── Send_GzipBody_Request.sh
│   └── Send_Identity_Request.sh
│
├── src/                          # all source files
│   ├── Cgi.cpp                   # fork/execve/pipe CGI execution
│   ├── Config.cpp                # config file parser (uses Lexer)
│   ├── DirectoryListing.cpp      # autoindex HTML page generator
│   ├── Lexer.cpp                 # config file lexer (tokenizer)
│   ├── Location.cpp              # location block matching logic
│   ├── ParseDirective.cpp        # directive-level config parsing
│   ├── ParseRequest.cpp          # HTTP/1.1 request parser
│   ├── Response.cpp              # response builder + static file serving
│   ├── Server.cpp                # per-server epoll loop + client I/O
│   ├── SessionManager.cpp        # session create/read/destroy
│   └── SetupServers.cpp          # bind/listen/epoll init for all servers
│
├── uploads/                      # uploaded files land here
│   ├── corn.jpg
│   ├── forest_nature_landscape_...
│   ├── free-nature-images.jpg
│   ├── images.jpeg / images.jpg / images.png
│   ├── jpg_44-2.jpeg
│   └── ok.png
│
├── www/                          # web root — static content + CGI
│   ├── cgi-bin/                  # CGI scripts (Python, PHP, bash, ...)
│   ├── gallery/                  # image gallery static pages
│   ├── html/                     # main HTML pages
│   └── cookie_demo.html          # cookie + session demo page
│
├── .gitignore
├── Dockerfile
├── Makefile
├── README.md
└── webserv.cpp                   # main() — loads config, starts servers
```

---

## Architecture

```
webserv.cpp (main)
    │
    ├── Config::parse("configs/webserv.conf")
    │       └── Lexer → tokens → ParseDirective → vector<ServerConfig>
    │                                                     └── vector<Location>
    │
    └── SetupServers::init()
            └── for each ServerConfig:
                    socket() bind() listen() epoll_ctl(ADD)
                              │
                    ┌─────────▼──────────────────────────────┐
                    │           epoll event loop              │
                    │  (Server.cpp — one loop, all clients)   │
                    └─────────┬──────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
        EPOLLIN          EPOLLOUT        EPOLLHUP/ERR
        recv()           send()          close_client()
              │
       ParseRequest::parse()
              │
    ┌─────────┴────────────────────────┐
    │                                  │
static file                      CGI request
Response::serveFile()             Cgi::execute()
DirectoryListing (autoindex)      fork/pipe/execve
    │                                  │
    └──────────┬───────────────────────┘
               │
         Response::build()
         + SessionManager (Set-Cookie / read session)
               │
         epoll_ctl(MOD → EPOLLOUT)
               │
            send()
```

---

## How It Works — Request Lifecycle

```
1. epoll_wait() fires EPOLLIN on listening socket
       │
       ▼
2. accept() → new client fd → epoll_ctl(ADD, EPOLLIN | EPOLLET)
       │
       ▼
3. epoll_wait() fires EPOLLIN on client fd
       │
       ▼
4. recv() raw bytes into client buffer
       │
       ▼
5. ParseRequest::parse()
       │  → method, URI, path, query string
       │  → headers map (lowercase keys)
       │  → body (plain / chunked / multipart)
       │
       ├── Static file    → Response::serveFile()       (www/)
       ├── Autoindex      → DirectoryListing::generate()
       ├── CGI script     → Cgi::execute()              (www/cgi-bin/)
       ├── Upload POST    → write to uploads/
       └── DELETE         → unlink file → 204
                │
                ▼
6. Response::build()
       │  → status line + headers + body
       │  → SessionManager: read Cookie / write Set-Cookie
       │  → error_pages/ if 4xx or 5xx
       │
       ▼
7. epoll_ctl(MOD, EPOLLOUT) on client fd
       │
       ▼
8. send() response bytes (loop until EAGAIN)
       │
       ▼
9. Connection: keep-alive → reset, back to step 3
   Connection: close      → close(fd), epoll_ctl(DEL)
```

---

## Module 1 — HTTP Request Parser

**Owner:** Member 1  
**Files:** `inc/ParseRequest.hpp` · `src/ParseRequest.cpp` · `inc/Lexer.hpp` · `src/Lexer.cpp` · `inc/ParseDirective.hpp` · `src/ParseDirective.cpp`

> **Note:** The `Lexer` and `ParseDirective` components serve double duty — they power both the **HTTP request parser** and the **configuration file parser** (Module 3). The lexer tokenizes a raw byte stream; the directive parser interprets those tokens into structured data.

---

### Concept — HTTP/1.1 Request Format

A raw HTTP/1.1 request arriving on the socket:

```
POST /upload HTTP/1.1\r\n
Host: localhost:8080\r\n
Content-Type: multipart/form-data; boundary=----Boundary\r\n
Content-Length: 512\r\n
Cookie: session_id=abc123\r\n
Connection: keep-alive\r\n
\r\n
<binary body bytes>
```

Always structured as:
1. **Request line** — `METHOD URI HTTP/VERSION\r\n`
2. **Headers** — `Key: Value\r\n` pairs
3. **Empty line** — `\r\n` signals end of headers
4. **Body** — optional; size determined by `Content-Length` or `Transfer-Encoding: chunked`

---

### Lexer (`Lexer.cpp/hpp`)

> **Concept:** A lexer (tokenizer) reads raw characters and groups them into meaningful **tokens** — the smallest units the parser cares about. Instead of parsing character-by-character throughout the codebase, everything goes through the lexer first, producing a clean token stream.

```
Raw input: "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
                │
                ▼  Lexer::tokenize()
                │
Token stream: [ TOKEN_WORD("GET") ][ TOKEN_URI("/index.html") ]
              [ TOKEN_VERSION("HTTP/1.1") ][ TOKEN_CRLF ]
              [ TOKEN_KEY("Host") ][ TOKEN_VALUE("localhost") ]
              [ TOKEN_CRLF ][ TOKEN_CRLF ]  ← blank line = end of headers
```

The same lexer is reused for config file parsing — it recognizes `{`, `}`, `;`, and directive keywords when in config mode.

---

### ParseDirective (`ParseDirective.cpp/hpp`)

> **Concept:** A directive parser consumes a token stream and maps tokens to structured fields. For HTTP requests it extracts method/URI/version from the request line; for config files it extracts directive names and their values.

```
Tokens → ParseDirective → structured data
  │
  ├── request line  →  method, raw URI, HTTP version
  ├── URI           →  path (before ?) + queryString (after ?)
  └── headers       →  map<string, string> (keys stored lowercase)
```

---

### ParseRequest (`ParseRequest.cpp/hpp`)

The top-level parser that orchestrates lexing, directive parsing, and body reading:

```
recv() buffer
      │
      ▼
┌──────────────────────────────────┐
│ 1. Feed to Lexer → token stream  │
│ 2. ParseDirective → request line │
│ 3. ParseDirective → header map   │
│ 4. Detect body strategy:         │
│      Content-Length → read N bytes│
│      Transfer-Encoding: chunked  │
│        → reassemble chunk frames │
│      multipart → parse boundaries│
│ 5. Validate (400 / 405 / 413)    │
└──────────────────────────────────┘
      │
      ▼
  HttpRequest object (ready for routing)
```

**Chunked Transfer Encoding reassembly:**
```
a\r\n           ← chunk size in hex (10 bytes)
0123456789\r\n  ← chunk data
5\r\n
Hello\r\n
0\r\n           ← terminal chunk = body complete
\r\n
```
Each chunk frame is stripped of its size prefix and concatenated into a single body buffer.

**Body size enforcement:** `Content-Length` is checked against `client_max_body_size` from the config before reading. Returns `413 Content Too Large` immediately if exceeded.

**`request_tests/` scripts** send raw requests with `Send_DeflateBody_Request.sh`, `Send_GzipBody_Request.sh`, and `Send_Identity_Request.sh` to exercise the parser with different `Content-Encoding` values.

---

## Module 2 — Server, epoll & HTTP Response

**Owner:** Member 2  
**Files:** `src/Server.cpp/hpp` · `src/SetupServers.cpp/hpp` · `src/Response.cpp/hpp` · `src/DirectoryListing.cpp` · `webserv.cpp`  
**Relevant dirs:** `www/` · `uploads/` · `error_pages/`

---

### SetupServers (`SetupServers.cpp/hpp`)

> **Concept:** Before entering the event loop, the server must prepare one listening socket per `server` block in the config. `SetupServers` iterates over the parsed `Config`, creates and binds each socket, and registers them all with the same `epoll` instance.

```cpp
for each ServerConfig in config:
    fd = socket(AF_INET, SOCK_STREAM, 0)
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR)  // survive restart without waiting
    fcntl(fd, F_SETFL, O_NONBLOCK)            // mandatory for epoll ET
    bind(fd, host:port)
    listen(fd, BACKLOG)
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, EPOLLIN)
```

Multiple `server` blocks on the same port are supported — the right config is selected later by matching the `Host` request header against each block's `server_name`.

---

### Server & epoll Event Loop (`Server.cpp/hpp`)

> **Concept:** `epoll` lets a single thread watch thousands of file descriptors simultaneously. Instead of looping over all fds (like `select`/`poll`), `epoll_wait` returns **only the fds that are ready**, making the loop O(1) per event regardless of total connection count.

```cpp
// Core loop (simplified)
while (true)
{
    int n = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT_MS);

    for (int i = 0; i < n; i++)
    {
        int fd = events[i].data.fd;

        if (is_listening_socket(fd))
            accept_new_client(fd);                  // EPOLLIN on listener
        else if (events[i].events & EPOLLIN)
            read_from_client(fd);                   // data arrived
        else if (events[i].events & EPOLLOUT)
            write_to_client(fd);                    // socket ready to send
        else if (events[i].events & (EPOLLHUP | EPOLLERR))
            close_client(fd);                       // connection dropped
    }

    check_timeouts();   // kill idle keep-alive + hung CGI children
}
```

**Edge-triggered mode (`EPOLLET`):** `epoll_wait` fires **once** when the fd transitions to ready. The code must call `recv()`/`send()` in a loop until `EAGAIN` or `EWOULDBLOCK` is returned — otherwise data will be silently missed.

**Client State Machine:**

```
READING ──► PROCESSING ──► CGI_WAITING ──► WRITING ──► DONE
                                                         │
                              (keep-alive) ◄─────────────┘
```

| State | What happens |
|-------|-------------|
| `READING` | `recv()` loop fills the request buffer until `\r\n\r\n` + full body |
| `PROCESSING` | `ParseRequest` → route → build response or launch CGI |
| `CGI_WAITING` | Parent watches `stdout_pipe[0]` via epoll; waits for child output |
| `WRITING` | `send()` loop drains the response buffer until `EAGAIN` |
| `DONE` | Keep-alive → reset to `READING`; else `close()` + `epoll_ctl(DEL)` |

---

### Response Builder (`Response.cpp/hpp`)

The response builder constructs a valid HTTP/1.1 response from the routing result:

```
HTTP/1.1 200 OK\r\n
Content-Type: text/html; charset=utf-8\r\n
Content-Length: 1024\r\n
Connection: keep-alive\r\n
Set-Cookie: session_id=abc123; Path=/; HttpOnly\r\n
\r\n
<body>
```

**Status codes and their matching error page:**

| Code | Meaning | Error page file |
|------|---------|----------------|
| `200 OK` | Success | — |
| `201 Created` | File uploaded | — |
| `204 No Content` | DELETE success | — |
| `301 Moved Permanently` | Redirect | `Moved_Permanently.html` |
| `400 Bad Request` | Malformed request | `Bad_Request.html` |
| `403 Forbidden` | Permission denied | `Forbidden.html` |
| `404 Not Found` | File missing | `Not_Found.html` |
| `405 Method Not Allowed` | Method blocked | `Method_Not_Allowed.html` |
| `408 Request Timeout` | Client too slow | `Request_Timeout.html` |
| `409 Conflict` | Upload conflict | `Conflict.html` |
| `411 Length Required` | Missing Content-Length | `Length_Required.html` |
| `413 Content Too Large` | Body too big | `Content_Too_Large.html` |
| `414 URI Too Long` | URI exceeds limit | `URI_Too_Long.html` |
| `431 Request Header Fields Too Large` | Headers too big | `Request_Header_Fields_Too_Large.html` |
| `500 Internal Server Error` | Server crash / CGI fail | `Internal_Server_Error.html` |
| `501 Not Implemented` | Unknown method | `Not_Implemented.html` |
| `502 Bad Gateway` | CGI bad output | `Bad_Gateway.html` |
| `504 Gateway Timeout` | CGI timeout | `Gateway_Timeout.html` |
| `505 HTTP Version Not Supported` | Not HTTP/1.1 | `HTTP_Version_Not_Supported.html` |
| `415 Unsupported Media Type` | Bad Content-Type | `Unsupported_Media_Type.html` |

**MIME type detection** maps file extensions to `Content-Type` headers (`.html`, `.css`, `.js`, `.png`, `.jpg`, `.jpeg`, `.pdf`, `.gif`, etc.).

---

### DirectoryListing (`DirectoryListing.cpp`)

> **Concept:** When a request targets a directory and `autoindex on` is set in the matching `Location` block, instead of returning `403` the server generates an HTML page listing the directory's contents dynamically — file names, sizes, and last-modified timestamps — using `opendir`/`readdir`.

---

## Module 3 — CGI, Config Parser & Cookies

**Owner:** Member 3  
**Files:** `src/Cgi.cpp/hpp` · `src/Config.cpp/hpp` · `src/Lexer.cpp/hpp` · `src/Location.cpp/hpp` · `src/ParseDirective.cpp/hpp` · `src/SessionManager.cpp/hpp` · `inc/WebServ.hpp`  
**Relevant dirs:** `www/cgi-bin/` · `configs/`

---

### CGI — Common Gateway Interface (`Cgi.cpp/hpp`)

> **Concept:** CGI is a protocol that lets the web server hand a request off to an external program and use that program's stdout as the HTTP response body. The server communicates request metadata via **environment variables** and the request body via **stdin**. The script writes its response headers + body to **stdout**.

#### Execution Flow

```
1. Routing identifies a CGI request (e.g. /cgi-bin/script.py)
       │
       ▼
2. pipe(stdin_pipe)    → body goes in  (parent writes, child reads)
   pipe(stdout_pipe)   → output comes out (child writes, parent reads)
       │
       ▼
3. fork()
       │
       ├── CHILD:
       │     dup2(stdin_pipe[0],  STDIN_FILENO)   // redirect stdin
       │     dup2(stdout_pipe[1], STDOUT_FILENO)  // redirect stdout
       │     close all other pipe ends
       │     setenv(all CGI env vars)
       │     execve("/usr/bin/python3", [script_path], envp)
       │     // child process is now replaced by the interpreter
       │
       └── PARENT:
             write request body → stdin_pipe[1]
             close(stdin_pipe[1])          // signals EOF to child
             epoll_ctl(ADD, stdout_pipe[0], EPOLLIN)
             // event loop reads CGI output asynchronously
             waitpid(child, WNOHANG) in each epoll cycle
```

#### CGI Environment Variables

| Variable | Value |
|----------|-------|
| `REQUEST_METHOD` | `GET`, `POST`, etc. |
| `QUERY_STRING` | Everything after `?` in the URI |
| `CONTENT_TYPE` | Value of `Content-Type` header |
| `CONTENT_LENGTH` | Value of `Content-Length` header |
| `SCRIPT_FILENAME` | Absolute path to the CGI script |
| `PATH_INFO` | Extra path component after the script name |
| `SERVER_NAME` | Server hostname from config |
| `SERVER_PORT` | Port the server is listening on |
| `HTTP_COOKIE` | Full value of the `Cookie` request header |
| `HTTP_*` | All other request headers, uppercased, prefixed with `HTTP_` |

#### CGI Output Parsing

The CGI script writes its own headers followed by a blank line, then the body:

```
Content-Type: text/html\r\n
Set-Cookie: user=bob; Path=/\r\n
\r\n
<html>...</html>
```

The server reads and splits this on `\r\n\r\n`, merges the script-provided headers with its own (`Content-Length`, `Connection`, server cookies), and forwards the complete response to the client.

#### CGI Timeout

A hung script is killed with `SIGKILL` after a configurable timeout. The client receives `504 Gateway Timeout` (`Gateway_Timeout.html`). `waitpid()` is called with `WNOHANG` each epoll iteration to reap finished children and prevent zombies.

---

### Configuration File Parser (`Config.cpp/hpp` + `Lexer.cpp/hpp` + `ParseDirective.cpp/hpp` + `Location.cpp/hpp`)

> **Concept:** The config file is processed in two phases: **lexing** (the `Lexer` splits raw text into tokens) and **parsing** (the `Config`/`ParseDirective` layer interprets the token stream into a tree of `ServerConfig` and `Location` objects). This separation keeps each component focused and testable.

#### Config File Format (`configs/webserv.conf`)

```nginx
server {
    listen       8080;
    server_name  localhost;

    client_max_body_size 10M;

    error_page 404 /error_pages/Not_Found.html;
    error_page 500 /error_pages/Internal_Server_Error.html;

    location / {
        root        ./www/html;
        index       index.html;
        methods     GET POST;
        autoindex   off;
    }

    location /gallery {
        root        ./www/gallery;
        methods     GET;
        autoindex   on;
    }

    location /upload {
        root        ./uploads;
        methods     POST DELETE;
        upload_path ./uploads;
    }

    location /cgi-bin {
        root        ./www/cgi-bin;
        methods     GET POST;
        cgi_pass    .py  /usr/bin/python3;
        cgi_pass    .php /usr/bin/php-cgi;
    }

    location /old {
        return 301 /new;
    }
}
```

`configs/default.conf` provides a minimal fallback used in testing.

#### Parsed Config Object Tree

```
Config
  └── vector<ServerConfig>
          ├── host, port
          ├── server_name
          ├── client_max_body_size
          ├── map<int, string> error_pages
          └── vector<Location>
                  ├── path          (e.g. "/cgi-bin")
                  ├── root
                  ├── index
                  ├── allowed_methods
                  ├── autoindex
                  ├── upload_path
                  ├── redirect      (code + target URI)
                  └── map<string, string> cgi_pass  (extension → interpreter)
```

#### Location Matching (`Location.cpp/hpp`)

When a request comes in, the server finds the best `Location` block using **longest prefix matching** — the most specific path wins:

```
Request: GET /cgi-bin/hello.py

Candidates:
  /          → match, length 1
  /cgi-bin   → match, length 8   ← WINNER (most specific)
```

---

### Session Manager (`SessionManager.cpp/hpp`)

> **Concept:** HTTP is **stateless** — the server forgets each client after the response is sent. Cookies solve this by storing a small token in the browser that is sent back on every subsequent request. The `SessionManager` maps those tokens to server-side session data.

#### Cookie & Session Flow

```
First visit (no cookie):
  Server generates random session_id
  Response includes:
    Set-Cookie: session_id=<random>; Path=/; HttpOnly; Max-Age=3600
         │
         ▼
  Browser stores the cookie

Subsequent requests:
  Browser sends:
    Cookie: session_id=<random>
         │
         ▼
  SessionManager::get(session_id) → session data map
  (user login state, preferences, upload history, etc.)
```

`www/cookie_demo.html` provides a frontend demo of the full cookie/session cycle.

#### `Set-Cookie` Attributes

| Attribute | Purpose |
|-----------|---------|
| `Path=/` | Cookie is sent for every path on this host |
| `HttpOnly` | JavaScript cannot read this cookie — XSS protection |
| `Max-Age=N` | Cookie expires after N seconds; omit for session-only |
| `Secure` | Only sent over HTTPS |
| `SameSite=Lax` | Not sent on cross-site sub-requests — CSRF protection |

---

## HTTP/1.1 Concepts

### Persistent Connections (keep-alive)

HTTP/1.1 defaults to **persistent connections**: after the server sends a response, the TCP socket stays open and the same client can send another request without the overhead of a new handshake. The server resets its parser state and goes back to `READING`. Connections are closed when the client sends `Connection: close` or when they idle past the configured timeout.

### `Content-Length` vs `Transfer-Encoding: chunked`

When the response size is known ahead of time, the server sends `Content-Length`. When it is not (e.g. dynamically generated CGI output), it uses chunked encoding — the body is sent in sized frames, each prefixed with its length in hexadecimal, ending with a zero-length frame.

### Virtual Hosts

HTTP/1.1 **requires** a `Host` header. A single IP:port combination can serve multiple websites — the server reads `Host` and selects the matching `server_name` block from the config. If no block matches, the first block is used as the default. Missing `Host` header → `400 Bad Request`.

---

## epoll Deep Dive

`epoll` is a Linux kernel mechanism for **scalable I/O multiplexing**. It watches many file descriptors and reports only those that are ready, without scanning the full set on every call.

### Why not `select` or `poll`?

| | `select` | `poll` | `epoll` |
|--|---------|--------|---------|
| Max fds | 1024 | Unlimited | Unlimited |
| Per-call scan | O(n) | O(n) | O(1) |
| Returns | Modified bitmask | Modified array | Only ready fds |
| Scalability | Poor | Medium | Excellent |

### Edge-Triggered vs Level-Triggered

| Mode | Behavior | Usage |
|------|----------|-------|
| **Level-triggered** (default) | Fires repeatedly while data is available | Safe to read partially |
| **Edge-triggered** (`EPOLLET`) | Fires **once** on state transition | Must loop until `EAGAIN` |

We use **edge-triggered** mode with `O_NONBLOCK` sockets. All `recv()` and `send()` calls loop until they get `EAGAIN`/`EWOULDBLOCK`, then yield back to `epoll_wait`.

### Key epoll calls

```cpp
int epoll_fd = epoll_create1(0);

// Register a new client
struct epoll_event ev;
ev.events  = EPOLLIN | EPOLLET;
ev.data.fd = client_fd;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

// Switch to write-ready watching when we have data to send
ev.events  = EPOLLOUT | EPOLLET;
epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);

// Block until events arrive (5 second timeout)
struct epoll_event events[MAX_EVENTS];
int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 5000);

// Clean up when connection closes
epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
close(client_fd);
```

---

## CGI Deep Dive

### `fork()` + `execve()` + `pipe()`

```
Parent process                   Child process (after fork)
────────────────────────         ──────────────────────────────
pipe(stdin_pipe)                 dup2(stdin_pipe[0],  STDIN_FILENO)
pipe(stdout_pipe)                dup2(stdout_pipe[1], STDOUT_FILENO)
fork()                           close all other pipe ends
write body → stdin_pipe[1]       setenv(REQUEST_METHOD, ...)
close(stdin_pipe[1])  ← EOF      execve("/usr/bin/python3",
epoll watches stdout_pipe[0]              ["script.py"], envp)
waitpid(WNOHANG) loop            ── replaced by interpreter ──
```

- `pipe()` creates a unidirectional byte channel: `[0]` = read end, `[1]` = write end.
- `dup2(src, dst)` replaces descriptor `dst` with `src` — so the child's fd 0 (stdin) becomes the read end of the pipe; fd 1 (stdout) becomes the write end of the output pipe.
- `execve()` replaces the child's process image entirely with the interpreter binary. All environment variables, including CGI metadata, are passed as the `envp` array.
- The parent **must** close `stdin_pipe[1]` after writing — otherwise the child's `read(stdin)` never returns EOF and the script hangs waiting for more input.

### Zombie Process Prevention

After `fork()`, a child that has exited but whose parent never called `wait()` becomes a **zombie** — consuming a process table slot indefinitely. The server calls `waitpid(pid, &status, WNOHANG)` on each epoll iteration to reap finished CGI children without blocking the event loop.

---

## Cookies & Session Management

The full demo is accessible at `www/cookie_demo.html`. The flow:

1. Client visits for the first time — no `Cookie` header.
2. Server generates a random `session_id` via `SessionManager::create()`.
3. Response includes `Set-Cookie: session_id=<id>; Path=/; HttpOnly; Max-Age=3600`.
4. Browser stores the cookie and sends it on all future requests to this host.
5. Server calls `SessionManager::get(session_id)` to retrieve state (login, preferences, etc.).
6. On logout or expiry, `SessionManager::destroy(session_id)` and `Set-Cookie: session_id=; Max-Age=0` invalidate the session.

---

## Error Handling

The server must **never crash**. Defense-in-depth is applied at every layer:

- `SIGPIPE` is ignored (`signal(SIGPIPE, SIG_IGN)`) — writing to a closed socket must not kill the process.
- `recv()` / `send()` return values are always checked; `EAGAIN` is not an error.
- All `new` / `malloc` failures result in a `500` response, not a crash.
- CGI processes are killed with `SIGKILL` on timeout; zombies are reaped with `WNOHANG`.
- The config parser exits with a descriptive error on invalid syntax — the server never starts with a broken config.
- Every HTTP error code has a matching custom page in `error_pages/`.
- Client connections idle past the timeout are closed gracefully to prevent fd exhaustion.

---

## Compilation & Usage

```bash
# Build
make

# Run with the full config
./webserv configs/webserv.conf

# Run with the minimal default config
./webserv configs/default.conf
```

### Testing

```bash
# Basic GET
curl -v http://localhost:8080/

# POST body
curl -v -X POST -d "hello=world" http://localhost:8080/cgi-bin/echo.py

# File upload
curl -v -F "file=@corn.jpg" http://localhost:8080/upload

# DELETE uploaded file
curl -v -X DELETE http://localhost:8080/upload/corn.jpg

# Test chunked / compressed bodies
bash request_tests/Send_Identity_Request.sh
bash request_tests/Send_GzipBody_Request.sh
bash request_tests/Send_DeflateBody_Request.sh

# Stress test
ab -n 10000 -c 100 http://localhost:8080/
siege -c 50 -t 30s http://localhost:8080/
```

---

## Mandatory Rules

- [x] Written in **C++98** — no C++11 or later
- [x] No external libraries — everything from scratch
- [x] `fork()` used **only** for CGI execution
- [x] All I/O is **non-blocking** — `O_NONBLOCK` on every fd
- [x] A single `epoll_wait` handles all sockets (clients, listeners, CGI pipes)
- [x] No `select`, `poll`, or `kqueue`
- [x] `GET`, `POST`, and `DELETE` supported
- [x] CGI works for at least Python and/or PHP
- [x] Multiple `server` blocks (virtual hosts) supported
- [x] `client_max_body_size` enforced — `413` on violation
- [x] Custom error pages for every standard HTTP error code
- [x] `autoindex` configurable per location block
- [x] `SIGPIPE` handled (ignored)
- [x] Server never crashes on any client input — tested with `ab`, `siege`, `nc`
- [x] Compiles clean with `c++ -Wall -Wextra -Werror -std=c++98`

---

## Authors

| Login | Role |
|-------|------|
| `mthamir` | HTTP Request Parser — `ParseRequest`, `Lexer`, `ParseDirective` |
| `hael-ghd` | Server + epoll + Response — `Server`, `SetupServers`, `Response`, `DirectoryListing` |
| `mrezki` | CGI + Config + Sessions — `Cgi`, `Config`, `Location`, `SessionManager` |

**School:** 1337 from 42
**Project:** Webserv

