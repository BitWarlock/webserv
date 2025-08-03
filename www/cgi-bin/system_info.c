#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <string.h>

int main() {
    struct sysinfo si;
    time_t now;
    struct tm *tm_info;
    
    printf("Content-Type: text/html\r\n\r\n");
    printf("<!DOCTYPE html>\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>System Information - C CGI</title>\n");
    printf("<style>\n");
    printf("body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }\n");
    printf(".container { max-width: 800px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }\n");
    printf("h1 { color: #4caf50; text-align: center; margin-bottom: 30px; }\n");
    printf(".info-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-bottom: 30px; }\n");
    printf(".info-card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid #4caf50; }\n");
    printf(".info-title { color: #4caf50; font-weight: bold; margin-bottom: 10px; }\n");
    printf(".info-value { color: #cccccc; font-family: monospace; }\n");
    printf(".memory-bar { background: #333; height: 20px; border-radius: 10px; overflow: hidden; margin: 10px 0; }\n");
    printf(".memory-used { background: linear-gradient(45deg, #4caf50, #45a049); height: 100%; transition: width 0.3s; }\n");
    printf(".back-btn { background: #666; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 20px; }\n");
    printf("</style>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<div class='container'>\n");
    printf("<h1>System Information</h1>\n");
    
    // Get system information
    if (sysinfo(&si) == 0) {
        printf("<div class='info-grid'>\n");
        
        // System uptime
        printf("<div class='info-card'>\n");
        printf("<div class='info-title'>System Uptime</div>\n");
        int days = si.uptime / 86400;
        int hours = (si.uptime % 86400) / 3600;
        int minutes = (si.uptime % 3600) / 60;
        printf("<div class='info-value'>%d days, %d hours, %d minutes</div>\n", days, hours, minutes);
        printf("</div>\n");
        
        // Memory information
        printf("<div class='info-card'>\n");
        printf("<div class='info-title'>Memory Usage</div>\n");
        unsigned long total_mem = si.totalram * si.mem_unit;
        unsigned long free_mem = si.freeram * si.mem_unit;
        unsigned long used_mem = total_mem - free_mem;
        int mem_percent = (int)((used_mem * 100) / total_mem);
        
        printf("<div class='info-value'>Total: %.2f GB</div>\n", total_mem / (1024.0 * 1024.0 * 1024.0));
        printf("<div class='info-value'>Used: %.2f GB</div>\n", used_mem / (1024.0 * 1024.0 * 1024.0));
        printf("<div class='info-value'>Free: %.2f GB</div>\n", free_mem / (1024.0 * 1024.0 * 1024.0));
        printf("<div class='memory-bar'>\n");
        printf("<div class='memory-used' style='width: %d%%'></div>\n", mem_percent);
        printf("</div>\n");
        printf("<div class='info-value'>Usage: %d%%</div>\n", mem_percent);
        printf("</div>\n");
        
        // Load average
        printf("<div class='info-card'>\n");
        printf("<div class='info-title'>Load Average</div>\n");
        printf("<div class='info-value'>1 min: %.2f</div>\n", si.loads[0] / 65536.0);
        printf("<div class='info-value'>5 min: %.2f</div>\n", si.loads[1] / 65536.0);
        printf("<div class='info-value'>15 min: %.2f</div>\n", si.loads[2] / 65536.0);
        printf("</div>\n");
        
        // Current time
        printf("<div class='info-card'>\n");
        printf("<div class='info-title'>Current Time</div>\n");
        time(&now);
        tm_info = localtime(&now);
        char time_str[26];
        strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        printf("<div class='info-value'>%s</div>\n", time_str);
        printf("</div>\n");
        
        printf("</div>\n");
    }
    
    // Environment variables
    printf("<div class='info-card'>\n");
    printf("<div class='info-title'>CGI Environment Variables</div>\n");
    printf("<div class='info-value'>REQUEST_METHOD: %s</div>\n", getenv("REQUEST_METHOD") ? getenv("REQUEST_METHOD") : "N/A");
    printf("<div class='info-value'>QUERY_STRING: %s</div>\n", getenv("QUERY_STRING") ? getenv("QUERY_STRING") : "N/A");
    printf("<div class='info-value'>SCRIPT_NAME: %s</div>\n", getenv("SCRIPT_NAME") ? getenv("SCRIPT_NAME") : "N/A");
    printf("<div class='info-value'>CONTENT_LENGTH: %s</div>\n", getenv("CONTENT_LENGTH") ? getenv("CONTENT_LENGTH") : "N/A");
    printf("<div class='info-value'>HTTP_USER_AGENT: %s</div>\n", getenv("HTTP_USER_AGENT") ? getenv("HTTP_USER_AGENT") : "N/A");
    printf("</div>\n");
    
    printf("<a href='/cgi-bin/cgi_test.html' class='back-btn'>Back to CGI Testing</a>\n");
    printf("</div>\n");
    printf("</body>\n");
    printf("</html>\n");
    
    return 0;
} 