#!/bin/bash
set -eu

if ! command -v scons &> /dev/null; then
	PATH="$PATH:/opt/homebrew/bin"
fi

scons $@
