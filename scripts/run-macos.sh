#!/bin/bash

set -eu

./compile-shaders.sh && DYLD_LIBRARY_PATH=/opt/homebrew/lib ./bin/vulkun
