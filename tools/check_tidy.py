#!/usr/bin/env python

# Copyright (c) 2021-present Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# note: this uses `black` for formatting.

import os
import subprocess
import sys

from argparse import ArgumentParser
from difflib import unified_diff
from os.path import join, relpath, splitext
from distutils import spawn

TERM_RED = "\033[1;31m"
TERM_GREEN = "\033[1;32m"
TERM_YELLOW = "\033[1;33m"
TERM_PURPLE = "\033[35m"
TERM_EMPTY = "\033[0m"


clang_format_exts = [".cc", ".h"]
skip_dirs = []
skip_files = []


class Stats(object):
    def __init__(self):
        self.files = 0
        self.lines = 0
        self.empty_lines = 0
        self.errors = 0


def is_checked_by_clang(file):
    _, ext = splitext(file)
    return ext in clang_format_exts and file not in skip_files


def check_tidy(src_dir, update, base, stats):
    clang_format = spawn.find_executable(base)
    if not clang_format:
        clang_format = spawn.find_executable("clang-format-8")
        if clang_format:
            print("Using %s instead of %s" % (clang_format, base))
        else:
            print("No %s found, skipping checks!" % base)

    print("%sprocessing directory: %s%s" % (TERM_PURPLE, src_dir, TERM_EMPTY))

    for dirpath, _, filenames in os.walk(src_dir):
        print("- walk: %s" % (relpath(dirpath)))

        if relpath(dirpath) in skip_dirs:
            print("- skip: %s" % (relpath(dirpath)))
            continue

        if relpath(dirpath, src_dir) in skip_dirs:
            print("- skip: %s" % (relpath(dirpath, src_dir)))
            continue

        skip = False
        for d in skip_dirs:
            if relpath(dirpath, src_dir).startswith(d):
                skip = True
                break

        if skip:
            continue

        for file in [
            join(dirpath, name) for name in filenames if is_checked_by_clang(name)
        ]:

            def report_error(msg, line=None):
                print(
                    "%s%s:%s %s%s"
                    % (TERM_YELLOW, file, "%d:" % line if line else "", msg, TERM_EMPTY)
                )
                stats.errors += 1

            with open(file, "r") as f:
                original = f.readlines()
            formatted = subprocess.check_output(
                [clang_format, "-style=file", file], encoding="utf-8"
            )

            if update:
                with open(file, "w") as f:
                    f.write(formatted)

            stats.files += 1
            stats.lines += len(original)

            for lineno, line in enumerate(original):
                lineno += 1

                if "\t" in line:
                    report_error("TAB character", lineno)
                if "\r" in line:
                    report_error("CR character", lineno)
                if line.endswith(" \n") or line.endswith("\t\n"):
                    report_error("trailing whitespace", lineno)
                if not line.endswith("\n"):
                    report_error("line ends without NEW LINE character", lineno)

                if not line.strip():
                    stats.empty_lines += 1

            diff = list(unified_diff(original, formatted.splitlines(True)))
            if diff:
                report_error("format error")
                for diffline in diff:
                    print(diffline, end="")


class Filters:
    def __init__(self):
        self.plus_dirs = []
        self.plus_files = []
        self.minus_dirs = []
        self.minus_files = []

    def GetPlusDirs(self):
        return self.plus_dirs[:]

    def GetPlusFiles(self):
        return self.plus_files[:]

    def GetMinusDirs(self):
        return self.minus_dirs[:]

    def GetMinusFiles(self):
        return self.minus_files[:]

    def AddFilters(self, filters):
        for one in filters.split(","):
            stripped = one.strip()
            if stripped:
                self._parse(one)

    def _isdir(self, name):
        return True if name.endswith("/") else False

    def _parse(self, filter):
        name = filter[1:]
        if filter.startswith("+"):
            if self._isdir(name):
                self.plus_dirs.append(name[0:-1])
            else:
                self.plus_files.append(name)
        elif filter.startswith("-"):
            if self._isdir(name):
                self.minus_dirs.append(name[0:-1])
            else:
                self.minus_files.append(name)
        else:
            raise ValueError(
                "Every filter must start with + or -" " (%s does not)" % one
            )


def main():
    parser = ArgumentParser(description="Source Format Checker and Updater")
    parser.add_argument(
        "--clang-format",
        metavar="PATH",
        default="clang-format-8",
        help="set the path to clang-format (default: %(default)s)",
    )
    parser.add_argument("--update", action="store_true", help="reformat files")
    parser.add_argument(
        "--dir",
        nargs="*",
        default=["."],
        dest="dir",
        help="set directories to process formatting (default: .)",
    )
    parser.add_argument(
        "--filter",
        action="append",
        default=[],
        dest="filters",
        help="specify a comma-separated list of file/directory filters. \
            can be used multiple times. every filter must start with + or -. \
            a directory must end with a slash, /. \
            example: --filter=-file.cc,+dir/sub_dir/",
    )
    args = parser.parse_args()

    stats = Stats()

    filters = Filters()
    for filts in args.filters:
        filters.AddFilters(filts)

    plus_dirs = filters.GetPlusDirs()
    plus_files = filters.GetPlusFiles()

    args.dir = plus_dirs if plus_dirs else args.dir
    if plus_files:
        print(
            "%swarn: setting files isn't yet supported.%s" % (TERM_YELLOW, TERM_EMPTY)
        )
        print(plus_files)

    global skip_dirs, skip_files
    skip_dirs += filters.GetMinusDirs()
    skip_files += filters.GetMinusFiles()

    for dir in args.dir:
        check_tidy(dir, args.update, args.clang_format, stats)

    print()
    print("* Total number of files: %d" % stats.files)
    print("* Total lines of code: %d" % stats.lines)
    print("* Total non-empty lines of code: %d" % (stats.lines - stats.empty_lines))
    print(
        "%s* Total number of errors: %d%s"
        % (TERM_RED if stats.errors else TERM_GREEN, stats.errors, TERM_EMPTY)
    )

    if args.update:
        print()
        print("All files reformatted, check for changes with `git diff`.")

    sys.exit(1 if stats.errors else 0)


if __name__ == "__main__":
    main()
