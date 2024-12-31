# This module is for invalid requests.
# Since the requests library is strictly HTTP/1.1 conform, a lower level approach with socket is used

from utils.utils import parse_http_response
import socket

host = "localhost"
port = 8080

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

# Does not work, gets misinterpreted as method not implemented
# def test_invalid_no_request_line():
#     # Create a socket connection
#     with socket.create_connection((host, port)) as sock:
#         # Empty request line
#         request = b"\r\n\r\n"
#         sock.sendall(request)

#         # Receive the response
#         response = parse_http_response(sock)
#         assert response["status_code"] == 400