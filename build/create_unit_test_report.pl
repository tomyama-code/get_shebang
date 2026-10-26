#!/usr/bin/env perl
################################################################################
## TEST_RESULTS_FOR_EACH_ENV.PL -- Merging unit test results and lcov reports.
##
## - Created based on r1.10 of 'genIndexHtml.pl'
##
## - Author: tomyama   2026
## - Version: 1 ( $Revision: 1.5 $ )
## - Only for personal use !
################################################################################


use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Class::Struct;                  # first released with perl 5.004
use Cwd qw();                       # first released with perl 5
use File::Basename qw();            # first released with perl 5
use File::Path qw();                # first released with perl 5.001
use Getopt::Long qw();              # first released with perl 5
use Time::Piece;                    # first released with perl v5.9.5


my $outputFile = "cov/index.html";

struct DevDir => {
    ProjName  => '$',
    Envs      => '@',
};

struct EnvEntry => {
    EnvName      => '$',
    Date         => '$',
    UTestTotal   => '$',
    UTestPassed  => '$',
    UTestRate    => '$',
    LineCovTotal => '$',
    LineCovHit   => '$',
    LineCov      => '$',
    FuncCovTotal => '$',
    FuncCovHit   => '$',
    FuncCov      => '$',
};

my $devDirTop;

my $envInfo;
my %lcovConf;


sub pl_main( @ )
{
    init_script();      # 初期化処理
    parse_arg( @_ );    # 引数解析
    collect_project_name_and_version();
    param_print();
    chdir( "$main::apppath/.." );

    my $cov_dir = 'cov';
    if( -d $cov_dir ){
        File::Path::rmtree( $cov_dir );
        print( qq{'$cov_dir': Delete the existing directory.\n} );
    }
    mkdir( $cov_dir );
    print( qq{'$cov_dir': Create directory.\n} );

    my $devDirTop = Cwd::getcwd();
    #printf( qq{\$devDirTop = "$devDirTop"\n} );
    my $projName = File::Basename::basename( $devDirTop );
    #printf( qq{\$projName = "$projName"\n} );

    %lcovConf = parseRcFile( "/etc/lcovrc", \%lcovConf );
    %lcovConf = parseRcFile( "$ENV{'HOME'}/.lcovrc", \%lcovConf );
    %lcovConf = parseRcFile( "./.lcovrc", \%lcovConf );
    #for my $k( sort( keys( %lcovConf ) ) ){
    #    printf( qq{check: %s = "%s"\n}, $k, $lcovConf{ $k } );
    #}

    $envInfo = DevDir->new();
    $envInfo->ProjName( $projName );
    #printf( qq{ProjName = "%s"\n}, $envInfo->ProjName );

    my @devDirs = ();
    my $build_dir = './build';
    opendir( BUILDDIR, $build_dir ) ||
        die( qq{opendir( "$build_dir" ): failure: $!} );
    while( my $file = readdir( BUILDDIR ) ){
        #printf( "F: $file\n" );
        if( ( $file eq '.' ) ||
            ( $file eq '..' ) ||
            ( $file eq 'dummy_build' ) ){
            next;
        }
        if( ! -d "$build_dir/$file" ){
            #print( "skip\n" );
            next;
        }
        my $envName = $file;
        #printf( qq{Env: $envName\n} );
        push( @devDirs, $envName );
    }
    closedir( BUILDDIR );

    ## 名前順
    for my $envName( sort( @devDirs ) ){
        #printf( qq{\$envName="$envName"\n} );

        my $srcdir = "$build_dir/$envName/cov";
        my $dstdir = "$devDirTop/$cov_dir/$envName";
        if( -d $srcdir ){
            my $exit_status = exec_cmd( 'cp', '-R', $srcdir, $dstdir );
            if( $exit_status != 0 ){
                die( qq{error: cp command was failure\n} );
            }
        }
        my %cmd_conf = (
            DISPLAY_PROMPT => 0,
            DISPLAY_OUTPUT => 0,
            DISPLAY_EXIT_STATUS => 0,
        );
        my $exit_status = exec_cmd( \%cmd_conf, 'cp', '-f',
            "$build_dir/$envName/unittest_Result.txt",
            "$devDirTop/$cov_dir/$envName.txt" );
        if( $exit_status != 0 ){
            die( qq{error: cp command was failure\n} );
        }

        my $idxLast = scalar( @{ $envInfo->Envs } );
        $envInfo->Envs( $idxLast, mkEnvEntry( $dstdir ) );
    }

    my $ret_status = genIndexHtml( $envInfo, $outputFile );

    exec_cmd( 'tar', 'czf', 'test_result.tar.gz', 'cov' );

    return $ret_status;
}

