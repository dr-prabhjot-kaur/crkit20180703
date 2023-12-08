#!/bin/sh
#
# Copyright 2009 Children's Hospital Boston
#
# crlViz startup command for linux.
# http://www.crl.med.harvard.edu
#

BUNDLE="`dirname "$0" `"/..

#BUNDLE=/opt/el7/build/crkit/nightly/20180703/crkit_build/crkit/bingcc

export PATH=$BUNDLE/MacOS:$BUNDLE/bin:$PATH

export LD_LIBRARY_PATH=$BUNDLE/lib:$BUNDLE/bin:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$BUNDLE/Frameworks/InsightToolkit:$BUNDLE/Frameworks/vtk-6.3:$BUNDLE/Frameworks/qt-5.6.2/lib:$LD_LIBRARY_PATH

exec crlViz
exit 0

