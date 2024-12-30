import pytest
import os
import subprocess
import time
import requests

def pytest_addoption(parser):
    print("Registering custom pytest options")
    parser.addoption("--server-executable", action="store", default="/workspaces/webserv/webserv", help="Path to the server executable")
    parser.addoption("--config-file", action="store", default="/workspaces/webserv/config_files/valid_config.conf", help="Path to the server config file")
    parser.addoption("--with-coverage", action="store_true", help="Run server with kcov for coverage")
    parser.addoption("--kcov-output-dir", action="store", default="/workspaces/webserv/.vscode/coverage", help="Path to the kcov output directory")
    parser.addoption("--kcov-excl-path", action="store", default="--exclude-path=/usr/include,/usr/lib,/usr/local", help="Path to exclude from kcov coverage")

@pytest.fixture(scope="session")
def start_server():
    def start_server_function(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path):
        if with_coverage:
        # Create the kcov output directory if it doesn't exist
            os.makedirs(kcov_output_dir, exist_ok=True)
            server_process = subprocess.Popen(["kcov", kcov_excl_path, kcov_output_dir, server_executable, config_file])
            print("Running server with coverage (kcov)...")
        else:
            server_process = subprocess.Popen([server_executable, config_file])
            print("Running server without coverage...")
        return server_process
    return start_server_function

@pytest.fixture(scope="session")
def wait_for_startup():
    def wait_for_startup_function():
    # Wait for the server to start
        timeout = 10
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                # Replace with your server's health-check endpoint
                response = requests.get("http://localhost:8080/health")
                if response.status_code == 200:
                    break
            except requests.ConnectionError:
                pass
            time.sleep(0.1)  # Small delay between retries
        else:
            # If the loop ends without breaking, raise an error
            pytest.fail("Server did not start within the timeout period.")
    return wait_for_startup_function

@pytest.fixture(scope="session")
def stop_server():
    def stop_server_function(server_process):
        server_process.terminate()
        server_process.wait()
        print("Server stopped from test fixture start_cpp_server.")
    return stop_server_function
