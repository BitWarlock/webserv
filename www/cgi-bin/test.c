#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char *request_method = getenv("REQUEST_METHOD");
    char *query_string   = getenv("QUERY_STRING");
    char *script_name    = getenv("SCRIPT_NAME");
    char *content_length = getenv("CONTENT_LENGTH");

    printf("Content-Type: text/html\r\n\r\n");
    printf("<html>\n");
    printf("<head><title>C CGI Test</title></head>\n");
    printf("<body>\n");
    printf("<h1>C CGI Test</h1>\n");

    printf("<p>Request Method: %s</p>\n", request_method ? request_method : "N/A");
    printf("<p>Query String: %s</p>\n", query_string ? query_string : "N/A");
    printf("<p>Script Name: %s</p>\n", script_name ? script_name : "N/A");
    printf("<p>Content Length: %s</p>\n", content_length ? content_length : "N/A");

    if (request_method && strcmp(request_method, "POST") == 0) {
        if (content_length) {
            int len = atoi(content_length);
            if (len > 0) {
                char *post_data = malloc(len + 1);
                if (post_data) {
                    fread(post_data, 1, len, stdin);
                    post_data[len] = '\0';
                    printf("<p>Post data: %s</p>\n", post_data);
                    free(post_data);
                }
            }
        }
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("<p>Current Working Directory: %s</p>\n", cwd);
    } else {
        printf("<p>Current Working Directory: N/A</p>\n");
    }

    if (argc > 0) {
        printf("<p>Script Path: %s</p>\n", argv[0]);
    } else {
        printf("<p>Script Path: No script path</p>\n");
    }

    printf("<p>Executable Path: %s</p>\n", "/usr/bin/env cgi-bin-executable"); // No exact Python equivalent

    printf("</body>\n");
    printf("</html>\n");

    return 0;
}
