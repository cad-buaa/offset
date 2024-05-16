#!/usr/bin/env python
# -*- coding: utf-8 -*-
import argparse
import os
import os.path as op
import subprocess


def cmd(command: str):
    """execute command."""
    try:
        result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True, encoding='utf-8')
        return result
    except Exception as e:
        print(f"Command Error: {e}")


def get_files_with_extensions(directoris: list, extensions: list, exclude_dirs: list):
    """get all files with given extensions in directory."""
    matching_files = set()

    for directory in directoris:
        for root, dirs, files in os.walk(directory):
            dirs[:] = [d for d in dirs if d not in exclude_dirs]  # exclude
            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    matching_files.add(op.join(root, file))

    return matching_files


def clang_format(files: list):
    """clang-format for files"""
    command = f"clang-format -i {' '.join(files)}"
    cmd(command)


def cmake_format(files: list):
    """cmake-format for files"""
    command = f"cmake-format --config .cmake-format -i {' '.join(files)}"
    cmd(command)


def run(opt):
    if opt.clang_format:  # clang-format
        search_dirs = ["include", "src", "tests", "benchs", "samples", "application"]
        search_extensions = [".h", ".c", ".hpp", ".cpp", ".hxx", ".cxx"]
        search_exclude_dirs = []
        files = get_files_with_extensions(
            search_dirs, search_extensions, search_exclude_dirs)
        clang_format(files)
    print("[INFO] ========================================================")
    if opt.cmake_format:  # cmake-format
        search_dirs = ["."]
        search_extensions = ["CMakeLists.txt", ".cmake"]
        search_exclude_dirs = [".git", "build", "install", "out", "scripts"]
        files = get_files_with_extensions(
            search_dirs, search_extensions, search_exclude_dirs)
        cmake_format(files)
    print("[INFO] Over: Format.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--clang_format', type=bool,
                        default=True, help='do clang-format')
    parser.add_argument('--cmake_format', type=bool,
                        default=True, help='do cmake-format')
    run(parser.parse_args())
