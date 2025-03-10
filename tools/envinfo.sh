#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
[ -f "$SCRIPT_DIR/logo.sh" ] && source "$SCRIPT_DIR/logo.sh"

print_version() {
  local cmd=$1
  local prefix=$2
  if command -v $cmd &> /dev/null; then
    if [ -z "$prefix" ]; then
      echo -e "$($cmd --version)\n"
    else
      echo -e "$prefix: $($cmd --version)\n"
    fi
  else
    echo -e "$cmd not found\n"
  fi
}

print_env_var() {
  local var_name=$1
  local var_value=$2
  local delimiter=$3

  if [[ -n $var_value ]]; then
    echo "envvar($var_name):"
    if [[ -n $delimiter ]]; then
      echo "$var_value" | tr "$delimiter" '\n'
    else
      echo "$var_value"
    fi
    echo
  fi
}

print_version cmake
print_version gcc
print_version make
print_version ninja ninja
print_version node node
print_version npm npm
print_version python python
print_version clang
print_version clang-format
print_version valgrind

echo "nproc: $(getconf _NPROCESSORS_ONLN)" && echo
print_env_var "RUNNER_NAME" "$RUNNER_NAME"
print_env_var "CC" "$CC"
print_env_var "CXX" "$CXX"
print_env_var "CXXFLAGS" "$CXXFLAGS" ' '
