#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, std::string> mydictionary;
    mydictionary.insert(std::pair<std::string, std::string>("starwberry", "die dbree"));
    mydictionary.insert(std::pair<std::string, std::string>("orange", "die orange"));
    mydictionary.insert(std::pair<std::string, std::string>("apple", "der apfel"));
    mydictionary.insert(std::pair<std::string, std::string>("banana", "die banana"));
    for (auto pair : mydictionary){
        std::cout << pair.first << " " << pair.second << "\n";
    }
}
