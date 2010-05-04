#! /bin/sh

aclocal                          &&
autoconf                         &&
libtoolize --force --copy        &&
automake --add-missing --copy    &&
echo You may now run ./configure
