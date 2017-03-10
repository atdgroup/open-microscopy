# only needed if you change Makefile.am or configure.in

aclocal -I autoconfmacros
autoheader
automake --add-missing --copy --foreign
autoconf
