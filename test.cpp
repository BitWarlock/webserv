#include "src/ParseRequest.cpp"
#include "inc/ParseRequest.hpp"
#include <fstream>
#include <iostream>

int main() {
    ParseRequest parser;

    // Load compressed file into BufferBody
    std::ifstream file("body.gz", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open body.gz\n";
        return 1;
    }

    std::string compressed((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    parser.getBufferBody() = compressed;
    parser.setContentEncodingType(GZIP);

    parser.DecompressBody();
    std::cout << parser.getBufferDecompressedBody();

    std::string expected = "Hello, this is a test string for compression!";
    if (parser.getBufferDecompressedBody() == expected) {
        std::cout << "✅ Decompression test passed!\n";
    } else {
        std::cerr << "❌ Decompression test failed!\n";
        std::cerr << "Expected: " << expected << "\n";
        std::cerr << "Got: " << parser.getBufferDecompressedBody() << "\n";
    }

    return 0;
}
