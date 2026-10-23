/****************************************************************//**
 * \file  unit_test.c
 *
 * \brief  単体テストを行う／もしくはその機能を実装するモジュール
 *
 * \author   tomyama  May 2022
 * \version  $Revision: 1.12 $
 * \date     $Date: 2023-04-03 03:46:45+09 $
 ********************************************************************/


/********************************************************************/
/*        include files                                             */
/********************************************************************/

#include <stdio.h>
#include <stdlib.h>     // atoi()
#ifndef _WIN32
 //#include <stdlib.h>    // exit()
 #include <errno.h>
 #include <signal.h>    // signal()
 #include <sys/types.h> // fork()
 #include <sys/wait.h>  // waitpid()
// #include <fcntl.h>
#endif

// C11: 存在する場合のみインクルードする
#ifdef __has_include
 #if __has_include("config.h")
  #include "config.h"
 #endif
#endif

#include "../../test/unit_test.h"
#include "../../test/unit_test_user_define.c"
#include "get_shebang.h"


/********************************************************************/
/*        defines                                                   */
/********************************************************************/

#ifndef REPORT_PREFIX
 #define REPORT_PREFIX "./unittest_"
#endif

#ifndef FAILURE_REPORT
 #define FAILURE_REPORT REPORT_PREFIX "Fail.txt"
#endif

#ifndef RESULT_FILE
 #define RESULT_FILE REPORT_PREFIX "Result.tab"
#endif


#if 0
 #define UT_MSG_OK      "ok"
 #define UT_MSG_NG      "NG"
#else
 #define UT_MSG_OK      UT_MSG_OK_COLOR
 #define UT_MSG_NG      UT_MSG_NG_COLOR
#endif


/********************************************************************/
/*        types                                                     */
/********************************************************************/


/********************************************************************/
/*        function prototype (static)                               */
/********************************************************************/

static void unte_init( UNTE_PROGRAM_TESTS_S *pProgTests );
static void unte_cleanup( UNTE_PROGRAM_TESTS_S *pProgTests );
static void unte_resultPrint( UNTE_PROGRAM_TESTS_S *pProgTests );
static void unte_resultFileOut( UNTE_PROGRAM_TESTS_S *pProgTests );
static void unte_exec_all_test( UNTE_PROGRAM_TESTS_S *pProgTests );
static void unte_exec_test( UNTE_PROGRAM_TESTS_S *pProgTests, int mod_seq );
//static void unte_call_test( void );
static void unte_interactive_test( void );
static void unte_showUsage( void );
static void unte_sighdl_term( int sigNum );


/********************************************************************/
/*        static value                                              */
/********************************************************************/

static const char UT_MSG_OK_COLOR[] = { 0x1B, '[', '3', '2', 'm', 'o', 'k', 0x1B, '[', '3', '9', 'm', '\0' };
static const char UT_MSG_NG_COLOR[] = { 0x1B, '[', '3', '3', 'm', 'N', 'G', 0x1B, '[', '3', '9', 'm', '\0' };


static UNTE_PROGRAM_TESTS_S unte_defineUnitTest = {
    ( sizeof pDefineModules ) / ( sizeof pDefineModules[ 0 ] ),
    pDefineModules
};


static int iNeedToContinueProcessing = 1;

//static ARGUMENT_DEF_S arg_defs[] = {
//    CMDL_ARG_DEF( "-i", NULL        , CMDL_FALSE, unte_interactive_test ),
//    CMDL_ARG_DEF( "-m", NULL        , CMDL_TRUE , unte_call_test ),
//    CMDL_ARG_DEF( "-h", "--help"    , CMDL_FALSE, unte_showUsage )
//};
//#define ARG_DEFS_LEN ( ( sizeof arg_defs ) / ( sizeof arg_defs[ 0 ] ) )


#if USE_PSEUDO_PIPE_IN
static int bCldLoopSw = 1;
#endif /* USE_PSEUDO_PIPE_IN */


/********************************************************************/
/*        globals variable                                          */
/********************************************************************/

FILE *pFailReport = NULL;


/********************************************************************/
/*        function (global)                                         */
/********************************************************************/

