import pytest
import subprocess
import time
import os

def pytest_addoption(parser):
    print("Registering custom pytest options")
    parser.addoption("--server-executable", action="store", default="/workspaces/webserv/webserv", help="Path to the server executable")
    parser.addoption("--config-file", action="store", default="/workspaces/webserv/config_files/valid_config.conf", help="Path to the server config file")
    parser.addoption("--with-coverage", action="store_true", help="Run server with kcov for coverage")
    parser.addoption("--kcov-output-dir", action="store", default="/workspaces/webserv/.vscode/coverage", help="Path to the kcov output directory")
    parser.addoption("--kcov-excl-path", action="store", default="--exclude-path=/usr/include,/usr/lib,/usr/local", help="Path to exclude from kcov coverage")

# Fixture to start the server for normal tests
@pytest.fixture(scope="session")
def start_cpp_server(request):
    cpp_server_executable = request.config.getoption("--server-executable")
    config_file = request.config.getoption("--config-file")

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    # Path to the C++ server executable and kcov output
    kcov_excl_path = request.config.getoption("--kcov-excl-path")
    kcov_output_dir = request.config.getoption("--kcov-output-dir")

    if with_coverage:
        # Create the kcov output directory if it doesn't exist
        os.makedirs(kcov_output_dir, exist_ok=True)
        server_process = subprocess.Popen(
            ["kcov", kcov_excl_path, kcov_output_dir, cpp_server_executable, config_file]
        )
        print("Running server with coverage (kcov)...")
        delay_time = 10
    else:
        server_process = subprocess.Popen(
            [cpp_server_executable, config_file]
        )
        print("Running server without coverage...")
        delay_time = 3

    # Wait for the server to start
    time.sleep(delay_time)

    # Provide the server process to the tests
    yield server_process

    # Stop the server after the tests are done
    server_process.terminate()
    server_process.wait()
    print("Server stopped from test fixture start_cpp_server.")
