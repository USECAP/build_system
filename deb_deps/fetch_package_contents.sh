#!/usr/bin/env bash

for outfile in "$@"; do
    distro=$(basename $outfile)
    curl http://archive.ubuntu.com/ubuntu/dists/${distro}/Contents-amd64.gz | gunzip > "$outfile"
done

