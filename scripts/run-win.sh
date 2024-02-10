#!/bin/bash
set -eu

./scripts/compile-shaders.sh && ./bin/vulkun.exe
