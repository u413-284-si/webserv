# This module is for invalid requests.
# Since the requests library is strictly HTTP/1.1 conform, a lower level approach with socket is used

import socket
import requests
import time

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

def test_invalid_directory_traversal():
    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # Craft HTTP request with directory traversal try
        request = b"GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_epoll_partial_and_complete_requests():
    # Create two socket
    client1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect first client socket and send half of the request
    client1.connect((host, port))
    client1.sendall(b"GET /partial ")

    # Wait a bit to ensure the next parts trigger epoll simultaneously
    time.sleep(0.1)

    # Send the second half of the first request and create second client socket
    client1.sendall(b"HTTP/1.1\r\n\r\n")
    client2.connect((host, port))

    # And send full new request
    client2.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")

    # Read responses
    response1 = parse_http_response(client1)
    response2 = parse_http_response(client2)

    assert response1["status_code"] == 400
    assert response2["status_code"] == 200

    client1.close()
    client2.close()
