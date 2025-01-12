# This module is for requests which result in errors

import requests
import os
import stat

def test_file_not_found():
    print("Request for /nonexistent.html")
    response = requests.get("http://localhost:8080/nonexistent.html")
    assert response.status_code == 404

def test_permission_denied_for_file():
    print("Chmod 000 index.html and request it")
    file_path = "/workspaces/webserv/html/index.html"
    # Get and store the current permissions
    original_permissions = stat.S_IMODE(os.stat(file_path).st_mode)
    # Change permissions to 000
    os.chmod(file_path, 0o000)
    # Try to access
    response = requests.get("http://localhost:8080/index.html")
    # Restore the original permissions
    os.chmod(file_path, original_permissions)
    assert response.status_code == 403

def test_permission_denied_in_path():
    print("Chmod 000 directory and request file in it")
    directory_path = "/workspaces/webserv/html/directory"
    # Get and store the current permissions
    original_permissions = stat.S_IMODE(os.stat(directory_path).st_mode)
    # Change permissions to 000
    os.chmod(directory_path, 0o000)
    # Try to access
    response = requests.get("http://localhost:8080/directory/random.txt")
    # Restore the original permissions
    os.chmod(directory_path, original_permissions)
    assert response.status_code == 403

def test_directory_no_autoindex():
    print("Request for directory without autoindex")
    response = requests.get("http://localhost:8080/css/")
    assert response.status_code == 403

def test_no_permission_to_append():
    print("Chmod 000 existing_file and try to append")
    # Body to send
    existing_content = "Hello, World!\n"
    payload = "It is me!"
    dst_file_path = "/workspaces/webserv/html/uploads/existing_file.txt"
    with open(dst_file_path, "w") as file:
        file.write(existing_content)
    original_permissions = stat.S_IMODE(os.stat(dst_file_path).st_mode)
    # Change permissions to 000
    os.chmod(dst_file_path, 0o000)

    response = requests.post("http://localhost:8080/uploads/existing_file.txt", data=payload)

    # Restore the original permissions
    os.chmod(dst_file_path, original_permissions)

    assert response.status_code == 403
    # Check if file was not appended
    with open(dst_file_path, "r") as file:
        content = file.read()
        assert content.find(payload) == -1
    # Delete created file
    os.remove(dst_file_path)

def test_missing_dir_in_path():
    print("Request to /workspaces/webserv/html/uploads/not_exist/upload.txt")
    payload = "Hello World!"

    response = requests.post("http://localhost:8080/uploads/not_exist/upload.txt", data=payload)

    assert response.status_code == 404
