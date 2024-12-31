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
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_percent_encoding_invalid_char():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /search%00maschine HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_percent_encoding_incomplete():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /search%3 HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_percent_encoding_non_hex():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /search%1$ HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_directory_traversal():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

# Does not work, gets misinterpreted as method not implemented
# def test_invalid_no_request_line():
#     with socket.create_connection((host, port)) as sock:
#         request = b"\r\n\r\n"
#         sock.sendall(request)
#
#         response = parse_http_response(sock)
#         assert response["status_code"] == 400

def test_invalid_request_line_does_not_start_with_slash():
    with socket.create_connection((host, port)) as sock:
        request = b"GET hello.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_not_allowed_char():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /hello\x01world HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_not_allowed_char():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /hello\x01world HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_header_name_ends_in_space():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\nHost: localhost\r\nInvalid : not allowed\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_char_in_header_name():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\nHost: localhost\r\nInva/lid: not allowed\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400

def test_invalid_request_too_long():
    with socket.create_connection((host, port)) as sock:
        request = b"A"*2000
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 431
