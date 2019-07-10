#!/bin/bash

awk '{ n=$1; $1=""; printf "%9d\t%s\n", n, substr($0, 2) }'
