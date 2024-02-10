#!/bin/bash
set -eu

./compile-shaders.sh && ./bin/vulkun.exe
