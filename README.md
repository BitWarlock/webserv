### 1. Parse Configuration File
- [x] Implement configuration file parsing (default path if none provided)
	- [x] Parse server blocks with:
	- [x] Port and host combinations
- [x] Server names (optional)
	- [x] Default server flag for host:port combinations
	- [x] Parse route configurations for each server:
- [x] Accepted HTTP methods (GET, POST, DELETE)
	- [x] HTTP redirections
	- [x] Root directories for file serving
- [x] Directory listing settings (on/off)
	- [x] Default index files for directories
- [ ] CGI configurations (file extensions, executable paths)
	- [ ] File upload locations
	- [ ] Parse global settings:
	- [ ] Client body size limits
	- [ ] Default error pages
	- [ ] Validate configuration:
	- [ ] Check for valid ports
	- [ ] Verify file/directory paths exist
	- [ ] Ensure no port conflicts between servers

### 2. Create and Initialize Servers
	- [x] For each server in configuration:
- [x] Create non-blocking socket(s)
	- [x] Bind to specified host:port combinations
	- [x] Start listening on sockets
	- [x] Initialize server data structures:
	- [ ] Route configurations
	- [ ] Error pages
	- [ ] CGI settings
	- [x] Client size limits
	- [ ] Create pollfd array for all server sockets
	- [ ] Initialize data structures for:
	- [ ] Client connections
	- [ ] Request buffers
	- [ ] Response queues
	- [ ] CGI processes

### 3. Main Event Loop with poll()
	- [ ] Set up main loop using poll():
		- [ ] Monitor all FDs (server + client) for read/write events
		      - [ ] Handle timeouts for hung connections
		      - [ ] Implement clean-up for disconnected clients
		      - [ ] For each poll() event:
	- [ ] Distinguish between server and client sockets
	- [ ] Handle new connections (server sockets)
	      - [ ] Process client I/O (client sockets)

### 4. Handle New Connections
	      - [ ] Accept new connections (non-blocking)
	- [ ] Add to pollfd set
	- [ ] Initialize client state:
	- [ ] Request buffer
	- [ ] Response queue
	- [ ] Timeout timer
- [ ] Connection info (IP, port)
	- [ ] Apply connection limits if needed

### 5. Parse HTTP Requests
- [ ] Read request data (non-blocking, through poll())
	- [ ] Parse request line:
- [ ] Method (GET, POST, DELETE)
	- [ ] URI
	- [ ] HTTP version
	- [ ] Parse headers:
	- [ ] Content-Length
- [ ] Transfer-Encoding (chunked)
	- [ ] Host header
	- [ ] Cookies
- [ ] Content-Type (for uploads)
	- [ ] Handle chunked encoding if present
	- [ ] Validate request against configuration:
	- [ ] Check allowed methods for route
	- [ ] Verify client body size limits
	- [ ] Match host to server configuration
	- [ ] Prepare error responses for invalid requests

### 6. Route Requests
	- [ ] Match request URI to configured routes
	- [ ] Handle special cases:
- [ ] Redirections (send 30x response)
	- [ ] Directory requests:
	- [ ] Serve index file if exists
	- [ ] Generate directory listing if enabled
	- [ ] 403 if directory listing disabled
	- [ ] File requests:
	- [ ] Check file exists and is readable
	- [ ] Handle range requests if needed
	- [ ] Handle CGI requests:
	- [ ] Determine correct CGI program based on extension
	- [ ] Set up environment variables:
	- [ ] PATH_INFO, SCRIPT_NAME
	- [ ] QUERY_STRING
	- [ ] REQUEST_METHOD
	- [ ] CONTENT_TYPE, CONTENT_LENGTH
	- [ ] HTTP headers as SERVER_PROTOCOL, etc.
	- [ ] Handle file uploads:
	- [ ] Verify upload directory exists and is writable
	- [ ] Process multipart/form-data
	- [ ] Save files to configured location
	- [ ] Handle partial uploads

### 7. Execute CGI Scripts
	- [ ] Fork and execute CGI process:
	- [ ] Set up pipes for stdin/stdout/stderr
	- [ ] Set working directory
	- [ ] Pass request body to CGI stdin
	- [ ] Monitor CGI process with timeout
	- [ ] Read CGI output:
	- [ ] Parse CGI headers
	- [ ] Handle both Content-Length and chunked/EOF responses
	- [ ] Stream response to client
	- [ ] Clean up finished CGI processes
- [ ] Handle CGI errors (502, 504 responses)

### 8. Generate and Send Responses
	- [ ] Build response headers:
	- [ ] Status line
	- [ ] Content-Type, Length
	- [ ] Connection, Server
	- [ ] Set-Cookie if needed
	- [ ] For static files:
	- [ ] Use sendfile() if available
	- [ ] Implement chunked sending for large files
	- [ ] For errors:
	- [ ] Use configured error pages
	- [ ] Fallback to default error pages
	- [ ] Queue responses for non-blocking write
	- [ ] Handle partial writes
	- [ ] Implement keep-alive support

### 9. Client Connection Management
	- [ ] Track connection state:
	- [ ] Active requests
	- [ ] Keep-alive status
	- [ ] Timeout timers
	- [ ] Clean up finished connections:
	- [ ] Close sockets
	- [ ] Remove from poll set
	- [ ] Free resources
	- [ ] Handle client disconnects gracefully

### 10. Additional Features
	- [ ] Implement cookie handling
	- [ ] Basic session management
	- [ ] Support for Range requests
	- [ ] Connection timeouts
	- [ ] Graceful shutdown on signals

### 11. Testing and Validation
	- [ ] Create comprehensive test suite:
	- [ ] Unit tests for parsing
	- [ ] Integration tests for all HTTP methods
	- [ ] Stress tests with many concurrent connections
	- [ ] CGI test cases
	- [ ] File upload tests
	- [ ] Verify against browser behavior
	- [ ] Compare with NGINX for edge cases
	- [ ] Ensure no memory leaks


/Desktop/webserver/hello.html

location /webserver/
{

}

location /Desktop/webserver/

server {
    listen localhost:8002;

    allowed_methods GET;
    root /home/;
    error_page 403 /webServer/error_pages/Forbidden.html;
    autoindex on;

    location /webserver/ {
        allowed_methods GET;
    }

    location /Desktop/ {
        allowed_methods GET;
    }
}
