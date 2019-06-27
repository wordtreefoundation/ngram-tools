#!/bin/bash

cc -g -Wall -pedantic -Wextra -I./tkvdb stream-increment.c tkvdb/tkvdb.c -o stream-increment