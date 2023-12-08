#!/bin/bash
export TBBROOT="/opt/el7/pkgs/crkit/nightly/20180703/crkit/intel-tbb/intel-tbb" #
tbb_bin=/opt/el7/pkgs/crkit/nightly/20180703/crkit/intel-tbb/intel-tbb/build/inteltbb-release
if [ -z "$CPATH" ]; then #
    export CPATH="${TBBROOT}/include" #
else #
    export CPATH="${TBBROOT}/include:$CPATH" #
fi #
if [ -z "$LIBRARY_PATH" ]; then #
    export LIBRARY_PATH="${tbb_bin}" #
else #
    export LIBRARY_PATH="${tbb_bin}:$LIBRARY_PATH" #
fi #
if [ -z "$LD_LIBRARY_PATH" ]; then #
    export LD_LIBRARY_PATH="${tbb_bin}" #
else #
    export LD_LIBRARY_PATH="${tbb_bin}:$LD_LIBRARY_PATH" #
fi #
 #
