from utils.utils import make_request
import os

def test_DELETE_file(test_path_cleanup):
    existing_content = "Please delete"
    file_path = "/workspaces/webserv/html/uploads/delete_file.txt"
    with open(file_path, "w") as file:
        file.write(existing_content)
    test_path_cleanup.append(file_path)
    url = "http://localhost:8080/uploads/delete_file.txt"
    response = make_request(url, method="DELETE")
    assert response.status_code == 200
    assert response.headers["content-type"] == "application/json"
    assert file_path in response.json().get("file")
    assert not os.path.isfile(file_path)

def test_DELETE_directory(test_path_cleanup):
    dir_path = "/workspaces/webserv/html/uploads/delete_dir"
    os.mkdir(dir_path)
    test_path_cleanup.append(dir_path)
    url = "http://localhost:8080/uploads/delete_dir"
    response = make_request(url, method="DELETE")
    assert response.status_code == 403
    assert response.headers["content-type"] == "text/html"
    assert os.path.isdir(dir_path)

def test_DELETE_no_permissions(test_path_cleanup, temp_permission_change):
    existing_content = "You can't delete me"
    file_path = "/workspaces/webserv/html/uploads/immune_to_delete.txt"
    with open(file_path, "w") as file:
        file.write(existing_content)
    test_path_cleanup.append(file_path)
    # ability to delete is determined by permissions of the directory containing the file.
    dir_path = "/workspaces/webserv/html/uploads"
    temp_permission_change(dir_path, 0o555)
    url = "http://localhost:8080/uploads/immune_to_delete.txt"
    response = make_request(url, method="DELETE")
    assert response.status_code == 403
    assert response.headers["content-type"] == "text/html"
    