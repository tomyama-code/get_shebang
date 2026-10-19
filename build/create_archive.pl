#!/usr/bin/env perl
################################################################################
## - $Revision: 1.8 $
################################################################################

use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use File::Basename qw();            # first released with perl 5
use File::Path qw();                # first released with perl 5.001
use Cwd qw();                       # first released with perl 5
use Getopt::Long qw();              # first released with perl 5


##########
## スクリプトのエントリポイント
sub pl_main( @ )
{
    ## 初期化処理
    init_script();

    chdir( "$main::apppath/../.." );

    ## 引数解析
    parse_arg( @_ );

    param_print();

    my @file_list = (
        'get_shebang/.clang-format',
        'get_shebang/.gitignore',
        'get_shebang/build/build_msvc.bat',
        'get_shebang/build/build_unix.sh',
        'get_shebang/build/CMakeLists.txt',
        'get_shebang/build/collect_file.sh',
        'get_shebang/build/create_arch_dir.pl',
        'get_shebang/build/create_archive.pl',
        'get_shebang/ChangeLog.md',
        'get_shebang/CMakeLists.txt',
        'get_shebang/config.h.in',
        'get_shebang/Doxyfile',
        'get_shebang/include/get_shebang.h',
        'get_shebang/LICENSE',
        'get_shebang/README.md',
        'get_shebang/src/CMakeLists.txt',
        'get_shebang/src/get_shebang.c',
        'get_shebang/src/main.c',
        'get_shebang/test/CMakeLists.txt',
        'get_shebang/test/testcase_shebang_function.c',
        'get_shebang/test/testfiles/broken_shebang',
        'get_shebang/test/testfiles/broken_shebang_2',
        'get_shebang/test/testfiles/env_go_script',
        'get_shebang/test/testfiles/env_java_11',
        'get_shebang/test/testfiles/env_java_25',
        'get_shebang/test/testfiles/env_perl_pl',
        'get_shebang/test/testfiles/env_python2_py',
        'get_shebang/test/testfiles/env_python3_py',
        'get_shebang/test/testfiles/env_python_py',
        'get_shebang/test/testfiles/perl_script_bom_pl',
        'get_shebang/test/testfiles/perl_script_crlf_pl',
        'get_shebang/test/testfiles/perl_script_cr_pl',
        'get_shebang/test/testfiles/perl_script_pl',
        'get_shebang/test/testfiles/perl_script_utf16be_bom_pl',
        'get_shebang/test/testfiles/perl_script_utf16le_bom_pl',
        'get_shebang/test/testfiles/perl_script_utf32be_bom_pl',
        'get_shebang/test/testfiles/perl_script_utf32le_bom_pl',
        'get_shebang/test/testfiles/shell_script_bash',
        'get_shebang/test/testfiles/shell_script_sh',
        'get_shebang/test/testfiles/text_txt',
        'get_shebang/test/tmym_common.c',
        'get_shebang/test/tmym_common.h',
        'get_shebang/test/unit_test.c',
        'get_shebang/test/unit_test.h',
        'get_shebang/test/unit_test_user_define.c',
        'get_shebang/test/unit_test_user_define.h',
    );

    chdir( "$main::apppath/.." );

    open( CMAKELISTS, '<', 'CMakeLists.txt' ) ||
        die( qq{'CMakeLists.txt': could not open file: $!} );
    my $proj_name = '';
    my $proj_ver  = '';
    while( <CMAKELISTS> ){
        my $line = $_;
        $line =~ s/\r?\n$//go;
        #print( qq{L: $line\n} );

        if( $line =~ m/project\s*\(\s*([^\s][^\)]+)\s*\)/o ){
            #print( qq{"$1"\n} );
            ## "get_shebang VERSION 0.0.1 LANGUAGES C"
            my( $project_name, @fields ) = split( /\s+/, $1 );
            $proj_name = $project_name;
            my $version_found = 0;
            for my $f( @fields ){
                #print( qq{F: $f\n} );
                if( $version_found ){
                    $version_found = 0;
                    $proj_ver = $f;
                    last;
                }elsif( lc( $f ) eq 'version' ){
                    $version_found = 1;
                    next;
                }
            }
            last;
        }
    }
    close( CMAKELISTS );
    my $archive_file_name = "${proj_name}";
    $archive_file_name .= "-$proj_ver" if( $proj_ver ne '' );
    $archive_file_name .= ".tar.gz";
    print( qq{\$archive_file_name="$archive_file_name"\n} );

    if( ! -d 'archive' ){
        mkdir( 'archive' );
    }

    my $archive_file_path = Cwd::abs_path(
        "$main::apppath/../archive/$archive_file_name" );

    chdir( "$main::apppath/../.." );

    #my $dum_build_stdout = '';
    #my $exit_status = exec_cmd( \$dum_build_stdout, 'tar',
    my $exit_status = exec_cmd( 'tar',
        'czf', "$archive_file_path", @file_list );

    return $exit_status;
}

