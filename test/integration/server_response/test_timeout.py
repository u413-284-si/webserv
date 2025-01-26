# This module is for requests which time out.

from utils.utils import parse_http_response
from utils.utils import make_request
import socket
import time
import pytest

host = "localhost"
port = 8080
wait_for_timeout = 61

@pytest.mark.timeout
def test_timeout_partial_request_sent():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\n"
        sock.sendall(request)
        time.sleep(wait_for_timeout)

        response = parse_http_response(sock)
        assert response["status_code"] == 408
        assert response["headers"].get("connection") == "close"

@pytest.mark.timeout
def test_timeout_full_request_partial_body():
    with socket.create_connection((host, port)) as sock:
        request = b"POST /uploads/tooslow.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 300\r\n\r\nI am too slow"
        sock.sendall(request)
        time.sleep(wait_for_timeout)

        response = parse_http_response(sock)
        assert response["status_code"] == 408
        assert response["headers"].get("connection") == "close"

@pytest.mark.timeout
def test_timeout_full_request_then_nothing():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)
        response = parse_http_response(sock)
        assert response["status_code"] == 200
        time.sleep(wait_for_timeout)

        response = parse_http_response(sock)
        assert response["status_code"] == 408
        assert response["headers"].get("connection") == "close"

@pytest.mark.timeout
def test_timeout_infinite_CGI():
    url = "http://localhost:8080/cgi-bin/infinite.sh"
    response = make_request(url, timeout=wait_for_timeout)
    assert response.status_code == 408
    assert response.headers["connection"] == "close"
