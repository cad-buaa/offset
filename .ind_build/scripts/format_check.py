#!/usr/bin/env python
# -*- coding: utf-8 -*-
import argparse
import os
import os.path as op
import subprocess


def cmd(command: str, err_info: bool = True):
    """execute command."""
    try:
        result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True, encoding='utf-8')
        return result
    except Exception as e:
        if err_info:
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


def check_clang_format(files: list) -> int:
    """clang-format for files"""
    unformated_file_num = 0
    for file in files:
        command = f"clang-format -output-replacements-xml -i {file}"
        result = cmd(command)
        if "</replacement>" in result.stdout:
            unformated_file_num += 1
            print(f"[ERROR] clang-format needed: {file}")
    return unformated_file_num


def check_cmake_format(files: list) -> int:
    """cmake-format for files"""
    unformated_file_num = 0
    for file in files:
        command = f"cmake-format --config .cmake-format --check {file}"
        result = cmd(command, False)
        if result is None:
            unformated_file_num += 1
            print(f"[ERROR] cmake-format needed: {file}")
    return unformated_file_num


def run(opt):
    ret = 0
    if opt.clang_format:  # clang-format
        search_dirs = ["include", "src", "tests", "benchs", "samples", "application"]
        search_extensions = [".h", ".c", ".hpp", ".cpp", ".hxx", ".cxx"]
        search_exclude_dirs = ["acis"]
        files = get_files_with_extensions(
            search_dirs, search_extensions, search_exclude_dirs)
        result_num = check_clang_format(files)
        ret += result_num
    print("[INFO] ========================================================")
    if opt.cmake_format:  # cmake-format
        search_dirs = ["."]
        search_extensions = ["CMakeLists.txt", ".cmake"]
        search_exclude_dirs = [".git", "build", "install", "out", "scripts"]
        files = get_files_with_extensions(
            search_dirs, search_extensions, search_exclude_dirs)
        result_num = check_cmake_format(files)
        ret += result_num
    assert ret == 0
    print("[INFO] Over: Format check.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--clang_format', type=bool,
                        default=True, help='do clang-format check')
    parser.add_argument('--cmake_format', type=bool,
                        default=True, help='do cmake-format check')
    run(parser.parse_args())
