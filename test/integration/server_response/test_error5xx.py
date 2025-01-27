# This module is for requests which result in status code 5xx

from utils.utils import make_request
from utils.utils import parse_http_response
import socket

host = "localhost"
port = 8080

def test_5xx_max_recurision():
    print("Request for /recursion/")
    url = "http://localhost:8080/recursion/"
    response = make_request(url)
    assert response.status_code == 500

def test_5xx_method_not_implemented():

    # Create a socket connection
    with socket.create_connection((host, port)) as sock:
        # Craft a malformed HTTP request
        request = b"INVALIDREQUEST / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        # Receive the response
        response = parse_http_response(sock)
        assert response["status_code"] == 501

def test_5xx_cgi_with_no_header():
    url = "http://localhost:8080/cgi-bin/no_header.sh"
    response = make_request(url)
    assert response.status_code == 500

def test_5xx_cgi_with_no_cgi_field():
    url = "http://localhost:8080/cgi-bin/no_cgi_field.sh"
    response = make_request(url)
    assert response.status_code == 500

def test_5xx_cgi_with_invalid_header_status():
    url = "http://localhost:8080/cgi-bin/invalid_header_status.sh"
    response = make_request(url)
    assert response.status_code == 500

def test_5xx_cgi_with_invalid_header_connection():
    url = "http://localhost:8080/cgi-bin/invalid_header_connection.sh"
    response = make_request(url)
    assert response.status_code == 500

def test_5xx_http_version_0_9():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/0.9\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)
        response = parse_http_response(sock)
        assert response["status_code"] == 505
        assert response["headers"].get("connection") == "keep-alive"
