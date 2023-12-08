#!/bin/csh
setenv TBBROOT "/opt/el7/pkgs/crkit/nightly/20180703/crkit/intel-tbb/intel-tbb" #
setenv tbb_bin "/opt/el7/pkgs/crkit/nightly/20180703/crkit/intel-tbb/intel-tbb/build/linux_intel64_gcc_cc4.8.5_libc2.17_kernel3.10.0_release" #
if (! $?CPATH) then #
    setenv CPATH "${TBBROOT}/include" #
else #
    setenv CPATH "${TBBROOT}/include:$CPATH" #
endif #
if (! $?LIBRARY_PATH) then #
    setenv LIBRARY_PATH "${tbb_bin}" #
else #
    setenv LIBRARY_PATH "${tbb_bin}:$LIBRARY_PATH" #
endif #
if (! $?LD_LIBRARY_PATH) then #
    setenv LD_LIBRARY_PATH "${tbb_bin}" #
else #
    setenv LD_LIBRARY_PATH "${tbb_bin}:$LD_LIBRARY_PATH" #
endif #
 #