##########
## 初期化処理
sub init_script()
{
    my $exec_file = $0;
    $exec_file =~ s!^.*tests/(.*)\.t$!$1!;

    ### GLOBAL ###
    $main::execution_dir = Cwd::getcwd();
    $main::apppath = File::Basename::dirname( $0 );
    $main::appname = File::Basename::basename( $0 );
    #$main::debug = 0;
    $main::argc = 0;
    @main::argv = ();
    $main::PROJ_NAME = '';
    $main::PROJ_VERSION = '';
    my $t = localtime();    # use Time::Piece
    $main::GEN_DATE_AND_TIME = $t->strftime( '%Y-%m-%d %H:%M:%S %z' );
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
             qq{     \$main::PROJ_NAME = "%s"\n} .
             qq{     \$main::PROJ_VERSION = "%s"\n} .
             qq{     \$main::GEN_DATE_AND_TIME = "%s"\n} .
             qq{     \$main::debug = %d\n},
        $main::execution_dir,
        $main::apppath,
        $main::appname,
        $main::argc,
        array_to_str( ', ', @main::argv ),
        $main::PROJ_NAME,
        $main::PROJ_VERSION,
        $main::GEN_DATE_AND_TIME,
        $main::debug );
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
    Getopt::Long::Configure( "bundling" );

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

sub getModTime( $ )
{
    my $fi = $_[ 0 ];
    if( ! -f $fi ){
        printf STDERR ( qq{"$fi": file not found\n} );
        return -1;
    }

    return ( stat( $fi ) )[ 9 ];
}

sub mkEnvEntry( $ )
{
    my( $envDir ) = @_;

    my $envName = File::Basename::basename( $envDir );
    my $utResPath = "$envDir.txt";

    my $date = '';
    my $utRate = 0;
    my $utPassed = 0;
    my $utNums = 0;
    my $lineCovRate = 0;
    my $lineCovPassed = 0;
    my $lineCovNums = 0;
    my $funcCovRate = 0;
    my $funcCovPassed = 0;
    my $funcCovNums = 0;

    if( ! -f $utResPath ){
        die( qq{"$utResPath": File not found\n} );
    }

    open( UTRESULT, '<', $utResPath ) ||
        die( qq{$utResPath: Could not open file: $!} );
    my $bTable = 0;
    while( my $line = <UTRESULT> ){
        $line =~ s/\r?\n$//o;
        #print( qq{L: $line\n} );

        if( $bTable ){
            my( $_utRate, undef, $_utPassed, undef, $_utNums, undef,
                $_lineCovRate, undef, $_lineCovPassed, undef, $_lineCovNums, undef,
                $_funcCovRate, undef, $_funcCovPassed, undef, $_funcCovNums, undef,
                $_modName ) = split( /\s+/, $line );
            if( $_modName eq 'Total' ){
                $_utRate =~ s/%//o;
                $_lineCovRate =~ s/%//o;
                $_funcCovRate =~ s/%//o;

                $utRate   = $_utRate;
                $utPassed = $_utPassed;
                $utNums   = $_utNums;
                $lineCovRate   = $_lineCovRate;
                $lineCovPassed = $_lineCovPassed;
                $lineCovNums   = $_lineCovNums;
                $funcCovRate   = $_funcCovRate;
                $funcCovPassed = $_funcCovPassed;
                $funcCovNums   = $_funcCovNums;
            }
        }else{
            if( $line =~ m/^-------------------------/o ){
                $bTable = 1;
                next;
            }elsif( $line =~ m/^Date: (.*)$/o ){
                $date = $1;
            }
        }
    }
    close( UTRESULT );

    my $ee = EnvEntry->new();
    $ee->EnvName( $envName );
    $ee->Date( $date );
    $ee->UTestTotal( $utNums );
    $ee->UTestPassed( $utPassed );
    $ee->UTestRate( $utRate );
    $ee->LineCovTotal( $lineCovNums );
    $ee->LineCovHit( $lineCovPassed );
    $ee->LineCov( $lineCovRate );
    $ee->FuncCovTotal( $funcCovNums );
    $ee->FuncCovHit( $funcCovPassed );
    $ee->FuncCov( $funcCovRate );

    return $ee;
}

