import subprocess
import os
import argparse
import shutil

def parse_cmd_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--clean", nargs="?", const="soft", type=str, choices=["soft", "hard"])
    parser.add_argument("--cmake-prefix", type=str, required=True)
    cmd_line_arguments = parser.parse_args()
    return cmd_line_arguments

def main():
    os.makedirs("build", exist_ok=True)

    cmd_line_arguments = parse_cmd_line_args()

    cmake_prefix_path = cmd_line_arguments.cmake_prefix

    with open(os.devnull, 'w') as devnull:
        if cmd_line_arguments.clean == "soft":
            subprocess.run(["make", "clean"], cwd="build", stdout=devnull, check=True)

        if cmd_line_arguments.clean == "hard":
            shutil.rmtree("build", ignore_errors=True)
            shutil.rmtree(".cache", ignore_errors=True)
            subprocess.run(["mkdir", "build"])

        subprocess.run(["cmake", "..", "-DBUILD_SHARED_LIBS=ON", f"-DCMAKE_INSTALL_PREFIX={cmake_prefix_path}"], stdout=devnull, cwd="build", check=True)
        subprocess.run(["make"], cwd="build", stdout=devnull, check=True)
        subprocess.run(["make", "install"], stdout=devnull, cwd="build", check=True)

main()



