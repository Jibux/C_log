#!/bin/bash


make clean || true

for pattern in $(grep -v '^#' .gitignore | grep -v '/'); do
	find . -name "$pattern" -print0 | xargs -0 rm -rf
done

