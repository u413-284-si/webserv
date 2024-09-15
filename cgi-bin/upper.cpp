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

    // Read the body data from stdin
    if (contentLength != NULL) {
        char* end = NULL;
        len = std::strtol(contentLength, &end, 10);
        if (*end != '\0' || len < 0) {
            std::cout << "Invalid content length." << std::endl;
            return 1;
        }

        // Check if the body size exceeds the maximum allowed size
        if (len > MAX_BODY_SIZE - 1) {
            std::cout << "Body too large." << std::endl;
            return 1;
        }

        char buffer[MAX_BODY_SIZE];
        std::cin.read(buffer, len);
        buffer[len] = '\0';  // Null-terminate the string

        std::string body(buffer);  // Store the body in a std::string

        // Convert the body to uppercase
        toUppercase(body);

        // Output the modified body
        std::cout << "Modified body: " << body << std::endl;
    } else {
        // If CONTENT_LENGTH is not set, read until EOF
        std::string body;
        char buffer[MAX_BODY_SIZE];  // Buffer size of 1024 bytes
        while (std::cin.read(buffer, sizeof(buffer)) || std::cin.gcount() > 0) {
            body.append(buffer, std::cin.gcount());
            if (std::cin.eof()) break;  // Exit loop if end-of-file is reached
        }

        // Check body size
        if (body.size() > MAX_BODY_SIZE - 1) {
            std::cout << "Body too large." << std::endl;
            return 1;
        }

        // Convert the body to uppercase
        toUppercase(body);

        // Output the modified body
        std::cout << "Modified body: " << body << std::endl;
    }

    return 0;
}
