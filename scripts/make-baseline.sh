#!/bin/bash

LIBRARY=${1:-../library}
SUFFIX=${2:-.tallied}
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -d "${LIBRARY}" ]; then
    echo "Library location '${LIBRARY}' not found."
    exit 1
fi

trap ctrl_c INT

function ctrl_c() {
    echo "ctrl-c"
    exit 1
}


ROUND=1
while true; do
    echo "Round ${ROUND}:"

    OUTDIR=$(printf "r%03d" $ROUND)
    mkdir -p "$OUTDIR"

    TOC_PREV="$TOC_FILE"
    TOC_FILE="$(mktemp)"
    
    mkdir -p "$LIBRARY"
    find "$LIBRARY" -name "*$SUFFIX" \
        | shuf \
        > "${TOC_FILE}"
    
    LINES=`wc "$TOC_FILE" | awk '{print $1}'`
    echo "  Reading ${LIBRARY} ($LINES files)..."

    if (( "$LINES" <= "1" )); then
        break
    fi

    echo "  Writing to $OUTDIR..."

    time parallel -a "$TOC_FILE" \
        --progress \
        -j$(nproc) \
        --xargs \
        $DIR/merge.sh {} \
        '|' $DIR/sum-consecutive.sh \
        '>' $OUTDIR/{#}$SUFFIX

    # Clean up directories as we go, because data can get Very Big
    # if (( "$ROUND" > "1" )); then
    #     if [ -d "$LIBRARY_PREV" ]; then
    #         if [[ "$LIBRARY_PREV" == round* ]]; then
    #             rm -rf "$OUTDIR_PREV"
    #         fi
    #     fi
    # fi

    LIBRARY_PREV="$LIBRARY"
    LIBRARY="$OUTDIR"
    ((ROUND++))
    echo
done