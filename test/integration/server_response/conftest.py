import pytest
import subprocess
from pytest import FixtureRequest
from typing import Generator
from utils.utils import start_server, wait_for_startup, stop_server

# Fixture to start the server for normal tests
@pytest.fixture(scope="package", autouse=True)
def init_server_instance(request: FixtureRequest) -> Generator[subprocess.Popen, None, None]:
    """
    Fixture to initialize and start a C++ server instance before tests and stop it after tests in the package.

    This fixture starts the server with the specified executable and configuration,
    waits for the server to start, and provides the server process to the tests. After
    the tests are completed, it stops the server.

    Args:
        request (FixtureRequest): The pytest request object, used to access command-line options.

    Yields:
        subprocess.Popen: The process object representing the running server.

    """

    # Get the command-line options passed to pytest
    server_executable = request.config.getoption("--server-executable")
    config_file = request.config.getoption("--config-file")

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    kcov_output_dir = request.config.getoption("--kcov-output-dir")
    kcov_excl_path = request.config.getoption("--kcov-excl-path")

    server_process = start_server(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path)

    wait_for_startup(server_process)

    # Provide the server process to the tests
    yield server_process

    stop_server(server_process)
