import pytest
import time
import requests

# Fixture to start the server for normal tests
@pytest.fixture(scope="package", autouse=True)
def start_cpp_server(request, start_server, wait_for_startup, stop_server):
    server_executable = request.config.getoption("--server-executable")
    config_file = request.config.getoption("--config-file")

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    # Path to the C++ server executable and kcov output
    kcov_output_dir = request.config.getoption("--kcov-output-dir")
    kcov_excl_path = request.config.getoption("--kcov-excl-path")

    server_process = start_server(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path)

    wait_for_startup()

    # Provide the server process to the tests
    yield server_process

    stop_server(server_process)
