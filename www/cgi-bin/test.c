#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Content-Type: text/html\r\n\r\n");
    printf("<html>\n");
    printf("<head><title>C CGI Test</title></head>\n");
    printf("<body>\n");
    printf("<h1>C CGI Test</h1>\n");
    
    const char* env_vars[] = {
        "REQUEST_METHOD", "QUERY_STRING", 
        "SCRIPT_NAME", "CONTENT_LENGTH"
    };
    
    for (int i = 0; i < 4; i++) {
        printf("<p>%s: %s</p>\n", 
               env_vars[i], 
               getenv(env_vars[i]) ? getenv(env_vars[i]) : "N/A");
    }
    
    char cwd[1024];
    printf("<p>Current Working Directory: %s</p>\n", 
           getcwd(cwd, sizeof(cwd)) ? cwd : "N/A");
    printf("<p>C Compiler Path: /usr/bin/gcc</p>\n");

	while (4)
	{
		printf("Hang");
	}

    
    printf("</body>\n");
    printf("</html>\n");
    return 0;
}
