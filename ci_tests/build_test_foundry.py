import subprocess
import os

def main():
    os.environ["LOGGER_FOUNDRY_BUILD_TESTING_PATH"] = os.path.join(os.getcwd(), "logger_foundry_lib")
    subprocess.run(["python3", "build_logger_foundry.py", "--cmake-prefix", os.environ["LOGGER_FOUNDRY_BUILD_TESTING_PATH"]], cwd="..")

main()