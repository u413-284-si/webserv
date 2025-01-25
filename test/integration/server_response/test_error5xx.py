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
        print(response["status_code"])
        assert response["status_code"] == 501
