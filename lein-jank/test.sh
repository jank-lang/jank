#!/bin/bash
set -xe
lein install
cd test-project
lein jank run 1 | grep 'lein jank works!'
