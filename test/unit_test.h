/*! \page page_unte  Unit Test
@section  sec_unte_overview  概要
単体テストを行う／もしくはその機能を実装するモジュール

@section  sec_unte_howtouse  使い方
- リポジトリ
    - <a href="https://github.com/tomyama-code/Skelton">https://github.com/tomyama-code/Skelton</a>
- 開発環境: GNU Make(必須), Cygwin(Windowsでは必須), Visual Studio, GNU GCC, もしくは clang
    - (必須) GNU make
    - Windows
        - Visual Studio の「～ Tools Command Prompt」、例えば「x64 Native Tools Command Prompt」
        - CygwinをインストールしてWindowsの環境変数 PATH に Cygwin のbin（例えば C:\\cygwin64\\bin ）を追加しておくこと
    - Cygwin
        - 開発ツールとして GCC がインストールされていること
    - Linux
        - 開発ツールとして GCC がインストールされていること
    - Android termux
        - 開発ツールとしてclang, もしくは GCC がインストールされていること
- ビルド方法
    - make
- 単体テストを実行
    - make unittest
- 単体テストにおけるコードカバレッジ率を閲覧
    - <a href="../../cov/index.html">code coverage report</a>

@section  sec_unte_description  説明
以下のように A, B というモジュールを持つプログラムの単体テストを実施する方法について説明する。

\dot
digraph program_structure {
  m [shape=box label="main.c"];
  a [shape=box label="module_A.c"];
  b [shape=box label="module_B.c"];
  m -> a [color=blue];
  m -> b [color=blue];
}
\enddot

このツールでは、以下のような構成に変更して単体テストを実施する。
つまり、 main.c の単体テストはおこなわない。
main.c の品質は、結合テストで担保すること。
青字で示した部分はユーザーが作成する必要のあるファイルである。

\dot
digraph test_program_structure {
  m [shape=box label="main.c"];
  tm [shape=box                           label="unit_test.c"];
  tu [shape=box color=blue fontcolor=blue label="unit_test_user_define.c"];
  ta [shape=box color=blue fontcolor=blue label="testcase_module_A.c"];
  tb [shape=box color=blue fontcolor=blue label="testcase_module_B.c"];
  a [shape=box label="module_A.c"];
  b [shape=box label="module_B.c"];
  m -> a [style=dotted];
  m -> b [style=dotted];
  tu -> tm [color=blue arrowhead = diamond];
  tm -> ta [color=blue];
  tm -> tb [color=blue];
  ta -> a [color=blue];
  tb -> b [color=blue];
}
\enddot

[testcase_module_A.c]の記述例
\code
#include "unittest/unit_test.h"
#include "module_A.h"   // func_a1(), func_a2()

static void testfunc_func_a1( void );
static void testfunc_func_a2( void );

static UNTE_TESTFUNC_S testfuncs[] = {
    UNTE_DEF_FUNC(testfunc_func_a1),
    UNTE_DEF_FUNC(testfunc_func_a2)
};

UNTE_MODULE_TESTS_S module_A_tests = {
    "Module A",
    ( sizeof testfuncs ) / ( sizeof testfuncs[ 0 ] ),
    testfuncs
};

static void testfunc_func_a1( void )
{
    int expectValue = 5;
    int returnValue = func_a1( 2, 3 );
    UNTE_ASSERT( returnValue == expectValue );
    return;
}

static void testfunc_func_a2( void )
{
    int expectValue = -1;
    int returnValue = func_a2( 2, 3 );
    UNTE_ASSERT( returnValue == expectValue );
    return;
}
\endcode

[unit_test_user_define.c]の記述例
\code
extern UNTE_MODULE_TESTS_S module_A_tests;
extern UNTE_MODULE_TESTS_S module_B_tests;

UNTE_MODULE_TESTS_S *pDefineModules[] = {
    &module_A_tests,
    &module_B_tests
};
\endcode

@section  sec_unte_history  変更履歴
- 2023-04-03: r1.12
    - 長い関数からコードを切り出して整理
        - unte_resultPrint()
        - unte_resultFileOut()
    - 上記２つの処理はテスト未実施のとき（-i で テストをしないでexitしたときなど）には
      実行しないようにした（古いファイルを残す）。
- 2023-03-30: r1.11
    - 単体テストの結果を一覧出力する際に、tmym_common に追加した cmAlignNumber()関数群を
      用いて桁揃えするようにした。
- 2023-03-22: r1.10
    - モジュールごとにテストを実行できるようにした。
        - "-m <module_number>": \<module_number\>番目のモジュールをテスト
        - "-i": 対話的にテストするモジュールを指定
    - ファイルから擬似的に標準入力に入力する機能を追加
        - read_mbr_gptで使っていたものを移植
        - この機能はVisual Studioでは使えない
- 2023-03-18: r1.9
    - テスト結果をファイルに出力する機能を追加。フォーマットはタブ区切り形式。
- 2023-03-11: r1.7
    - 出力関数を tmym_cmPrtf() を使うように変更。
- 2023-03-07: r1.6
    - 関数マクロ UNTE_FAIL_PRT() を追加。
- 2022-07-24: r1.5
    - テスト結果の"NG"を色付けして見易くした。（Windowsのコマンドプロンプトには未対応）
- 2022-07-23: r1.4
    - 最初の管理番号

\htmlonly
<a onclick="history.back()">前のページへ戻る</a>
\endhtmlonly
*/

