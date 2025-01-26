# This module is for requests which result in status code 4xx

import os
from utils.utils import make_request
from utils.utils import parse_http_response
import socket
import time

host = "localhost"
port = 8080

def test_4xx_file_not_found():
    print("Request for /nonexistent.html")
    url = "http://localhost:8080/nonexistent.html"
    response = make_request(url)
    assert response.status_code == 404

def test_4xx_permission_denied_for_file(temp_permission_change):
    print("Chmod 000 index.html and request it")
    file_path = "/workspaces/webserv/html/index.html"

    temp_permission_change(file_path, 0o000)

    url = "http://localhost:8080/index.html"
    response = make_request(url)
    assert response.status_code == 403

def test_4xx_permission_denied_in_path(temp_permission_change):
    print("Chmod 000 directory and request file in it")
    directory_path = "/workspaces/webserv/html/directory"

    temp_permission_change(directory_path, 0o000)

    url = "http://localhost:8080/directory/random.txt"
    response = make_request(url)
    assert response.status_code == 403

def test_4xx_directory_no_autoindex():
    print("Request for directory without autoindex")
    url = "http://localhost:8080/css/"
    response = make_request(url)
    assert response.status_code == 403

def test_4xx_directory_forbidden():
    print("Request for /secret/")
    url = "http://localhost:8080/secret/"
    response = make_request(url)
    assert response.status_code == 403

def test_4xx_returns_forbidden_error_page_returns_other_code():
    print("Request for /another")
    url = "http://localhost:8080/another"
    response = make_request(url)
    assert response.status_code == 403
    assert "Return Message" in response.text

def test_4xx_missing_indices_tries_directory():
    print("Request for /missingIndex/")
    url = "http://localhost:8080/missingIndex/"
    response = make_request(url)
    assert response.status_code == 403

def test_4xx_character_device():
    print("Request for /tty")
    url = "http://localhost:8080/tty"
    response = make_request(url)
    assert response.status_code == 403

def test_4xx_missing_host_header():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_percent_encoding_invalid_char():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /search%00maschine HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_percent_encoding_incomplete():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /search%3 HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_percent_encoding_non_hex():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /search%1$ HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_directory_traversal():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

# Does not work, gets misinterpreted as method not implemented > a empty line can't enter RequestParser
# def test_4xx_no_request_line():
#     with socket.create_connection((host, port)) as sock:
#         request = b"\r\n\r\n"
#         sock.sendall(request)
#
#         response = parse_http_response(sock)
#         assert response["status_code"] == 400

def test_4xx_request_line_does_not_start_with_slash():
    with socket.create_connection((host, port)) as sock:
        request = b"GET hello.html HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_not_allowed_char():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /hello\x01world HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_not_allowed_char():
    with socket.create_connection((host, port)) as sock:
        request = b"GET /hello\x01world HTTP/1.1\r\nHost: localhost\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_header_name_ends_in_space():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\nHost: localhost\r\nInvalid : not allowed\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_char_in_header_name():
    with socket.create_connection((host, port)) as sock:
        request = b"GET / HTTP/1.1\r\nHost: localhost\r\nInva/lid: not allowed\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 400
        assert response["headers"].get("connection") == "close"

def test_4xx_request_too_long():
    with socket.create_connection((host, port)) as sock:
        request = b"A"*2000
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 431
        assert response["headers"].get("connection") == "close"

def test_4xx_chunk_size_too_big():
    with socket.create_connection((host, port)) as sock:
        request = b"POST /uploads/badboy.txt HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFF\r\n0\r\n\r\n"
        sock.sendall(request)

        response = parse_http_response(sock)
        assert response["status_code"] == 413
        assert response["headers"].get("connection") == "close"

def test_4xx_epoll_partial_and_complete_requests():
    # Create two sockets
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
    assert response1["headers"].get("connection") == "close"
    assert response2["status_code"] == 200
    assert response2["headers"].get("connection") == "keep-alive"

    client1.close()
    client2.close()

def test_4xx_no_permission_to_append(temp_permission_change, test_file_cleanup):
    print("Chmod 000 existing_file and try to append")
    # Body to send
    existing_content = "Hello, World!\n"
    dst_file_path = "/workspaces/webserv/html/uploads/existing_file.txt"
    with open(dst_file_path, "w") as file:
        file.write(existing_content)

    # Make file readonly
    temp_permission_change(dst_file_path, 0o444)
    test_file_cleanup.append(dst_file_path)

    url = "http://localhost:8080/uploads/existing_file.txt"
    payload = "It is me!"
    response = make_request(url, method="POST", data=payload)

    assert response.status_code == 403
    # Check if file was not appended
    with open(dst_file_path, "r") as file:
        content = file.read()
        assert content.find(payload) == -1

def test_4xx_missing_dir_in_path():
    print("Request to /workspaces/webserv/html/uploads/not_exist/upload.txt")
    url = "http://localhost:8080/uploads/not_exist/upload.txt"
    payload = "Hello World!"

    response = make_request(url, method="POST", data=payload)

    assert response.status_code == 404

def test_4xx_file_too_big():
    print("Request for /uploads/cat.jpg")

    src_file_path = "/workspaces/webserv/html/images/cat.jpg"
    with open(src_file_path, 'rb') as f:
        binary_data = f.read()
    dst_file_path = "/workspaces/webserv/html/uploads/cat.jpg"
    url = "http://localhost:8080/uploads/cat.jpg"

    response = make_request(url, method="POST", data=binary_data)

    assert response.status_code == 413
    assert response.headers["connection"] == "close"
    # Check that file does not exist
    assert os.path.isfile(dst_file_path) == False

def test_method_not_allowed():
    print("Request to /")
    url = "http://localhost:8080/"
    payload = "Hello World!"

    response = make_request(url, method="POST", data=payload)

    assert response.status_code == 405
    assert response.headers["allow"] == "GET"
