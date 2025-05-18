# Config file.

**server directives**

    listen: Port number to listen on (e.g., listen 8002;)
    server_name: Server's domain name (e.g., server_name localhost;)
    host: IP address to bind to (e.g., host 127.0.0.1;)
    root: Root directory for serving files (e.g., root docs/fusion_web/;)
    index: Default index file (e.g., index index.html;)
    error_page: Custom error page for HTTP codes (e.g., error_page 404  error_pages/404.html;)
    client_max_body_size: Maximum size of client request body (e.g.,    client_max_body_size 3000000;)

**Location Block Directives**
##### Location blocks define settings for specific URI paths:
    allow_methods: HTTP methods allowed for this location (e.g., allow_methods DELETE POST GET;)
    autoindex: Enable/disable directory listing (e.g., autoindex on;)
    index: Default index file for this location (e.g., index tours1.html;)
    root: Root directory for this location (e.g., root ./;)
    return: Redirect to a different path (e.g., return /tours;)

**CGI Configuration**
##### For CGI support, special directives are available within location blocks

    cgi_path: Path to interpreters (e.g., cgi_path /usr/bin/python3 /bin/bash;)
    cgi_ext: File extensions to be handled by CGI (e.g., cgi_ext .py .sh;)

**Configuration Processing**

##### The configurationsystem processes files through several components

    1.ConfigFile: Handles file operations and reading configuration files
    2.ConfigParser: Parses the configuration content and creates server configurations
    3.ServerConfig: Represents a server configuration with all its settings
    4.Location: Represents a location block within a server configuration

**Validation**

##### The configuration system performs extensive validation to ensure the configuration is valid

    File validation: Checks if paths exist and are accessible
    Syntax validation: Ensures proper syntax with server blocks and directives
    Server validation: Checks for duplicate servers and required directives
    Location validation: Validates location blocks and their directives

### Required Elements (must be specified in each server block)

##### listen: Port the server listens on (no default, must be specified)

    `Example: listen 8080; or listen 127.0.0.1:80`

##### host: IP address to bind to (default: 0.0.0.0 if not specified)
    `Can be combined with port in listen directive`

### Optional Elements with Defaults

##### 1.server_name (default:empty-becomes default server for its listen address)
    Multiple can be specified: `server_name example.com www.example.com;`
    First server block for a host:port becomes default when no name matches

##### 2.root (default: typically none, but often set to something like /var/www/html in practice)
    Should be specified at server or location level

##### 3.index (default: typically `index.html` or `index.htm`)
    Example: `index index.php index.html index.htm;`

##### 4.error_page (default: server-generated basic error pages)
    Example: `error_page 404 /404.html;`

##### 5.client_max_body_size (default: typically 1MB in web servers)
    Example: `client_max_body_size 2M;`(for 2MB limit)

### Location-Specific Directives
##### For route configuration (inside location blocks), these are optional but commonly used:

    - allowed_methods - default:typically GET and HEAD iif not specified
    - return - For HTTP redirections (no default)
    - autoindex - Directory listing (default:off)
    - cgi_extension - For CGI processing (no default)
    - upload_store - For file uploads (no default)

Implementation Recommendations:
##### 1.For each server block, require at least:
    server {
        listen 80;                  # Required
        # host is 0.0.0.0 by default
        server_name example.com;    # Optional (makes this server name-based)
        root /var/www/html;         # Strongly recommended
        index index.html;           # Optional with default
        error_page 404 /404.html;   # Optional
        client_max_body_size 1M;    # Optional with default
    
        location / {
            # At least one location should exist
        }
    }

