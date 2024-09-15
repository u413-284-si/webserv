#include <iostream>
#include <cstdlib>  // For getenv and atoi
#include <string>
#include <cctype>   // For toupper

#define MAX_BODY_SIZE 1024

// Function to convert a string to uppercase
void toUppercase(std::string &str) {
    for (std::string::size_type i = 0; i < str.length(); ++i) {
        str[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[i])));
    }
}

int main() {
    // Fetch the content length from the environment
    const char* contentLength = std::getenv("CONTENT_LENGTH");
    long len = 0;

    // Set the content type for the response (CGI header)
    std::cout << "Content-Type: text/plain\n\n";

    // Check if content length is available
    if (contentLength != 0) {
        char* end = NULL;
        len = std::strtol(contentLength, &end, 10);
        if (*end != '\0') {
            std::cout << "Invalid content length." << std::endl;
            return 1;
        }
    } else {
        std::cout << "No body data received." << std::endl;
        return 1;
    }

    // Check if the body size exceeds the maximum allowed size
    if (len > MAX_BODY_SIZE - 1) {
        std::cout << "Body too large." << std::endl;
        return 1;
    }

    // Read the body data from stdin (CGI POST body is passed via stdin)
    char buffer[MAX_BODY_SIZE];
    std::cin.read(buffer, len);
    buffer[len] = '\0';  // Null-terminate the string

    std::string body(buffer);  // Store the body in a std::string

    // Convert the body to uppercase
    toUppercase(body);

    // Output the modified body
    std::cout << "Modified body: " << body << std::endl;

    return 0;
}
