import signal
import os
import time
import subprocess

# Special test for shutdown behavior
def test_server_shutdown_behavior(request):

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    # Path to the C++ server executable
    kcov_output_dir = request.config.getoption("--kcov-output-dir")
    kcov_excl_path = request.config.getoption("--kcov-excl-path")
    cpp_server_executable = request.config.getoption("--server-executable")
    config_file = config_file = request.config.getoption("--config-file")

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
        print("Running server...")
        delay_time = 3

    time.sleep(delay_time)
    print("Testing server shutdown behavior...")
    os.kill(server_process.pid, signal.SIGQUIT)
    time.sleep(3)
    server_process.terminate()
    server_process.wait()
    print("Server stopped.")


