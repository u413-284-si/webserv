# This module is for invalid requests.
# Since the requests library is strictly HTTP/1.1 conform, a lower level approach with socket is used

import socket

host = "localhost"
port = 8080

def parse_http_response(sock):
    # Receive the full response
    response_data = sock.recv(4096)

    # Decode response
    response_str = response_data.decode('utf-8')

    # Split into lines
    response_lines = response_str.split('\r\n')

    # Status line (first line)
    status_line = response_lines[0]
    status_code = int(status_line.split()[1])

    # Headers (remaining lines)
    headers = {}
    for line in response_lines[1:]:
        if line.strip() == "":
            # End of headers
            break
        key, value = line.split(":", 1)
        headers[key.strip()] = value.strip()

    return {
        "status_code": status_code,
        "headers": headers,
        "body": response_str.split('\r\n\r\n', 1)[1] if '\r\n\r\n' in response_str else ""
    }



def test_invalid_method_not_implemented():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # Craft a malformed HTTP request
        request = b"INVALIDREQUEST / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        print(response["status_code"])
        assert response["status_code"] == 501

def test_invalid_missing_host_header():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # Craft a malformed HTTP request
        request = b"GET / HTTP/1.1\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_percent_encoding_invalid_char():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # %00 is NUL char
        request = b"GET /search%00maschine HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_percent_encoding_incomplete():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # %00 is NUL char
        request = b"GET /search%3 HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_percent_encoding_non_hex():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # %00 is NUL char
        request = b"GET /search%1$ HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_big_chunk_size():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # Craft a malformed HTTP request
        request = b"POST /uploads/badboy.txt HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFF\r\n0\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 413
