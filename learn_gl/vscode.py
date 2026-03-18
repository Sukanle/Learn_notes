# Generate a Visual Studio Code task configuration(tasks.json) for building and running a C++ project.
import json
import os
import platform
import argparse
import sys

parser = argparse.ArgumentParser()
parser.add_argument(
    "--generate",
    help="Generate json files for Visual Studio Code (e.g., all, task(s), launch)",
    default="all",
)
parser.add_argument("--launch-type", help="Launch type for debugging (e.g., cppdbg, gdb, lldb)", default="lldb-dap")
parser.add_argument("--force", help="Force overwrite existing tasks.json and launch.json files", action="store_true")
args = parser.parse_args()

class TaskGenerator:
    def __init__(self, _cmd, _cfgs):
        self.cmd = _cmd
        self.cfgs = _cfgs

def generate_tasks_json(task_gen):
    tasks = {
        "version": "2.0.0",
        "tasks": [
            {
                "label": "xmake build",
                "type": "process",
                "command": task_gen.cmd,
                "group": {
                    "kind": "build",
                    "isDefault": True
                },
                "problemMatcher": [],
                "dependsOn": "xmake configure"
            },
            {
                "label": "xmake configure",
                "type": "process",
                "command": task_gen.cmd,
                "args": task_gen.cfgs,
                "group": {
                    "kind": "build",
                    "isDefault": False
                },
                "problemMatcher": []
            }
        ]
    }
    with open(os.path.join('.vscode', 'tasks.json'), 'w') as f:
        json.dump(tasks, f, indent=4)

def generate_launch_json(debug_type, executable_path):
    launch = {
        "version": "0.2.0",
        "configurations": [
            {
                "type": debug_type,
                "request": "launch",
                "name": "Debug C++",
                "program": executable_path,
                "args": [],
                "env": [],
                "cwd": "${workspaceFolder}",
            }
        ]
    }
    with open(os.path.join('.vscode', 'launch.json'), 'w') as f:
        json.dump(launch, f, indent=4)



def main() ->int:
    build_cmd = "xmake"
    build_plat = platform.system()
    exe_Ext = ".exe" if build_plat.startswith('Windows') else ""
    if build_plat.startswith('Windows'):
        build_plat = "windows"
    elif build_plat.startswith('Linux'):
        build_plat = "linux"
    elif build_plat.startswith('Darwin'):
        build_plat = "macosx"

    machine_arch = platform.machine()
    if machine_arch == "AMD64" or machine_arch == "x86_64":
        machine_arch = "x86_64"
    elif machine_arch == "i386" or machine_arch == "i686":
        machine_arch = "i386"
    elif machine_arch == "arm64" or machine_arch == "aarch64":
        machine_arch = "arm64"

    executable_path = "${workspaceFolder}/build/" + build_plat + "/" + machine_arch + "/debug/" + "${fileBasenameNoExtension}" + exe_Ext

    if not os.path.exists('.vscode'):
        os.makedirs('.vscode')

    if not args.force:
        if os.path.exists(os.path.join('.vscode', 'tasks.json')):
            print("tasks.json already exists. Use --force to overwrite.")
            return 1
        if os.path.exists(os.path.join('.vscode', 'launch.json')):
            print("launch.json already exists. Use --force to overwrite.")
            return 1
        
    
    task_gen = TaskGenerator(build_cmd, ["f", "-p", build_plat, "-a", machine_arch, "-m", "debug"])

    generate_mode = args.generate.strip().lower()
    if generate_mode.endswith("s"):
        generate_mode = generate_mode[:-1]

    if generate_mode in ["all", "task"]:
        generate_tasks_json(task_gen)
    if generate_mode in ["all", "launch"]:
        generate_launch_json(args.launch_type, executable_path)
    return 0

if __name__ == "__main__":
    sys.exit(main())