#! /bin/bash
set -ux

CodeChecker analyze -o results build/compile_commands.json --config .codechecker.json

CodeChecker parse --export html --output ./reports_html ./results

CodeChecker parse  ./results

case $? in
    1) echo Parse failed ;;
    2) exit 0 ;;
esac
