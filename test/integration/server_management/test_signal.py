# This module is for signal handling

import signal
import os
import time
import subprocess

# Special test for shutdown behavior
def test_server_shutdown_behavior(request, start_server, wait_for_startup, stop_server):

    server_executable = request.config.getoption("--server-executable")
    config_file = config_file = request.config.getoption("--config-file")

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    # Path to the C++ server executable
    kcov_output_dir = request.config.getoption("--kcov-output-dir")
    kcov_excl_path = request.config.getoption("--kcov-excl-path")

    server_process = start_server(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path)

    wait_for_startup()

    if with_coverage:
        pid = int(subprocess.check_output(["pgrep", "webserv"]).decode().strip())
    else:
        pid = server_process.pid

    print("Testing server shutdown behavior...")
    os.kill(pid, signal.SIGQUIT)
    time.sleep(3)

    stop_server(server_process)

