#! /bin/bash
set -eux

CodeChecker analyze -o results build/compile_commands.json --config .codechecker.json

CodeChecker parse --export html --output ./reports_html ./results