int main( int argc, char *argv[] )
{
    unte_init( &unte_defineUnitTest );

//    tmym_caParsingAndProcArg( arg_defs, ARG_DEFS_LEN, argc, argv );

    if( iNeedToContinueProcessing != 0 ){
        unte_exec_all_test( &unte_defineUnitTest );
    }

    unte_cleanup( &unte_defineUnitTest );

#if defined( EXIT_CODE_IS_ALWAYS_SUCCESS ) && ( EXIT_CODE_IS_ALWAYS_SUCCESS != 0 )
    return 0;
#else
    return ( ( unte_defineUnitTest.cnt_failed > 0 ) ? 1 : 0 );
#endif
}


UNTE_BOOL unte_assert( UNTE_BOOL logic, const char *pFName, unsigned int line, const char *pFunc, const char *pLogic )
{
    UNTE_BOOL ret_val = UNTE_FAILURE;

    if( logic ){
        unte_defineUnitTest.cnt_passed++;
        ret_val = UNTE_SUCCESS;
    }else{
        unte_defineUnitTest.cnt_failed++;
        ret_val = UNTE_FAILURE;
        tmym_cmFPrtf( stdout, "[33mFAIL[0m: %s[%d]: %s(): ( %s )\n", pFName, line, pFunc, pLogic );
        if( pFailReport != NULL ){
            tmym_cmFPrtf( pFailReport, "[31mFAIL[0m: %s[%d]: %s(): ( %s )\n", pFName, line, pFunc, pLogic );
        }
    }
    unte_defineUnitTest.cnt_assert++;

    return ret_val;
}


unte_u8 unte_sum8( unte_u8 *ptr, unte_size_t size )
{
    unte_u8 retSum;
    unte_size_t idx;

    retSum = 0;
    for( idx=0; idx<size; idx++ ){
        retSum += *( ptr + idx );
    }

    return retSum;
}


