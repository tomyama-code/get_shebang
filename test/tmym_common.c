/****************************************************************//**
 * \file  tmym_common.c
 *
 * \brief  プログラムの共通機能を実装するモジュール
 *
 * - 文字コード・改行コード
 *     - 文字コードは UTF-8(BOM無し)
 *     - 改行コードは LF
 *
 * \author   tomyama  May 2022
 * \version  $Revision: 1.9 $
 * \date     $Date: 2026-10-18 01:53:48+09 $
 *
 * \example unittest/testcase_tmym_common.c
 ********************************************************************/


/********************************************************************/
/*        include files                                             */
/********************************************************************/

#include <stdio.h>
#include <errno.h>
#ifdef _WIN32
 #include <windows.h>   // _isatty(), _fileno(), _get_osfhandle(), GetConsoleMode()
 #include <io.h>
#else
 #include <unistd.h>    // isatty(), fileno()
#endif
#include "tmym_common.h"
#ifndef NO_USE_GET_VERSION
 #include "tmym_getVersion.h"
#endif


/********************************************************************/
/*        defines                                                   */
/********************************************************************/

#define CM_LINEAR_DATA_STRUCT_SIZE ( sizeof( cmConsolidatedData_s ) )

/* MSVC: C4996 */

#ifdef HAVE_STRERROR_S
#define strerror(x) _cm_strerror(x)
#endif

#ifdef HAVE_SPRINTF_S
#define sprintf(buf,fmt,...)    sprintf_s(buf,sizeof(buf),fmt,##__VA_ARGS__)
#endif


/********************************************************************/
/*        types                                                     */
/********************************************************************/

enum cmPrType{
    CM_PR_TYPE_ALOCMEM = 0,
    CM_PR_TYPE_MUTEX,
    CM_PR_TYPE_LEN
};


struct cmProgramResource{
    enum cmPrType eType;
    void *pRsrc;
    size_t sz;
};


/********************************************************************/
/*        function prototype (static)                               */
/********************************************************************/

INTERNAL void *tmym_cmCdGetKey( cmConsolidatedData_s *pCD );
INTERNAL int tmym_cmCdCompare( void *a, void *b );
INTERNAL void tmym_cmCdSwap( cmCdManager_s *pCM, cmConsolidatedData_s *a, cmConsolidatedData_s *b );
INTERNAL void tmym_cmCdBubbleSort( cmCdManager_s *pCM );
INTERNAL void tmym_cmCdListup( cmCdManager_s *pCM, const char *func, void *pNew, void *pGet );
INTERNAL void *tmym_cmPrGetKey( cmConsolidatedData_s *pCD );
INTERNAL int tmym_cmPrClean( struct _cmCdManager *pCM );
INTERNAL int tmym_cmUseColorConsole( FILE *fp );
INTERNAL int tmym_cmDoesItSupportAnsiEscSeq( void );
INTERNAL int tmym_cmRemoveAnsiEscSeq( char *Str );
INTERNAL int tmym_cmVFPrtf( FILE *fp, char *pFmt, va_list argPtr );
INTERNAL TMYM_BOOL tmym_cmIsThisPrintableChar( char ch );

#ifdef HAVE_STRERROR_S
static char *_cm_strerror( int errnum );
#endif


/********************************************************************/
/*        static value                                              */
/********************************************************************/

static TMYM_CM_ENDIAN_E endian = TMYM_UNKNOWN_ENDIAN;


static TMYM_BOOL bCmRegisteredCleanFunc = TMYM_FALSE;
static cmCdManager_s cmCdManAllocMem;


/********************************************************************/
/*        globals variable                                          */
/********************************************************************/


/********************************************************************/
/*        function (global)                                         */
/********************************************************************/

#ifndef NO_USE_GET_VERSION
/********************************************//**
 * \brief  このモジュールのバージョンを保持している関数（RCSで更新する）
 ************************************************/

char *cmmn_getVersion( char **pBuffer, int *pLenBuffer, const char *pPrefix )
{
    const char *pRCSfile  = "$RCSfile: tmym_common.c,v $";
    const char *pRevision = "$Revision: 1.9 $";
    const char *pDate     = "$Date: 2026-10-18 01:53:48+09 $";
    const char *pState    = "$State: Rel $";

    return tmym_gvFormatVersion( pBuffer, pLenBuffer, pPrefix, pRCSfile, pRevision, pDate, pState );
}
#endif

/********************************************//**
 * \brief  「exit()時に tmym_cmMalloc() で確保したメモリの開放漏れをリカバリするための処理」を登録する関数
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

int tmym_cmRegCleanFunc( void (*pFunc)( void ) )
{
    int iRetVal;
    int iFuncRet;

    iRetVal = TMYM_FAILURE;

    //! - 最初に呼び出された時に一度だけ実行
    if( bCmRegisteredCleanFunc == TMYM_FALSE ){
        bCmRegisteredCleanFunc =  TMYM_TRUE;

        //!     - メモリ管理用スタックを初期化
        iFuncRet = tmym_cmCdInit( &cmCdManAllocMem, CM_DATA_TYPE_STACK );
        // LCOV_EXCL_START
        if( iFuncRet != TMYM_SUCCESS ){
            WARN( "tmym_cmCdInit( &cmCdManAllocMem ): failure\n" );
        }
        // LCOV_EXCL_STOP
        //!     - キー関数を変更
        iFuncRet = tmym_cmCdRegFuncGetKey( &cmCdManAllocMem, tmym_cmPrGetKey );
        // LCOV_EXCL_START
        if( iFuncRet != TMYM_SUCCESS ){
            WARN( "tmym_cmCdRegFuncGetKey( &cmCdManAllocMem ): failure\n" );
        }
        // LCOV_EXCL_STOP
        //!     - Clean()関数を変更
        iFuncRet = tmym_cmCdRegFuncClean( &cmCdManAllocMem, tmym_cmPrClean );
        // LCOV_EXCL_START
        if( iFuncRet != TMYM_SUCCESS ){
            WARN( "tmym_cmCdRegFuncClean( &cmCdManAllocMem ): failure\n" );
        }
        // LCOV_EXCL_STOP

        // LCOV_EXCL_START
        //!     - exit()時に実行される処理として pFunc を登録
        if( atexit( pFunc ) != 0 ){
            bCmRegisteredCleanFunc = TMYM_FALSE;
            ERRP( "atexit() is failure\n" );
            return TMYM_FAILURE;
        }
        // LCOV_EXCL_STOP

        iRetVal = TMYM_SUCCESS;
    }

    return iRetVal;
}


/********************************************//**
 * \brief  tmym_cmMalloc(), tmym_cmFree() を管理するスタックの .Clean() メソッドに登録する関数
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

void tmym_cmClean( void )
{
    tmym_cmPrtf( "called: %s()\n", __func__ );
    // LCOV_EXCL_START
    if( cmCdManAllocMem.Clean( &cmCdManAllocMem ) != TMYM_SUCCESS ){
        WARN( "Clean( &cmCdManAllocMem ): failure\n" );
    }
    // LCOV_EXCL_STOP
    return;
}


/********************************************//**
 * \brief  システムのエンディアンを取得するAPI
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

TMYM_CM_ENDIAN_E tmym_cmEndian( void )
{
    if( endian == TMYM_UNKNOWN_ENDIAN ){
        //! - memory image
        //!     - 0x00000001
        //!         - little endian. : 01 00 00 00
        //!         - big endian.    : 00 00 00 01
        int x = 1;
        if( *( char * )&x ){
            endian = TMYM_LITTLE_ENDIAN;
// Android(Termux(ARM7)) で使用している clang では最適化で消えずコードが残ってしまう
#if defined( __LITTLE_ENDIAN__ ) && __LITTLE_ENDIAN__ != 1
        }else{
            endian = TMYM_BIG_ENDIAN;
#endif
        }
    }
    return endian;
}


/********************************************//**
 * \brief  スリープ関数
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmSleep( int milliSec )
{
#ifdef _WIN32
    Sleep( milliSec );

#else /* _WIN32 */
    int iMicroSec;
    int iFuncRet;

    errno = 0;

    iMicroSec = milliSec * 1000;
    iFuncRet = usleep( iMicroSec );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  usleep() が失敗した場合はエラーを返す
        ERRP( "usleep( iMicroSec=%d ): failure\n", iMicroSec );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