my $tmplIndexHtmlHeader = q{<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">

<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <title>%%<PROJ_NAME>%%</title>
  <link rel="stylesheet" type="text/css" href="%%<ResourceDir>%%/gcov.css">
  <style>
    .modern-button {
      display: inline-flex;
      align-items: center;
      gap: 10px;
      padding: 3px 3px;
      background: #ffffff;
      border: 2px solid currentColor;
      cursor: pointer;
      font-size: 16px;
      font-weight: bold;
      border-radius: 6px;
    }

    .modern-button svg {
      /* アイコンのサイズ調整 */
      width: 20px;
      height: 20px;
    }
  </style>
</head>

<body>

  <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr><td class="title">%%<PROJ_NAME>%% : %%<PROJ_VERSION>%% : Unit Test Report</td></tr>
    <tr><td class="ruler"><img src="%%<ResourceDir>%%/glass.png" width=3 height=3 alt=""></td></tr>
    <tr>
      <td width="100%">
        <table cellpadding=1 border=0>
          <tr>
            <td class="headerItem">Generation date and time:</td>
            <td class="headerValue">%%<GEN_DATE_AND_TIME>%%</td>
          </tr>
        </table>
      </td>
    </tr>
    <tr><td class="ruler"><img src="%%<ResourceDir>%%/glass.png" width=3 height=3 alt=""></td></tr>
    <tr>
      <table width="100%" cellpadding=1 cellspacing=1 border=0>

        <tr>
          <td width="44%"><br></td>
          <td width="16%"></td>
          <td width="8%"></td>
          <td width="8%"></td>
          <td width="8%"></td>
          <td width="8%"></td>
          <td width="8%"></td>
        </tr>

        <tr>
          <td class="tableHead">Test Environment</td>
          <td class="tableHead">Date</td>
          <td class="tableHead" colspan=2>Success Rate</td>
          <td class="tableHead" colspan=3>Line Coverage <span class="tableHeadSort"></td>
          <td class="tableHead" colspan=2>Functions</td>
        </tr>
};
my $tmplIndexHtmlEntry  = q{        <tr>
          <td class="coverFile">%%<EnvLink>%%</td>
          <td class="coverFile">%%<TestExecDate>%%</td>
          <td class="%%<classTestCoverPer>%%">%%<UTestRate>%%&nbsp;%</td>
          <td class="%%<classTestCoverNum>%%">%%<UTestPassed>%% / %%<UTestTotal>%%</td>
          <td class="coverBar" align="center">
            <table border=0 cellspacing=0 cellpadding=1><tr><td class="coverBarOutline"><img src="%%<ResourceDir>%%/%%<LineCovBarImg>%%" width=%%<BarWLineCov>%% height=10 alt="%%<LineCov>%%%"><img src="%%<ResourceDir>%%/snow.png" width=%%<BarWLineCovRemain>%% height=10 alt="%%<LineCov>%%%"></td></tr></table>
          </td>
          <td class="%%<classLineCoverPer>%%">%%<LineCov>%%&nbsp;%</td>
          <td class="%%<classLineCoverNum>%%">%%<LineCovHit>%% / %%<LineCovTotal>%%</td>
          <td class="%%<classFuncCoverPer>%%">%%<FuncCov>%%&nbsp;%</td>
          <td class="%%<classFuncCoverNum>%%">%%<FuncCovHit>%% / %%<FuncCovTotal>%%</td>
        </tr>
};
my $tmplIndexHtmlFooter = q{      </table>
    </tr>

  </table>

  <br>

  <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr><td class="ruler"><img src="%%<ResourceDir>%%/glass.png" width=3 height=3 alt=""></td></tr>
    <tr><td class="versionInfo">Generated by: %%<ProgName>%%</td></tr>
  </table>
  <br>
  <p>
    <a class="modern-button" href="..">
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
        <path d="M5 9l5-5l5 5" />
        <path d="M10 22l7 0" />
        <path d="M10 22V4" />
      </svg>
    </a>
    <a href="..">Up</a>
  </p>

</body>
</html>
};

