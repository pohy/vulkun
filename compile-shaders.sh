#!/bin/bash
set -eux

cd shaders
ls | grep -v spv | xargs -I % glslangValidator -V % -o %.spv
