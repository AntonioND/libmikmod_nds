#!/bin/sh

# sudo apt-get install texinfo

makeinfo --no-split --html -o mikmod.html mikmod.texi
makeinfo --no-split -o mikmod.info mikmod.texi
