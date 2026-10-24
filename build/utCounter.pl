#!/usr/bin/env perl
################################################################################
## - $Revision: 1.12 $
################################################################################

use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Class::Struct;                  # first released with perl 5.004
use Cwd qw();                       # first released with perl 5
use File::Basename qw();            # first released with perl 5
use Getopt::Long qw();              # first released with perl 5

struct Project => {
    Arch => '$',
    DateEpoch => '$',
    DateStr => '$',
    Modu => '@',
    Lines => '$',
    LinePass => '$',
    LineCov => '$',
    Functions => '$',
    FuncPass => '$',
    FuncCov => '$',
    UTAsserts => '$',
    UTSuccess => '$',
    UTFailure => '$',
    UTSuccessRate => '$',
};

struct Module =>{
    Name => '$',
    SrcPath => '$',
    Func => '@',
    Lines => '$',
    LinePass => '$',
    LineCov => '$',
    Functions => '$',
    FuncPass => '$',
    FuncCov => '$',
    UTAsserts => '$',
    UTSuccess => '$',
    UTFailure => '$',
    UTSuccessRate => '$',
};

struct FuncInfo =>{
    Name => '$',
    LineNumber => '$',
    Calls => '$',
    Lines => '$',
    LinePass => '$',
    LineCov => '$',
};

sub pl_main( @ )
{
    ## 初期化処理
    init_script();

    ## 引数解析
    parse_arg( @_ );

    param_print();

    my $ProjInfoUT = readUtResult( $main::utResult );
    my $ProjInfoLC = readLcovInfo( $main::lcovInfo );
    my $ProjInfo = mergeInfo( $ProjInfoUT, $ProjInfoLC );

    printf( qq{\$lcovInfo = "%s"\n}, $main::lcovInfo );
    printf( qq{\$utResult = "%s"\n}, $main::utResult );

    outputUtInfo( $ProjInfo );

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
    $main::bVerbose = 0;
    $main::lcovInfo = undef;
    $main::utResult = undef;
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
             qq{     \$main::debug = %d\n} .
             qq{     \$main::bVerbose = "%s"\n} .
             qq{     \$main::lcovInfo = "%s"\n} .
             qq{     \$main::utResult = "%s"\n},
        $main::execution_dir,
        $main::apppath,
        $main::appname,
        $main::argc,
        array_to_str( ', ', @main::argv ),
        $main::debug,
        $main::bVerbose,
        $main::lcovInfo,
        $main::utResult );
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
    Getopt::Long::Configure( "bundling" );

    Getopt::Long::GetOptions(
        'debug|d'         => \$main::debug,
        'verbose|v'       => \$main::bVerbose,
    ) || die( qq{parse_arg(): error: Failed to parse option switches.\n} );

    for my $arg( @ARGV ){
        if( !defined( $main::lcovInfo ) ){
            $main::lcovInfo = $arg;
        }elsif( !defined( $main::utResult ) ){
            $main::utResult = $arg;
        }else{
            die( qq{"$arg": unknown option\n} );
        }
    }


    if( ( ! defined( $main::lcovInfo ) ) || ( ! defined( $main::utResult ) ) ){
        printf STDERR ( "usage: $main::appname [-v|--verbose] <lcov.info> <unit_test_Result.tab>\n" );
        return;
    }
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

sub outputUtInfo( $ )
{
    my $ProjInfo = $_[ 0 ];

    printf( "Date: %s\n", $ProjInfo->DateStr );

    printf( "Unit Test                  Line Coverage              Functions            Name\n" );
    printf( "-------------------------  -------------------------  -------------------  --------------------\n" );
    printf( "%6.2f%% ( %5d / %5d )  %6.2f%% ( %5d / %5d )  %6.2f%% ( %2d / %2d )  Total\n",
            $ProjInfo->UTSuccessRate, $ProjInfo->UTSuccess, $ProjInfo->UTAsserts,
            $ProjInfo->LineCov, $ProjInfo->LinePass, $ProjInfo->Lines,
            $ProjInfo->FuncCov, $ProjInfo->FuncPass, $ProjInfo->Functions );
    for my $mod( sort{ $a->Name cmp $b->Name }( @{$ProjInfo->Modu} ) ){
        printf( "%6.2f%% ( %5d / %5d )  %6.2f%% ( %5d / %5d )  %6.2f%% ( %2d / %2d )    %s\n",
                $mod->UTSuccessRate, $mod->UTSuccess, $mod->UTAsserts,
                $mod->LineCov, $mod->LinePass, $mod->Lines,
                $mod->FuncCov, $mod->FuncPass, $mod->Functions, $mod->Name );
        next if( $main::bVerbose == 0 );
        for my $func( sort{ $a->Name cmp $b->Name }( @{$mod->Func} ) ){
            printf( "---.--%% ( ----- / ----- )  %6.2f%% ( %5d / %5d )  ---.--%% ( -- / -- )      %s()\n",
                    $func->LineCov, $func->LinePass, $func->Lines, $func->Name );
        }
    }

    return 0;
}

