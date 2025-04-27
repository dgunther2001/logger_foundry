import subprocess
import os
import argparse

def parse_cmd_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--clean", nargs="?", type=str)
    cmd_line_arguments = parser.parse_args()
    return cmd_line_arguments

def main():
    cmd_line_args = parse_cmd_line_args()
    os.environ["LOGGER_FOUNDRY_BUILD_TESTING_PATH"] = os.path.join(os.getcwd(), "logger_foundry_lib")
    build_test_foundry = ["python3", "build_logger_foundry.py", "--cmake-prefix", os.environ["LOGGER_FOUNDRY_BUILD_TESTING_PATH"]]
    
    if (cmd_line_args.clean):
        build_test_foundry.append(cmd_line_args.clean)

    subprocess.run(build_test_foundry, cwd="..")

main()