##########
## 初期化処理
sub init_script()
{
    my $exec_file = $0;
    $exec_file =~ s!^.*tests/(.*)\.t$!$1!;

    ### GLOBAL ###
    $main::apppath = Cwd::abs_path( File::Basename::dirname( $exec_file ) );
    $main::appname = File::Basename::basename( $exec_file );
    $main::argc = 0;
    @main::argv = ();
    $main::debug = 0;
    ##############
}
sub param_print()
{
    dPrintf( qq{Parameter Print\n} .
             qq{     \$main::apppath = "%s"\n} .
             qq{     \$main::appname = "%s"\n} .
             qq{     \$main::argc = %d\n} .
             qq{     \@main::argv = (%s)\n} .
             qq{     \$main::debug = %d\n},
        $main::apppath,
        $main::appname,
        $main::argc,
        array_to_str( ', ', @main::argv ),
        $main::debug );
}

sub array_to_str( $@ )
{
    my( $delim, @array ) = @_;
    my $array_len = scalar( @array );
    for( my $idx=0; $idx<$array_len; $idx++ ){
        $array[ $idx ] = qq{'$array[ $idx ]'};
    }
    return '' if( $array_len == 0 );
    return ' ' . join( $delim, @array ) . ' ';
}

##########
## 引数解析
sub parse_arg( @ )
{
    @main::argv = @_;
    $main::argc = scalar( @main::argv );

    # Getopt::Long が動くように局所的に変更して騙す
    local @ARGV = @main::argv;

    # 設定: ショートオプションのまとめ指定（バンドリング）を有効化
    Getopt::Long::Configure("bundling");

    Getopt::Long::GetOptions(
        'debug|d'         => \$main::debug,
    ) || die( qq{parse_arg(): error: Failed to parse option switches.\n} );
}

#sub dPrint( @ )
#{
#    if( $main::debug ){
#        print( 'dbg: ', @_ );
#    }
#}

sub dPrintf( @ )
{
    if( $main::debug ){
        print( 'dbg: ' );
        printf( @_ );
    }
}

sub exec_cmd( @ )
{
    my @args = @_;
    my $cmd = shift( @args );
    my $ref_stdout = undef;
    if( ref( $cmd ) ){
        $ref_stdout = $cmd;
        $cmd = shift( @args );
    }

    my $disp_cmd = qq{\n\e[34m\$ \e[0m$cmd} . array_to_str( ' ', @args );
    print( qq{$disp_cmd\n} );

#    my $raw_status = system( $cmd, @args );
    open( EXEC_CMD, '-|', $cmd, @args ) ||
        die( qq{"$cmd": could not execute: $!} );
    $$ref_stdout = '' if( defined( $ref_stdout ) );
    while( <EXEC_CMD> ){
        my $line = $_;
        $line =~ s/\r?\n$//o;
        $$ref_stdout .= qq{$line\n} if( defined( $ref_stdout ) );
        print( qq{$line\n} );
    }
    close( EXEC_CMD );
    my $raw_status = $?;
    my $exit_status = $raw_status >> 8;
    return $exit_status;
}

# uncoverable branch false
exit( pl_main( @ARGV ) ) unless caller();
