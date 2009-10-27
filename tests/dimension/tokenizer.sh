#!/bin/sh

braces=$(${top_builddir}/dimension/dimension --tokenize ${srcdir}/braces.pov)
braces_exp='({ { } } } } { {)'

if [ "$braces" != "$braces_exp" ]; then
  echo "braces.pov tokenized as \"$braces\"" >&2
  echo " -- expected \"$braces_exp\"" >&2
  exit 1;
fi
