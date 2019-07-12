#!/bin/bash

# Merges two or more tallied ngrams files. This operation
# expects each tallied ngrams file to be in sorted order, e.g.
#
#        5       a added providing for
#        1       a added providing that
#        1       a added provisions relative
#        6       a added regulating the
#        1       a added regulating workmens
#       18       a added relative to
#        7       a added requiring the
#        1       a added restricting the
#        1       a added to prevent
#        1       a added under heading
#        1       a adjacent to erithmo
#        2       a adopt for the
#
# Params:
# LC_ALL=C: use "C" language sort order (as opposed to locale)
# -t$'\t': use tabs as column separator (instead of space, which is default)
# --merge: tell sort to assume input is in sorted order and merge instead of sort
# --key=2,2: use the second column as the merge criterion, merge alphabetically

LC_ALL=C sort -t$'\t' --merge --key=2,2 "$@"
