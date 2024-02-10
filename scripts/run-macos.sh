#!/bin/bash

set -eu

./scripts/compile-shaders.sh && DYLD_LIBRARY_PATH=/opt/homebrew/lib ./bin/vulkun
