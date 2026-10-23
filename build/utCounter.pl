#!/usr/bin/env perl
################################################################################
## - $Revision: 1.9 $
################################################################################

use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Class::Struct;                  # first released with perl 5.004
use File::Basename qw();            # first released with perl 5

my $bVerbos = 0;

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
    if( defined( $_[ 0 ] ) && ( ( $_[ 0 ] eq '-v' ) || ( $_[ 0 ] eq '--verbos' ) ) ){
        $bVerbos = 1;
        shift( @_ );
    }

    if( ( ! defined( $_[ 0 ] ) ) || ( ! defined( $_[ 1 ] ) ) ){
        printf STDERR ( "usage: prog [-v|--verbos] <lcov.info> <unit_test_Result.tab>\n" );
        return 1;
    }
    my $lcovInfo = $_[ 0 ];
    printf( qq{\$lcovInfo = "%s"\n}, $lcovInfo );
    my $utResult = $_[ 1 ];
    printf( qq{\$utResult = "%s"\n}, $utResult );

    my $ProjInfo = readLcovInfo( $lcovInfo );
    $ProjInfo = readUtResult( $ProjInfo, $utResult );
    outputUtInfo( $ProjInfo );

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
        next if( $bVerbos == 0 );
        for my $func( sort{ $a->Name cmp $b->Name }( @{$mod->Func} ) ){
            printf( "---.--%% ( ----- / ----- )  %6.2f%% ( %5d / %5d )  ---.--%% ( -- / -- )      %s()\n",
                    $func->LineCov, $func->LinePass, $func->Lines, $func->Name );
        }
    }

    return 0;
}

sub readLcovInfo( $ )
{
    my $lcovInfo = $_[ 0 ];
    if( ! -f $lcovInfo ){
        fprintf stderr ( qq{"%s": file not found\n}, $lcovInfo );
        return 1;
    }

    my $ProjInfo = Project->new();
    $ProjInfo->Lines( 0 );
    $ProjInfo->LinePass( 0 );
    $ProjInfo->Functions( 0 );
    $ProjInfo->FuncPass( 0 );
    $ProjInfo->UTSuccess( 0 );
    $ProjInfo->UTFailure( 0 );
    $ProjInfo->UTAsserts( 0 );
    my $Mod;

    open( LCOVINFO, "<$lcovInfo" ) || die( "$lcovInfo: could not open." );
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
            if( ( $#{$ProjInfo->Modu} > -1 ) &&
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
            $curModule->Functions( 0 );
            $curModule->FuncPass( 0 );
            $curModule->UTSuccess( 0 );
            $curModule->UTFailure( 0 );
            $curModule->UTAsserts( 0 );
            $ProjInfo->Modu( $#{$ProjInfo->Modu} + 1, $curModule );
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

sub readUtResult( $$ )
{
    my $ProjInfo = $_[ 0 ];
    my $utResult = $_[ 1 ];

    if( ! -f $utResult ){
        printf STDERR ( qq{"%s": file not found\n}, $utResult );
        return 1;
    }

    $ProjInfo->DateEpoch( getModTime( $utResult ) );
    my( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) = localtime( $ProjInfo->DateEpoch );
    $ProjInfo->DateStr( sprintf( "%04d-%02d-%02d %02d:%02d:%02d", $year+1900, $mon+1, $mday, $hour, $min, $sec ) );

    open( UTRESULT, "<$utResult" ) || die( "$utResult: could not open." );
    my $curModName = '';
    my $curModIdx = -1;
    my $curModIdxMax = $#{$ProjInfo->Modu} + 1;
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
            for( $curModIdx=0; $curModIdx<$curModIdxMax; $curModIdx++ ){
                if( $ProjInfo->Modu->[ $curModIdx ]->Name eq $curModName ){
                    last;
                }
            }
        }
        next if( $curModIdx == $curModIdxMax );

        $ProjInfo->Modu->[ $curModIdx ]->UTSuccess( $ProjInfo->Modu->[ $curModIdx ]->UTSuccess + $passed );
        $ProjInfo->Modu->[ $curModIdx ]->UTFailure( $ProjInfo->Modu->[ $curModIdx ]->UTFailure + $failed );
        $ProjInfo->Modu->[ $curModIdx ]->UTAsserts( $ProjInfo->Modu->[ $curModIdx ]->UTAsserts + $asserts );

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

exit( pl_main( @ARGV ) );
