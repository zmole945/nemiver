#!/bin/sh

PROJECT=nemiver
PKG_NAME=$PROJECT

topsrcdir=`dirname $0`
if test x$topsrcdir = x ; then
	topsrcdir=.
fi

(test -f $topsrcdir/configure.ac \
  && test -f $topsrcdir/README \
  && test -d $topsrcdir/src) || {
    echo -n "**Error**: Directory "\`$topsrcdir\'" does not look like the"
    echo " top-level $PROJECT directory"
    exit 1
}

which gnome-autogen.sh || {
    echo "You need to install gnome-common from the GNOME CVS"
    exit 1
}

srcdir=$topsrcdir . gnome-autogen.sh
