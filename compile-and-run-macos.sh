#!/bin/bash

set -eux

scons -Q && DYLD_LIBRARY_PATH=/opt/homebrew/lib ./bin/vulkun
