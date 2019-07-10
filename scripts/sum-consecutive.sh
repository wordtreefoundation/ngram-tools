#!/bin/bash

awk -F $'\t' 'NF>1 && $2!=p{ if (NR>1) printf "%9d\t%s\n", s, p; p=$2; s=0} {s+=$1} END{printf "%9d\t%s\n", s, p}'
