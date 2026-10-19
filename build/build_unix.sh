#!/bin/sh
################################################################################
## - $Revision: 1.2 $
################################################################################

sh_exec()
{
    echo -e "\n[34m$ [0m$@"
    "$@"
    exit_status=$?
    echo "exit_status=$exit_status"
    if [ $exit_status -ne 0 ]; then
        exit 1
    fi
    return $exit_status
}

if [ "$TERMUX_VERSION" != '' ]; then
    cmake_opts='-DCMAKE_INSTALL_PREFIX=/data/data/com.termux/files/usr/local'
fi

sh_exec cmake $cmake_opts ../..
sh_exec make
sh_exec make test
if [ -f "./src/CMakeFiles/get_shebang_ut_obj.dir/get_shebang.c.gcda" ]; then
    sh_exec lcov --capture --directory ./src/CMakeFiles/get_shebang_ut_obj.dir --output-file coverage.info
    sh_exec genhtml coverage.info --output-directory cov
fi
sh_exec ../create_archive.pl
