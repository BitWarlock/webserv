#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> v = {10, 20, 30, 40};
    std::vector<int>::iterator it = std::find(v.begin(), v.end(), 30);

    if (it != v.end()) {
        std::cout << "Found: " << *it << std::endl;
    }
}
