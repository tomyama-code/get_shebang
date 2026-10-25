#!/bin/sh
################################################################################
## - $Revision: 1.13 $
################################################################################

## Revision: 1.1
sh_exec()
{
    b_count_up=0
    if [ "$SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS" = '' ]; then
        b_count_up=0
        SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS=1
        export SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS
    else
        b_count_up=1
        SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS=`expr "$SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS" '+' '1'`
        export SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS
    fi
    my_echo "\n[34m${SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS}-$ [0m$@"
    "$@"
    exit_status=$?
    echo "$SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS: exit_status=$exit_status"
    if [ $b_count_up -eq 0 ]; then
        unset SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS
    else
        SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS=`expr "$SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS" '-' '1'`
        export SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS
    fi
    if [ $exit_status -ne 0 ]; then
        exit 1
    fi
    return $exit_status
}

## Support for environments where the -e option of the echo command does not work.
my_echo()
{
    #perl -e 'print( @ARGV )' "$@"

    perl -e '
        my %escapes = (
            q{n} => "\n",   # 改行
            q{t} => "\t",   # 水平タブ
            q{r} => "\\r",   # 復帰
            q{f} => "\\f",   # 改ページ
            q{a} => "\\a",   # 警報音
            q{e} => "\\e",   # ESC
        );

        my @args = @ARGV;

        my $argc = scalar( @args );
        if( ( $argc > 0 ) && ( $args[ 0 ] eq q{-e} ) ){
            $argc--;
            shift( @args );
        }else{
            $args[ $argc - 1 ] .= "\n";
        }
        for( my $idx=0; $idx<$argc; $idx++ ){
            #print( qq{\$args[ $idx ] = "$args[ $idx ]"\n} );
        #    $args[ $idx ] =~ s/\\n/\n/go;
        #    $args[ $idx ] =~ s/\\a/\a/go;
            $args[ $idx ] =~ s/\\([ntrfae])/$escapes{ $1 }/go;
            #print( qq{\$args[ $idx ] = "$args[ $idx ]"\n} );
        }
        print( join( q{ }, @args ) );
    ' "$@"
}

errp()
{
    echo "$@" 1>&2
}

create_lcov_epilog_file()
{
    lcov_epilog_file="$1"
    test_env="$2"
cat <<__EOD__ >"$lcov_epilog_file"
  <p>Test environment: $test_env</p>
  <p><a href="../index.html">Up</a></p>

</body>
</html>
__EOD__
}

summarize_test_results()
{
    ../utCounter.pl "$1" "$2" | tee "$3"
}

lcov_gen_html()
{
    sh_exec create_lcov_epilog_file "lcovEpilog-file" "$workdirname"
    sh_exec genhtml --html-epilog "lcovEpilog-file" "$proj_name_ver.info" --output-directory "cov"
    sh_exec summarize_test_results $proj_name_ver.info unittest_Result.tab unittest_Result.txt
}

execution_dir="`pwd`"
builddir="`dirname \"$execution_dir\"`"
if [ "`basename \"$builddir\"`" != 'build' ]; then
    errp 'error: It must be executed from within "build/*".'
    errp ''
    errp 'If the "build/*" directory does not exist, please first run "perl create_arch_dir.pl".'
    errp ' or'
    errp 'The directory name can be anything, so you can also use the following command: "mkdir aaa && cd aaa".'
    exit 1
fi
workdir="$execution_dir"
projdir="`dirname \"$builddir\"`"
workdirname="`basename \"$workdir\"`"

has_pending_changes=0
if [ -d "$projdir/.git" ]; then
    git diff --exit-code "$projdir" >/dev/null
    has_pending_changes=$?
    if [ $has_pending_changes -ne 0 ]; then
        echo "Modified from the repository !"
    fi
else
    echo "Not a Git clone directory !"
fi

if [ "$TERMUX_VERSION" != '' ]; then
    cmake_opts='-DCMAKE_INSTALL_PREFIX=/data/data/com.termux/files/usr/local'
fi

sh_exec cmake $cmake_opts ../..
sh_exec ../parse_config_h.pl --input config.h --output config.ini
sh_exec . ./config.ini
proj_name_ver="${PROJECT_NAME}-${PROJECT_VERSION}"
sh_exec make
sh_exec make test
if [ -f "./src/CMakeFiles/get_shebang_ut_obj.dir/get_shebang.c.gcda" ]; then
    sh_exec lcov --capture --directory ./src/CMakeFiles/get_shebang_ut_obj.dir --output-file $proj_name_ver.info --test-name "$proj_name_ver"
    # cov ディレクトリにHTMLレポートを出力
    lcov_gen_html
elif [ -f "./default.profraw" ]; then
    sh_exec llvm-profdata merge -sparse default.profraw -o default.profdata
    sh_exec llvm-cov report ./test/get_shebang_test -instr-profile=default.profdata
    # cov_llvm ディレクトリにHTMLレポートを出力
    sh_exec llvm-cov show ./test/get_shebang_test -instr-profile=default.profdata -format=html -output-dir=cov_llvm -show-branches=count

    which genhtml >/dev/null 2>&1
    genhtml_missing=$?
    if [ $genhtml_missing -eq 0 ]; then
        # lcov が理解できる「coverage.info」ファイルに変換
        llvm-cov export ./test/get_shebang_test -instr-profile=default.profdata -format=lcov > $proj_name_ver.info
        # cov ディレクトリにHTMLレポートを出力
        lcov_gen_html
    fi
fi
sh_exec ../create_archive.pl
workdir_archive="$builddir/$workdirname.tar.gz"
#echo "\$workdir_archive=\"$workdir_archive\""
sh_exec cd ..
sh_exec tar czf "$workdir_archive" "$workdirname/"
