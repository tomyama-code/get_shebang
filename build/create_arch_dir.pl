#!/usr/bin/env perl
################################################################################
## - $Revision: 1.11 $
################################################################################

use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Cwd qw();                       # first released with perl 5
use File::Basename qw();            # first released with perl 5
use File::Path qw();                # first released with perl 5.001
use File::Spec qw();                # first released with perl 5.00405
use Getopt::Long qw();              # first released with perl 5


##########
## スクリプトのエントリポイント
sub pl_main( @ )
{
    ## 初期化処理
    init_script();

    chdir( $main::apppath );

    ## 引数解析
    parse_arg( @_ );

    param_print();

    my $dum_dir = 'dummy_build';
    if( -d $dum_dir ){
        File::Path::rmtree( $dum_dir );
        print( qq{'$dum_dir': Delete the existing directory.\n} );
    }
    mkdir( $dum_dir );

    my $dum_build_stdout = '';
    exec_cmd( \$dum_build_stdout,
              'cmake',
              '-S', '.',
              '-B', "$dum_dir" );
    #print( qq{stdout="$dum_build_stdout"\n} );

    my %host_info = ();
    for my $line( split( /\n/, $dum_build_stdout ) ){
        $line =~ s/\r?\n$//o;
        #print( qq{$line\n} );
        if( $line =~ m/^-- (.+)='(.*)'$/o ){
            print( qq{"$1" is "$2"\n} );
            $host_info{$1} = $2;
        }
    }

    my $osname = $host_info{OS_NAME};
    $osname = $host_info{DISTRIB_PRETTY_NAME} if( $host_info{DISTRIB_PRETTY_NAME} ne "" );
    my $target_arch = '';
    $target_arch = "_x86" if( defined( $ENV{VSCMD_ARG_TGT_ARCH} ) && $ENV{VSCMD_ARG_TGT_ARCH} eq 'x86' );
    my $arch_dir = "$host_info{OS_PLATFORM}_${osname}_$host_info{HOSTNAME}_$host_info{COMPILER_ID}$target_arch";
    #print( qq{D: "$arch_dir"\n} );
    # D: "AMD64_Windows_DESKTOP-I0VU1AC_MSVC"
    # D: "AMD64_Windows_DESKTOP-I0VU1AC_MSVC_x86"
    # D: "x86_64_CYGWIN_NT-10.0-19045_DESKTOP-I0VU1AC_GNU"
    # D: "x86_64_CentOS_Stream_9_pug_GNU"
    # D: "amd64_FreeBSD_15.1-RELEASE-p3_bull_Clang"
    # D: "armv7l_Linux_localhost_Clang"

    if( ! -d $arch_dir ){
        mkdir( $arch_dir );
        print( qq{"$arch_dir": Create a directory.\n} );
    }else{
        print( qq{"$arch_dir": The directory already existed.\n} );
    }

    chdir( $main::execution_dir );

    print( qq{\n* * * Subsequent steps * * *\n} );
    printf( qq{cd "%s"\n}, File::Spec->abs2rel( "$main::apppath/$arch_dir" ) );
    print( qq{[unix]\n../build_unix.sh\n} );
    print( qq{[windows]\n..\\build_msvc.bat\n} );

    return 0;
}

##########
## 初期化処理
sub init_script()
{
    my $exec_file = $0;
    $exec_file =~ s!^.*tests/(.*)\.t$!$1!;

    ### GLOBAL ###
    $main::execution_dir = Cwd::getcwd();
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
             qq{     \$main::execution_dir = "%s"\n} .
             qq{     \$main::apppath = "%s"\n} .
             qq{     \$main::appname = "%s"\n} .
             qq{     \$main::argc = %d\n} .
             qq{     \@main::argv = (%s)\n} .
             qq{     \$main::debug = %d\n},
        $main::execution_dir,
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

    my $disp_cmd = qq{\n\e[34m-\$ \e[0m$cmd} . array_to_str( ' ', @args );
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