#if USE_PSEUDO_PIPE_IN
int unte_pseudoPipeInput( char *pFilePath, struct pseudoPipeIn *pPI )
{
    int iFuncRes;

    DBGP( "begin: pFilePath = \"%s\"\n", pFilePath );

    pPI->fdStdin = fileno( stdin );
    tmym_cmPrtf( "fdStdin = \"%d\"\n", pPI->fdStdin );
    pPI->fdStdinBackup = dup( pPI->fdStdin );
    tmym_cmPrtf( "fdStdinBackup = \"%d\"\n", pPI->fdStdinBackup );

    //pPI->fdStdout = fileno( stdout );
    //tmym_cmPrtf( "fdStdout = \"%d\"\n", pPI->fdStdout );
    //pPI->fdStdoutBackup = dup( fdStdout );
    //tmym_cmPrtf( "fdStdoutBackup = \"%d\"\n", pPI->fdStdoutBackup );

    if( pipe( pPI->fdPipe ) != 0 ){
        ERRP( "could not create pipe.\n" );
        return 1;
    }

    iFuncRes = fork();
    if( iFuncRes == -1 ){
        ERRP( "fork(): failure\n" );
        return 1;
    }else if( iFuncRes == 0 ){
        // 子プロセス
        close( pPI->fdPipe[ PIPE_R ] );

        int iFileSize;
        FILE *pfin;
        static char chBuff[ 1024 ];
        size_t szBlk;

#ifdef HAVE_FOPEN_S
        errno_t err = fopen_s( &pfin, pFilePath, "rb" );
        if( err != 0 ){
#else
        pfin = fopen( pFilePath, "rb" );
        if( pfin == NULL ){
#endif
            ERRP( "CLD: fopen( \"%s\" )\n", pFilePath );
            exit( 1 );
        }

        iFileSize = 0;
        szBlk = 512;
        bCldLoopSw = 1;
        if( signal( SIGTERM, unte_sighdl_term ) == SIG_ERR ){
            ERRP( "CLD: signal( SIGTERM ): failure\n" );
            exit( 1 );
        }
        while( bCldLoopSw ){
            iFuncRes = fread( chBuff, szBlk, 1, pfin );
            if( iFuncRes != 1 ){
                //tmym_cmPrtf( "errno=%d\n", errno );
                if( ferror( pfin ) ){
                    ERRP( "CLD: Error reading pfin\n" );
                    clearerr( pfin );
                }else if( feof( pfin ) ){
                    if( szBlk > 1 ){
                        szBlk = 1;
                        clearerr( pfin );
                        iFuncRes = fseek( pfin, iFileSize, SEEK_SET );
                        if( iFuncRes != 0 ){
                            ERRP( "CLD: fseek(): failure\n" );
                            break;
                        }
                        continue;
                    }
                    iFuncRes = 0;
                    DBGP( "CLD: pfin: EOF found\n" );
                    clearerr( pfin );
                }
                break;
            }
            iFileSize += szBlk;
            //DBGP( "iFileSize=%d\n", iFileSize );

            // 注：FIFOから読み出してくれないと、FIFOの上限（65536）までしか書けない。
            //     別スレッド、または別プロセスにして、FIFOの書き込み側と読み出し側を
            //     分ける必要がある。
            iFuncRes = write( pPI->fdPipe[ PIPE_W ], chBuff, szBlk );
            if( iFuncRes == -1 ){
                ERRP( "CLD: write() error\n" );
                break;
            }
            //tmym_cmPrtf( "write() = %d\n", iFuncRes );
//            if( sem_post( pPI->pSem ) == -1 ){
//                ERRP( "sem_post(): failure: errno=%d\n", errno );
//                break;
//            }
        }
        close( pPI->fdPipe[ PIPE_W ] );

        if( fclose( pfin ) != 0 ){
            WARN( "CLD: fclose( pfin=\"%s\" )\n", pFilePath );
            //exit( 1 );
        }

//        if( sem_close( pPI->pSem ) != 0 ){
//            ERRP( "CLD: sem_close(): failure: errno=%d\n", errno );
//            return 1;
//        }

        DBGP( "CLD: end: iFuncRes=%d, iFileSize=%d\n", iFuncRes, iFileSize );
        exit( iFuncRes );
    }

    // 親プロセス
    close( pPI->fdPipe[ PIPE_W ] );

    pid_t pidCld;

    pidCld = iFuncRes;

    iFuncRes = dup2( pPI->fdPipe[ PIPE_R ], pPI->fdStdin );
    if( iFuncRes != pPI->fdStdin ){
        ERRP( "PAR: dup2(): failure\n" );
        return -1;
    }
    close( pPI->fdPipe[ PIPE_R ] );
    //dup2( pPI->fdPipe[ PIPE_W ], pPI->fdStdout );

//    if( sem_wait( pPI->pSem ) == -1 ){
//        ERRP( "sem_wait(): failure: errno=%d\n", errno );
//        return -1;
//    }

    return pidCld;
}


int unte_pseudoPipeInputClean( struct pseudoPipeIn *pPI, pid_t pidCld )
{
    int iFuncRes;
    int iCldStatus;
    pid_t pidFuncRes;
    int szBlk;
    static char chBuff[ 512 ];

    // stdinから全て読み切っておく
    szBlk = 512;
    while( ! feof( stdin ) ){
        iFuncRes = fread( chBuff, szBlk, 1, stdin );
        if( iFuncRes != 1 ){
            if( ferror( stdin ) ){
                ERRP( "Error reading stdin\n" );
                clearerr( stdin );
                break;
            }else if( feof( stdin ) ){
                if( szBlk > 1 ){
                    szBlk = 1;
                    clearerr( stdin );
                    continue;
                }
                iFuncRes = 0;
                DBGP( "InputClean(): stdin EOF found\n" );
                clearerr( stdin );
            }
            break;
        }
    }

    // 元に戻しておく
    iFuncRes = dup2( pPI->fdStdinBackup, pPI->fdStdin );
    if( iFuncRes != pPI->fdStdin ){
        ERRP( "dup2(): failure\n" );
        return -1;
    }
    //dup2( pPI->fdStdoutBackup, pPI->fdStdout );
    close( pPI->fdStdinBackup );

    // 子プロセスの回収
    iFuncRes = kill( pidCld, SIGTERM );
    if( iFuncRes != 0 ){
        ERRP( "kill( %d, SIGTERM ): failure\n", pidCld );
        return -1;
    }
    pidFuncRes = waitpid( pidCld, &iCldStatus, 0 );
    if( pidFuncRes == -1 ){
        ERRP( "waitpid( %d ): failure: errno=%d\n", pidCld, errno );
        return -1;
    }
    tmym_cmPrtf( "waitpid( %d ) => %d: exit_status=%d\n",
            pidCld, pidFuncRes,
            ( WIFEXITED( iCldStatus ) ? WEXITSTATUS( iCldStatus ) : -100 ) );

    return 0;
}
#endif /* USE_PSEUDO_PIPE_IN */


/********************************************************************/
/*        function (static)                                         */
/********************************************************************/

static void unte_init( UNTE_PROGRAM_TESTS_S *pProgTests )
{
    int mod_seq, mod_qty;
    UNTE_MODULE_TESTS_S *pModTest;
    int test_seq, test_qty;
    UNTE_TESTFUNC_S *pTestFuncS;
    int lenModuleName;

#ifdef HAVE_FOPEN_S
    errno_t err = fopen_s( &pFailReport, FAILURE_REPORT, "wb" );
    if( err != 0 ){
#else
    pFailReport = fopen( FAILURE_REPORT, "wb" );
    if( pFailReport == NULL ){
#endif
      return;
    }

    pProgTests->lenModNameMax = 11;     // "module_name"
    pProgTests->cnt_assert = 0;
    pProgTests->cnt_passed = 0;
    pProgTests->cnt_failed = 0;

    mod_qty = pProgTests->moduleNum;
    for( mod_seq=0; mod_seq<mod_qty; mod_seq++ ){
        pModTest = *( pProgTests->pModTests + mod_seq );

        lenModuleName = tmym_cmStrLen( pModTest->strModuleName );
        if( lenModuleName > pProgTests->lenModNameMax ){
            pProgTests->lenModNameMax = lenModuleName;
        }

        pModTest->cnt_assert = 0;
        pModTest->cnt_passed = 0;
        pModTest->cnt_failed = 0;

        test_qty = pModTest->testNum;
        for( test_seq=0, pTestFuncS=pModTest->pTestFunc; test_seq<test_qty; test_seq++, pTestFuncS++ ){
            pTestFuncS->cnt_assert = 0;
            pTestFuncS->cnt_passed = 0;
            pTestFuncS->cnt_failed = 0;
        }
    }

    tmym_cmPrtf( "\n" );
    tmym_cmPrtf( "     ##############################\n" );
    tmym_cmPrtf( "     ##                          ##\n" );
    tmym_cmPrtf( "     ## UNIT TEST: Start         ##\n" );
    tmym_cmPrtf( "     ##                          ##\n" );
    tmym_cmPrtf( "     ##############################\n" );
    tmym_cmPrtf( "\n" );

    return;
}


static void unte_cleanup( UNTE_PROGRAM_TESTS_S *pProgTests )
{
    if( pProgTests->cnt_assert > 0 ){
        unte_resultPrint( pProgTests );
        unte_resultFileOut( pProgTests );
    }

    if( pFailReport != NULL ){
        fclose( pFailReport );
        pFailReport = NULL;
    }

    tmym_cmPrtf( "\n" );
    tmym_cmPrtf( "     ##############################\n" );
    tmym_cmPrtf( "     ##                          ##\n" );
    tmym_cmPrtf( "     ## UNIT TEST: End           ##\n" );
    tmym_cmPrtf( "     ##                          ##\n" );
    tmym_cmPrtf( "     ##############################\n" );
    tmym_cmPrtf( "\n" );

    return;
}


static void unte_resultPrint( UNTE_PROGRAM_TESTS_S *pProgTests )
{
    static CM_ALIGN_NUM_S fieldPassed, fieldFailed, fieldAssert, fieldModule;
    int mod_seq, test_seq;
    UNTE_MODULE_TESTS_S *pModTest;
    UNTE_TESTFUNC_S *pTestFuncS;
    static float fSuccessRate;

    tmym_cmSetupAlignNumStruct( &fieldPassed,  4, pProgTests->cnt_passed, CM_ALIGN_NUM_RADIX_10 );
    tmym_cmSetupAlignNumStruct( &fieldFailed,  4, pProgTests->cnt_failed, CM_ALIGN_NUM_RADIX_10 );
    tmym_cmSetupAlignNumStruct( &fieldAssert,  4, pProgTests->cnt_assert, CM_ALIGN_NUM_RADIX_10 );
    tmym_cmSetupAlignNumStruct( &fieldModule, pProgTests->lenModNameMax, 0, CM_ALIGN_NUM_RADIX_10 );

    tmym_cmPrtf( "\n" );
    tmym_cmPrtf( "**************************************************\n" );
    tmym_cmPrtf( "* Summary\n*\n" );
    tmym_cmPrtf( "res.  %s  %s  %s  %s  test_function\n",
        tmym_cmAlignLabel( "pass", &fieldPassed ),
        tmym_cmAlignLabel( "fail", &fieldFailed ),
        tmym_cmAlignLabel( "test", &fieldAssert ),
        tmym_cmAlignLabel( "module_name", &fieldModule ) );
    tmym_cmPrtf( "----  %s  %s  %s  %s  --------------------\n",
        tmym_cmAlignSeparator( '-', &fieldPassed ),
        tmym_cmAlignSeparator( '-', &fieldFailed ),
        tmym_cmAlignSeparator( '-', &fieldAssert ),
        tmym_cmAlignSeparator( '-', &fieldModule ) );

    for( mod_seq=0; mod_seq<pProgTests->moduleNum; mod_seq++ ){
        pModTest = *( pProgTests->pModTests + mod_seq );

        for( test_seq=0; test_seq<pModTest->testNum; test_seq++ ){
            pTestFuncS = pModTest->pTestFunc + test_seq;

            tmym_cmPrtf( " %-2s   %s  %s  %s  %s  %s()\n",
                ( ( pTestFuncS->cnt_assert == 0 ) ? "--" :
                  ( pTestFuncS->cnt_failed > 0 ) ? UT_MSG_NG : UT_MSG_OK ),
                tmym_cmAlignNumber( pTestFuncS->cnt_passed, &fieldPassed ),
                tmym_cmAlignNumber( pTestFuncS->cnt_failed, &fieldFailed ),
                tmym_cmAlignNumber( pTestFuncS->cnt_assert, &fieldAssert ),
                tmym_cmAlignLabel( ( char *)pModTest->strModuleName, &fieldModule ),
                pTestFuncS->strFuncName );
        }
    }

    tmym_cmPrtf( "----  %s  %s  %s  %s  --------------------\n",
        tmym_cmAlignSeparator( '-', &fieldPassed ),
        tmym_cmAlignSeparator( '-', &fieldFailed ),
        tmym_cmAlignSeparator( '-', &fieldAssert ),
        tmym_cmAlignSeparator( '-', &fieldModule ) );
    tmym_cmPrtf( "      %s  %s  %s\n",
        tmym_cmAlignNumber( pProgTests->cnt_passed, &fieldPassed ),
        tmym_cmAlignNumber( pProgTests->cnt_failed, &fieldFailed ),
        tmym_cmAlignNumber( pProgTests->cnt_assert, &fieldAssert ) );

    fSuccessRate = 0.0f;
    if( pProgTests->cnt_assert != 0 ){
        fSuccessRate = ( ( float )pProgTests->cnt_passed / ( float )pProgTests->cnt_assert ) * 100.0f;
    }
    tmym_cmPrtf( "*\n*   %s: Success Rate %9.5f%%  ( %d / %d )\n",
        ( ( fSuccessRate == 100.0 ) ? UT_MSG_OK : UT_MSG_NG ),
        fSuccessRate, pProgTests->cnt_passed, pProgTests->cnt_assert );
    tmym_cmPrtf( "**************************************************\n" );

    return;
}

static void unte_resultFileOut( UNTE_PROGRAM_TESTS_S *pProgTests )
{
    FILE *pResFile = NULL;
    int mod_seq, test_seq;
    UNTE_MODULE_TESTS_S *pModTest;
    UNTE_TESTFUNC_S *pTestFuncS;

#ifdef HAVE_FOPEN_S
    errno_t err = fopen_s( &pResFile, RESULT_FILE, "wb" );
    if( err != 0 ){
#else
    pResFile = fopen( RESULT_FILE, "wb" );
    if( pResFile == NULL ){
#endif
        return;
    }

    tmym_cmFPrtf( pResFile, "res.\tpassed\tfailed\tassert\tmodule_name\ttest_function\n" );

    for( mod_seq=0; mod_seq<pProgTests->moduleNum; mod_seq++ ){
        pModTest = *( pProgTests->pModTests + mod_seq );

        for( test_seq=0; test_seq<pModTest->testNum; test_seq++ ){
            pTestFuncS = pModTest->pTestFunc + test_seq;

            tmym_cmFPrtf( pResFile, "%s\t%d\t%d\t%d\t%s\t%s()\n",
                ( pTestFuncS->cnt_failed > 0 ) ? UT_MSG_NG : UT_MSG_OK,
                pTestFuncS->cnt_passed, pTestFuncS->cnt_failed, pTestFuncS->cnt_assert,
                pModTest->strModuleName, pTestFuncS->strFuncName );
        }
    }

    tmym_cmFPrtf( pResFile, "%s\t%d\t%d\t%d\tTotal\n", ( pProgTests->cnt_failed > 0 ) ? UT_MSG_NG : UT_MSG_OK,
                            pProgTests->cnt_passed, pProgTests->cnt_failed, pProgTests->cnt_assert );

    fclose( pResFile );
    pResFile = NULL;

    return;
}


static void unte_exec_all_test( UNTE_PROGRAM_TESTS_S *pProgTests )
{
    int mod_seq, mod_qty;

//  tmym_cmPrtf( "UNTE_MODULE_TESTS_S : size = %lu\n", sizeof( UNTE_MODULE_TESTS_S ) );
    mod_qty = pProgTests->moduleNum;
//  tmym_cmPrtf( "mod_qty=%d\n", mod_qty );

    for( mod_seq=0; mod_seq<mod_qty; mod_seq++ ){
        unte_exec_test( pProgTests, mod_seq );
    }

    return;
}


static void unte_exec_test( UNTE_PROGRAM_TESTS_S *pProgTests, int mod_seq )
{
    UNTE_MODULE_TESTS_S *pModTest;
    int mod_qty;
    int test_seq, test_qty;
    UNTE_TESTFUNC_S *pTestFuncS;
    float fFailRate;
    unte_s32 _num_test_base, _num_failure_base, _num_success_base;

    pModTest = *( pProgTests->pModTests + mod_seq );
    mod_qty = pProgTests->moduleNum;
//  tmym_cmPrtf( "pModTest: 0x%016lX\n", ( long unsigned int )pModTest );

    pProgTests->cnt_assert -= pModTest->cnt_assert;
    pProgTests->cnt_failed -= pModTest->cnt_failed;
    pProgTests->cnt_passed -= pModTest->cnt_passed;
    pModTest->cnt_assert = 0;
    pModTest->cnt_failed = 0;
    pModTest->cnt_passed = 0;

    tmym_cmPrtf( "\n" );
    tmym_cmPrtf( "**************************************************\n" );
    tmym_cmPrtf( "**\n" );
    tmym_cmPrtf( "** Module[%d/%d]: %s\n", mod_seq+1, mod_qty, pModTest->strModuleName );
    tmym_cmPrtf( "**\n" );
    tmym_cmPrtf( "**************************************************\n" );
    tmym_cmPrtf( "\n" );

    test_qty = pModTest->testNum;
//  tmym_cmPrtf( "Module: %s, test_qty=%d\n", pModTest->strModuleName, test_qty );

    for( test_seq=0, pTestFuncS=pModTest->pTestFunc; test_seq<test_qty; test_seq++, pTestFuncS++ ){
        _num_test_base    = pProgTests->cnt_assert;
        _num_failure_base = pProgTests->cnt_failed;
        _num_success_base = pProgTests->cnt_passed;
        tmym_cmPrtf( "=========+=========+=========+=========+=========+\n" );
        tmym_cmPrtf( "- %s: %03d %s()\n", pModTest->strModuleName, test_seq+1, pTestFuncS->strFuncName );

        pTestFuncS->pFunc();

        pTestFuncS->cnt_assert = ( pProgTests->cnt_assert - _num_test_base    );
        pTestFuncS->cnt_failed = ( pProgTests->cnt_failed - _num_failure_base );
        pTestFuncS->cnt_passed = ( pProgTests->cnt_passed - _num_success_base );
        fFailRate = ( ( pTestFuncS->cnt_assert == 0 ) ?
            0.0f :
            ( ( ( float )pTestFuncS->cnt_failed ) / ( ( float )pTestFuncS->cnt_assert ) * 100.0f ) );
        tmym_cmPrtf( "-  %s: FailRate %.2f%% ( %d / %d )\n",
            ( ( fFailRate == 0.0 ) ? UT_MSG_OK : UT_MSG_NG ),
            fFailRate, pTestFuncS->cnt_failed, pTestFuncS->cnt_assert );

        pModTest->cnt_assert += pTestFuncS->cnt_assert;
        pModTest->cnt_failed += pTestFuncS->cnt_failed;
        pModTest->cnt_passed += pTestFuncS->cnt_passed;
    }
    tmym_cmPrtf( "=========+=========+=========+=========+=========+\n" );

    return;
}


//static void unte_call_test( void )
//{
//    char *pArg;
//    int menuNo, lenMenuNo;
//
//    iNeedToContinueProcessing = 0;
//
//    pArg = tmym_caGetArgumentsArguments( arg_defs, ARG_DEFS_LEN, "-m" );
//    if( pArg == NULL ){
//        UNTE_FAIL_PRT( "pArg == NULL: usage: -m <menu_no>\n" );
//        return;
//    }
//    menuNo = atoi( pArg );
//    lenMenuNo = unte_defineUnitTest.moduleNum;
//    if( ( menuNo <= 0 ) || ( lenMenuNo < menuNo ) ){
//        UNTE_FAIL_PRT( "menuNo=%d: out of range ( 1 - %d )\n", menuNo, lenMenuNo );
//        return;
//    }
//    tmym_cmPrtf( "menuNo = %d\n", menuNo );
//    unte_exec_test( &unte_defineUnitTest, menuNo-1 );
//
//    return;
//}


static void unte_interactive_test( void )
{
    int mod_seq, mod_qty;
    UNTE_MODULE_TESTS_S *pModTest;
    unte_s32 num;

    iNeedToContinueProcessing = 0;

    tmym_cmPrtf( "  menu  module\n" );
    mod_qty = unte_defineUnitTest.moduleNum;
    for( mod_seq=0; mod_seq<mod_qty; mod_seq++ ){
        pModTest = *( unte_defineUnitTest.pModTests + mod_seq );
        tmym_cmPrtf( "  %4d: %s\n", mod_seq+1, pModTest->strModuleName );
    }
    tmym_cmPrtf( "  %4d: %s\n", 0, "EXIT" );
    do{
        tmym_cmPrtf( "select menu[0-%d]: ", mod_qty );
        fflush( stdout );
        num = tmym_cmGetNum( stdin );
    }while( ( num < 0 ) || ( mod_qty < num ) );

    tmym_cmPrtf( "menuNo = %d\n", num );
    if( num == 0 ){
        return;
    }
    unte_exec_test( &unte_defineUnitTest, num-1 );

    return;
}


static void unte_showUsage( void )
{
    iNeedToContinueProcessing = 0;
    tmym_cmPrtf( "usage: program [ -h | -i ]\n" );
    return;
}


#if USE_PSEUDO_PIPE_IN
static void unte_sighdl_term( int sigNum )
{
    //DBGP( "%s(): SIGTERM\n", __FUNCTION__ );
    bCldLoopSw = 0;
    return;
}
#endif /* USE_PSEUDO_PIPE_IN */
