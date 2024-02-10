#!/bin/bash
set -eu

cd shaders
# ls | grep -v spv | xargs -I % glslangValidator -V % -o %.spv
find . -type f -maxdepth 1 -not -name "*.spv" | xargs -I % glslc -std=450 --target-env=vulkan -o %.spv %
