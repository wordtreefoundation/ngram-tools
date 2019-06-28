# Take a stream of counts and ngrams as input, and produce
# a stream of summed counts and ngrams as output

awk '{ c=$1; $1=""; sum[$0] += c } END { for (id in sum) { printf "%7d%s\n", sum[id], id } }'