sub parseRcFile( $\% )
{
    my $rcFile = $_[ 0 ];
    my $ref_config = $_[ 1 ];

    if( ! -f $rcFile ){
        return %{ $ref_config };
    }
    if( ! -r $rcFile ){
        printf STDERR ( qq{warn: $rcFile: could not read file.\n} );
        return %{ $ref_config };
    }
    #printf( "$rcFile: exists !\n" );

    open( RCFILE, "<$rcFile" ) || die( "$rcFile: could not open: $!" );
    my $nr = 0;
    while( <RCFILE> ){
        $nr++;
        my $line = $_;
        $line =~ s/\r?\n//o;

        $line =~ s/^\s+//o;
        $line =~ s/\s+$//o;
        $line =~ s/\s*#.*$//o;
        next if( "$line" eq "" );
        #printf( "L: %5d: %s\n", $nr, $line );

        if( $line =~ m!^([\S]+)\s*=\s*(.+)$!o ){
            $ref_config->{ $1 } = $2;
            #printf( qq{%s = "%s"\n}, $1, $2 );
        }
    }
    close( RCFILE );

    return %{ $ref_config };
}

sub htmlReplaceKeyword( $$$ )
{
    my $devInfo = $_[ 0 ];
    my $envIdx  = $_[ 1 ];
    my $tmplHtml = $_[ 2 ];

    my $curr_dir = Cwd::getcwd();
    #print( qq{\$curr_dir = "$curr_dir"\n} );

    $tmplHtml =~ s!%%<PROJ_NAME>%%!$main::PROJ_NAME!go;
    $tmplHtml =~ s!%%<PROJ_VERSION>%%!$main::PROJ_VERSION!go;
    $tmplHtml =~ s!%%<GEN_DATE_AND_TIME>%%!$main::GEN_DATE_AND_TIME!go;

    my $ResourceDir = '.';
    my $envs_num = scalar( @{ $devInfo->Envs } );
    #print( qq{\$envs_num = $envs_num\n} );
    for( my $idx=0; $idx<$envs_num; $idx++ ){
        my $env_name = $devInfo->Envs->[ $idx ]->EnvName;
        my $gcov_css = "build/$env_name/cov/gcov.css";
        #print( qq{\$gcov_css="$gcov_css"\n} );
        if( -f $gcov_css ){
            $ResourceDir = $env_name;
            last;
        }
    }
    #print( qq{\$ResourceDir="$ResourceDir"\n} );
    $tmplHtml =~ s!%%<ResourceDir>%%!$ResourceDir!go;

    my $ProgName = $main::appname;
    $tmplHtml =~ s!%%<ProgName>%%!$ProgName!go;

    if( $envIdx >= 0 ){
        my $EnvName = $devInfo->Envs->[ $envIdx ]->EnvName;
        $tmplHtml =~ s!%%<EnvName>%%!$EnvName!go;

        my $EnvLink = $EnvName;
        if( -f "build/$EnvName/cov/gcov.css" ){
            $EnvLink = qq{<a href="$EnvName/index.html">$EnvName</a>};
        }
        $tmplHtml =~ s!%%<EnvLink>%%!$EnvLink!go;

        my $TestExecDate = $devInfo->Envs->[ $envIdx ]->Date;
        $tmplHtml =~ s!%%<TestExecDate>%%!$TestExecDate!go;

        my $UTestTotal = $devInfo->Envs->[ $envIdx ]->UTestTotal;
        $tmplHtml =~ s!%%<UTestTotal>%%!$UTestTotal!go;

        my $UTestPassed = $devInfo->Envs->[ $envIdx ]->UTestPassed;
        $tmplHtml =~ s!%%<UTestPassed>%%!$UTestPassed!go;

        my $UTestRate = $devInfo->Envs->[ $envIdx ]->UTestRate;
        $tmplHtml =~ s!%%<UTestRate>%%!$UTestRate!go;

        my $LineCovTotal = $devInfo->Envs->[ $envIdx ]->LineCovTotal;
        $tmplHtml =~ s!%%<LineCovTotal>%%!$LineCovTotal!go;

        my $LineCovHit = $devInfo->Envs->[ $envIdx ]->LineCovHit;
        $tmplHtml =~ s!%%<LineCovHit>%%!$LineCovHit!go;

        my $LineCov = sprintf( "%.2f", $devInfo->Envs->[ $envIdx ]->LineCov );
        $tmplHtml =~ s!%%<LineCov>%%!$LineCov!go;

        my $BarWLineCov = int( $devInfo->Envs->[ $envIdx ]->LineCov );
        $tmplHtml =~ s!%%<BarWLineCov>%%!$BarWLineCov!go;
        my $BarWLineCovRemain = 100 - $BarWLineCov;
        $tmplHtml =~ s!%%<BarWLineCovRemain>%%!$BarWLineCovRemain!go;

        my $FuncCovTotal = $devInfo->Envs->[ $envIdx ]->FuncCovTotal;
        $tmplHtml =~ s!%%<FuncCovTotal>%%!$FuncCovTotal!go;

        my $FuncCovHit = $devInfo->Envs->[ $envIdx ]->FuncCovHit;
        $tmplHtml =~ s!%%<FuncCovHit>%%!$FuncCovHit!go;

        my $FuncCov = sprintf( "%.2f", $devInfo->Envs->[ $envIdx ]->FuncCov );
        $tmplHtml =~ s!%%<FuncCov>%%!$FuncCov!go;

        ## genhtmlに倣って、カバレッジ網羅率によって、グラフの色、文字修飾を変更する
        my $hi_limit  = ( defined( $lcovConf{ 'genhtml_hi_limit' }  ) ? $lcovConf{ 'genhtml_hi_limit'  } : 90 );
        my $med_limit = ( defined( $lcovConf{ 'genhtml_med_limit' } ) ? $lcovConf{ 'genhtml_med_limit' } : 75 );
        my $classTestCoverPer = 'coverPerHi';
        my $classTestCoverNum = 'coverPerHi';
        if( $devInfo->Envs->[ $envIdx ]->UTestRate < $med_limit ){
            $classTestCoverPer = 'coverPerLo';
            $classTestCoverPer = 'coverNumLo';
        }elsif( $devInfo->Envs->[ $envIdx ]->UTestRate < $hi_limit ){
            $classTestCoverPer = 'coverPerMed';
            $classTestCoverPer = 'coverPerNum';
        }
        my $LineCovBarImg = 'emerald.png';
        my $classLineCoverPer = 'coverPerHi';
        my $classLineCoverNum = 'coverNumHi';
        if( $devInfo->Envs->[ $envIdx ]->LineCov < $med_limit ){
            $LineCovBarImg = 'amber.png';
            $classLineCoverPer = 'coverPerLo';
            $classLineCoverNum = 'coverNumLo';
        }elsif( $devInfo->Envs->[ $envIdx ]->LineCov < $hi_limit ){
            $LineCovBarImg = 'ruby.png';
            $classLineCoverPer = 'coverPerMed';
            $classLineCoverNum = 'coverPerNum';
        }
        my $classFuncCoverPer = 'coverPerHi';
        my $classFuncCoverNum = 'coverNumHi';
        if( $devInfo->Envs->[ $envIdx ]->FuncCov < $med_limit ){
            $classFuncCoverPer = 'coverPerLo';
            $classFuncCoverNum = 'coverNumLo';
        }elsif( $devInfo->Envs->[ $envIdx ]->FuncCov < $hi_limit ){
            $classFuncCoverPer = 'coverPerMed';
            $classFuncCoverNum = 'coverPerNum';
        }
        $tmplHtml =~ s!%%<classTestCoverPer>%%!$classTestCoverPer!o;
        $tmplHtml =~ s!%%<classTestCoverNum>%%!$classTestCoverNum!o;
        $tmplHtml =~ s!%%<LineCovBarImg>%%!$LineCovBarImg!o;
        $tmplHtml =~ s!%%<classLineCoverPer>%%!$classLineCoverPer!o;
        $tmplHtml =~ s!%%<classLineCoverNum>%%!$classLineCoverNum!o;
        $tmplHtml =~ s!%%<classFuncCoverPer>%%!$classFuncCoverPer!o;
        $tmplHtml =~ s!%%<classFuncCoverNum>%%!$classFuncCoverNum!o;
    }

    return $tmplHtml;
}

