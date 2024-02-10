#!/bin/bash
set -eu

cd shaders
ls | grep -v spv | xargs -I % glslangValidator -V % -o %.spv