#endif /* _WIN32 */

    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  スレッドを生成するAPI
 *
 * \sa \ref subpage_cmmn_thread
 * \param[in]   pHandle  スレッドハンドルのアドレスを指定
 * \param[in]   routine  スレッドのルーチンを指定
 * \param[in,out]  pArg  スレッドルーチンが使用する引数を指定
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmThrdCreate( tmym_thread *pHandle, tmym_thread_routine routine, void *pArg )
{
#ifdef _WIN32
    DWORD dwThreadId;
    *pHandle = CreateThread(
                NULL,           // default security attributes
                0,              // use default stack size
                routine,        // thread function name
                pArg,           // argument to thread function
                0,              // use default creation flags
                &dwThreadId );  // returns the thread identifier
    if( *pHandle == NULL ){
        //! \retval  TMYM_FAILURE  CreateThread() が失敗した場合はエラーを返す
        ERRP( "CreateThread(): failure\n" );
        return TMYM_FAILURE;
    }
    //tmym_cmPrtf( "dwThreadId = %d\n", dwThreadId );

#else /* _WIN32 */
    int iFuncRet;
    iFuncRet = pthread_create( pHandle, NULL, routine, pArg );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_create() が失敗した場合はエラーを返す
        ERRP( "pthread_create(): failure: return=%d\n", iFuncRet );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

#endif /* _WIN32 */

    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  スレッドと待ち合わせをするAPI
 *
 * \sa \ref subpage_cmmn_thread
 * \param[in]   handle     スレッドのハンドルを指定
 * \param[out]  pExitCode  Exit Code を格納する変数のアドレスを指定する。
 *                         NULLが指定された場合は無視する。
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmThrdWait( tmym_thread handle, void **pExitCode )
{
#ifdef _WIN32
    DWORD dwFuncRet, dwExitCode, *pdwExitCode;
    dwFuncRet = WaitForSingleObject( handle, INFINITE );
    if( dwFuncRet != WAIT_OBJECT_0 ){
        //! \retval  TMYM_FAILURE  WaitForSingleObject() が失敗した場合はエラーを返す
        ERRP( "WaitForSingleObject(): failure\n" );
        return TMYM_FAILURE;
    }

    if( pExitCode != NULL ){
        if( GetExitCodeThread( handle, &dwExitCode ) == 0 ){
            //! \retval  TMYM_FAILURE  GetExitCodeThread() が失敗した場合はエラーを返す
            ERRP( "GetExitCodeThread(): failure\n" );
            return TMYM_FAILURE;
        }
        pdwExitCode = ( DWORD *)pExitCode;
        *pdwExitCode = dwExitCode;
    }

    if( CloseHandle( handle ) == 0 ){
        //! \retval  TMYM_FAILURE  CloseHandle() が失敗した場合はエラーを返す
        ERRP( "CloseHandle(): failure\n" );
        return TMYM_FAILURE;
    }

#else /* _WIN32 */
    int iFuncRet;
    iFuncRet = pthread_join( handle, pExitCode );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_join() が失敗した場合はエラーを返す
        ERRP( "pthread_join(): failure: return=%d\n", iFuncRet );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP
    //DBGP( "Thread return %d\n", *pExitCode );

#endif /* _WIN32 */

    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  ミューテックスを初期化する
 * \sa \ref subpage_cmmn_mutex
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmMtxInit( cmMutexResource_s *pMR )
{

#ifdef _WIN32
//! - _WIN32 の場合
    //!     - Create a mutex with no initial owner
    pMR->pMtx = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL );            // unnamed mutex
    if( pMR->pMtx == NULL ){
        //! \retval  TMYM_FAILURE  CreateMutex() が失敗した場合はエラーを返す
        ERRP( "CreateMutex error: %ld\n", GetLastError() );
        return TMYM_FAILURE;
    }
    //DBGP( "CreateMutex() = 0x%08X\n", ( unsigned int )pMR->pMtx );

#else /* _WIN32 */
//! - _WIN32 以外 の場合
    int iFuncRet;

    //!     - mutex の attribute をセットアップする

    pMR->pMtxAttr = &pMR->mutexAttr;

    iFuncRet = pthread_mutexattr_init( pMR->pMtxAttr );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutexattr_init() が失敗した場合はエラーを返す
        ERRP( "pthread_mutexattr_init(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

    iFuncRet = pthread_mutexattr_settype( pMR->pMtxAttr, PTHREAD_MUTEX_NORMAL );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutexattr_settype() が失敗した場合はエラーを返す
        ERRP( "pthread_mutexattr_settype(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

    //!     - attribute を使って mutex を初期化する

    pMR->pMtx = &pMR->mutex;

    iFuncRet = pthread_mutex_init( pMR->pMtx, pMR->pMtxAttr );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutex_init() が失敗した場合はエラーを返す
        ERRP( "pthread_mutex_init(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

#endif /* _WIN32 */

    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  ミューテックスを破棄する
 * \sa \ref subpage_cmmn_mutex
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmMtxDestroy( cmMutexResource_s *pMR )
{
    DBGP( "Begin\n" );

#ifdef _WIN32
//! - _WIN32 の場合
    BOOL bFuncRet;

    //!     - Close mutex handles
    bFuncRet = CloseHandle( pMR->pMtx );  // 関数が成功すると、戻り値は 0 以外
    if( bFuncRet == 0 ){
        //! \retval  TMYM_FAILURE  CloseHandle() が失敗した場合はエラーを返す
        ERRP( "CloseHandle(): failre\n" );
        return TMYM_FAILURE;
    }

#else /* _WIN32 */
//! - _WIN32 以外 の場合
    int iFuncRet;

    //!     - mutex を破棄する
    iFuncRet = pthread_mutex_destroy( pMR->pMtx );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutex_destroy() が失敗した場合はエラーを返す
        ERRP( "pthread_mutex_destroy(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP
    pMR->pMtx = NULL;

    //!     - mutex の attribute を破棄する
    iFuncRet = pthread_mutexattr_destroy( pMR->pMtxAttr );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutexattr_destroy() が失敗した場合はエラーを返す
        ERRP( "pthread_mutexattr_destroy(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP
    pMR->pMtxAttr = NULL;

#endif /* _WIN32 */

    DBGP( "End\n" );
    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  ミューテックスをロックする
 * \sa \ref subpage_cmmn_mutex
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmMtxLock( cmMutexResource_s *pMR )
{
#ifdef _WIN32
//! - _WIN32 の場合
    DWORD dwFuncRet;

    //!     - Request ownership of mutex.
    dwFuncRet = WaitForSingleObject(
        pMR->pMtx,   // handle to mutex
        INFINITE );  // no time-out interval
    if( dwFuncRet != WAIT_OBJECT_0 ){
        //! \retval  TMYM_FAILURE  WaitForSingleObject() が失敗した場合はエラーを返す
        ERRP( "WaitForSingleObject() = %d: mutext lock failure\n", dwFuncRet );
        return TMYM_FAILURE;
    }

#else /* _WIN32 */
//! - _WIN32 以外 の場合
    int iFuncRet;

    //!     - mutex をロックする
    iFuncRet = pthread_mutex_lock( pMR->pMtx );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutex_lock() が失敗した場合はエラーを返す
        ERRP( "pthread_mutex_lock(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

#endif /* _WIN32 */

    //DBGP( "Lock\n" );
    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  ミューテックスをアンロックする
 * \sa \ref subpage_cmmn_mutex
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmMtxUnlock( cmMutexResource_s *pMR )
{
#ifdef _WIN32
//! - _WIN32 の場合
    DWORD dwFuncRet;

    //!     - mutex をアンロックする
    dwFuncRet = ReleaseMutex( pMR->pMtx );  //!         - 関数が成功すると、戻り値は 0 以外
    if( dwFuncRet == 0 ){
        //! \retval  TMYM_FAILURE  ReleasedMutex() が失敗した場合はエラーを返す
        ERRP( "ReleaseMutex() = %d: mutext unlock failure\n", dwFuncRet );
        return TMYM_FAILURE;
    }

#else /* _WIN32 */
//! - _WIN32 以外 の場合
    int iFuncRet;

    //!     - mutex をアンロックする
    iFuncRet = pthread_mutex_unlock( pMR->pMtx );
    // LCOV_EXCL_START
    if( iFuncRet != 0 ){
        //! \retval  TMYM_FAILURE  pthread_mutex_unlock() が失敗した場合はエラーを返す
        ERRP( "pthread_mutex_unlock(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

#endif /* _WIN32 */

    //DBGP( "Unlock\n" );
    //! \retval  TMYM_SUCCESS  関数が成功した場合はSUCCESSを返す
    return TMYM_SUCCESS;
}


    /* Manage */

/********************************************//**
 * \brief  管理データを初期化するAPI
 *
 * 初期化済みの更新データを更に初期化して使う事を許容する。
 * ただしその場合は、残っている連結データは .Clean() を用いて削除される。
 *
 * \sa \ref subpage_cmmn_consolidated_data
 * \param[out]  pCM   管理データのポインタを指定
 * \param[in]   type  連結データのタイプ
 * \return     関数の成否を返す
 ************************************************/

int tmym_cmCdInit( cmCdManager_s *pCM, cmCdType_e type )
{
    int iFuncRet;

    errno = 0;

    if( ( ( pCM->type == CM_DATA_TYPE_STACK ) && ( pCM->Add == tmym_cmCdPush    ) && ( pCM->Eject == tmym_cmCdPop   ) ) ||
        ( ( pCM->type == CM_DATA_TYPE_LIST  ) && ( pCM->Add == tmym_cmCdUnshift ) && ( pCM->Eject == tmym_cmCdShift ) ) ||
        ( ( pCM->type == CM_DATA_TYPE_FIFO  ) && ( pCM->Add == tmym_cmCdPush    ) && ( pCM->Eject == tmym_cmCdShift ) ) ||
        ( ( pCM->type == CM_DATA_TYPE_SORT  ) && ( pCM->Add == tmym_cmCdInsert  ) ) ){

        if( ( pCM->iCdNums > 0 ) && ( pCM->pHead != NULL ) && ( pCM->pTail != NULL ) ){

            WARN( "pCD is not empty.\n" );
            tmym_cmCdListup( pCM, __func__, NULL, NULL );
            pCM->Clean( pCM );
        }

        tmym_cmMtxDestroy( &pCM->mutexRsrc );
    }

    pCM->pHead = NULL;
    pCM->pTail = NULL;
    pCM->iCdNums = 0;
    pCM->type = type;
    switch( pCM->type ){
    case CM_DATA_TYPE_STACK:
        tmym_cmCdRegFuncGetKey( pCM, NULL );
        pCM->Add   = tmym_cmCdPush;
        pCM->Eject = tmym_cmCdPop;
        pCM->Sort  = tmym_cmCdBubbleSort;
        tmym_cmCdRegFuncCompare( pCM, NULL );
        tmym_cmCdRegFuncClean( pCM, NULL );
        break;
    case CM_DATA_TYPE_LIST:
        tmym_cmCdRegFuncGetKey( pCM, NULL );
        pCM->Add   = tmym_cmCdUnshift;
        pCM->Eject = tmym_cmCdShift;
        pCM->Sort  = tmym_cmCdBubbleSort;
        tmym_cmCdRegFuncCompare( pCM, NULL );
        tmym_cmCdRegFuncClean( pCM, NULL );
        break;
    case CM_DATA_TYPE_FIFO:
        tmym_cmCdRegFuncGetKey( pCM, NULL );
        pCM->Add   = tmym_cmCdPush;
        pCM->Eject = tmym_cmCdShift;
        pCM->Sort  = tmym_cmCdBubbleSort;
        tmym_cmCdRegFuncCompare( pCM, NULL );
        tmym_cmCdRegFuncClean( pCM, NULL );
        break;
    case CM_DATA_TYPE_SORT:
        tmym_cmCdRegFuncGetKey( pCM, NULL );
        pCM->Add   = tmym_cmCdInsert;
        pCM->Eject = tmym_cmCdShift;
        pCM->Sort  = tmym_cmCdBubbleSort;
        tmym_cmCdRegFuncCompare( pCM, NULL );
        tmym_cmCdRegFuncClean( pCM, NULL );
        break;
    default:
        //! \retval  TMYM_FAILURE  type に不正な値が指定された場合はエラーを返す
        ERRP( "type=%d: unknown type\n", type );
        pCM->type = CM_DATA_TYPE_UNKNOWN;
        return TMYM_FAILURE;
    }

    iFuncRet = tmym_cmMtxInit( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet == TMYM_FAILURE ){
        //! \retval  TMYM_FAILURE  ミューテックスの初期化に失敗した場合はエラーを返す
        ERRP( "tmym_cmMtxInit(): failure\n" );
        return TMYM_FAILURE;
    }
    // LCOV_EXCL_STOP

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, NULL, NULL );
#endif
    //! \retval  TMYM_SUCCESS  関数の成功時にはSUCCESSを返す
    return TMYM_SUCCESS;
}

/********************************************//**
 * \brief  管理データを破棄するAPI
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

int tmym_cmCdDestroy( cmCdManager_s *pCM )
{
    int iFuncRet;

    iFuncRet = tmym_cmMtxDestroy( &pCM->mutexRsrc );

    return iFuncRet;
}


/********************************************//**
 * \brief  管理データの .Clean() メソッドに登録するデフォルトの関数
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

int tmym_cmCdClean( struct _cmCdManager *pCM )
{
    if( tmym_cmCdGetLen( pCM ) > 0 ){
        while( pCM->Eject( pCM ) != NULL );
    }

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, NULL, NULL );
#endif
    return TMYM_SUCCESS;
}


/********************************************//**
 * \brief  連結データのデータ数を取得するAPI
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

int tmym_cmCdGetLen( cmCdManager_s *pCM )
{
//    cmConsolidatedData_s *pCD;
//    int iRetNum;
//
//    if( pCM->pHead == NULL ){
//        return 0;
//    }
//    pCD = pCM->pHead;
//    iRetNum = 0;
//    while( pCD != NULL ){
//        iRetNum++;
//        pCD = pCD->pNext;
//    }
//
//    return iRetNum;
    return pCM->iCdNums;
}


/********************************************//**
 * \brief  管理データの .Clean() メソッドに登録するAPI
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

int tmym_cmCdRegFuncClean( cmCdManager_s *pCM, int(*Clean)(struct _cmCdManager*) )
{
    pCM->Clean = Clean;
    if( Clean == NULL ){
        // default
        pCM->Clean = tmym_cmCdClean;
    }
    return TMYM_SUCCESS;
}


int tmym_cmCdRegFuncGetKey( cmCdManager_s *pCM, void*(*GetKey)(cmConsolidatedData_s*) )
{
    pCM->GetKey = GetKey;
    if( GetKey == NULL ){
        // default
        pCM->GetKey = tmym_cmCdGetKey;
    }
    return TMYM_SUCCESS;
}


int tmym_cmCdRegFuncCompare( cmCdManager_s *pCM, int(*Compare)(void*,void*) )
{
    pCM->Compare = Compare;
    if( Compare == NULL ){
        // default
        pCM->Compare = NULL;
        if( pCM->type == CM_DATA_TYPE_SORT ){
            pCM->Compare = tmym_cmCdCompare;
        }
    }
    return TMYM_SUCCESS;
}


    /* Stack */

/********************************************//**
 * \brief  連結データの最後尾に新しいアイテムを積む
 *
 * \sa \ref subpage_cmmn_consolidated_data
 * \param[in]  pCM       連結データへの参照を管理する構造体
 * \param[in]  pNewItem  連結データ（ \ref _cmConsolidatedData ）の pData にセットする値（値もしくはデータへの参照をセットして使う）
 * \return     関数の成否を返す
 * \retval     TMYM_SUCCESS  Success
 * \retval     TMYM_FAILURE  Failure
 * \author     tomyama
 ************************************************/

int tmym_cmCdPush( cmCdManager_s *pCM, void *pNewItem )
{
    int iFuncRet;
    cmConsolidatedData_s *pNewCD, *pPrevCD;

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    pPrevCD = pCM->pTail;
    pNewCD = malloc( CM_LINEAR_DATA_STRUCT_SIZE );
    if( pNewCD == NULL ){
        //! - 新しいアイテムの連結データの領域を確保できなかった場合はエラーを返す
        // LCOV_EXCL_START
        ERRP( "malloc( %lu ) is failure\n", CM_LINEAR_DATA_STRUCT_SIZE );
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return TMYM_FAILURE;
        // LCOV_EXCL_STOP
    }
    pNewCD->pPrev = pPrevCD;
    pNewCD->pNext = NULL;
    pNewCD->pData = pNewItem;

    if( pPrevCD == NULL ){
        pCM->pHead = pNewCD;
    }else{
        pPrevCD->pNext = pNewCD;
    }
    pCM->pTail = pNewCD;

    pCM->iCdNums++;

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, pNewItem, NULL );
#endif
    return TMYM_SUCCESS;
}


void *tmym_cmCdPop( cmCdManager_s *pCM )
{
    int iFuncRet;
    cmConsolidatedData_s *pCD, *pPrevCD;
    void *pRetPtr;

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    if( pCM->pTail == NULL ){
        ERRP( "pCD is empty\n" );
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return NULL;
    }

    pCD = pCM->pTail;
    pPrevCD = pCD->pPrev;

    if( pPrevCD == NULL ){
        pCM->pHead = NULL;
    }else{
        pPrevCD->pNext = NULL;
    }
    pCM->pTail = pPrevCD;

    pRetPtr = pCD->pData;

    free( pCD );

    pCM->iCdNums--;

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, NULL, pRetPtr );
#endif
    return pRetPtr;
}


    /* Common */
void *tmym_cmCdRemove( cmCdManager_s *pCM, void *pRmKey )
{
    cmConsolidatedData_s *pCD;
    void *pRetPtr;
    int iFuncRet;

    pRetPtr = NULL;

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    if( pCM->pHead == NULL ){
        ERRP( "pCD is empty: pRmKey=%p\n", pRmKey );
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return NULL;
    }

    pCD = pCM->pHead;
    do{
        if( pCM->GetKey( pCD ) == pRmKey ){
            if( pCD->pPrev == NULL ){
                pCM->pHead = pCD->pNext;
            }else{
                pCD->pPrev->pNext = pCD->pNext;
            }
            if( pCD->pNext == NULL ){
                pCM->pTail = pCD->pPrev;
            }else{
                pCD->pNext->pPrev = pCD->pPrev;
            }
            pRetPtr = pCD->pData;
            free( pCD );
            pCM->iCdNums--;
            break;
        }
    }while( ( pCD = pCD->pNext ) != NULL );

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, pRmKey, NULL );
#endif
    return pRetPtr;
}


    /* List */
int tmym_cmCdUnshift( cmCdManager_s *pCM, void *pNewItem )
{
    int iFuncRet;
    cmConsolidatedData_s *pNewCD, *pNextCD;

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    pNextCD = pCM->pHead;
    pNewCD = malloc( CM_LINEAR_DATA_STRUCT_SIZE );
    if( pNewCD == NULL ){
        // LCOV_EXCL_START
        ERRP( "malloc( %lu ) is failure\n", CM_LINEAR_DATA_STRUCT_SIZE );
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return TMYM_FAILURE;
        // LCOV_EXCL_STOP
    }
    pNewCD->pPrev = NULL;
    pNewCD->pNext = pNextCD;
    pNewCD->pData = pNewItem;

    if( pNextCD == NULL ){
        pCM->pTail = pNewCD;
    }else{
        pNextCD->pPrev = pNewCD;
    }
    pCM->pHead = pNewCD;

    pCM->iCdNums++;

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, pNewItem, NULL );
#endif
    return TMYM_SUCCESS;
}


void *tmym_cmCdShift( cmCdManager_s *pCM )
{
    int iFuncRet;
    cmConsolidatedData_s *pCD, *pNextCD;
    void *pRetPtr;

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    if( pCM->pHead == NULL ){
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return NULL;
    }

    pCD = pCM->pHead;
    pNextCD = pCD->pNext;

    if( pNextCD == NULL ){
        pCM->pTail = NULL;
    }else{
        pNextCD->pPrev = NULL;
    }
    pCM->pHead = pNextCD;

    pRetPtr = pCD->pData;

    free( pCD );

    pCM->iCdNums--;

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, NULL, pRetPtr );
#endif
    return pRetPtr;
}


    /* Sort */
int tmym_cmCdInsert( cmCdManager_s *pCM, void *pNewItem )
{
    int iFuncRet;
    cmConsolidatedData_s *pNewCD, *pMoveCD, *pPrevCD;

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    pNewCD = malloc( CM_LINEAR_DATA_STRUCT_SIZE );
    if( pNewCD == NULL ){
        // LCOV_EXCL_START
        ERRP( "malloc( %lu ) is failure\n", CM_LINEAR_DATA_STRUCT_SIZE );
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return TMYM_FAILURE;
        // LCOV_EXCL_STOP
    }

    pNewCD->pData = pNewItem;

    if( pCM->pHead == NULL ){
        pCM->pHead = pNewCD;
        pCM->pTail = pNewCD;
        pNewCD->pPrev = NULL;
        pNewCD->pNext = NULL;
    }else{
        pMoveCD = pCM->pHead;
        pPrevCD = NULL;
        do{
            if( pCM->Compare( pCM->GetKey( pNewCD ), pCM->GetKey( pMoveCD ) ) < 0 ){
                break;
            }
            pPrevCD = pMoveCD;
            pMoveCD = pMoveCD->pNext;
        }while( pMoveCD != NULL );
        if( pMoveCD == NULL ){
            pNewCD->pNext = NULL;
            pCM->pTail = pNewCD;
        }else{
            pNewCD->pNext = pMoveCD;
            pMoveCD->pPrev = pNewCD;
        }
        if( pPrevCD == NULL ){
            pNewCD->pPrev = NULL;
            pCM->pHead = pNewCD;
        }else{
            pNewCD->pPrev = pPrevCD;
            pPrevCD->pNext = pNewCD;
        }
    }

    pCM->iCdNums++;

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, pNewItem, NULL );
#endif
    return TMYM_SUCCESS;
}


char *tmym_cmGetLine( FILE *fp, TMYM_BOOL bDiscarding )
{
    #define LEN_GET_LINE_BUFF 1024
    //#define LEN_GET_LINE_BUFF 5
    static char _cmGetLineBuff[ LEN_GET_LINE_BUFF ];
    char *pRetPtr;
    int lenStr;
    char *pLastCh;

    errno = 0;

    memset( _cmGetLineBuff, '\0', LEN_GET_LINE_BUFF );
    pRetPtr = fgets( _cmGetLineBuff, LEN_GET_LINE_BUFF, fp );
    if( pRetPtr == NULL ){
        return NULL;
    }

    lenStr = tmym_cmStrLen( pRetPtr );
    pLastCh = &( _cmGetLineBuff[ lenStr - 1 ] );
    if( *pLastCh == '\n' ){
        *pLastCh = '\0';
    }else{
        errno = ENOBUFS;
        if( bDiscarding == TMYM_TRUE ){
            // 1行読み切っておく
            while( getchar() != '\n' );
        }
    }

    return pRetPtr;
}


char *tmym_cmGetStr( FILE *fp )
{
    char *pGetLine, *pCleanStr;

    pGetLine = tmym_cmGetLine( fp , TMYM_TRUE );
    if( pGetLine == NULL ){
        //if( errno == 0 ){
        //    errno = EIO;
        //}
        return NULL;
    }
    pCleanStr = tmym_cmStrClean( pGetLine, CM_STRCLN_GENERAL );

    return pCleanStr;
}


char tmym_cmGetOneChar( FILE *fp )
{
    char *pCleanStr;
    char chRetVal;
    int lenStr;

    errno = 0;

    pCleanStr = tmym_cmGetStr( fp );
    if( pCleanStr == NULL ){
        return '\0';
    }
    if( errno != 0 ){
        WARN( "errno=%d\n", errno );
        // return through
    }

    lenStr = tmym_cmStrLen( pCleanStr );
    if( lenStr == 0 ){
        errno = EINVAL;
        return '\0';
    }else if( lenStr > 1 ){
        WARN( "pCleanStr is long: length=%d\n", lenStr );
        // return through
    }

    chRetVal = *pCleanStr;
    if( tmym_cmIsThisPrintableChar( chRetVal ) == TMYM_FALSE ){
        errno = EILSEQ;
        ERRP( "invalid input: 0x%02X\n", chRetVal & 0xFF );
        return '\0';
    }

    return chRetVal;
}


tmym_s32 tmym_cmGetNum( FILE *fp )
{
    char *pCleanStr, *pCh;
    tmym_s32 retVal;
    TMYM_BOOL bNegSign;
    int ch;

    errno = 0;

    pCleanStr = tmym_cmGetStr( fp );
    if( pCleanStr == NULL ){
        return 0;
    }

    retVal = 0;
    bNegSign = TMYM_ERROR;
    pCh = pCleanStr;
    while( ( ch = *pCh++ ) != '\0' ){
        if( ( '0' <= ch ) && ( ch <= '9' ) ){
            if( bNegSign == TMYM_ERROR ){
                bNegSign = TMYM_FALSE;
            }
            retVal = ( retVal * 10 ) + ( ch - '0' );
        }else if( ( bNegSign == TMYM_ERROR ) && ( ch == '-' ) ){
            bNegSign = TMYM_TRUE;
        }else if( ( bNegSign == TMYM_ERROR ) && ( ch == '+' ) ){
            bNegSign = TMYM_FALSE;
        }else{
            WARN( "ch=0x%02X: out of range ( 0x%02X - 0x%02X )\n", ch&0xFF, '0', '9' );
            errno = EILSEQ;
            continue;
        }
        //DBGP( "0x%02X, retVal=%d\n", ch & 0xFF, retVal );
    }

    if( bNegSign == TMYM_TRUE ){
        retVal *= -1;
    }
    return retVal;
}


tmym_s32 tmym_cmStrLen( const char *pStr )
{
    tmym_s32 len;
    int idx;

    len = 0;
    for( idx=0; idx<TMYM_PATH_MAX; idx++ ){
        if( *( pStr + idx ) == '\0' ){
            len = idx;
            break;
        }
    }

    return len;
}


// 行頭の空白を削除
char *tmym_cmStrRemoveLeadingWhiteSpace( char *pStr )
{
    while( ( *pStr == ' ' ) || ( *pStr == '\t' ) ){
        pStr++;
    }
    return pStr;
}


// 行末の空白を削除
char *tmym_cmStrRemoveTrailingWhiteSpace( char *pStr )
{
    int lenStr;
    char *pLastChar;

    lenStr = tmym_cmStrLen( pStr );
    pLastChar = ( pStr + lenStr - 1 );
    while( ( *pLastChar == ' ' ) || ( *pLastChar == '\t' ) ){
        *pLastChar-- = '\0';
        if( --lenStr == 0 ){
            break;
        }
    }

    return pStr;
}


// バックスペースを削除する
int tmym_cmStrRemoveBackSpace( char *pStr )
{
    #define CM_ASC_BACKSPACE 0x08
    //#define CM_ASC_DELETE    0x7F
    char *pPtr, *pCurCh;
    int lenEditStr;

    pPtr = pCurCh = pStr;
    lenEditStr = 0;
    while( *pCurCh != '\0' ){
        if( *pCurCh == CM_ASC_BACKSPACE ){
            while( *pCurCh == CM_ASC_BACKSPACE ){
                if( lenEditStr > 0 ){
                    lenEditStr -= 1;
                    pPtr--;
                    //DBGP( "Delete: 0x%02X '%c': lenEditStr=%d\n", *pPtr & 0xFF, *pPtr, lenEditStr );
                }
                pCurCh++;
            }
        }
        if( pPtr < pCurCh ){
            //DBGP( "*pPtr[0x%02X] <= *pCurCh[%02X]: lenEditStr=%d\n", *pPtr & 0xFF, *pCurCh & 0xFF, lenEditStr );
            *pPtr = *pCurCh;
            if( *pCurCh == '\0' ){
                lenEditStr--;
            }
        }
        lenEditStr++;
        pPtr++;
        pCurCh++;
    }
    *pPtr = '\0';

    return lenEditStr;
}


char *tmym_cmStrClean( char *pStr, TMYM_CM_STRCLN_E action )
{
    #define LEN_STR_CLEAN_BUFF 1024
    static char _cmStrCleanBuff[ LEN_STR_CLEAN_BUFF ];
    int lenSrcStr;
    char *pRetPtr;

    //memset( _cmStrCleanBuff, '\0', LEN_STR_CLEAN_BUFF );
    lenSrcStr = tmym_cmStrLen( pStr );
    memcpy( _cmStrCleanBuff, pStr, lenSrcStr + 1 );

    pRetPtr = _cmStrCleanBuff;
    if( action & CM_STRCLN_REMOVE_BACKSPACE ){
        tmym_cmStrRemoveBackSpace( pRetPtr );
    }
    if( action & CM_STRCLN_REMOVE_ANSIESCSEQ ){
        tmym_cmRemoveAnsiEscSeq( pRetPtr );
    }
    if( action & CM_STRCLN_REMOVE_LEADING_WHITESPACE ){
        pRetPtr = tmym_cmStrRemoveLeadingWhiteSpace( pRetPtr );
    }
    if( action & CM_STRCLN_REMOVE_TRAILING_WHITESPACE ){
        pRetPtr = tmym_cmStrRemoveTrailingWhiteSpace( pRetPtr );
    }

    return pRetPtr;
}


void tmym_cmReverseOrder( tmym_u8 *pU8, tmym_size_t size )
{
    int idx1, idx2;
    tmym_u8 u8taihi, *p1U8, *p2U8;
    for( idx1=0, idx2=((int)size-1); idx1<idx2; idx1++, idx2-- ){
        p1U8 = ( pU8 + idx1 );
        p2U8 = ( pU8 + idx2 );
        u8taihi = *p1U8;
        *p1U8   = *p2U8;
        *p2U8   = u8taihi;
    }
    return;
}


int tmym_cmSetupAlignNumStruct( CM_ALIGN_NUM_S *pInfo, int numberOfDigits, tmym_size_t numberMax, CM_ALIGN_NUM_RADIX_E radix )
{
    int lenOutStr;

    errno = 0;

    pInfo->radix = radix;
    pInfo->numberOfDigits = 0;

    if( numberOfDigits > CM_ALIGN_NUM_OF_DIGITS_MAX ){
        WARN( "specified %d for numberOfDigits, but %d is used\n",
            numberOfDigits, CM_ALIGN_NUM_OF_DIGITS_MAX );
        numberOfDigits = CM_ALIGN_NUM_OF_DIGITS_MAX;
    }

    switch( pInfo->radix & CM_ALIGN_NUM_RADIX_MASK ){
    case CM_ALIGN_NUM_RADIX_8_BIT:
        sprintf( pInfo->outBuf, "%lo", ( unsigned long )numberMax );
        break;
    case CM_ALIGN_NUM_RADIX_10_BIT:
        sprintf( pInfo->outBuf, "%lu", ( unsigned long )numberMax );
        break;
    case CM_ALIGN_NUM_RADIX_16U_BIT:
    case CM_ALIGN_NUM_RADIX_16L_BIT:
        sprintf( pInfo->outBuf, "%lX", ( unsigned long )numberMax );
        break;
    default:
        ERRP( "invalid argument: radix=0x%04X\n", radix );
        pInfo->radix = 0;
        return TMYM_FAILURE;
    }

    lenOutStr = tmym_cmStrLen( pInfo->outBuf );
    pInfo->numberOfDigits = ( ( lenOutStr > numberOfDigits ) ? lenOutStr : numberOfDigits );

    //DBGP( "pInfo{ outBuf=\"%s\"[addr:0x%08X,size:%d], radix=0x%04X, numberOfDigits=%d }\n",
    //      pInfo->outBuf, &( pInfo->outBuf[ 0 ] ), CM_ALIGN_NUM_BUFF_SIZE,
    //      pInfo->radix, pInfo->numberOfDigits );
    memset( &( pInfo->outBuf[ 0 ] ), '\0', CM_ALIGN_NUM_BUFF_SIZE );

    return TMYM_SUCCESS;
}


char *tmym_cmAlignNumber( tmym_size_t number, CM_ALIGN_NUM_S *pInfo )
{
    int lenOutStr, iMissingDigits;
    char *pCh;
    char chPadding;
    int idx;

    switch( pInfo->radix & CM_ALIGN_NUM_RADIX_MASK ){
    case CM_ALIGN_NUM_RADIX_8_BIT:
        sprintf( pInfo->outBuf, "%lo", ( unsigned long )number );
        break;
    case CM_ALIGN_NUM_RADIX_10_BIT:
        sprintf( pInfo->outBuf, "%lu", ( unsigned long )number );
        break;
    case CM_ALIGN_NUM_RADIX_16U_BIT:
        sprintf( pInfo->outBuf, "%lX", ( unsigned long )number );
        break;
    case CM_ALIGN_NUM_RADIX_16L_BIT:
        sprintf( pInfo->outBuf, "%lx", ( unsigned long )number );
        break;
    default:
        ERRP( "invalid argument: pInfo->radix=0x%04X\n", pInfo->radix );
        pInfo->outBuf[ 0 ] = '\0';
        return NULL;
    }

    lenOutStr = tmym_cmStrLen( pInfo->outBuf );
    //DBGP( "lenOutStr=%d, outBuf=\"%s\"\n", lenOutStr, pInfo->outBuf );
    if( lenOutStr < pInfo->numberOfDigits ){
        iMissingDigits = pInfo->numberOfDigits - lenOutStr;
        //DBGP( "iMissingDigits=%d, lenOutStr=%d\n", iMissingDigits, lenOutStr );
        //memcpy( ( pInfo->outBuf + iMissingDigits ), pInfo->outBuf, lenOutStr + 1 );
        // コピーする領域が重なっていて且つ後ろ方向にずらす操作の場合、clangだとうまくコピーできない
        //  4  0 \0  x  x
        //        ↓
        //  4  0  4  0  4 <- '\0' ではなく書き換え後の 4 をコピーしてしまう（null終端文字列だと破壊された状態）
        // null終端から添え字0までのcharを1文字ずつコピー
        for( idx=lenOutStr; idx>=0; idx-- ){
            *( pInfo->outBuf + idx +iMissingDigits ) = *( pInfo->outBuf + idx );
        }

        chPadding = ' ';

        if( pInfo->radix & CM_ALIGN_NUM_PADDING_ZERO_BIT ){
            chPadding = '0';
        }

        pCh = pInfo->outBuf;
        while( lenOutStr < pInfo->numberOfDigits ){
            *pCh++ = chPadding;
            lenOutStr++;
        }
    }

    //DBGP( "pInfo->outBuf=\"%s\"\n", pInfo->outBuf );
    return pInfo->outBuf;
}


char *tmym_cmAlignSeparator( char ch, CM_ALIGN_NUM_S *pInfo )
{
    int lenStr;
    char *pCh;

    if( ( pInfo->numberOfDigits <= 0 ) || ( CM_ALIGN_NUM_OF_DIGITS_MAX < pInfo->numberOfDigits ) ){
        ERRP( "invalid argument: pInfo->numberOfDigits=%d\n", pInfo->numberOfDigits );
        pInfo->outBuf[ 0 ] = '\0';
        return NULL;
    }

    lenStr = 0;
    pCh = pInfo->outBuf;
    while( lenStr < pInfo->numberOfDigits ){
        *pCh++ = ch;
        lenStr++;
    }
    *pCh = '\0';

    return pInfo->outBuf;
}


char *tmym_cmAlignLabel( char *pLabelStr, CM_ALIGN_NUM_S *pInfo )
{
    int lenStr;
    char *pCh;

    if( ( pInfo->numberOfDigits <= 0 ) || ( CM_ALIGN_NUM_OF_DIGITS_MAX < pInfo->numberOfDigits ) ){
        ERRP( "invalid argument: pInfo->numberOfDigits=%d\n", pInfo->numberOfDigits );
        pInfo->outBuf[ 0 ] = '\0';
        return NULL;
    }

    lenStr = tmym_cmStrLen( pLabelStr );
    if( lenStr > CM_ALIGN_NUM_OF_DIGITS_MAX ){
        lenStr = CM_ALIGN_NUM_OF_DIGITS_MAX;
    }
    memcpy( pInfo->outBuf, pLabelStr, lenStr );
    pCh = ( pInfo->outBuf + lenStr );
    while( lenStr < pInfo->numberOfDigits ){
        *pCh++ = ' ';
        lenStr++;
    }
    *pCh = '\0';

    return pInfo->outBuf;
}


/********************************************//**
 * \brief  output data by a hex digit
 *
 * \version    Revision: 1.2
 * \param[in]  buf      address of input data.
 * \param[in]  offset   offset from buf.
 * \param[in]  dmplen   length of output data ( >0 ).
 * \param[in]  col_len  length of 1 line ( 1 <= col_len <= 256 ).
 * \return     on success, output data size
 * \retval     >0  Success
 * \retval     -1  Failure
 *  - \a buf must be not NULL
 *  - \a dmplen must be more than to 0
 *  - \a col_len must be more than or equal to 1 and
 *                       less than or equal to 256
 * \attention  none
 * \note       none
 * \todo       none
 * \bug        none
 * \author     tomyama
 ************************************************/

int tmym_cmDataDump( char *buf, const int offset, const int dmplen, const int col_len )
{
    int ret;
    unsigned char *ubuf;
    static char line_buff[ 1024 ];
    int dump_cntr, dump_line_s;
    int _dumpLen, _colLen;
    int idx;
    int row_max;
    int addrMax;
    static CM_ALIGN_NUM_S addrRad10, addrRad16;
    char chCur, chPrt;
    int remain_col;

    //DBGP( "begin: buf=0x%08lX, offset=%d, dmplen=%d, col_len=%d\n",
    //      ( unsigned long int )buf, offset, dmplen, col_len );

    /* set initial value */
    ret = 0;
    errno = 0;
    idx = 0;

    /* parameter check */
    if( buf  == NULL )
        { errno = EINVAL; ret = -1; goto LBL_FUNC_END; }
    if( dmplen  <=  0 )
        { errno = EDOM;   ret = -1; goto LBL_FUNC_END; }
    if( col_len < 1 || 256 < col_len )
        { errno = EDOM;   ret = -1; goto LBL_FUNC_END; }

    ubuf = ( ( unsigned char * )buf + offset );

    _dumpLen = dmplen;
    _colLen = ( ( _dumpLen < col_len ) ? _dumpLen : col_len );
    row_max = ( _dumpLen + _colLen - 1 ) / _colLen;
    addrMax = ( row_max - 1 ) * _colLen;
    tmym_cmSetupAlignNumStruct( &addrRad10, 0, addrMax, CM_ALIGN_NUM_RADIX_10 );
    tmym_cmSetupAlignNumStruct( &addrRad16, 0, addrMax, CM_ALIGN_NUM_RADIX_16U );
    //DBGP( "_dumpLen=%d, _colLen=%d, row_max=%d, addrMax=%d\n",
    //    _dumpLen, _colLen, row_max, addrMax );
    memset( line_buff, '\0', sizeof line_buff );
    dump_cntr = 0;
    dump_line_s = dump_cntr;
    sprintf( line_buff, "%sd: %sh:",
        tmym_cmAlignNumber( dump_cntr, &addrRad10 ),
        tmym_cmAlignNumber( dump_cntr, &addrRad16 ) );
    idx = ( int )strlen( line_buff );
    for( dump_cntr=0; dump_cntr<_dumpLen; dump_cntr++ ){
        sprintf( &( line_buff[ idx ] ), " %02hX", *( ubuf + dump_cntr ) );
        idx += 3;
        remain_col = ( _colLen - ( ( dump_cntr + 1 ) % _colLen ) );
        if( remain_col == _colLen ){
            remain_col = 0;
        }
        if( ( remain_col == 0 ) || ( ( dump_cntr + 1 ) == _dumpLen ) ){
            while( remain_col ){
                sprintf( &( line_buff[ idx ] ), "   " );
                idx += 3;
                remain_col--;
            }
            sprintf( &( line_buff[ idx ] ), "  >" );
            idx += 3;
            while( dump_line_s <= dump_cntr ){
                chCur = *( ubuf + dump_line_s );
                chPrt = ' ';
                if( tmym_cmIsThisPrintableChar( chCur ) ){

                    chPrt = chCur;
                }
                line_buff[ idx++ ] = chPrt;
                dump_line_s++;
            }
            remain_col = ( _colLen - ( ( dump_cntr + 1 ) % _colLen ) );
            if( remain_col == _colLen ){
                remain_col = 0;
            }
            while( remain_col ){
                line_buff[ idx++ ] = ' ';
                remain_col--;
            }
            line_buff[ idx++ ] = '<';
            puts( line_buff );

            memset( line_buff, '\0', sizeof line_buff );
            sprintf( line_buff, "%sd: %sh:",
                tmym_cmAlignNumber( dump_cntr + 1, &addrRad10 ),
                tmym_cmAlignNumber( dump_cntr + 1, &addrRad16 ) );
            idx = ( int )strlen( line_buff );
        }
    }

    ret = dump_cntr;

LBL_FUNC_END:
    //DBGP( "end: ret=%d, errno=%d(%s)\n", ret, errno, strerror( errno ) );
    return ret;
}


// LCOV_EXCL_START
// cmCdManAllocMem
cmCdManager_s *tmym_cmDbgGetCmCdManAllocMem( void )
{
    return &cmCdManAllocMem;
}
// LCOV_EXCL_STOP


void *tmym_cmMalloc( size_t size )
{
    void *pBuf;
    struct cmProgramResource *pPr;
    int iFuncRet;

    if( tmym_cmRegCleanFunc( tmym_cmClean ) == TMYM_SUCCESS ){
        tmym_cmPrtf( "register: tmym_cmClean() in %s()\n", __func__ );
    }

    pBuf = malloc( size );
    if( pBuf != NULL ){
        pPr = malloc( sizeof( struct cmProgramResource ) );
        pPr->eType = CM_PR_TYPE_ALOCMEM;
        pPr->sz    = size;
        pPr->pRsrc = ( void *)pBuf;
        iFuncRet = cmCdManAllocMem.Add( &cmCdManAllocMem, ( void *)pPr );
        // LCOV_EXCL_START
        if( iFuncRet != TMYM_SUCCESS ){
            WARN( "cmCdManAllocMem.Add(): failure\n" );
        }
        // LCOV_EXCL_STOP
    }
    return pBuf;
}


void tmym_cmFree( void *ptr )
{
    void *pFuncRet;
    pFuncRet = tmym_cmCdRemove( &cmCdManAllocMem, ptr );
    if( pFuncRet == NULL ){
        // LCOV_EXCL_START
        WARN( "tmym_cmCdRemove( &cmCdManAllocMme, ptr ): failure\n" );
        // LCOV_EXCL_STOP
    }else{
        free( pFuncRet );
    }
    free( ptr );
    return;
}


/********************************************//**
 * \brief  printf()の拡張
 *
 * - 出力先によって不要になるANSIエスケープシーケンスによる修飾子を除去する
 *
 * \param[in]  pFmt     printf()相当の書式文字列
 * \param[in]  ...      書式文字列に求められる可変長引数
 * \return     出力したバイト数
 * \retval     >0  Success
 *
 * \attention  none
 * \note       none
 * \todo       none
 * \bug        none
 * \author     tomyama
 ************************************************/

int tmym_cmPrtf( char *pFmt, ... )
{
    va_list argPtr;
    int lenWrote;

    va_start( argPtr, pFmt );
    lenWrote = tmym_cmVFPrtf( stdout, pFmt, argPtr );
    va_end( argPtr );

    return lenWrote;
}


/********************************************//**
 * \brief  fprintf()の拡張
 *
 * - 出力先によって不要になるANSIエスケープシーケンスによる修飾子を除去する
 *
 * \param[in]  fp       出力ストリーム
 * \param[in]  pFmt     printf()相当の書式文字列
 * \param[in]  ...      書式文字列に求められる可変長引数
 * \return     出力したバイト数
 * \retval     >0  Success
 *
 * \attention  none
 * \note       none
 * \todo       none
 * \bug        none
 * \author     tomyama
 ************************************************/

int tmym_cmFPrtf( FILE *fp, char *pFmt, ... )
{
    va_list argPtr;
    int lenWrote;

    va_start( argPtr, pFmt );
    lenWrote = tmym_cmVFPrtf( fp, pFmt, argPtr );
    va_end( argPtr );

    return lenWrote;
}


/********************************************************************/
/*        function (static)                                         */
/********************************************************************/

INTERNAL void *tmym_cmCdGetKey( cmConsolidatedData_s *pCD )
{
    return pCD->pData;
}


INTERNAL int tmym_cmCdCompare( void *a, void *b )
{
    //DBGP( "a=%lu, b=%lu\n", ( unsigned long *)a, ( unsigned long *)b );
    return ( int )( ( unsigned long *)a - ( unsigned long *)b );
}


// a は b よりも手前側のリストを指定すること
// b は a よりも後方のリストを指定すること
INTERNAL void tmym_cmCdSwap( cmCdManager_s *pCM, cmConsolidatedData_s *a, cmConsolidatedData_s *b )
{
    cmConsolidatedData_s *pPrevTmp, *pNextTmp;

    // a と b を入れ替える
    pPrevTmp = a->pPrev;
    pNextTmp = a->pNext;
    //if( pPrevTmp != NULL ){
    //    if( pPrevTmp == b ){
    //        pPrevTmp = a;
    //    }
    //}
    if( pNextTmp != NULL ){
        if( pNextTmp == b ){
            pNextTmp = a;
        }
    }

    a->pPrev = b->pPrev;
    a->pNext = b->pNext;
    if( a->pPrev != NULL ){
        if( a->pPrev == a ){
            a->pPrev = b;
        }
    }
    //if( a->pNext != NULL ){
    //    if( a->pNext == a ){
    //        a->pNext = b;
    //    }
    //}

    b->pPrev = pPrevTmp;
    b->pNext = pNextTmp;

    // b の前、および a の後を書き換える
    if( b->pPrev == NULL ){
        pCM->pHead = b;
    }else{
        b->pPrev->pNext = b;
    }
    if( a->pNext == NULL ){
        pCM->pTail = a;
    }else{
        a->pNext->pPrev = a;
    }

    return;
}


INTERNAL void tmym_cmCdBubbleSort( cmCdManager_s *pCM )
{
    int iFuncRet;
    cmConsolidatedData_s *pCdA, *pCdB, *pCdTmp;

    //! - 排他処理開始
    // LCOV_EXCL_START
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    if( pCM->pHead == NULL ){
        tmym_cmMtxUnlock( &pCM->mutexRsrc );
        return;
    }
    for( pCdA = pCM->pHead; pCdA != pCM->pTail; pCdA=pCdA->pNext ){
        //DBGP( "pCdA[0x%08X]\n", pCdA );
        for( pCdB = pCM->pTail; pCdB != pCdA; pCdB=pCdB->pPrev ){
            pCdTmp = pCdB->pPrev;
            //DBGP( "pCdTmp[0x%08X]  pCdB[0x%08X]\n", pCdTmp, pCdB );
            if( pCM->Compare( pCM->GetKey( pCdTmp ), pCM->GetKey( pCdB ) ) < 0 ){
//#if _UNIT_TEST_
//                tmym_cmCdListup( pCM, __func__, NULL, NULL );
//#endif
                tmym_cmCdSwap( pCM, pCdTmp, pCdB );
                if( pCdA == pCdTmp ){
                    pCdA = pCdB;
                }
                pCdB = pCdTmp;
//#if _UNIT_TEST_
//                tmym_cmCdListup( pCM, __func__, NULL, NULL );
//#endif
            }
        }
    }

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

#if _UNIT_TEST_
    tmym_cmCdListup( pCM, __func__, NULL, NULL );
#endif
    return;
}


INTERNAL void tmym_cmCdListup( cmCdManager_s *pCM, const char *func, void *pNew, void *pGet )
{
    int iFuncRet;
    int idx;
    cmConsolidatedData_s *pCD;
    const char *pDataType;
    static char chFunc[ 1024 ];

    //! - 排他処理開始
    iFuncRet = tmym_cmMtxLock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxLock(): failure\n" );
    }
    // LCOV_EXCL_STOP

    memset( chFunc, '\0', 1024 );
    if      ( ( pNew == NULL ) && ( pGet == NULL ) ){
        sprintf( chFunc, "%s()", func );
    }else if( ( pNew == NULL ) && ( pGet != NULL ) ){
        sprintf( chFunc, "%s()=%p", func, pGet );
    }else{
        sprintf( chFunc, "%s(%p)", func, pNew );
    }

    pDataType = ( ( pCM->type == CM_DATA_TYPE_STACK ) ? "Stack" : (
                  ( pCM->type == CM_DATA_TYPE_LIST  ) ? "List"  : (
                  ( pCM->type == CM_DATA_TYPE_FIFO  ) ? "FIFO"  : "Sort" ) ) );
    tmym_cmPrtf( "%s: pCM  : %-10s: ", chFunc, pDataType );
    if( pCM->pHead != NULL ){
        pCD = pCM->pHead;
        tmym_cmPrtf( "pCD  : " );
        for( idx=0; pCD != NULL; idx++ ){
            tmym_cmPrtf( "-- %4d --  ", idx );
            pCD = pCD->pNext;
        }
    }
    tmym_cmPrtf( "\n" );

    tmym_cmPrtf( "%s: Addr : %08p: ", chFunc, pCM );
    if( pCM->pHead != NULL ){
        pCD = pCM->pHead;
        tmym_cmPrtf( "Addr : " );
        for( idx=0; pCD != NULL; idx++ ){
            tmym_cmPrtf( "%p  ", pCD );
            pCD = pCD->pNext;
        }
    }
    tmym_cmPrtf( "\n" );

    tmym_cmPrtf( "%s: pHead: %p: ", chFunc, pCM->pHead );
    if( pCM->pHead != NULL ){
        pCD = pCM->pHead;
        tmym_cmPrtf( "pPrev: " );
        for( idx=0; pCD != NULL; idx++ ){
            tmym_cmPrtf( "%p  ", pCD->pPrev );
            pCD = pCD->pNext;
        }
    }
    tmym_cmPrtf( "\n" );

    tmym_cmPrtf( "%s: pTail: %p: ", chFunc, pCM->pTail );
    if( pCM->pHead != NULL ){
        pCD = pCM->pHead;
        tmym_cmPrtf( "pNext: " );
        for( idx=0; pCD != NULL; idx++ ){
            tmym_cmPrtf( "%p  ", pCD->pNext );
            pCD = pCD->pNext;
        }
    }
    tmym_cmPrtf( "\n" );

    if( pCM->pHead != NULL ){
        pCD = pCM->pHead;
        tmym_cmPrtf( "%s:                  : pData: ", chFunc);
        for( idx=0; pCD != NULL; idx++ ){
            tmym_cmPrtf( "%p  ", pCD->pData );
            pCD = pCD->pNext;
        }
        tmym_cmPrtf( "\n" );
    }

    iFuncRet = tmym_cmMtxUnlock( &pCM->mutexRsrc );
    // LCOV_EXCL_START
    if( iFuncRet != TMYM_SUCCESS ){
        WARN( "tmym_cmMtxUnlock(): failure\n" );
    }
    // LCOV_EXCL_STOP
    //! - 排他処理終了

    return;
}


INTERNAL void *tmym_cmPrGetKey( cmConsolidatedData_s *pCD )
{
    return ( void *)( ( struct cmProgramResource *)pCD->pData )->pRsrc;
}


INTERNAL int tmym_cmPrClean( struct _cmCdManager *pCM )
{
    struct cmProgramResource *pPR;
    int iRetVal;

    errno = 0;

    iRetVal = TMYM_SUCCESS;
    if( tmym_cmCdGetLen( pCM ) > 0 ){
        while( ( pPR = ( struct cmProgramResource *)pCM->Eject( pCM ) ) != NULL ){
            if( pPR->eType == CM_PR_TYPE_ALOCMEM ){
                WARN( "Clean: pPR[%p] = { eType=%d[CM_PR_TYPE_ALOCMEM], sz=%lu, pRsrc=0x%08lX }\n",
                    pPR, pPR->eType, pPR->sz, pPR->pRsrc );
                free( pPR->pRsrc );
                free( pPR );
            }else{
                // LCOV_EXCL_START
                WARN( "Unknown type: %d\n", pPR->eType );
                iRetVal = TMYM_FAILURE;
                // LCOV_EXCL_STOP
            }
        }
    }

    return iRetVal;
}


INTERNAL int tmym_cmUseColorConsole( FILE *fp )
{
    int bIsATty;

#ifdef _WIN32
    bIsATty = _isatty( _fileno( fp ) );
#else
    bIsATty =  isatty(  fileno( fp ) );
#endif

    return bIsATty && tmym_cmDoesItSupportAnsiEscSeq();
}


static int bDoesItSupportAnsiEscSeq = -1;
INTERNAL int tmym_cmDoesItSupportAnsiEscSeq( void )
{
    if( bDoesItSupportAnsiEscSeq == -1 ){
        bDoesItSupportAnsiEscSeq = 1;

#ifdef _WIN32
        int fh;
        DWORD dwMode;

        fh = _fileno( stdout );
        HANDLE hCon = ( HANDLE )_get_osfhandle( fh );
        if( ! GetConsoleMode( hCon, &dwMode ) ){
            return 0;
        }
        bDoesItSupportAnsiEscSeq = ( dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING );
#endif
    }

    return bDoesItSupportAnsiEscSeq;
}


INTERNAL int tmym_cmRemoveAnsiEscSeq( char *pStr )
{
    int idx, lenStr;
    char chCur;
    int iSeq;
    int iMStart, iMLen, iLenCopy;

    lenStr = ( int )tmym_cmStrLen( pStr );
    iSeq = iMStart = iMLen = 0;
    for( idx=0; idx<lenStr; idx++ ){
        chCur = *( pStr + idx );
        //DBGP( "idx = %5d: 0x%02X '%c'\n", idx, ( chCur & 0xFF ),
        //      ( ( ( 0x20 <= chCur ) && ( chCur <= 0x7E ) ) ? chCur : ' ' ) );

        switch( iSeq ){
        // Seq 0: ESC ( 0x1B )
        case 0:
            if( chCur == 0x1B ){
                iSeq = 1;
                iMStart = idx;
                iMLen = 1;
                continue;
            }
            break;
        // Seq 1: [ ( 0x5B )
        case 1:
            if( chCur == '[' ){
                iSeq = 2;
                iMLen++;
                continue;
            }
            break;
        // Seq 2: n ( 0x30 〜 0x39 ) or [mJK]
        case 2:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 3;
                iMLen++;
                continue;
            }else if( ( chCur == 'm' ) ||
                      ( chCur == 'J' ) ||
                      ( chCur == 'K' ) ){
                iMLen++;
                iLenCopy = lenStr - iMStart - iMLen + 1;
                memcpy( ( pStr + iMStart ), ( pStr + iMStart + iMLen ), iLenCopy );
                idx -= iMLen;
                lenStr -= iMLen;
                iSeq = iMStart = iMLen = 0;
                continue;
            }
            break;
        // Seq 3: n ( 0x30 〜 0x39 ) or ; ( 1st. ) or [mABCDEFGSTJK]
        case 3:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 3;
                iMLen++;
                continue;
            }else if( chCur == ';' ){
                iSeq = 4;
                iMLen++;
                continue;
            }else if( ( chCur == 'm' ) ||
                      ( chCur == 'A' ) ||
                      ( chCur == 'B' ) ||
                      ( chCur == 'C' ) ||
                      ( chCur == 'D' ) ||
                      ( chCur == 'E' ) ||
                      ( chCur == 'F' ) ||
                      ( chCur == 'G' ) ||
                      ( chCur == 'S' ) ||
                      ( chCur == 'T' ) ||
                      ( chCur == 'J' ) ||
                      ( chCur == 'K' ) ){
                iMLen++;
                iLenCopy = lenStr - iMStart - iMLen + 1;
                memcpy( ( pStr + iMStart ), ( pStr + iMStart + iMLen ), iLenCopy );
                idx -= iMLen;
                lenStr -= iMLen;
                iSeq = iMStart = iMLen = 0;
                continue;
            }
            break;
        // Seq 4: n ( 0x38 〜 0x39 )
        case 4:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 5;
                iMLen++;
                continue;
            }
            break;
        // Seq 5: n ( 0x30 〜 0x39 ) or ; ( 2nd. ) or [Hf]
        case 5:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 5;
                iMLen++;
                continue;
            }else if( chCur == ';' ){
                iSeq = 6;
                iMLen++;
                continue;
            }else if( ( chCur == 'H' ) ||
                      ( chCur == 'f' ) ){
                iMLen++;
                iLenCopy = lenStr - iMStart - iMLen + 1;
                memcpy( ( pStr + iMStart ), ( pStr + iMStart + iMLen ), iLenCopy );
                idx -= iMLen;
                lenStr -= iMLen;
                iSeq = iMStart = iMLen = 0;
                continue;
            }
            break;
        // Seq 6: n ( 0x38 〜 0x39 )
        case 6:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 7;
                iMLen++;
                continue;
            }
            break;
        // Seq 7: n ( 0x30 〜 0x39 ) or ; ( 3rd. ) or [m]
        case 7:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 7;
                iMLen++;
                continue;
            }else if( chCur == ';' ){
                iSeq = 8;
                iMLen++;
                continue;
            }else if( chCur == 'm' ){
                iMLen++;
                iLenCopy = lenStr - iMStart - iMLen + 1;
                memcpy( ( pStr + iMStart ), ( pStr + iMStart + iMLen ), iLenCopy );
                idx -= iMLen;
                lenStr -= iMLen;
                iSeq = iMStart = iMLen = 0;
                continue;
            }
            break;
        // Seq 8: n ( 0x30 〜 0x39 )
        case 8:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 9;
                iMLen++;
                continue;
            }
            break;
        // Seq 9: n ( 0x30 〜 0x39 ) or ; ( 4th. )
        case 9:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 9;
                iMLen++;
                continue;
            }else if( chCur == ';' ){
                iSeq = 10;
                iMLen++;
                continue;
            }
            break;
        // Seq 10: n ( 0x38 〜 0x39 )
        case 10:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 11;
                iMLen++;
                continue;
            }
            break;
        // Seq 11: n ( 0x30 〜 0x39 ) or m
        case 11:
            if( ( 0x30 <= chCur ) && ( chCur <= 0x39 ) ){
                iSeq = 11;
                iMLen++;
                continue;
            }else if( chCur == 'm' ){
                iMLen++;
                iLenCopy = lenStr - iMStart - iMLen + 1;
                memcpy( ( pStr + iMStart ), ( pStr + iMStart + iMLen ), iLenCopy );
                idx -= iMLen;
                lenStr -= iMLen;
                iSeq = iMStart = iMLen = 0;
                continue;
            }
            break;
        }
        if( iSeq > 0 ){
            idx--;
        }
        iSeq = iMStart = iMLen = 0;
    }

    return lenStr;
}


