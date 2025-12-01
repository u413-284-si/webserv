import pytest

def pytest_addoption(parser):
    print("Registering custom pytest options")
    parser.addoption("--server-executable", action="store", default="/workspaces/webserv/webserv", help="Path to the server executable")
    parser.addoption("--config-file", action="store", default="/workspaces/webserv/config_files/example.conf", help="Path to the server config file")
    parser.addoption("--with-coverage", action="store_true", help="Run server with kcov for coverage")
    parser.addoption("--kcov-output-dir", action="store", default="/workspaces/webserv/kcov_report", help="Path to the kcov output directory")
    parser.addoption("--kcov-excl-path", action="store", default="--exclude-path=/usr/include,/usr/lib,/usr/local", help="Path to exclude from kcov coverage")

def pytest_collection_modifyitems(items):
    for item in items:
        if "signal" in item.nodeid:
            item.add_marker(pytest.mark.signal)
        elif "CGI" in item.nodeid:
            item.add_marker(pytest.mark.cgi)
        elif "3xx" in item.nodeid:
            item.add_marker(pytest.mark.redirect)
        elif "4xx" in item.nodeid:
            item.add_marker(pytest.mark.error)
        elif "5xx" in item.nodeid:
            item.add_marker(pytest.mark.error)
        elif "GET" in item.nodeid:
            item.add_marker(pytest.mark.get)
        elif "POST" in item.nodeid:
            item.add_marker(pytest.mark.post)
        elif "timeout" in item.nodeid:
            item.add_marker(pytest.mark.timeout)
