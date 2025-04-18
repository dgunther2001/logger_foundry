

import subprocess
import os
import argparse


def main():
    subprocess.run(["rm", "-rf", "build"])
    os.makedirs("build", exist_ok=True)

    parser = argparse.ArgumentParser()
    parser.add_argument("--cmake-prefix", type=str, required=True)
    cmd_line_arguments = parser.parse_args()

    cmake_prefix_path = cmd_line_arguments.cmake_prefix

    with open(os.devnull, 'w') as devnull:
        subprocess.run(["cmake", "..", "-DBUILD_SHARED_LIBS=ON", f"-DCMAKE_INSTALL_PREFIX={cmake_prefix_path}"], stdout=devnull, cwd="build", check=True)
        subprocess.run(["make"], cwd="build", stdout=devnull, check=True)
        subprocess.run(["make", "install"], stdout=devnull, cwd="build", check=True)

main()



