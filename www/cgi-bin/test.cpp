#include <iostream>
#include <cstdlib>
#include <unistd.h>

int main() {
    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<html>\n";
    std::cout << "<head><title>C++ CGI Test</title></head>\n";
    std::cout << "<body>\n";
    std::cout << "<h1>C++ CGI Test</h1>\n";
    

    std::cout << "<p>C++ Binary Path: /usr/bin/g++</p>\n";
    std::cout << "</body>\n</html>\n";
    return 0;
}
