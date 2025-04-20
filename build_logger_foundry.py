import subprocess
import os
import argparse
import shutil
import platform

def check_dependencies():
    print(f"[*] Ensuring dependency installation for Logger Foundry...")
    subprocess.run(["python3", "install.py"])

def parse_cmd_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--clean", nargs="?", const="soft", type=str, choices=["soft", "hard"])
    parser.add_argument("--cmake-prefix", type=str, required=True)
    cmd_line_arguments = parser.parse_args()
    return cmd_line_arguments

def main():
    check_dependencies()

    os.makedirs("build", exist_ok=True)

    cmd_line_arguments = parse_cmd_line_args()

    cmake_prefix_path = cmd_line_arguments.cmake_prefix

    with open(os.devnull, 'w') as devnull:
        os.makedirs(cmake_prefix_path, exist_ok=True)

        if cmd_line_arguments.clean == "soft":
            subprocess.run(["make", "clean"], cwd="build", stdout=devnull, check=True)

        if cmd_line_arguments.clean == "hard":
            shutil.rmtree("build", ignore_errors=True)
            shutil.rmtree(".cache", ignore_errors=True)
            subprocess.run(["mkdir", "build"])

        subprocess.run(["cmake", "..", "-DBUILD_SHARED_LIBS=ON", f"-DCMAKE_INSTALL_PREFIX={cmake_prefix_path}", \
                        "-DCMAKE_C_COMPILER=clang", "-DCMAKE_CXX_COMPILER=clang++", "-DCMAKE_CXX_STANDARD=20","-DCMAKE_CXX_STANDARD_REQUIRED=ON"], stdout=devnull, cwd="build", check=True)
        subprocess.run(["make"], cwd="build", stdout=devnull, check=True)

        config_relative_path = os.path.join("lib", "logger_foundryConfig.cmake")
        config_absolute_path = os.path.join(cmake_prefix_path, config_relative_path)

        if ((cmd_line_arguments.clean == "hard") or (not os.path.isfile(config_absolute_path))):
            subprocess.run(["make", "install"], stdout=devnull, cwd="build", check=True)

main()