sub collect_project_name_and_version()
{
    ## $main::PROJ_NAME, $main::PROJ_VERSION を取得
    open( CMAKELISTS, '<', "$main::apppath/../CMakeLists.txt" ) ||
        die( qq{'CMakeLists.txt': could not open file: $!} );
    while( my $line = <CMAKELISTS> ){
        $line =~ s/\r?\n$//go;
        #print( qq{L: $line\n} );

        if( $line =~ m/project\s*\(\s*([^\s][^\)]+)\s*\)/o ){
            #print( qq{"$1"\n} );
            ## "get_shebang VERSION 0.0.1 LANGUAGES C"
            my( $project_name, @fields ) = split( /\s+/, $1 );
            $main::PROJ_NAME = $project_name;
            my $version_found = 0;
            for my $f( @fields ){
                #print( qq{F: $f\n} );
                if( $version_found ){
                    $version_found = 0;
                    $main::PROJ_VERSION = $f;
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
}

sub genIndexHtml( $$ )
{
    my $devInfo = $_[ 0 ];
    my $outFile = $_[ 1 ];

    #printf( qq{ProjName = "%s"\n}, $devInfo->ProjName );

    my $htmlHeader = htmlReplaceKeyword( $devInfo, -1, $tmplIndexHtmlHeader );
    my $htmlFooter = htmlReplaceKeyword( $devInfo, -1, $tmplIndexHtmlFooter );

    open( OUTPHTML, ">$outFile" ) || die( "$outFile: could not open file: $!" );
    print OUTPHTML ( "$htmlHeader" );

    for my $idx( sort{
        ## テスト環境名の名前順に出力する
        lc( $devInfo->Envs->[ $a ]->EnvName ) cmp lc( $devInfo->Envs->[ $b ]->EnvName )
#        ## テストの実行日時が新しい順に出力する
#        $devInfo->Envs->[ $b ]->Date cmp $devInfo->Envs->[ $a ]->Date
    }( @{ [ 0..$#{ $devInfo->Envs } ] } ) ){
        my $htmlEntry  = htmlReplaceKeyword( $devInfo, $idx, $tmplIndexHtmlEntry );
        print OUTPHTML ( "$htmlEntry" );
    }
    print OUTPHTML ( "$htmlFooter" );
    close( OUTPHTML );

    return 0;
}

## Revision: 1.4
sub exec_cmd( @ )
{
    my @args = @_;

    my $ref_num = 0;
    for my $arg( @args ){
        last if( ref( $arg ) eq '' );
        $ref_num++;
    }
    #print( qq{\$ref_num=$ref_num\n} );

    my $ref_stdout = undef;
    my $ref_config = undef;
    if( $ref_num == 1 ){
        $ref_config = shift( @args );
    }elsif( $ref_num == 2 ){
        $ref_config = shift( @args );
        $ref_stdout = shift( @args );
    }
    my $cmd = shift( @args );

    my $b_count_up = 0;
    if( defined( $ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS} ) ){
        $b_count_up = 1;
        $ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS}++;
    }else{
        $b_count_up = 0;
        $ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS} = 1;
    }

    if( !( defined( $ref_config->{DISPLAY_PROMPT} ) &&
            $ref_config->{DISPLAY_PROMPT} == 0 ) ){
        my $disp_cmd = qq{\n\e[34m$ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS}-\$ \e[0m$cmd} . array_to_str( ' ', @args );
        print( qq{$disp_cmd\n} );
    }

#    my $raw_status = system( $cmd, @args );
    open( EXEC_CMD, '-|', $cmd, @args ) ||
        die( qq{"$cmd": could not execute: $!} );
    $$ref_stdout = '' if( defined( $ref_stdout ) );
    while( <EXEC_CMD> ){
        my $line = $_;
        $line =~ s/\r?\n$//o;
        $$ref_stdout .= qq{$line\n} if( defined( $ref_stdout ) );
        if( !( defined( $ref_config->{DISPLAY_OUTPUT} ) &&
                $ref_config->{DISPLAY_OUTPUT} == 0 ) ){
            print( qq{$line\n} );
        }
    }
    close( EXEC_CMD );
    my $raw_status = $?;
    my $exit_status = $raw_status >> 8;

    if( !( defined( $ref_config->{DISPLAY_EXIT_STATUS} ) &&
            $ref_config->{DISPLAY_EXIT_STATUS} == 0 ) ){
        print( qq{$ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS}: exit_status=$exit_status\n} );
    }

    if( $b_count_up ){
        $ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS}--;
    }else{
        delete( $ENV{SH_EXEC_NUMBER_OF_PROMPT_RECURSIONS} );
    }

    return $exit_status;
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


exit( pl_main( @ARGV ) );