/****************************************************************//**
 * \file  unit_test.h
 *
 * \brief  単体テストを行う／もしくはその機能を実装するモジュール
 *
 * \author   tomyama  May 2022
 * \version  $Revision: 1.12 $
 * \date     $Date: 2023-04-03 03:46:45+09 $
 ********************************************************************/

#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

/********************************************************************/
/*        include files                                             */
/********************************************************************/

#include <stdio.h>
#include <string.h>             // strcmp(), memcpy()
#ifndef _WIN32
 #include <unistd.h>            // pid_t
#endif
#include "tmym_common.h"        // tmym_cmFPrtf()


/********************************************************************/
/*        defines                                                   */
/********************************************************************/

#define UNTE_ASSERT(logic)  unte_assert((logic),__FILE__,__LINE__,__func__,#logic)
#define UNTE_DEF_FUNC(f)    {f,#f,0,0,0}

#define UNTE_SUCCESS    UNTE_FALSE
#define UNTE_FAILURE    UNTE_TRUE

#define EXIT_CODE_IS_ALWAYS_SUCCESS 1

#define UNTE_FAIL_PRT(fmt,...)   \
            tmym_cmFPrtf(stderr," [33mFAIL[0m: %s[%d]: %s(): "fmt,__FILE__,__LINE__,__func__,##__VA_ARGS__);\
            if(pFailReport!=NULL){tmym_cmFPrtf(pFailReport," FAIL: %s[%d]: %s(): "fmt,__FILE__,__LINE__,__func__,##__VA_ARGS__);}

#include "../../test/unit_test_user_define.h"


#ifndef _WIN32
 #define USE_PSEUDO_PIPE_IN 1

 #define PIPE_R 0
 #define PIPE_W 1
 #define PIPE_N 2
#endif /* _WIN32 */


/********************************************************************/
/*        types                                                     */
/********************************************************************/

typedef signed char         unte_s8;
typedef unsigned char       unte_u8;
typedef signed short        unte_s16;
typedef unsigned short      unte_u16;
typedef signed int          unte_s32;
typedef unsigned int        unte_u32;
typedef signed long long    unte_s64;
typedef unsigned long long  unte_u64;

typedef size_t unte_size_t;


#if USE_PSEUDO_PIPE_IN
struct pseudoPipeIn{
    int fdStdin;
    int fdStdinBackup;
    //int fdStdout;
    //int fdStdoutBackup;
    int fdPipe[ PIPE_N ];
};
#endif /* USE_PSEUDO_PIPE_IN */


/********************************************//**
 * \brief  真偽値
 ************************************************/

typedef enum _UNTE_BOOL{
    UNTE_FALSE = 0,
    UNTE_TRUE
} UNTE_BOOL;


/********************************************//**
 * \brief  テスト関数を実装する際の関数プロトタイプ
 *
 * テスト関数を実装する際は、戻り値をvoid、
 * 引数もvoidで実装する事。
 *
 * 例えば、次のような関数を対象にテストする場合、
 *
 * \code
 * int target_func( int a, int b )
 * {
 *     return a + b;
 * }
 * \endcode
 *
 * テスト関数は次のように実装する。
 *
 * \code
 * // 対象関数の関数プロトタイプを記述するかヘッダーファイルをインクルードする
 * extern int target_func( int a, int b );
 *
 * static void test_func1( void )
 * {
 *     int ret;
 *
 *     ret = target_func( 3, 5 );
 *     UNTE_ASSERT( ret == 8 );
 *
 *     return;
 * }
 * \endcode
 *
 ************************************************/

typedef void (*UNTE_TESTFUNC)( void );


/********************************************//**
 * \brief  実装したテスト関数を定義する構造体
 *
 * この構造体は UNTE_DEF_FUNC マクロを使うと容易に定義できる。
 *
 * \code
 * static void test_func1( void );
 * static void test_func2( void );
 * static void test_func3( void );
 * static void test_func4( void );
 *
 * static UNTE_TESTFUNC_S testfuncs[] = {
 *     UNTE_DEF_FUNC(test_func1),
 *     UNTE_DEF_FUNC(test_func2),
 *     UNTE_DEF_FUNC(test_func3),
 *     UNTE_DEF_FUNC(test_func4)
 * };
 * \endcode
 *
 * 上記の構造体定義は、マクロプリプロセッサによって以下のように展開される。
 *
 * \code
 * static UNTE_TESTFUNC_S testfuncs[] = {
 *     {test_func1,"test_func1",0,0,0},
 *     {test_func2,"test_func2",0,0,0},
 *     {test_func3,"test_func3",0,0,0},
 *     {test_func4,"test_func4",0,0,0}
 * };
 * \endcode
 *
 * \sa     _UNTE_PROGRAM_TESTS_S, UNTE_DEF_FUNC
 ************************************************/

typedef struct _UNTE_TESTFUNC_S{
    UNTE_TESTFUNC       pFunc;          //!< \brief テスト関数の関数ポインタ
    const char          *strFuncName;   //!< \brief テスト関数のシンボル名
    unte_s32            cnt_assert;
    unte_s32            cnt_passed;
    unte_s32            cnt_failed;
} UNTE_TESTFUNC_S;


/********************************************//**
 * \brief  モジュール（ソースファイル）ごとのテスト関数の一覧を定義する構造体
 *
 * 定義例
 *
 * 実体を定義する。
 * \code
 * UNTE_MODULE_TESTS_S module_A_tests = {
 *     "Module_A",
 *     ( sizeof testfuncs ) / ( sizeof testfuncs[ 0 ] ),
 *     testfuncs
 * };
 * \endcode
 *
 * 実体のアドレスを配列にする。
 * \code
 * UNTE_MODULE_TESTS_S *pDefineModules[] = {
 *     &module_A_tests,
 *     &module_B_tests,
 *     &module_C_tests
 * };
 * \endcode
 *
 * \sa     _UNTE_PROGRAM_TESTS_S
 ************************************************/

typedef struct _UNTE_MODULE_TESTS_S{
    const char          *strModuleName; //!< \brief 出力時に用いるモジュール名もしくはタイトルを定義
    unte_s32            testNum;
    UNTE_TESTFUNC_S     *pTestFunc;
    unte_s32            cnt_assert;
    unte_s32            cnt_passed;
    unte_s32            cnt_failed;
} UNTE_MODULE_TESTS_S;


/********************************************//**
 * \brief  全てのモジュールの一覧を定義する構造体
 *
 * 定義例
 *
 * \code
 * UNTE_PROGRAM_TESTS_S unte_defineUnitTest = {
 *     ( sizeof pDefineModules ) / ( sizeof pDefineModules[ 0 ] ),
 *     pDefineModules
 * };
 * \endcode
 *
 * \sa     _UNTE_PROGRAM_TESTS_S
 ************************************************/

typedef struct _UNTE_PROGRAM_TESTS_S{
    unte_s32            moduleNum;
    UNTE_MODULE_TESTS_S **pModTests;
    int                 lenModNameMax;
    unte_s32            cnt_assert;
    unte_s32            cnt_passed;
    unte_s32            cnt_failed;
} UNTE_PROGRAM_TESTS_S;


/********************************************************************/
/*        function prototype                                        */
/********************************************************************/

extern UNTE_BOOL unte_assert( UNTE_BOOL expr, const char *pFName, unsigned int line, const char *pFunc, const char *pLogic );
extern unte_u8 unte_sum8( unte_u8 *ptr, unte_size_t size );
#if USE_PSEUDO_PIPE_IN
extern int unte_pseudoPipeInput( char *pFilePath, struct pseudoPipeIn *pPI );
extern int unte_pseudoPipeInputClean( struct pseudoPipeIn *pPI, pid_t pidCld );
#endif /* USE_PSEUDO_PIPE_IN */


/********************************************************************/
/*        globals variable                                          */
/********************************************************************/

extern FILE *pFailReport;


#endif /* _UNIT_TEST_H_ */
