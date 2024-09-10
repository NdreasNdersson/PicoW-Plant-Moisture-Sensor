#! /bin/bash
set -eux

CodeChecker analyze -o results build/compile_commands.json --enable sensitive -i skip.list

CodeChecker parse --export html --output ./reports_html ./results
