import pytest
import subprocess
import time
import os

# Fixture to start the server for normal tests
@pytest.fixture(scope="package", autouse=True)
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
