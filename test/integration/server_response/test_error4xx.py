# This module is for requests which result in status code 4xx

import os
import stat
from utils.utils import make_request

def test_4xx_file_not_found():
    print("Request for /nonexistent.html")
    url = "http://localhost:8080/nonexistent.html"
    response = make_request(url)
    assert response.status_code == 404

def test_4xx_permission_denied_for_file():
    print("Chmod 000 index.html and request it")
    file_path = "/workspaces/webserv/html/index.html"
    # Get and store the current permissions
    original_permissions = stat.S_IMODE(os.stat(file_path).st_mode)
    # Change permissions to 000
    os.chmod(file_path, 0o000)
    # Try to access
    url = "http://localhost:8080/index.html"
    response = make_request(url)
    # Restore the original permissions
    os.chmod(file_path, original_permissions)
    assert response.status_code == 403

def test_4xx_permission_denied_in_path():
    print("Chmod 000 directory and request file in it")
    directory_path = "/workspaces/webserv/html/directory"
    # Get and store the current permissions
    original_permissions = stat.S_IMODE(os.stat(directory_path).st_mode)
    # Change permissions to 000
    os.chmod(directory_path, 0o000)
    # Try to access
    url = "http://localhost:8080/directory/random.txt"
    response = make_request(url)
    # Restore the original permissions
    os.chmod(directory_path, original_permissions)
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

