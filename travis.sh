#/usr/bin/env bash

set -eu

lein uberjar

lein test
