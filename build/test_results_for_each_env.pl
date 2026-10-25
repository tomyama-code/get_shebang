#!/usr/bin/env perl
################################################################################
## TEST_RESULTS_FOR_EACH_ENV.PL -- Merging unit test results and lcov reports.
##
## - Created based on r1.10 of 'genIndexHtml.pl'
##
## - Author: tomyama   2026
## - Version: 1 ( $Revision: 1.3 $ )
## - Only for personal use !
################################################################################


use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Class::Struct;                  # first released with perl 5.004
use Cwd qw();                       # first released with perl 5
use File::Basename qw();            # first released with perl 5
use File::Path qw();                # first released with perl 5.001
use Time::Piece qw();               # first released with perl v5.9.5


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

my $apppath;
my $appname;
my $devDirTop;

my $envInfo;
my %lcovConf;


sub pl_main( @ )
{
    initialize();
    #printf( qq{\$apppath="$apppath", \$appname="$appname"\n} );
    chdir( "$apppath/.." );

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
    printf( qq{ProjName = "%s"\n}, $envInfo->ProjName );

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
        my $exit_status = exec_cmd( 'cp', '-f',
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

## 初期化サブルーチン
sub initialize()
{
    ### GLOBAL ###
    $apppath = File::Basename::dirname( $0 );
    $appname = File::Basename::basename( $0 );
    #$main::debug = 0;
    ##############

    return 0;
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
  <title>%%<ProjName>%%</title>
  <link rel="stylesheet" type="text/css" href="%%<ResourceDir>%%/gcov.css">
</head>

<body>


  <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr><td class="title">%%<ProjName>%% : Unit Test Report</td></tr>
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
          <td class="tableHead">Environment</td>
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
  <p><a href="../../index.html">Up</a></p>

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

    my $ProjName    = $devInfo->ProjName;
    $tmplHtml =~ s!%%<ProjName>%%!$ProjName!go;

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

    my $ProgName = $appname;
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
        my $classTestCoverPer = 'coverFile';
        my $classTestCoverNum = 'coverFile';
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

sub genIndexHtml( $$ )
{
    my $devInfo = $_[ 0 ];
    my $outFile = $_[ 1 ];

    #printf( qq{ProjName = "%s"\n}, $devInfo->ProjName );

    my $htmlHeader = htmlReplaceKeyword( $devInfo, -1, $tmplIndexHtmlHeader );
    my $htmlFooter = htmlReplaceKeyword( $devInfo, -1, $tmplIndexHtmlFooter );

    open( OUTPHTML, ">$outFile" ) || die( "$outFile: could not open file: $!" );
    print OUTPHTML ( "$htmlHeader" );

    ## テストの実行日時が新しい順に出力する
    for my $idx( sort{ $devInfo->Envs->[ $b ]->Date cmp $devInfo->Envs->[ $a ]->Date }( @{[ 0..$#{$devInfo->Envs} ]} ) ){
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
