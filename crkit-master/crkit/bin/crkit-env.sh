
#
# Copyright 2009 Children's Hospital Boston
#
# crlViz environment variable settings, for Mac OS X and linux.
# http://www.crl.med.harvard.edu
#
# source this file from ~/.bashrc or ~/.profile:
# source /path/to/location/crkit-env.sh
#

# BASH_SOURCE tells us where this file is located.
# Modify to hold explicit path for use with other shells.

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

BUNDLE=${DIR}/..

export PATH=${BUNDLE}/bin:/usr/bin:/bin

export LD_LIBRARY_PATH=${BUNDLE}/Frameworks/InsightToolkit:${BUNDLE}/Frameworks/vtk-6.3:${BUNDLE}/Frameworks/qt-5.6.2/lib:${BUNDLE}/lib:${BUNDLE}/bin

# Export QT Plugin path relative to the binary files.
export QT_PLUGIN_PATH=${BUNDLE}/Frameworks/qt-5.6.2/plugins

# Ensure the DYLD_LIBRARY_PATH is not set.
export DYLD_LIBRARY_PATH=""

#export DYLD_PRINT_LIBRARIES=1

if [ -d ${BUNDLE}/intel-tbb ]; then
	source ${BUNDLE}/intel-tbb/intel-tbb/build/inteltbb-release/tbbvars.sh
fi
