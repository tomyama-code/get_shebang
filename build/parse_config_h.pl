#!/usr/bin/env perl
################################################################################
## - $Revision: 1.1 $
################################################################################

use strict;                         # first released with perl 5
use warnings;                       # first released with perl v5.6.0
use Cwd qw();                       # first released with perl 5
#use Data::Dumper qw();              # first released with perl 5.005
use File::Basename qw();            # first released with perl 5
use Getopt::Long qw();              # first released with perl 5


##########
## スクリプトのエントリポイント
sub pl_main( @ )
{
    if( ! -f './config.h' ){
        die( qq{Please run this in the directory that contains "config.h".\n} );
    }

    ## 初期化処理
    init_script();

    ## 引数解析
    parse_arg( @_ );

    param_print();

    print( qq{config_h_to_ini() >config.ini\n} );
    config_h_to_ini( "$main::execution_dir/config.h" );

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

##########
## ファイルの更新時間を比較する
## Revision: 1.1
sub isUpdateNecessary( $$ )
{
    my( $basefile, $genfile ) = @_;

    # ベースファイルの存在確認
    if( ! -f $basefile ){
        die( "$main::appname: error: $basefile: file not found\n" );
    }

    # 生成ファイルの存在確認
    if( -f $genfile ){
        # mtime（最終更新時刻）を取得
        my $mtime_base = ( stat( $basefile ) )[ 9 ];
        my $mtime_gen  = ( stat( $genfile ) )[ 9 ];

        # 生成ファイルがベースファイルより新しい（または等しい）場合
        if( $mtime_gen >= $mtime_base ){
            return 0;
        }
        return 1;
    }else{
        # 生成ファイルが存在しない場合
        return 2;
    }
}

sub config_h_to_ini( $ )
{
    my( $config_h ) = @_;
    my $conf = parse_config_h( $config_h );
    #print( Data::Dumper::Dumper( $conf ) );

    my $inifile = $config_h;
    $inifile =~ s/\.h$/.ini/o;
    #print( qq{\$inifile="$inifile"\n} );

    my $need_act = isUpdateNecessary( $config_h, $inifile );
    if( $need_act == 0 ){
        print( qq{config.ini: Already up to date\n} );
        return;
    }elsif( $need_act == 2 ){
        print( qq{config.ini: Create New\n} );
    }

    open( INIFILE, '>', $inifile ) ||
        die( qq{error: $inifile: could not open file: $!} );

    for my $k( sort( keys( %{ $conf } ) ) ){
        #print( qq{key: $k\n} );
        print INIFILE ( qq{$k="$conf->{ $k }"\n} );
    }

    close( INIFILE );
}

# ハッシュにしてリファレンスを返す（Perlスクリプト向け）
sub parse_config_h( $ )
{
    my( $file ) = @_;
    my %config;

    open( my $fh, '<', $file ) ||
        die( "Cannot open $file: $!" );
    while( my $line = <$fh> ){
        $line =~ s/\r?\n//go;

        # コメント行や空行、インクルードガードなどを無視
        if( $line =~ /^\s*\/\*/ ||
            $line =~ /^\s*\*/ ||
            $line =~ /^\s*#if/ ){
            next;
        }

        # 値を持つマクロの解析 (#define KEY VALUE)
        if( $line =~ /^\s*#\s?define\s+(\w+)\s+(.+)$/ ){
            my( $key, $val ) = ( $1, $2 );

            # コメントが後ろに付いている場合は除去
            $val =~ s!\s*/\*.*?\*/\s*$!!;
            $val =~ s!\s*//.*$!!;
            $val =~ s!\s+$!!;

            # ダブルクォーテーションで囲まれた文字列は中身を取り出す
            if( $val =~ /^"(.*)"$/ ){
                $config{$key} = $1;
            }else{
                $config{$key} = $val; # 数値やその他の値
            }
        # 値のないフラグマクロの解析 (#define KEY)
        }elsif( $line =~ /^\s*#\s?define\s+(\w+)\s*$/ ){
            $config{$1} = 1; # フラグが存在する場合は 1 をセット
        }
    }
    close( $fh );
    return \%config;
}

# 実行して中身をini形式でダンプ（シェルスクリプト向け）
# uncoverable branch false
exit( pl_main( @ARGV ) ) unless caller();

1;

__END__