INTERNAL int tmym_cmVFPrtf( FILE *fp, char *pFmt, va_list argPtr )
{
    #define CM_FPRTF_BUFF_LEN  2048
    static char chBuff[ CM_FPRTF_BUFF_LEN ];
    int lenWrote;

/* MSVC: C4996 */
#ifdef HAVE_VSPRINTF_S
    lenWrote = vsprintf_s( chBuff, CM_FPRTF_BUFF_LEN, pFmt, argPtr );
#else
    lenWrote = vsprintf( chBuff, pFmt, argPtr );
#endif
    //DBGP( "lenWrote = %d\n", lenWrote );
    if( lenWrote >= ( CM_FPRTF_BUFF_LEN - 1 ) ){
        ERRP( "%d(lenWrote) >= %d( CM_FPRTF_BUFF_LEN - 1 ): buffer over error\n",
              lenWrote, ( CM_FPRTF_BUFF_LEN - 1 ) );
        return -1;
    }

    if( ! tmym_cmUseColorConsole( fp ) ){
        lenWrote = tmym_cmRemoveAnsiEscSeq( chBuff );
    }

    fputs( chBuff, fp );

    return lenWrote;
}


INTERNAL TMYM_BOOL tmym_cmIsThisPrintableChar( char ch )
{
    if( ( 0x20 <= ch ) && ( ch <= 0x7E ) ){
        return TMYM_TRUE;
    }
    return TMYM_FALSE;
}

#ifdef HAVE_STRERROR_S
/* MSVC: C4996 */
static char *_cm_strerror( int errnum )
{
    static char msgbuf[ 1024 ];
    ( void )strerror_s( msgbuf, sizeof msgbuf, errnum );
    return msgbuf;
}
#endif
