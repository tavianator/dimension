#!/bin/sh

set -e

aclocal
autoconf
libtoolize -q --force --copy
automake --add-missing --copy
echo You may now run ./configure
