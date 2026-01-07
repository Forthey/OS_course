#!/usr/bin/env bash
set -euo pipefail

install_ncurses() {
    if pkg-config --exists ncursesw 2>/dev/null || pkg-config --exists ncurses 2>/dev/null; then
        echo "ncurses already installed"
        return
    fi

    echo "ncurses not found, trying to install..."

    if command -v apt-get >/dev/null 2>&1; then
        # Debian/Ubuntu
        sudo apt-get update
        sudo apt-get install -y libncurses-dev
    elif command -v dnf >/dev/null 2>&1; then
        # Fedora
        sudo dnf install -y ncurses-devel
    elif command -v yum >/dev/null 2>&1; then
        # RHEL/CentOS
        sudo yum install -y ncurses-devel
    elif command -v pacman >/dev/null 2>&1; then
        # Arch
        sudo pacman -Sy --noconfirm ncurses
    else
        echo "Unknown package manager. Please install ncurses dev package manually." >&2
        exit 1
    fi
}

echo "Looking for ncurses dev package..."

install_ncurses

threads=$(( $(nproc) - 2 ))
(( threads < 4 )) && threads=4

echo "Building project..."

cmake -S . -B build
cmake --build build/ --parallel "$threads"

echo "Moving executables..."

mkdir -p bin

mv build/bin/host bin/
mv build/bin/client_* bin/

echo "Removing build artifacts..."

rm -rf build