sub readLcovInfo( $ )
{
    my( $lcovInfo ) = @_;

    my $ProjInfo = Project->new();
    $ProjInfo->Arch( '' );
    $ProjInfo->DateEpoch( 0 );
    $ProjInfo->DateStr( '1999-01-01 00:00:00' );
    $ProjInfo->Lines( 0 );
    $ProjInfo->LinePass( 0 );
    $ProjInfo->LineCov( 0 );
    $ProjInfo->Functions( 0 );
    $ProjInfo->FuncPass( 0 );
    $ProjInfo->FuncCov( 0 );
    $ProjInfo->UTAsserts( 0 );
    $ProjInfo->UTSuccess( 0 );
    $ProjInfo->UTFailure( 0 );
    $ProjInfo->UTSuccessRate( 0 );

    if( ! -f $lcovInfo ){
        printf STDERR ( qq{"%s": file not found\n}, $lcovInfo );
        return $ProjInfo;
    }

    #my $Mod;

    open( LCOVINFO, '<', "$lcovInfo" ) ||
        die( "$lcovInfo: could not open: $!" );
    my $curModule;
    my $curModIdx = 0;
    my $curFuncName = '';
    my $curFuncIdx = -1;
    my $curFunc;
    my $nextFuncLN = -1;
    my $nr = 0;
    while( <LCOVINFO> ){
        $nr++;
        my $line = $_;
        $line =~ s/\r?\n$//o;

        if( $line eq 'end_of_record' ){
            next;
        }
        if( !( $line =~ m!^([^:]+):(.+)$!o ) ){
            printf( qq{Unknown: line %d: %s\n}, $nr );
            next;
        }
        my $Mnemonic = $1;
        my $Params   = $2;

        ## TN: Test Name
        if( $Mnemonic eq 'TN' ){
            $ProjInfo->Arch( $Params );
            next;
        ## SF: Source File Path
        }elsif( $Mnemonic eq 'SF' ){
            ## for Android Termux
            if( ( scalar( @{ $ProjInfo->Modu } ) > 0 ) &&
                ( $ProjInfo->Modu->[ $#{$ProjInfo->Modu} ]->SrcPath eq $Params ) ){
                $curModule = $ProjInfo->Modu->[ $#{$ProjInfo->Modu} ];
                $ProjInfo->Lines( $ProjInfo->Lines - $curModule->Lines );
                $ProjInfo->LinePass( $ProjInfo->LinePass - $curModule->LinePass );
                $curModule->Lines( 0 );
                $curModule->LinePass( 0 );
                my $idxMax = $#{$curModule->Func} + 1;
                for( my $idx=0; $idx<$idxMax; $idx++ ){
                    $curModule->Func->[ $idx ]->Lines( 0 );
                    $curModule->Func->[ $idx ]->LinePass( 0 );
                }
                $curFuncIdx = -1;
                $nextFuncLN = -1;
                next;
            }
            $curFuncIdx = -1;
            $nextFuncLN = -1;
            $curModule = Module->new();
            my $modName = File::Basename::basename( $Params );
            $curModule->Name( $modName );
            $curModule->SrcPath( $Params );
            $curModule->Lines( 0 );
            $curModule->LinePass( 0 );
            $curModule->LineCov( 0 );
            $curModule->Functions( 0 );
            $curModule->FuncPass( 0 );
            $curModule->FuncCov( 0 );
            $curModule->UTAsserts( 0 );
            $curModule->UTSuccess( 0 );
            $curModule->UTFailure( 0 );
            $curModule->UTSuccessRate( 0 );
            $ProjInfo->Modu( scalar( @{ $ProjInfo->Modu } ), $curModule );
            #printf( qq{"%s"\n}, $modName );
            next;
        ## FN: line number,function name
        }elsif( $Mnemonic eq 'FN' ){
            my( $lineNo, $funcName ) = split( /,/, $Params );
            #printf( qq{New Function: %s()\n}, $funcName );
            $curFuncIdx++;
            $curFuncName = $funcName;
            $curFunc = FuncInfo->new();
            $curFunc->Name( $funcName );
            $curFunc->LineNumber( $lineNo );
            $curFunc->Lines( 0 );
            $curFunc->LinePass( 0 );
            $curModule->Func( $curFuncIdx, $curFunc );
            next;
        }elsif( $Mnemonic eq 'FNDA' ){
            my( $calls, $funcName ) = split( /,/, $Params );
            $curFunc->Calls( $calls );
            $curModule->Functions( $curModule->Functions + 1 );
            $ProjInfo->Functions( $ProjInfo->Functions + 1 );
            if( $calls > 0 ){
                $curModule->FuncPass( $curModule->FuncPass + 1 );
                $ProjInfo->FuncPass( $ProjInfo->FuncPass + 1 );
            }
            next;
        ## DA: line number, hit count
        }elsif( $Mnemonic eq 'DA' ){
            my( $lineNo, $calls ) = split( /,/, $Params );
            my $bUpdateNextFuncLN = 0;
            if( $nextFuncLN eq -1 ){
                $curFuncIdx = -1;
                $bUpdateNextFuncLN = 1;
            }
            if( $lineNo >= $nextFuncLN ){
                $curFuncIdx++;
                $bUpdateNextFuncLN = 1;
            }
            if( $bUpdateNextFuncLN != 0 ){
                $bUpdateNextFuncLN = 0;
                my $nextFuncIdx = $curFuncIdx + 1;
                if( $nextFuncIdx > $#{$curModule->Func} ){
                    $nextFuncLN = 100000;
                }else{
                    $nextFuncLN = $curModule->Func->[ $nextFuncIdx ]->LineNumber;
                }
                #printf( "\$nextFuncLN = %d\n", $nextFuncLN );
            }

            $curModule->Func->[ $curFuncIdx ]->Lines( $curModule->Func->[ $curFuncIdx ]->Lines + 1 );
            $curModule->Lines( $curModule->Lines + 1 );
            $ProjInfo->Lines( $ProjInfo->Lines + 1 );
            if( $calls > 0 ){
                $curModule->Func->[ $curFuncIdx ]->LinePass( $curModule->Func->[ $curFuncIdx ]->LinePass + 1 );
                $curModule->LinePass( $curModule->LinePass + 1 );
                $ProjInfo->LinePass( $ProjInfo->LinePass + 1 );
            }
            #printf( qq{%5d: DA: \$Params="%s", %s()\n}, $nr, $Params, $curModule->Func->[ $curFuncIdx ]->Name );
            next;
        ## FNF: number functions found
        }elsif( $Mnemonic eq 'FNF' ){
            if( $Params != $curModule->Functions ){
                printf STDERR ( "WARN: FNF:%d: != \$curModule->Functions: The numbers are different.\n", $Params );
            }
            next;
        ## FNH: number hit
        }elsif( $Mnemonic eq 'FNH' ){
            if( $Params != $curModule->FuncPass ){
                printf STDERR ( "WARN: FNH:%d: != \$curModule->FuncPass: The numbers are different.\n", $Params );
            }
            next;
        ## BRDA: branch data: line, block, (expressions,count)+
        ##       <line_number>,<block_number>,<branch_number>,<taken>
        }elsif( $Mnemonic eq 'BRDA' ){
            next;
        ## BRF: branches found
        ##      そのファイル内に存在するすべての分岐ルートの総数（分母）
        }elsif( $Mnemonic eq 'BRF' ){
            next;
        ## BRH: Branches Hit
        ##      テストで1回以上実行された分岐ルートの総数（分子）
        }elsif( $Mnemonic eq 'BRH' ){
            next;
        ## LF: lines found
        }elsif( $Mnemonic eq 'LF' ){
            if( $Params != $curModule->Lines ){
                printf STDERR ( "WARN: LF:%d: != \$curModule->Lines: The numbers are different.\n", $Params );
            }
            next;
        ## LH: lines hit
        }elsif( $Mnemonic eq 'LH' ){
            if( $Params != $curModule->LinePass ){
                printf STDERR ( "WARN: LH:%d: != \$curModule->LinePass: The numbers are different.\n", $Params );
            }
            next;
        }

        printf STDERR ( qq{L: %5d: %s = "%s"\n}, $nr, $Mnemonic, $Params );
    }
    close( LCOVINFO );

    $ProjInfo->LineCov( ( $ProjInfo->Lines == 0 ) ? 0 : ( $ProjInfo->LinePass * 100 / $ProjInfo->Lines ) );
    $ProjInfo->FuncCov( ( $ProjInfo->Functions == 0 ) ? 0 : ( $ProjInfo->FuncPass * 100 / $ProjInfo->Functions ) );
    for my $mod( @{$ProjInfo->Modu} ){
        $mod->LineCov( ( $mod->Lines == 0 ) ? 0 : ( $mod->LinePass * 100 / $mod->Lines ) );
        $mod->FuncCov( ( $mod->Functions == 0 ) ? 0 : ( $mod->FuncPass * 100 / $mod->Functions ) );
        for my $func( @{$mod->Func} ){
            #printf( "[%3d] %s() %d / %d\n", $func->LineNumber, $func->Name, $func->LinePass, $func->Lines );
            $func->LineCov( ( $func->Lines == 0 ) ? 0 : ( $func->LinePass * 100 / $func->Lines ) );
        }
    }

    return $ProjInfo;
}

sub readUtResult( $ )
{
    my( $utResult ) = @_;

    my $ProjInfo = Project->new();
    $ProjInfo->Arch( '' );
    $ProjInfo->DateEpoch( 0 );
    $ProjInfo->DateStr( '1999-01-01 00:00:00' );
    $ProjInfo->Lines( 0 );
    $ProjInfo->LinePass( 0 );
    $ProjInfo->LineCov( 0 );
    $ProjInfo->Functions( 0 );
    $ProjInfo->FuncPass( 0 );
    $ProjInfo->FuncCov( 0 );
    $ProjInfo->UTAsserts( 0 );
    $ProjInfo->UTSuccess( 0 );
    $ProjInfo->UTFailure( 0 );
    $ProjInfo->UTSuccessRate( 0 );

    my $curModule;

    if( ! -f $utResult ){
        printf STDERR ( qq{"%s": file not found\n}, $utResult );
        return $ProjInfo;
    }

    $ProjInfo->DateEpoch( getModTime( $utResult ) );
    my( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) = localtime( $ProjInfo->DateEpoch );
    $ProjInfo->DateStr( sprintf( "%04d-%02d-%02d %02d:%02d:%02d", $year+1900, $mon+1, $mday, $hour, $min, $sec ) );

    open( UTRESULT, '<', $utResult ) ||
        die( "$utResult: could not open: $!" );
    my $curModName = '';
    my $curModIdx = -1;
    my $curModIdxLen = scalar( @{ $ProjInfo->Modu } );
    my $nr = 0;
    while( <UTRESULT> ){
        $nr++;
        my $line = $_;
        $line =~ s/\r?\n$//o;
        #printf( "L: %5d: %s\n", $nr, $line );

        my( $res, $passed, $failed, $asserts, $modName, $testFunction ) = split( /\t/, $line );
        next if( $nr == 1 );
        next if( $modName eq 'Total' );

        if( $modName ne $curModName ){
            $curModName = $modName;

            $curModule = Module->new();
            $curModule->Name( $curModName );
            $curModule->SrcPath( $curModName );
            $curModule->Lines( 0 );
            $curModule->LinePass( 0 );
            $curModule->LineCov( 0 );
            $curModule->Functions( 0 );
            $curModule->FuncPass( 0 );
            $curModule->FuncCov( 0 );
            $curModule->UTAsserts( $asserts );
            $curModule->UTSuccess( $passed );
            $curModule->UTFailure( $failed );
            $curModule->UTSuccessRate( 0 );
            $curModIdx = scalar( @{ $ProjInfo->Modu } );
            $ProjInfo->Modu( $curModIdx, $curModule );

        }else{

            $ProjInfo->Modu->[ $curModIdx ]->UTSuccess( $ProjInfo->Modu->[ $curModIdx ]->UTSuccess + $passed );
            $ProjInfo->Modu->[ $curModIdx ]->UTFailure( $ProjInfo->Modu->[ $curModIdx ]->UTFailure + $failed );
            $ProjInfo->Modu->[ $curModIdx ]->UTAsserts( $ProjInfo->Modu->[ $curModIdx ]->UTAsserts + $asserts );
        }

        $ProjInfo->UTSuccess( $ProjInfo->UTSuccess + $passed );
        $ProjInfo->UTFailure( $ProjInfo->UTFailure + $failed );
        $ProjInfo->UTAsserts( $ProjInfo->UTAsserts + $asserts );
    }
    close( UTRESULT );

    $ProjInfo->UTSuccessRate( ( $ProjInfo->UTAsserts == 0 ) ? 0 : ( $ProjInfo->UTSuccess * 100 / $ProjInfo->UTAsserts ) );
    my $idxMax = $#{$ProjInfo->Modu} + 1;
    for( my $idx=0; $idx<$idxMax; $idx++ ){
        $ProjInfo->Modu->[ $idx ]->UTSuccessRate(
            ( $ProjInfo->Modu->[ $idx ]->UTAsserts == 0 ) ? 0 :
            ( $ProjInfo->Modu->[ $idx ]->UTSuccess * 100 / $ProjInfo->Modu->[ $idx ]->UTAsserts ) );
    }

    return $ProjInfo;
}

sub mergeInfo( @ )
{
    my @PIs = @_;

    my $ProjInfo = Project->new();
    $ProjInfo->Arch( '' );
    $ProjInfo->DateEpoch( 0 );
    $ProjInfo->DateStr( '1999-01-01 00:00:00' );
    $ProjInfo->Lines( 0 );
    $ProjInfo->LinePass( 0 );
    $ProjInfo->LineCov( 0 );
    $ProjInfo->Functions( 0 );
    $ProjInfo->FuncPass( 0 );
    $ProjInfo->FuncCov( 0 );
    $ProjInfo->UTAsserts( 0 );
    $ProjInfo->UTSuccess( 0 );
    $ProjInfo->UTFailure( 0 );
    $ProjInfo->UTSuccessRate( 0 );

    my %mod2idx = ();
    my %func2idx = ();
    my $PIs_len = scalar( @PIs );
    for my $pinfo( @PIs ){
        if( $pinfo->Arch ne '' ){
            $ProjInfo->Arch( $pinfo->Arch )
        }
        if( $pinfo->DateEpoch != 0 ){
            $ProjInfo->DateEpoch( $pinfo->DateEpoch )
        }
        if( $pinfo->DateStr ne '1999-01-01 00:00:00' ){
            $ProjInfo->DateStr( $pinfo->DateStr )
        }
        if( $pinfo->Lines != 0 ){
            $ProjInfo->Lines( $pinfo->Lines )
        }
        if( $pinfo->LinePass != 0 ){
            $ProjInfo->LinePass( $pinfo->LinePass )
        }
        if( $pinfo->LineCov != 0 ){
            $ProjInfo->LineCov( $pinfo->LineCov )
        }
        if( $pinfo->Functions != 0 ){
            $ProjInfo->Functions( $pinfo->Functions )
        }
        if( $pinfo->FuncPass != 0 ){
            $ProjInfo->FuncPass( $pinfo->FuncPass )
        }
        if( $pinfo->FuncCov != 0 ){
            $ProjInfo->FuncCov( $pinfo->FuncCov )
        }
        if( $pinfo->UTAsserts != 0 ){
            $ProjInfo->UTAsserts( $pinfo->UTAsserts )
        }
        if( $pinfo->UTSuccess != 0 ){
            $ProjInfo->UTSuccess( $pinfo->UTSuccess )
        }
        if( $pinfo->UTFailure != 0 ){
            $ProjInfo->UTFailure( $pinfo->UTFailure )
        }
        if( $pinfo->UTSuccessRate != 0 ){
            $ProjInfo->UTSuccessRate( $pinfo->UTSuccessRate )
        }

        my $mod_len = scalar( @{ $pinfo->Modu } );
        for( my $idx=0; $idx<$mod_len; $idx++ ){
            my $mod_name = $pinfo->Modu->[ $idx ]->Name;
            my $mod;
            if( defined( $mod2idx{ $mod_name } ) ){
                $mod = $ProjInfo->Modu->[ $mod2idx{ $mod_name } ];
            }else{
                my $modIdx = scalar( @{ $ProjInfo->Modu } );
                $mod2idx{ $mod_name } = $modIdx;
                $mod = Module->new();
                $ProjInfo->Modu( $modIdx, $mod );

                $mod->Name( '' );
                $mod->SrcPath( '' );
                $mod->Lines( 0 );
                $mod->LinePass( 0 );
                $mod->LineCov( 0 );
                $mod->Functions( 0 );
                $mod->FuncPass( 0 );
                $mod->FuncCov( 0 );
                $mod->UTAsserts( 0 );
                $mod->UTSuccess( 0 );
                $mod->UTFailure( 0 );
                $mod->UTSuccessRate( 0 );
            }

            if( $pinfo->Modu->[ $idx ]->Name ne '' ){
                $mod->Name( $pinfo->Modu->[ $idx ]->Name );
            }
            if( $pinfo->Modu->[ $idx ]->SrcPath ne '' ){
                $mod->SrcPath( $pinfo->Modu->[ $idx ]->SrcPath );
            }
            if( $pinfo->Modu->[ $idx ]->Lines != 0 ){
                $mod->Lines( $pinfo->Modu->[ $idx ]->Lines );
            }
            if( $pinfo->Modu->[ $idx ]->LinePass != 0 ){
                $mod->LinePass( $pinfo->Modu->[ $idx ]->LinePass );
            }
            if( $pinfo->Modu->[ $idx ]->LineCov != 0 ){
                $mod->LineCov( $pinfo->Modu->[ $idx ]->LineCov );
            }
            if( $pinfo->Modu->[ $idx ]->Functions != 0 ){
                $mod->Functions( $pinfo->Modu->[ $idx ]->Functions );
            }
            if( $pinfo->Modu->[ $idx ]->FuncPass != 0 ){
                $mod->FuncPass( $pinfo->Modu->[ $idx ]->FuncPass );
            }
            if( $pinfo->Modu->[ $idx ]->FuncCov != 0 ){
                $mod->FuncCov( $pinfo->Modu->[ $idx ]->FuncCov );
            }
            if( $pinfo->Modu->[ $idx ]->UTAsserts != 0 ){
                $mod->UTAsserts( $pinfo->Modu->[ $idx ]->UTAsserts );
            }
            if( $pinfo->Modu->[ $idx ]->UTSuccess != 0 ){
                $mod->UTSuccess( $pinfo->Modu->[ $idx ]->UTSuccess );
            }
            if( $pinfo->Modu->[ $idx ]->UTFailure != 0 ){
                $mod->UTFailure( $pinfo->Modu->[ $idx ]->UTFailure );
            }
            if( $pinfo->Modu->[ $idx ]->UTSuccessRate != 0 ){
                $mod->UTSuccessRate( $pinfo->Modu->[ $idx ]->UTSuccessRate );
            }

            my $func_len = scalar( @{ $mod->Func } );
            for( my $idx2=0; $idx2<$func_len; $idx2++ ){
                my $func_name = $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Name;
                my $func;
                if( defined( $func2idx{ $mod_name-$func_name } ) ){
                    $func = $mod->Func->[ $func2idx{ $mod_name-$func_name } ];
                }else{
                    my $funcIdx = scalar( @{ $mod->Func } );
                    $func2idx{ $mod_name-$func_name } = $funcIdx;
                    $func = FuncInfo->new();
                    $mod->Func( $funcIdx, $func );

                    $func->Name( '' );
                    $func->LineNumber( 0 );
                    $func->Calls( 0 );
                    $func->Lines( 0 );
                    $func->LinePass( 0 );
                    $func->LineCov( 0 );
                }

                if( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Name ne '' ){
                    $func->Name( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Name );
                }
                if( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->LineNumber != 0 ){
                    $func->LineNumber( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->LineNumber );
                }
                if( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Calls != 0 ){
                    $func->Calls( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Calls );
                }
                if( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Lines != 0 ){
                    $func->Lines( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->Lines );
                }
                if( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->LinePass != 0 ){
                    $func->LinePass( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->LinePass );
                }
                if( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->LineCov != 0 ){
                    $func->LineCov( $pinfo->Modu->[ $idx ]->Func->[ $idx2 ]->LineCov );
                }
            }
        }
    }

    return $ProjInfo;
}

exit( pl_main( @ARGV ) );
