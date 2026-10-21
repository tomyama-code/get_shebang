#!/usr/bin/env perl
################################################################################
## - $Revision: 1.11 $
################################################################################

use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Cwd qw();                       # first released with perl 5
use File::Basename qw();            # first released with perl 5
use File::Path qw();                # first released with perl 5.001
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

    chdir( "$main::apppath/.." );

    open( CMAKELISTS, '<', 'CMakeLists.txt' ) ||
        die( qq{'CMakeLists.txt': could not open file: $!} );
    my $proj_name = '';
    my $proj_ver  = '';
    while( my $line = <CMAKELISTS> ){
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

    my @file_list = ();
    open( FILELILST, '<', 'FileList.txt' ) ||
        die( qq{'FileList.txt': could not open file: $!} );
    my $nr = 0;
    while( my $line = <FILELILST> ){
        $nr++;
        $line =~ s/\r?\n$//go;
        #print( qq{L: $line\n} );

        $line =~ s/#.*//o;          # コメント（#以降）を削除
        $line =~ s/^\s+|\s+$//go;   # 前後の空白を削除
        next if( $line eq '' );     # 空行はスキップ

        if( ! -f $line ){
            die( qq{error: FileList.txt: $nr: $line: File not found.\n} );
        }

        push( @file_list, "$proj_name/$line" );
    }
    close( FILELILST );

    if( ! -d 'archive' ){
        mkdir( 'archive' );
    }

    # Cwd::abs_path() は存在するパスしか最適化できない
    my $archive_path = Cwd::abs_path( "$main::apppath/../archive" );
    my $archive_file_path = "$archive_path/$archive_file_name";

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
