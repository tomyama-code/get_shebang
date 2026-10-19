/*! \page  page_cmmn  Common function
@section  sec_cmmn_overview  概要
プログラムの共通機能を実装するモジュール

@section  sec_cmmn_howtouse  使い方
- 開発環境: GNU Make(必須), Cygwin(Windowsでは必須), Visual Studio, GNU GCC, もしくは clang
    - (必須) GNU make
    - Windows
        - Visual Studio の「～ Tools Command Prompt」、例えば「x64 Native Tools Command Prompt」
        - CygwinをインストールしてWindowsの環境変数 PATH に Cygwin のbin（例えば C:\\cygwin64\\bin ）を
          追加しておくこと
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
- 使い方／実行例
    - make test

@section  sec_cmmn_subpage  サブページ
- \subpage  subpage_cmmn_thread
- \subpage  subpage_cmmn_mutex
- \subpage  subpage_cmmn_consolidated_data

@section  sec_cmmn_history  変更履歴
- 2023-04-05: r1.6
    - 関数を追加
        - 連結データ
            - tmym_cmCdInit()
            - tmym_cmCdClean()
            - tmym_cmCdGetLen()
            - tmym_cmCdRegFuncClean()
            - tmym_cmCdRegFuncGetKey()
            - tmym_cmCdRegFuncCompare()
            - tmym_cmCdPush()
            - tmym_cmCdPop()
            - tmym_cmCdRemove()
            - tmym_cmCdUnshift()
            - tmym_cmCdShift()
            - tmym_cmCdInsert()
            - tmym_cmCdGetKey()
            - tmym_cmCdCompare()
            - tmym_cmCdSwap()
            - tmym_cmCdBubbleSort()
            - tmym_cmCdListup()
        - メモリ管理
            - tmym_cmPrGetKey()
            - tmym_cmPrClean()
    - cmMalloc() と cmFree() がコールされた時に確保、開放するメモリを
      今回追加するスタックで管理するようにした
- 2023-03-30: r1.5
    - 関数を追加
        - tmym_cmSetupAlignNumStruct()
        - tmym_cmAlignNumber()
        - tmym_cmAlignSeparator()
        - tmym_cmAlignLabel()
    - 関数を変更
        - tmym_cmDataDump()
            - 引数リストのバッファサイズを廃止
            - 引数リストに先頭アドレスからのオフセット値を追加
            - ダンプ出力のアドレス部分に今回追加した関数群を使用して桁揃えするように変更
- 2023-03-24: r1.4
    - 関数を追加
        - tmym_cmGetLine()
        - tmym_cmGetStr()
        - tmym_cmGetOneChar()
        - tmym_cmStrRemoveLeadingWhiteSpace()
        - tmym_cmStrRemoveTrailingWhiteSpace()
        - tmym_cmStrRemoveBackSpace()
        - tmym_cmStrClean()
    - 追加した関数を利用するように変更
        - tmym_cmGetNum()
- 2023-03-22: r1.3
    - tmym_cmGetNum()関数を追加
- 2023-03-15: r1.2
    - 関数マクロ ERRP() で error の表示部分の文字色を赤色で出力するように変更。
- 2023-03-12: r1.1
    - 最初の管理番号

\ref index "メインメージへ"
*/

/*! \page  subpage_cmmn_consolidated_data  Consolidated Data [ 連結データ ]
@section  sec_cmmn_cd_overview  概要
連結データを実装

\dot "連結データ"
digraph graphCmConsolidatedData {
    graph [
      charset = "UTF-8",
      bgcolor = "#EDEDED",
      rankdir = LR,
      nodesep = 1.1,
      ranksep = 1.05
  ];

  node [
    shape = record,
    fontsize = 12
  ];

  // node define
  cdN [ label = "NULL" ];
  cdA [ label = "{<lA>\"A\"|{<lAP>pPrev|<lAN>pNext|pData}}" ];
  cdB [ label = "{<lB>\"B\"|{<lBP>pPrev|<lBN>pNext|pData}}" ];
  cdC [ label = "{<lC>\"C\"|{<lCP>pPrev|<lCN>pNext|pData}}" ];

  // edge define
  // prev                                  // next
  cdA:lAP -> cdN    [ color = "#CCCC00" ]; cdA:lAN -> cdB:lB [ color = "#00CC00" ];
  cdB:lBP -> cdA:lA [ color = "#CCCC00" ]; cdB:lBN -> cdC:lC [ color = "#00CC00" ];
  cdC:lCP -> cdB:lB [ color = "#CCCC00" ]; cdC:lCN -> cdN    [ color = "#00CC00" ];
}
\enddot

- 連結データ
    - pData の使い方はユーザーが決定する
        - デフォルトでは 値 が入っているものとして動作する
        - この動作は 管理データ の .GetKey() を書き換えることで変更できる

\dot "pData にアドレスをセットする例"
digraph graphCmConsolidatedData {
    graph [
      charset = "UTF-8",
      bgcolor = "#EDEDED",
      rankdir = LR,
      nodesep = 1.1,
      ranksep = 1.05
  ];

  node [
    shape = record,
    fontsize = 12
  ];

  // node define
  cdN [ label = "NULL" ];
  cdA [ label = "<lAP>pPrev|<lAN>pNext|<lAD>pData" ];
  cdB [ label = "<lBP>pPrev|<lBN>pNext|<lBD>pData" ];
  cdC [ label = "<lCP>pPrev|<lCN>pNext|<lCD>pData" ];

  cdAA [ label = "<lAA>\"Taro\"|国語60点|算数70点|理科75点" ];
  cdBB [ label = "<lBB>\"Hana\"|国語80点|算数74点|理科80点" ];
  cdCC [ label = "<lCC>\"Kota\"|国語70点|算数80点|理科60点" ];

  // edge define
  // prev  next                   // pData
  cdA:lAP -> cdN                   ; cdA:lAD -> cdAA:lAA;
  cdA:lAN -> cdB:lBN [ dir = both ]; cdB:lBD -> cdBB:lBB;
  cdB:lBN -> cdC:lCN [ dir = both ]; cdC:lCD -> cdCC:lCC;
  cdC:lCP -> cdN                   ;

  //{ rank = same; cdA; cdAA; }
  //{ rank = same; cdB; cdBB; }
  //{ rank = same; cdC; cdCC; }
}
\enddot

- 連結データ
    - 連結データは隣りあうデータ同士が互いの位置情報を持っているため、物理的に隣り合っている必要はない
    - しかし、論理的には連結しているものとみなす事ができるので次のようにも図示できる

\dot "連結データの省略記法"
digraph graphCmConsolidatedData {
    graph [
      charset = "UTF-8",
      bgcolor = "#EDEDED",
      rankdir = LR,
      nodesep = 1.1,
      ranksep = 1.05
  ];

  node [
    shape = record,
    fontsize = 12
  ];

  // node define
  cdABC [ label = "{\"A\"|\"B\"|\"C\"}" ];

  cdA [ label = "\"A\"" ];
  cdB [ label = "\"B\"" ];
  cdC [ label = "\"C\"" ];

  // edge define
  cdA -> cdB    [ color = "#000000", dir = both ];
  cdB -> cdC    [ color = "#000000", dir = both ];
}
\enddot

- 連結データの詳細
    - \ref _cmConsolidatedData

- 管理データ
    - 連結データは互いの連結情報以外のメタ情報を持っていない
    - そのため連結データとは別に管理用の構造体を定義している
        - 連結データはユーザーの操作により増減するが、管理データは常に存在し続ける
    - 管理データは次のような情報を持っている
        - 連結データの先頭と最後尾の位置
        - 連結データの数
        - 連結データのデータタイプ
        - 連結データのデフォルトの操作関数（データタイプによって変化）
            - .Add()
            - .Eject()
            - .Sort()
            - .Clean()
        - 操作関数の中で使われる関数
            - .GetKey()
            - .Compare()
- 管理データの詳細
    - \ref _cmCdManager

- データタイプとデフォルトのデータ操作API

\code
  ◎：.Add(), もしくは .Eject() を経由してコールできるデフォルトの動作
  〇：直接関数をコールすることで使用可能
  [*1]：どこに挿入されるかは既存データの並び次第。期待する位置に挿入するには事前に.Sort()をして整列しておく必要がある。
┏━━━━━━━━━┳━━━━━━━━━━┯━━━━━━━━━━┯━━━━━━━━━━┯━━━━━━━━━━┓
┃   API  ＼  TYPE  ┃ CM_DATA_TYPE_STACK │ CM_DATA_TYPE_LIST  │ CM_DATA_TYPE_FIFO  │ CM_DATA_TYPE_SORT  ┃
┣━━━━━━━━━╋━━━━━━━━━━┿━━━━━━━━━━┿━━━━━━━━━━┿━━━━━━━━━━┫
┃ tmym_cmCdPush    ┃         ◎         │         〇         │         ◎         │         〇         ┃
┠─────────╂──────────┼──────────┼──────────┼──────────┨
┃ tmym_cmCdPop     ┃         ◎         │         〇         │         〇         │         〇         ┃
┠─────────╂──────────┼──────────┼──────────┼──────────┨
┃ tmym_cmCdRemove  ┃         〇         │         〇         │         〇         │         〇         ┃
┠─────────╂──────────┼──────────┼──────────┼──────────┨
┃ tmym_cmCdUnshift ┃         〇         │         ◎         │         〇         │         〇         ┃
┠─────────╂──────────┼──────────┼──────────┼──────────┨
┃ tmym_cmCdShift   ┃         〇         │         ◎         │         ◎         │         ◎         ┃
┠─────────╂──────────┼──────────┼──────────┼──────────┨
┃ tmym_cmCdInsert  ┃         〇 *1      │         〇 *1      │         〇 *1      │         ◎         ┃
┗━━━━━━━━━┻━━━━━━━━━━┷━━━━━━━━━━┷━━━━━━━━━━┷━━━━━━━━━━┛
\endcode

- データタイプの詳細
    - \ref cmCdType_e

- データ操作APIの概要

\code
        ┏━┯━┯━┓  ┏━┓
        ┃ a│ b│ c┃←┃ d┃  tmym_cmCdPush( d )
        ┗━┷━┷━┛  ┗━┛
        ┏━┯━┯      ┏━┓
        ┃ a│ b│   → ┃ c┃  tmym_cmCdPop()
        ┗━┷━┷      ┗━┛
        ┏━┯  ↓  ┯━┓
        ┃ a│┏━┓│ c┃
        ┗━┷┃ b┃┷━┛      tmym_cmCdRemove( b )
              ┗━┛
                ×
┏━┓  ┏━┯━┯━┓
┃ d┃→┃ a│ b│ c┃          tmym_cmCdUnshift( d )
┗━┛  ┗━┷━┷━┛
┏━┓    ┯━┯━┓
┃ a┃ ← │ b│ c┃            tmym_cmCdShift()
┗━┛    ┷━┷━┛
              ┏━┓
              ┃ b┃
        ┏━┯┗━┛┯━┓      tmym_cmCdInsert( b )
        ┃ a│  ↓  │ c┃
        ┗━┷      ┷━┛
\endcode

\htmlonly
<a onclick="history.back()">前のページへ戻る</a>
\endhtmlonly

*/

/*! \page  subpage_cmmn_mutex Mutex [ ミューテックス ]
@section  sec_cmmn_mtx_overview  概要
ミューテックスを実装

\htmlonly
<table border=1 style="font-family: monospace">
  <tr>
    <th>操作</th>
    <th>Windows</th>
    <th>Posix</th>
    <th>tmym_common</th>
  </tr>
  <tr>
    <th>初期化</th>
    <td>
#include &lt;synchapi.h&gt;<br>
HANDLE CreateMutexA(<br>
&nbsp;&nbsp;LPSECURITY_ATTRIBUTES lpMutexAttributes,<br>
&nbsp;&nbsp;BOOL                  bInitialOwner,<br>
&nbsp;&nbsp;LPCSTR                lpName<br>
);
    </td>
    <td>
#define _OPEN_THREADS<br>
#define _OPEN_SYS<br>
#include &lt;pthread.h&gt;<br>
int pthread_mutexattr_init(<br>
&nbsp;&nbsp;pthread_mutexattr_t *attr<br>
);
<br>
#define _OPEN_THREADS<br>
#include &lt;pthread.h&gt;<br>
int pthread_mutex_init(<br>
&nbsp;&nbsp;pthread_mutex_t *mutex,<br>
&nbsp;&nbsp;pthread_mutexattr_t *attr<br>
);
    </td>
    <td>
#include "tmym_common.h"<br>
int tmym_cmMtxInit(<br>
&nbsp;&nbsp;cmMutexResource_s *pMR <br>
);
    </td>
  </tr>
  <tr>
    <th>ロック</th>
    <td>
#include &lt;synchapi.h&gt;<br>
DWORD WaitForSingleObject(<br>
&nbsp;&nbsp;HANDLE hHandle,<br>
&nbsp;&nbsp;DWORD  dwMilliseconds<br>
);
    </td>
    <td>
#define _OPEN_THREADS<br>
#include &lt;pthread.h&gt;<br>
int pthread_mutex_lock(<br>
&nbsp;&nbsp;pthread_mutex_t *mutex<br>
);
    </td>
    <td>
#include "tmym_common.h"<br>
int tmym_cmMtxLock(<br>
&nbsp;&nbsp;cmMutexResource_s *pMR<br>
);
    </td>
  </tr>
  <tr>
    <th>アンロック</th>
    <td>
#include &lt;synchapi.h&gt;<br>
BOOL ReleaseMutex(<br>
&nbsp;&nbsp;HANDLE hMutex<br>
);
    </td>
    <td>
#define _OPEN_THREADS<br>
#include &lt;pthread.h&gt;<br>
int pthread_mutex_unlock(<br>
&nbsp;&nbsp;pthread_mutex_t *mutex<br>
);
    </td>
    <td>
#include "tmym_common.h"<br>
int tmym_cmMtxUnlock(<br>
&nbsp;&nbsp;cmMutexResource_s *pMR<br>
);
    </td>
  </tr>
  <tr>
    <th>開放</th>
    <td>
#include &lt;handleapi.h&gt;<br>
BOOL CloseHandle(<br>
&nbsp;&nbsp;HANDLE hObject<br>
);
    </td>
    <td>
#define _OPEN_THREADS<br>
#include &lt;pthread.h&gt;<br>
int pthread_mutex_destroy(<br>
&nbsp;&nbsp;pthread_mutex_t *mutex<br>
);
    </td>
    <td>
#include "tmym_common.h"<br>
int tmym_cmMtxDestroy(<br>
&nbsp;&nbsp;cmMutexResource_s *pMR<br>
);
    </td>
  </tr>
</table>
\endhtmlonly

\htmlonly
<a onclick="history.back()">前のページへ戻る</a>
\endhtmlonly

*/

/*! \page  subpage_cmmn_thread Thread [ スレッド ]
@section  sec_cmmn_thrd_overview  概要
スレッド機能を実装


\htmlonly
<table border=1 style="font-family: monospace">
  <tr>
    <th>WIN32 version</th>
    <th>POSIX version</th>
    <th>tmym_common</th>
  </tr>
  <tr>
    <td>
<pre><code>
|
|  CreateThread()
|----------------------->o
|                        |
|                        |
|  WaitForSingleObject() |
|----------------------|>|
|                        |
|                        |ExitThread()
|<|----------------------|
|GetExitCodeThread()     x
|
|CloseHandle()
|
</code></pre>
    </td>
    <td>
<pre><code>
|
|  pthread_create()
|------------------>o
|                   |
|                   |
|  pthread_join()   |
|-----------------|>|
|                   |
|                   |pthread_exit()
|<|-----------------|
|                   x
|
|
|
</code></pre>
    </td>
    <td>
<pre><code>
|
|  tmym_cmThrdCreate()
|--------------------->o
|                      |
|                      |
|  tmym_cmThrdWait()   |
|--------------------|>|
|                      |
|                      |tmym_cmThrdExit()
|<|--------------------|
|                      x
|
|
|
</code></pre>
    </td>
  </tr>
</table>
\endhtmlonly

\htmlonly
<table border=1 style="font-family: monospace">
  <tr>
    <th>操作</th>
    <th>Windows</th>
    <th>Posix</th>
    <th>tmym_common</th>
  </tr>
  <tr>
    <th>スレッドの作成</th>
    <td>
#include &lt;MSCorEE.h&gt;<br>
typedef DWORD (__stdcall *LPTHREAD_START_ROUTINE) (<br>
&nbsp; LPVOID lpThreadParameter<br>
);<br>
<br>
#include &lt;processthreadsapi.h&gt;<br>
HANDLE CreateThread(<br>
&nbsp; LPSECURITY_ATTRIBUTES   lpThreadAttributes,<br>
&nbsp; SIZE_T                  dwStackSize,<br>
&nbsp; LPTHREAD_START_ROUTINE  lpStartAddress,<br>
&nbsp; __drv_aliasesMem LPVOID lpParameter,<br>
&nbsp; DWORD                   dwCreationFlags,<br>
&nbsp; LPDWORD                 lpThreadId<br>
);
    </td>
    <td>
#define _OPEN_THREADS<br>
#include &lt;pthread.h&gt;<br>
int pthread_create(<br>
&nbsp; pthread_t *thread,<br>
&nbsp; pthread_attr_t *attr,<br>
&nbsp; void *(*start_routine) (void *arg),<br>
&nbsp; void *arg<br>
);
    </td>
    <td>
<pre><code>
int tmym_cmThrdCreate(
  tmym_thread *pHandle,
  tmym_thread_routine routine,
  void *pArg
);
</code></pre>
    </td>
  </tr>
  <tr>
    <th>スレッドの回収</th>
    <td>
<pre><code>
#include &lt;synchapi.h&gt;
DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD  dwMilliseconds
);

#include &lt;processthreadsapi.h&gt;
BOOL GetExitCodeThread(
  HANDLE  hThread,
  LPDWORD lpExitCode
);

#include &lt;handleapi.h&gt;
BOOL CloseHandle(
&nbsp; HANDLE hObject
);
</code></pre>
    </td>
    <td>
<pre><code>
#define _OPEN_THREADS
#include &lt;pthread.h&gt;
int pthread_join(
  pthread_t th,
  void **thread_return
);
</code></pre>
    </td>
    <td>
<pre><code>
int tmym_cmThrdWait(
  tmym_thread handle,
  void **pExitCode
);
</code></pre>
    </td>
  </tr>
  <tr>
    <th>スレッドの終了</th>
    <td>
<pre><code>
#include &lt;processthreadsapi.h&gt;
void ExitThread(
  DWORD dwExitCode
);
</code></pre>
    </td>
    <td>
<pre><code>
#define _OPEN_THREADS
#include &lt;pthread.h&gt;

void pthread_exit(
  void *status
);
</code></pre>
    </td>
    <td>
<pre><code>
#ifdef _WIN32
 #define tmym_cmThrdExit(status)  ExitThread(status)
#else
 #define tmym_cmThrdExit(status)  pthread_exit((void*)status)
#endif
</code></pre>
    </td>
  <tr>
</table>
\endhtmlonly

\htmlonly
<a onclick="history.back()">前のページへ戻る</a>
\endhtmlonly

*/

/****************************************************************//**
 * \file  tmym_common.h
 *
 * \brief  プログラムの共通機能を実装するモジュール
 *
 * - 文字コード・改行コード
 *     - 文字コードは UTF-8(BOM無し)
 *     - 改行コードは LF
 *
 * \author   tomyama  May 2022
 * \version  $Revision: 1.9 $
 * \date     $Date: 2026-10-18 01:54:26+09 $
 ********************************************************************/

#ifndef _TMYM_COMMON_H_
#define _TMYM_COMMON_H_

/********************************************************************/
/*        include files                                             */
/********************************************************************/

#include <stdio.h>
#include <stdint.h>         /* uint32_t, uint16_t, uint8_t */
#include <stdlib.h>         // malloc(), free()
#include <stdarg.h>         // vsprintf()
#include <string.h>         // strerror()
#include <errno.h>          // errno
#ifdef    _WIN32
 #include <stdlib.h>        /* _MAX_PATH */
 #include <windows.h>       // mutex
#else  /* _WIN32 */
 #include <limits.h>        /* PATH_MAX */
 #include <pthread.h>       // mutex
#endif /* _WIN32 */

// C11: 存在する場合のみインクルードする
#ifdef __has_include
 #if __has_include("config.h")
  #include "config.h"
 #endif
#endif


/********************************************************************/
/*        defines                                                   */
/********************************************************************/

#ifndef  _UNIT_TEST_
 #define _UNIT_TEST_        0
#endif

#ifdef    _WIN32
 #define TMYM_PATH_MAX  _MAX_PATH
#else  /* _WIN32 */
 #define TMYM_PATH_MAX  PATH_MAX
#endif /* _WIN32 */


#if _UNIT_TEST_
 #define INTERNAL       extern
 #define DBGP(fmt,...)   \
            tmym_cmFPrtf(stderr,"[DBG] %s[%d]: %s(): "fmt,__FILE__,__LINE__,__func__,##__VA_ARGS__)
#endif /* _UNIT_TEST_ */


#ifndef  INTERNAL
 #define INTERNAL       static
#endif

#ifndef  DBGP
 /*! \brief  output for debug, when define _UNIT_TEST_ */
 #define DBGP(fmt,...)
#endif

#ifndef  ERRNOP
 #define ERRNOP(prefix)   \
            if(errno){tmym_cmFPrtf(stderr,prefix": %s[%d]: %s(): errno=%d([31m%s[0m)\n",__FILE__,__LINE__,__func__,errno,strerror(errno));}
#endif

#ifndef  ERRP
 #define ERRP(fmt,...)   \
            ERRNOP("[31merror[0m");tmym_cmFPrtf(stderr,"[31merror[0m: %s[%d]: %s(): "fmt,__FILE__,__LINE__,__func__,##__VA_ARGS__)
#endif

#ifndef  WARN
 #define WARN(fmt,...)   \
            ERRNOP("[34mwarn[0m");tmym_cmFPrtf(stderr,"[34mwarn[0m: %s[%d]: %s(): "fmt,__FILE__,__LINE__,__func__,##__VA_ARGS__)
#endif


#define TMYM_SUCCESS    TMYM_FALSE
#define TMYM_FAILURE    TMYM_TRUE


#define CM_ALIGN_NUM_BUFF_SIZE 32
#define CM_ALIGN_NUM_OF_DIGITS_MAX ( CM_ALIGN_NUM_BUFF_SIZE - 1 )


/********************************************************************/
/*        types                                                     */
/********************************************************************/

#ifndef TMYM_TYPES
  #define TMYM_TYPES 1
typedef int8_t   tmym_s8;
typedef uint8_t  tmym_u8;
typedef int16_t  tmym_s16;
typedef uint16_t tmym_u16;
typedef int32_t  tmym_s32;
typedef uint32_t tmym_u32;
typedef int64_t  tmym_s64;
typedef uint64_t tmym_u64;

typedef FILE TMYM_FILE;

typedef size_t  tmym_size_t;

typedef enum _TMYM_BOOL{
    TMYM_FALSE =  0,
    TMYM_TRUE  =  1,
    TMYM_ERROR = -1
} TMYM_BOOL;
#endif /* TMYM_TYPES */


typedef enum _TMYM_CM_ENDIAN_E{
    TMYM_UNKNOWN_ENDIAN = -1,
    TMYM_LITTLE_ENDIAN  =  1,
    TMYM_BIG_ENDIAN     =  2
} TMYM_CM_ENDIAN_E;


#ifdef _WIN32
 #define tmym_cmThrdExit(status)  ExitThread(status)
 typedef HANDLE tmym_thread;
 typedef LPTHREAD_START_ROUTINE tmym_thread_routine;
#else
 #define tmym_cmThrdExit(status)  pthread_exit((void*)status)
 typedef pthread_t tmym_thread;
 typedef void *(*tmym_thread_routine)(void*);
#endif


/********************************************//**
 * \brief  ミューテックス
 ************************************************/

#ifdef _WIN32
 typedef void tmym_mutex;
#else
 typedef pthread_mutex_t tmym_mutex;
#endif


/********************************************//**
 * \brief  連結データのデータタイプ
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

typedef enum{
    CM_DATA_TYPE_UNKNOWN = 0,
    CM_DATA_TYPE_STACK,         //!< 先入れ後出し もしくは 後入れ先出し, LIFO もしくは FILO。
                                //!< 古いデータほど、長く残るデータ格納方式。
                                //!< 最初に入れたデータは、最後まで格納されており、それがなくなれば、格納場所は空になる。
    CM_DATA_TYPE_LIST,          //!< CM_DATA_TYPE_STACK と似ているが 挿入場所が 最後尾 ではなく 先頭 である点が異なる。
    CM_DATA_TYPE_FIFO,          //!< 先入れ先出し もしくは 後入れ後出し, FIFO もしくは LILO。
                                //!< 先に入ったものを先に処理して出し、後に入ってきたものは先に入ったものより後から処理して出す。
    CM_DATA_TYPE_SORT,          //!< 定義したキー順に整列される
    CM_DATA_TYPE_LEN
} cmCdType_e;


/********************************************//**
 * \brief  連結データの構造体
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

typedef struct _cmConsolidatedData{
    struct _cmConsolidatedData *pPrev;  //!< 前のデータへの参照
    struct _cmConsolidatedData *pNext;  //!< 次のデータへの参照
    void *pData;                        //!< 値 もしくは データへの参照
} cmConsolidatedData_s;


#ifndef _WIN32
typedef pthread_mutexattr_t tmym_mutexAttr;
#endif

typedef struct cmMutexResource{
    tmym_mutex *pMtx;
#ifndef _WIN32
    tmym_mutexAttr *pMtxAttr;
    tmym_mutex mutex;
    tmym_mutexAttr mutexAttr;
#endif
} cmMutexResource_s;


/********************************************//**
 * \brief  管理データの構造体
 * \sa \ref subpage_cmmn_consolidated_data
 ************************************************/

typedef struct _cmCdManager{
    cmMutexResource_s mutexRsrc;        //!< mutex
    cmConsolidatedData_s *pHead;        //!< 連結データの先頭への参照
    cmConsolidatedData_s *pTail;        //!< 連結データの最後尾への参照
    int iCdNums;                        //!< 連結データの数
    cmCdType_e type;                    //!< データタイプ（タイプによってデフォルトの操作APIが異なる）
    int  ( *Add     )( struct _cmCdManager*, void* );   //!< 操作API: 連結データの追加メソッド
    void*( *Eject   )( struct _cmCdManager* );          //!< 操作API: 連結データを取り出すメソッド
    void ( *Sort    )( struct _cmCdManager* );          //!< 操作API: 連結データを定義した順番で並べ替えるメソッド
    void*( *GetKey  )( cmConsolidatedData_s* );         //!< tmym_cmCdRemove(), tmym_cmCdInsert(), および .Sort() で使用する
    int  ( *Compare )( void*, void* );                  //!< .Sort() で使用する順序を判定するメソッド
    int  ( *Clean   )( struct _cmCdManager* );          //!< 操作API: 全ての連結データを削除するメソッド
} cmCdManager_s;


typedef enum{
    CM_STRCLN_REMOVE_BACKSPACE = 0x01,
    CM_STRCLN_REMOVE_ANSIESCSEQ = 0x02,
    CM_STRCLN_REMOVE_LEADING_WHITESPACE = 0x04,
    CM_STRCLN_REMOVE_TRAILING_WHITESPACE = 0x08,
    CM_STRCLN_REMOVE_LEADING_AND_TRAILING_WS = ( CM_STRCLN_REMOVE_LEADING_WHITESPACE | CM_STRCLN_REMOVE_TRAILING_WHITESPACE ),
    CM_STRCLN_GENERAL = ( CM_STRCLN_REMOVE_BACKSPACE | CM_STRCLN_REMOVE_LEADING_AND_TRAILING_WS ),
    CM_STRCLN_USE_ALL_FUNCTION = ( CM_STRCLN_GENERAL | CM_STRCLN_REMOVE_ANSIESCSEQ )
} TMYM_CM_STRCLN_E;


typedef enum{
    CM_ALIGN_NUM_RADIX_8_BIT    = 0x0008,
    CM_ALIGN_NUM_RADIX_10_BIT   = 0x0010,
    CM_ALIGN_NUM_RADIX_16U_BIT  = 0x0016,
    CM_ALIGN_NUM_RADIX_16L_BIT  = 0x0116,
    CM_ALIGN_NUM_RADIX_MASK     = 0x0FFF,
    CM_ALIGN_NUM_PADDING_ZERO_BIT = 0x1000,
    CM_ALIGN_NUM_RAD8_ZERO_PAD   = ( CM_ALIGN_NUM_RADIX_8_BIT   | CM_ALIGN_NUM_PADDING_ZERO_BIT ),
    CM_ALIGN_NUM_RAD10_ZERO_PAD  = ( CM_ALIGN_NUM_RADIX_10_BIT  | CM_ALIGN_NUM_PADDING_ZERO_BIT ),
    CM_ALIGN_NUM_RAD16U_ZERO_PAD = ( CM_ALIGN_NUM_RADIX_16U_BIT | CM_ALIGN_NUM_PADDING_ZERO_BIT ),
    CM_ALIGN_NUM_RAD16L_ZERO_PAD = ( CM_ALIGN_NUM_RADIX_16L_BIT | CM_ALIGN_NUM_PADDING_ZERO_BIT ),
    CM_ALIGN_NUM_RAD8_SPC_PAD   = ( CM_ALIGN_NUM_RADIX_8_BIT   ),
    CM_ALIGN_NUM_RAD10_SPC_PAD  = ( CM_ALIGN_NUM_RADIX_10_BIT  ),
    CM_ALIGN_NUM_RAD16U_SPC_PAD = ( CM_ALIGN_NUM_RADIX_16U_BIT ),
    CM_ALIGN_NUM_RAD16L_SPC_PAD = ( CM_ALIGN_NUM_RADIX_16L_BIT ),
    CM_ALIGN_NUM_RADIX_8   = CM_ALIGN_NUM_RAD8_ZERO_PAD,
    CM_ALIGN_NUM_RADIX_10  = CM_ALIGN_NUM_RAD10_SPC_PAD,
    CM_ALIGN_NUM_RADIX_16U = CM_ALIGN_NUM_RAD16U_ZERO_PAD,
    CM_ALIGN_NUM_RADIX_16L = CM_ALIGN_NUM_RAD16L_ZERO_PAD,
} CM_ALIGN_NUM_RADIX_E;


typedef struct{
    CM_ALIGN_NUM_RADIX_E radix;
    int numberOfDigits;
    char outBuf[ CM_ALIGN_NUM_BUFF_SIZE ];
} CM_ALIGN_NUM_S;


/********************************************************************/
/*        function prototype                                        */
/********************************************************************/

#ifndef NO_USE_GET_VERSION
extern char *cmmn_getVersion( char **pBuffer, int *pLenBuffer, const char *pPrefix );
#endif
extern int tmym_cmRegCleanFunc( void (*pFunc)( void ) );
#ifdef __cplusplus                      /* the __cplusplus macro is     */
 extern "C" void tmym_cmClean( void );  /* automatically defined by the  */
#else                                   /* C++⁄MVS compiler              */
 extern void tmym_cmClean( void );
#endif
extern TMYM_CM_ENDIAN_E tmym_cmEndian( void );
extern int tmym_cmSleep( int milliSec );
extern int tmym_cmThrdCreate( tmym_thread *pHandle, tmym_thread_routine routine, void *pArg );
extern int tmym_cmThrdWait( tmym_thread handle, void **pExitCode );
extern int tmym_cmMtxInit( cmMutexResource_s *pMR );
extern int tmym_cmMtxDestroy( cmMutexResource_s *pMR );
extern int tmym_cmMtxLock( cmMutexResource_s *pMR );
extern int tmym_cmMtxUnlock( cmMutexResource_s *pMR );
/* ConsolidatedData: begin */
    /* Manage */
extern int tmym_cmCdInit( cmCdManager_s *pCM, cmCdType_e type );
extern int tmym_cmCdDestroy( cmCdManager_s *pCM );
extern int tmym_cmCdClean( struct _cmCdManager *pCM );
extern int tmym_cmCdGetLen( cmCdManager_s *pCM );
extern int tmym_cmCdRegFuncClean( cmCdManager_s *pCM, int(*Clean)(struct _cmCdManager*) );
extern int tmym_cmCdRegFuncGetKey( cmCdManager_s *pCM, void*(*GetKey)(cmConsolidatedData_s*) );
extern int tmym_cmCdRegFuncCompare( cmCdManager_s *pCM, int(*Compare)(void*,void*) );
    /* Stack */
extern int tmym_cmCdPush( cmCdManager_s *pCM, void *pNewItem );
extern void *tmym_cmCdPop( cmCdManager_s *pCM );
    /* Common */
extern void *tmym_cmCdRemove( cmCdManager_s *pCM, void *pRmKey );
    /* List */
extern int tmym_cmCdUnshift( cmCdManager_s *pCM, void *pNewItem );
extern void *tmym_cmCdShift( cmCdManager_s *pCM );
    /* Sort */
extern int tmym_cmCdInsert( cmCdManager_s *pCM, void *pNewItem );
/* ConsolidatedData: end */
extern char *tmym_cmGetLine( FILE *fp, TMYM_BOOL bDiscarding );
extern char *tmym_cmGetStr( FILE *fp );
extern char tmym_cmGetOneChar( FILE *fp );
extern tmym_s32 tmym_cmGetNum( FILE *fp );
extern tmym_s32 tmym_cmStrLen( const char *pStr );
extern char *tmym_cmStrRemoveLeadingWhiteSpace( char *pStr );
extern char *tmym_cmStrRemoveTrailingWhiteSpace( char *pStr );
extern int tmym_cmStrRemoveBackSpace( char *pStr );
extern char *tmym_cmStrClean( char *pStr, TMYM_CM_STRCLN_E action );
extern void tmym_cmReverseOrder( tmym_u8 *pU8, tmym_size_t size );
extern int tmym_cmSetupAlignNumStruct( CM_ALIGN_NUM_S *pInfo, int numberOfDigits, tmym_size_t numberMax, CM_ALIGN_NUM_RADIX_E radix );
extern char *tmym_cmAlignNumber( tmym_size_t number, CM_ALIGN_NUM_S *pInfo );
extern char *tmym_cmAlignSeparator( char ch, CM_ALIGN_NUM_S *pInfo );
extern char *tmym_cmAlignLabel( char *pLabelStr, CM_ALIGN_NUM_S *pInfo );
extern int tmym_cmDataDump( char *buf, const int buflen, const int dmplen, const int col_len );
extern cmCdManager_s *tmym_cmDbgGetCmCdManAllocMem( void );
extern void *tmym_cmMalloc( size_t size );
extern void tmym_cmFree( void *ptr );
extern int tmym_cmPrtf( char *pFmt, ... );
extern int tmym_cmFPrtf( FILE *fp, char *pFmt, ... );


/********************************************************************/
/*        globals variable                                          */
/********************************************************************/


#endif /* _TMYM_COMMON_H_ */

//>cl /nologo /PD /Zc:preprocessor /EP tmp.c
// [ Dell Inspiron 15 Visual Studio 2019 x64 ]
//tmp.c
//
// #define _M_X64 100
// #define _MSC_EXTENSIONS 1
// #define _MSC_BUILD 0
// #define _WIN64 1
// #define _IS_ASSIGNABLE_NOCHECK_SUPPORTED 1
// #define _M_AMD64 100
// #define _MT 1
// #define _MSVC_WARNING_LEVEL 1L
// #define __STDC_HOSTED__ 1
// #define _MSC_FULL_VER 192930136
// #define _MSC_VER 1929
// #define _MSVC_TRADITIONAL 0
// #define _INTEGRAL_MAX_BITS 64
// #define _WIN32 1

//>cl /nologo /PD /Zc:preprocessor /EP tmp.c
// [ Dell Inspiron 15 Visual Studio 2019 x86 ]
//tmp.c
//
// #define _MSC_EXTENSIONS 1
// #define _MSC_BUILD 0
// #define _IS_ASSIGNABLE_NOCHECK_SUPPORTED 1
// #define _M_IX86_FP 2
// #define _MT 1
// #define _MSVC_WARNING_LEVEL 1L
// #define __STDC_HOSTED__ 1
// #define _MSC_FULL_VER 192930136
// #define _MSC_VER 1929
// #define _MSVC_TRADITIONAL 0
// #define _M_IX86 600
// #define _INTEGRAL_MAX_BITS 64
// #define _WIN32 1

// $ echo | gcc -dM -E -
// [ Dell Inspiron 15 Cygwin ]
// #endif /* _TMYM_TYPES_H_ */
// #define __DBL_MIN_EXP__ (-1021)
// #define __UINT_LEAST16_MAX__ 0xffff
// #define __ATOMIC_ACQUIRE 2
// #define __FLT128_MAX_10_EXP__ 4932
// #define __FLT_MIN__ 1.17549435082228750796873653722224568e-38F
// #define __GCC_IEC_559_COMPLEX 2
// #define __UINT_LEAST8_TYPE__ unsigned char
// #define __SIZEOF_FLOAT80__ 16
// #define __INTMAX_C(c) c ## L
// #define __CHAR_BIT__ 8
// #define __UINT8_MAX__ 0xff
// #define __SCHAR_WIDTH__ 8
// #define __WINT_MAX__ 0xffffffffU
// #define __FLT32_MIN_EXP__ (-125)
// #define __ORDER_LITTLE_ENDIAN__ 1234
// #define __SIZE_MAX__ 0xffffffffffffffffUL
// #define __WCHAR_MAX__ 0xffff
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1
// #define __DBL_DENORM_MIN__ ((double)4.94065645841246544176568792868221372e-324L)
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1
// #define __GCC_ATOMIC_CHAR_LOCK_FREE 2
// #define __GCC_IEC_559 2
// #define __FLT32X_DECIMAL_DIG__ 17
// #define __FLT_EVAL_METHOD__ 0
// #define __FLT64_DECIMAL_DIG__ 17
// #define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 2
// #define __UINT_FAST64_MAX__ 0xffffffffffffffffUL
// #define __SIG_ATOMIC_TYPE__ int
// #define __DBL_MIN_10_EXP__ (-307)
// #define __FINITE_MATH_ONLY__ 0
// #define __FLT32X_MAX_EXP__ 1024
// #define __FLT32_HAS_DENORM__ 1
// #define __UINT_FAST8_MAX__ 0xff
// #define __FLT32_MAX_10_EXP__ 38
// #define _stdcall __attribute__((__stdcall__))
// #define __DEC64_MAX_EXP__ 385
// #define __INT8_C(c) c
// #define __INT_LEAST8_WIDTH__ 8
// #define __UINT_LEAST64_MAX__ 0xffffffffffffffffUL
// #define __SHRT_MAX__ 0x7fff
// #define __LDBL_MAX__ 1.18973149535723176502126385303097021e+4932L
// #define __FLT64X_MAX_10_EXP__ 4932
// #define __LDBL_IS_IEC_60559__ 2
// #define __FLT64X_HAS_QUIET_NAN__ 1
// #define __UINT_LEAST8_MAX__ 0xff
// #define __GCC_ATOMIC_BOOL_LOCK_FREE 2
// #define __FLT128_DENORM_MIN__ 6.47517511943802511092443895822764655e-4966F128
// #define __UINTMAX_TYPE__ long unsigned int
// #define __DEC32_EPSILON__ 1E-6DF
// #define __FLT_EVAL_METHOD_TS_18661_3__ 0
// #define __unix 1
// #define __UINT32_MAX__ 0xffffffffU
// #define __FLT128_MIN_EXP__ (-16381)
// #define __WINT_MIN__ 0U
// #define __FLT128_MIN_10_EXP__ (-4931)
// #define __FLT32X_IS_IEC_60559__ 2
// #define __INT_LEAST16_WIDTH__ 16
// #define __SCHAR_MAX__ 0x7f
// #define __FLT128_MANT_DIG__ 113
// #define __WCHAR_MIN__ 0
// #define __INT64_C(c) c ## L
// #define __GCC_ATOMIC_POINTER_LOCK_FREE 2
// #define __FLT32X_MANT_DIG__ 53
// #define __USER_LABEL_PREFIX__
// #define __FLT64X_EPSILON__ 1.08420217248550443400745280086994171e-19F64x
// #define __STDC_HOSTED__ 1
// #define __DEC64_MIN_EXP__ (-382)
// #define __DBL_DIG__ 15
// #define __FLT32_DIG__ 6
// #define __FLT_EPSILON__ 1.19209289550781250000000000000000000e-7F
// #define __SHRT_WIDTH__ 16
// #define __FLT32_IS_IEC_60559__ 2
// #define __LDBL_MIN__ 3.36210314311209350626267781732175260e-4932L
// #define __STDC_UTF_16__ 1
// #define __DBL_IS_IEC_60559__ 2
// #define __DEC32_MAX__ 9.999999E96DF
// #define __FLT64X_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951F64x
// #define __FLT32X_HAS_INFINITY__ 1
// #define __INT32_MAX__ 0x7fffffff
// #define __unix__ 1
// #define __INT_WIDTH__ 32
// #define __SIZEOF_LONG__ 8
// #define __UINT16_C(c) c
// #define __DECIMAL_DIG__ 21
// #define __FLT64_EPSILON__ 2.22044604925031308084726333618164062e-16F64
// #define __FLT128_IS_IEC_60559__ 2
// #define __FLT64X_MIN_10_EXP__ (-4931)
// #define __LDBL_HAS_QUIET_NAN__ 1
// #define __FLT64_MANT_DIG__ 53
// #define __FLT64X_MANT_DIG__ 64
// #define __GNUC__ 11
// #define _cdecl __attribute__((__cdecl__))
// #define __MMX__ 1
// #define __FLT_HAS_DENORM__ 1
// #define __SIZEOF_LONG_DOUBLE__ 16
// #define __BIGGEST_ALIGNMENT__ 16
// #define __FLT64_MAX_10_EXP__ 308
// #define __DBL_MAX__ ((double)1.79769313486231570814527423731704357e+308L)
// #define _thiscall __attribute__((__thiscall__))
// #define __INT_FAST32_MAX__ 0x7fffffffffffffffL
// #define __DBL_HAS_INFINITY__ 1
// #define __SIZEOF_FLOAT__ 4
// #define __HAVE_SPECULATION_SAFE_VALUE 1
// #define __DEC32_MIN_EXP__ (-94)
// #define __INTPTR_WIDTH__ 64
// #define __FLT64X_HAS_INFINITY__ 1
// #define __UINT_LEAST32_MAX__ 0xffffffffU
// #define __FLT32X_HAS_DENORM__ 1
// #define __INT_FAST16_TYPE__ long int
// #define __MMX_WITH_SSE__ 1
// #define _fastcall __attribute__((__fastcall__))
// #define __LDBL_HAS_DENORM__ 1
// #define __FLT128_HAS_INFINITY__ 1
// #define __DEC32_MIN__ 1E-95DF
// #define __DBL_MAX_EXP__ 1024
// #define __WCHAR_WIDTH__ 16
// #define __FLT32_MAX__ 3.40282346638528859811704183484516925e+38F32
// #define __DEC128_EPSILON__ 1E-33DL
// #define __SSE2_MATH__ 1
// #define __ATOMIC_HLE_RELEASE 131072
// #define __PTRDIFF_MAX__ 0x7fffffffffffffffL
// #define __amd64 1
// #define __ATOMIC_HLE_ACQUIRE 65536
// #define __LONG_LONG_MAX__ 0x7fffffffffffffffLL
// #define __SIZEOF_SIZE_T__ 8
// #define __FLT64X_MIN_EXP__ (-16381)
// #define __SIZEOF_WINT_T__ 4
// #define __LONG_LONG_WIDTH__ 64
// #define __FLT32_MAX_EXP__ 128
// #define __GXX_ABI_VERSION 1016
// #define __FLT_MIN_EXP__ (-125)
// #define __INT16_MAX__ 0x7fff
// #define __x86_64 1
// #define __INT_FAST64_TYPE__ long int
// #define __FLT64_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F64
// #define __DBL_MIN__ ((double)2.22507385850720138309023271733240406e-308L)
// #define __FLT128_EPSILON__ 1.92592994438723585305597794258492732e-34F128
// #define __FLT64X_NORM_MAX__ 1.18973149535723176502126385303097021e+4932F64x
// #define __SIZEOF_POINTER__ 8
// #define __SIZE_TYPE__ long unsigned int
// #define __LP64__ 1
// #define __DBL_HAS_QUIET_NAN__ 1
// #define __FLT32X_EPSILON__ 2.22044604925031308084726333618164062e-16F32x
// #define __DECIMAL_BID_FORMAT__ 1
// #define __FLT64_MIN_EXP__ (-1021)
// #define __GXX_TYPEINFO_EQUALITY_INLINE 0
// #define __FLT64_MIN_10_EXP__ (-307)
// #define __FLT64X_DECIMAL_DIG__ 21
// #define __DEC128_MIN__ 1E-6143DL
// #define __REGISTER_PREFIX__
// #define __UINT16_MAX__ 0xffff
// #define __DBL_HAS_DENORM__ 1
// #define __cdecl __attribute__((__cdecl__))
// #define __LDBL_HAS_INFINITY__ 1
// #define __FLT32_MIN__ 1.17549435082228750796873653722224568e-38F32
// #define __UINT8_TYPE__ unsigned char
// #define __FLT_DIG__ 6
// #define __NO_INLINE__ 1
// #define __DEC_EVAL_METHOD__ 2
// #define __DEC128_MAX__ 9.999999999999999999999999999999999E6144DL
// #define __FLT_MANT_DIG__ 24
// #define __LDBL_DECIMAL_DIG__ 21
// #define __VERSION__ "11.2.0"
// #define __UINT64_C(c) c ## UL
// #define __SEH__ 1
// #define __INT_LEAST32_MAX__ 0x7fffffff
// #define __GCC_ATOMIC_INT_LOCK_FREE 2
// #define __FLT128_MAX_EXP__ 16384
// #define __FLT32_MANT_DIG__ 24
// #define __FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__
// #define __FLT128_HAS_DENORM__ 1
// #define __FLT32_DECIMAL_DIG__ 9
// #define __FLT128_DIG__ 33
// #define __INT32_C(c) c
// #define __DEC64_EPSILON__ 1E-15DD
// #define __ORDER_PDP_ENDIAN__ 3412
// #define __DEC128_MIN_EXP__ (-6142)
// #define __INT_FAST32_TYPE__ long int
// #define __UINT_LEAST16_TYPE__ short unsigned int
// #define unix 1
// #define __UINT64_MAX__ 0xffffffffffffffffUL
// #define __FLT_IS_IEC_60559__ 2
// #define __GNUC_WIDE_EXECUTION_CHARSET_NAME "UTF-16LE"
// #define __FLT64X_DIG__ 18
// #define __INT8_TYPE__ signed char
// #define __GCC_ASM_FLAG_OUTPUTS__ 1
// #define __UINT32_TYPE__ unsigned int
// #define __FLT_RADIX__ 2
// #define __INT_LEAST16_TYPE__ short int
// #define __LDBL_EPSILON__ 1.08420217248550443400745280086994171e-19L
// #define __UINTMAX_C(c) c ## UL
// #define __SSE_MATH__ 1
// #define __k8 1
// #define __FLT32X_MIN__ 2.22507385850720138309023271733240406e-308F32x
// #define __SIG_ATOMIC_MAX__ 0x7fffffff
// #define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 2
// #define __SIZEOF_PTRDIFF_T__ 8
// #define __LDBL_DIG__ 18
// #define __CYGWIN__ 1
// #define __FLT64_IS_IEC_60559__ 2
// #define __x86_64__ 1
// #define __FLT32X_MIN_EXP__ (-1021)
// #define __DEC32_SUBNORMAL_MIN__ 0.000001E-95DF
// #define __INT_FAST16_MAX__ 0x7fffffffffffffffL
// #define __FLT64_DIG__ 15
// #define __UINT_FAST32_MAX__ 0xffffffffffffffffUL
// #define __UINT_LEAST64_TYPE__ long unsigned int
// #define __FLT_HAS_QUIET_NAN__ 1
// #define __FLT_MAX_10_EXP__ 38
// #define __LONG_MAX__ 0x7fffffffffffffffL
// #define __FLT64X_HAS_DENORM__ 1
// #define __DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL
// #define __FLT_HAS_INFINITY__ 1
// #define __GNUC_EXECUTION_CHARSET_NAME "UTF-8"
// #define __UINT_FAST16_TYPE__ long unsigned int
// #define __DEC64_MAX__ 9.999999999999999E384DD
// #define __INT_FAST32_WIDTH__ 64
// #define __CHAR16_TYPE__ short unsigned int
// #define __PRAGMA_REDEFINE_EXTNAME 1
// #define __SIZE_WIDTH__ 64
// #define __SEG_FS 1
// #define __INT_LEAST16_MAX__ 0x7fff
// #define __DEC64_MANT_DIG__ 16
// #define __INT64_MAX__ 0x7fffffffffffffffL
// #define __SEG_GS 1
// #define __FLT32_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F32
// #define __SIG_ATOMIC_WIDTH__ 32
// #define __INT_LEAST64_TYPE__ long int
// #define __INT16_TYPE__ short int
// #define __INT_LEAST8_TYPE__ signed char
// #define __STDC_VERSION__ 201710L
// #define __SIZEOF_INT__ 4
// #define __DEC32_MAX_EXP__ 97
// #define __INT_FAST8_MAX__ 0x7f
// #define __FLT128_MAX__ 1.18973149535723176508575932662800702e+4932F128
// #define __INTPTR_MAX__ 0x7fffffffffffffffL
// #define __FLT64_HAS_QUIET_NAN__ 1
// #define __stdcall __attribute__((__stdcall__))
// #define __FLT32_MIN_10_EXP__ (-37)
// #define __FLT32X_DIG__ 15
// #define __GXX_MERGED_TYPEINFO_NAMES 0
// #define __PTRDIFF_WIDTH__ 64
// #define __LDBL_MANT_DIG__ 64
// #define __FLT64_HAS_INFINITY__ 1
// #define __FLT64X_MAX__ 1.18973149535723176502126385303097021e+4932F64x
// #define __SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)
// #define __GCC_ATOMIC_LONG_LOCK_FREE 2
// #define __DEC32_MANT_DIG__ 7
// #define __k8__ 1
// #define __INTPTR_TYPE__ long int
// #define __UINT16_TYPE__ short unsigned int
// #define __WCHAR_TYPE__ short unsigned int
// #define __pic__ 1
// #define __UINTPTR_MAX__ 0xffffffffffffffffUL
// #define __INT_FAST64_WIDTH__ 64
// #define __INT_FAST64_MAX__ 0x7fffffffffffffffL
// #define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
// #define __FLT_NORM_MAX__ 3.40282346638528859811704183484516925e+38F
// #define __FLT32_HAS_INFINITY__ 1
// #define __FLT64X_MAX_EXP__ 16384
// #define __UINT_FAST64_TYPE__ long unsigned int
// #define __INT_MAX__ 0x7fffffff
// #define __code_model_medium__ 1
// #define __INT64_TYPE__ long int
// #define __FLT_MAX_EXP__ 128
// #define __ORDER_BIG_ENDIAN__ 4321
// #define __DBL_MANT_DIG__ 53
// #define __SIZEOF_FLOAT128__ 16
// #define __INT_LEAST64_MAX__ 0x7fffffffffffffffL
// #define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 2
// #define __DEC64_MIN__ 1E-383DD
// #define __WINT_TYPE__ unsigned int
// #define __UINT_LEAST32_TYPE__ unsigned int
// #define __SIZEOF_SHORT__ 2
// #define __FLT32_NORM_MAX__ 3.40282346638528859811704183484516925e+38F32
// #define __SSE__ 1
// #define __LDBL_MIN_EXP__ (-16381)
// #define __FLT64_MAX__ 1.79769313486231570814527423731704357e+308F64
// #define __amd64__ 1
// #define __WINT_WIDTH__ 32
// #define __INT_LEAST8_MAX__ 0x7f
// #define __INT_LEAST64_WIDTH__ 64
// #define __LDBL_MAX_EXP__ 16384
// #define __FLT32X_MAX_10_EXP__ 308
// #define __SIZEOF_INT128__ 16
// #define __FLT64X_IS_IEC_60559__ 2
// #define __LDBL_MAX_10_EXP__ 4932
// #define __ATOMIC_RELAXED 0
// #define __DBL_EPSILON__ ((double)2.22044604925031308084726333618164062e-16L)
// #define __thiscall __attribute__((__thiscall__))
// #define __FLT128_MIN__ 3.36210314311209350626267781732175260e-4932F128
// #define _LP64 1
// #define __UINT8_C(c) c
// #define __FLT64_MAX_EXP__ 1024
// #define __INT_LEAST32_TYPE__ int
// #define __SIZEOF_WCHAR_T__ 2
// #define __UINT64_TYPE__ long unsigned int
// #define __GNUC_PATCHLEVEL__ 0
// #define __FLT128_NORM_MAX__ 1.18973149535723176508575932662800702e+4932F128
// #define __FLT64_NORM_MAX__ 1.79769313486231570814527423731704357e+308F64
// #define __FLT128_HAS_QUIET_NAN__ 1
// #define __INTMAX_MAX__ 0x7fffffffffffffffL
// #define __INT_FAST8_TYPE__ signed char
// #define __fastcall __attribute__((__fastcall__))
// #define __FLT64X_MIN__ 3.36210314311209350626267781732175260e-4932F64x
// #define __GNUC_STDC_INLINE__ 1
// #define __FLT64_HAS_DENORM__ 1
// #define __FLT32_EPSILON__ 1.19209289550781250000000000000000000e-7F32
// #define __DBL_DECIMAL_DIG__ 17
// #define __STDC_UTF_32__ 1
// #define __INT_FAST8_WIDTH__ 8
// #define __FXSR__ 1
// #define __FLT32X_MAX__ 1.79769313486231570814527423731704357e+308F32x
// #define __DBL_NORM_MAX__ ((double)1.79769313486231570814527423731704357e+308L)
// #define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// #define __INTMAX_WIDTH__ 64
// #define __UINT32_C(c) c ## U
// #define __FLT_DENORM_MIN__ 1.40129846432481707092372958328991613e-45F
// #define __INT8_MAX__ 0x7f
// #define __LONG_WIDTH__ 64
// #define __PIC__ 1
// #define __UINT_FAST32_TYPE__ long unsigned int
// #define __FLT32X_NORM_MAX__ 1.79769313486231570814527423731704357e+308F32x
// #define __CHAR32_TYPE__ unsigned int
// #define __FLT_MAX__ 3.40282346638528859811704183484516925e+38F
// #define __SSE2__ 1
// #define __INT32_TYPE__ int
// #define __SIZEOF_DOUBLE__ 8
// #define __FLT_MIN_10_EXP__ (-37)
// #define __FLT64_MIN__ 2.22507385850720138309023271733240406e-308F64
// #define __INT_LEAST32_WIDTH__ 32
// #define __INTMAX_TYPE__ long int
// #define __DEC128_MAX_EXP__ 6145
// #define __FLT32X_HAS_QUIET_NAN__ 1
// #define __ATOMIC_CONSUME 1
// #define __GNUC_MINOR__ 2
// #define __INT_FAST16_WIDTH__ 64
// #define __UINTMAX_MAX__ 0xffffffffffffffffUL
// #define __FLT32X_DENORM_MIN__ 4.94065645841246544176568792868221372e-324F32x
// #define __DBL_MAX_10_EXP__ 308
// #define __LDBL_DENORM_MIN__ 3.64519953188247460252840593361941982e-4951L
// #define __INT16_C(c) c
// #define __STDC__ 1
// #define __PTRDIFF_TYPE__ long int
// #define __ATOMIC_SEQ_CST 5
// #define __FLT32X_MIN_10_EXP__ (-307)
// #define __UINTPTR_TYPE__ long unsigned int
// #define __DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD
// #define __DEC128_MANT_DIG__ 34
// #define __LDBL_MIN_10_EXP__ (-4931)
// #define __SIZEOF_LONG_LONG__ 8
// #define __FLT128_DECIMAL_DIG__ 36
// #define __GCC_ATOMIC_LLONG_LOCK_FREE 2
// #define __FLT32_HAS_QUIET_NAN__ 1
// #define __FLT_DECIMAL_DIG__ 9
// #define __UINT_FAST16_MAX__ 0xffffffffffffffffUL
// #define __LDBL_NORM_MAX__ 1.18973149535723176502126385303097021e+4932L
// #define __GCC_ATOMIC_SHORT_LOCK_FREE 2
// #define __UINT_FAST8_TYPE__ unsigned char
// #define __ATOMIC_ACQ_REL 4
// #define __ATOMIC_RELEASE 3
// #define __declspec(x) __attribute__((x))
//
// [ Amazon Fire 7 ]
// #define _ILP32 1
// #define __ANDROID_API__ __ANDROID_MIN_SDK_VERSION__
// #define __ANDROID_MIN_SDK_VERSION__ 24
// #define __ANDROID__ 1
// #define __APCS_32__ 1
// #define __ARMEL__ 1
// #define __ARM_32BIT_STATE 1
// #define __ARM_ACLE 200
// #define __ARM_ARCH 7
// #define __ARM_ARCH_7A__ 1
// #define __ARM_ARCH_ISA_ARM 1
// #define __ARM_ARCH_ISA_THUMB 2
// #define __ARM_ARCH_PROFILE 'A'
// #define __ARM_EABI__ 1
// #define __ARM_FEATURE_CLZ 1
// #define __ARM_FEATURE_DSP 1
// #define __ARM_FEATURE_LDREX 0xf
// #define __ARM_FEATURE_QBIT 1
// #define __ARM_FEATURE_SAT 1
// #define __ARM_FEATURE_SIMD32 1
// #define __ARM_FEATURE_UNALIGNED 1
// #define __ARM_FP 0xc
// #define __ARM_FP16_ARGS 1
// #define __ARM_FP16_FORMAT_IEEE 1
// #define __ARM_NEON 1
// #define __ARM_NEON_FP 0x4
// #define __ARM_NEON__ 1
// #define __ARM_PCS 1
// #define __ARM_SIZEOF_MINIMAL_ENUM 4
// #define __ARM_SIZEOF_WCHAR_T 4
// #define __ARM_VFPV2__ 1
// #define __ARM_VFPV3__ 1
// #define __ATOMIC_ACQUIRE 2
// #define __ATOMIC_ACQ_REL 4
// #define __ATOMIC_CONSUME 1
// #define __ATOMIC_RELAXED 0
// #define __ATOMIC_RELEASE 3
// #define __ATOMIC_SEQ_CST 5
// #define __BIGGEST_ALIGNMENT__ 8
// #define __BITINT_MAXWIDTH__ 128
// #define __BOOL_WIDTH__ 8
// #define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
// #define __CHAR16_TYPE__ unsigned short
// #define __CHAR32_TYPE__ unsigned int
// #define __CHAR_BIT__ 8
// #define __CHAR_UNSIGNED__ 1
// #define __CLANG_ATOMIC_BOOL_LOCK_FREE 2
// #define __CLANG_ATOMIC_CHAR16_T_LOCK_FREE 2
// #define __CLANG_ATOMIC_CHAR32_T_LOCK_FREE 2
// #define __CLANG_ATOMIC_CHAR_LOCK_FREE 2
// #define __CLANG_ATOMIC_INT_LOCK_FREE 2
// #define __CLANG_ATOMIC_LLONG_LOCK_FREE 2
// #define __CLANG_ATOMIC_LONG_LOCK_FREE 2
// #define __CLANG_ATOMIC_POINTER_LOCK_FREE 2
// #define __CLANG_ATOMIC_SHORT_LOCK_FREE 2
// #define __CLANG_ATOMIC_WCHAR_T_LOCK_FREE 2
// #define __CONSTANT_CFSTRINGS__ 1
// #define __DBL_DECIMAL_DIG__ 17
// #define __DBL_DENORM_MIN__ 4.9406564584124654e-324
// #define __DBL_DIG__ 15
// #define __DBL_EPSILON__ 2.2204460492503131e-16
// #define __DBL_HAS_DENORM__ 1
// #define __DBL_HAS_INFINITY__ 1
// #define __DBL_HAS_QUIET_NAN__ 1
// #define __DBL_MANT_DIG__ 53
// #define __DBL_MAX_10_EXP__ 308
// #define __DBL_MAX_EXP__ 1024
// #define __DBL_MAX__ 1.7976931348623157e+308
// #define __DBL_MIN_10_EXP__ (-307)
// #define __DBL_MIN_EXP__ (-1021)
// #define __DBL_MIN__ 2.2250738585072014e-308
// #define __DECIMAL_DIG__ __LDBL_DECIMAL_DIG__
// #define __ELF__ 1
// #define __FINITE_MATH_ONLY__ 0
// #define __FLT16_DECIMAL_DIG__ 5
// #define __FLT16_DENORM_MIN__ 5.9604644775390625e-8F16
// #define __FLT16_DIG__ 3
// #define __FLT16_EPSILON__ 9.765625e-4F16
// #define __FLT16_HAS_DENORM__ 1
// #define __FLT16_HAS_INFINITY__ 1
// #define __FLT16_HAS_QUIET_NAN__ 1
// #define __FLT16_MANT_DIG__ 11
// #define __FLT16_MAX_10_EXP__ 4
// #define __FLT16_MAX_EXP__ 16
// #define __FLT16_MAX__ 6.5504e+4F16
// #define __FLT16_MIN_10_EXP__ (-4)
// #define __FLT16_MIN_EXP__ (-13)
// #define __FLT16_MIN__ 6.103515625e-5F16
// #define __FLT_DECIMAL_DIG__ 9
// #define __FLT_DENORM_MIN__ 1.40129846e-45F
// #define __FLT_DIG__ 6
// #define __FLT_EPSILON__ 1.19209290e-7F
// #define __FLT_EVAL_METHOD__ 0
// #define __FLT_HAS_DENORM__ 1
// #define __FLT_HAS_INFINITY__ 1
// #define __FLT_HAS_QUIET_NAN__ 1
// #define __FLT_MANT_DIG__ 24
// #define __FLT_MAX_10_EXP__ 38
// #define __FLT_MAX_EXP__ 128
// #define __FLT_MAX__ 3.40282347e+38F
// #define __FLT_MIN_10_EXP__ (-37)
// #define __FLT_MIN_EXP__ (-125)
// #define __FLT_MIN__ 1.17549435e-38F
// #define __FLT_RADIX__ 2
// #define __GCC_ATOMIC_BOOL_LOCK_FREE 2
// #define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 2
// #define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 2
// #define __GCC_ATOMIC_CHAR_LOCK_FREE 2
// #define __GCC_ATOMIC_INT_LOCK_FREE 2
// #define __GCC_ATOMIC_LLONG_LOCK_FREE 2
// #define __GCC_ATOMIC_LONG_LOCK_FREE 2
// #define __GCC_ATOMIC_POINTER_LOCK_FREE 2
// #define __GCC_ATOMIC_SHORT_LOCK_FREE 2
// #define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
// #define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 2
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1
// #define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1
// #define __GNUC_MINOR__ 2
// #define __GNUC_PATCHLEVEL__ 1
// #define __GNUC_STDC_INLINE__ 1
// #define __GNUC__ 4
// #define __GXX_ABI_VERSION 1002
// #define __ILP32__ 1
// #define __INT16_C_SUFFIX__
// #define __INT16_FMTd__ "hd"
// #define __INT16_FMTi__ "hi"
// #define __INT16_MAX__ 32767
// #define __INT16_TYPE__ short
// #define __INT32_C_SUFFIX__
// #define __INT32_FMTd__ "d"
// #define __INT32_FMTi__ "i"
// #define __INT32_MAX__ 2147483647
// #define __INT32_TYPE__ int
// #define __INT64_C_SUFFIX__ LL
// #define __INT64_FMTd__ "lld"
// #define __INT64_FMTi__ "lli"
// #define __INT64_MAX__ 9223372036854775807LL
// #define __INT64_TYPE__ long long int
// #define __INT8_C_SUFFIX__
// #define __INT8_FMTd__ "hhd"
// #define __INT8_FMTi__ "hhi"
// #define __INT8_MAX__ 127
// #define __INT8_TYPE__ signed char
// #define __INTMAX_C_SUFFIX__ LL
// #define __INTMAX_FMTd__ "lld"
// #define __INTMAX_FMTi__ "lli"
// #define __INTMAX_MAX__ 9223372036854775807LL
// #define __INTMAX_TYPE__ long long int
// #define __INTMAX_WIDTH__ 64
// #define __INTPTR_FMTd__ "d"
// #define __INTPTR_FMTi__ "i"
// #define __INTPTR_MAX__ 2147483647
// #define __INTPTR_TYPE__ int
// #define __INTPTR_WIDTH__ 32
// #define __INT_FAST16_FMTd__ "hd"
// #define __INT_FAST16_FMTi__ "hi"
// #define __INT_FAST16_MAX__ 32767
// #define __INT_FAST16_TYPE__ short
// #define __INT_FAST16_WIDTH__ 16
// #define __INT_FAST32_FMTd__ "d"
// #define __INT_FAST32_FMTi__ "i"
// #define __INT_FAST32_MAX__ 2147483647
// #define __INT_FAST32_TYPE__ int
// #define __INT_FAST32_WIDTH__ 32
// #define __INT_FAST64_FMTd__ "lld"
// #define __INT_FAST64_FMTi__ "lli"
// #define __INT_FAST64_MAX__ 9223372036854775807LL
// #define __INT_FAST64_TYPE__ long long int
// #define __INT_FAST64_WIDTH__ 64
// #define __INT_FAST8_FMTd__ "hhd"
// #define __INT_FAST8_FMTi__ "hhi"
// #define __INT_FAST8_MAX__ 127
// #define __INT_FAST8_TYPE__ signed char
// #define __INT_FAST8_WIDTH__ 8
// #define __INT_LEAST16_FMTd__ "hd"
// #define __INT_LEAST16_FMTi__ "hi"
// #define __INT_LEAST16_MAX__ 32767
// #define __INT_LEAST16_TYPE__ short
// #define __INT_LEAST16_WIDTH__ 16
// #define __INT_LEAST32_FMTd__ "d"
// #define __INT_LEAST32_FMTi__ "i"
// #define __INT_LEAST32_MAX__ 2147483647
// #define __INT_LEAST32_TYPE__ int
// #define __INT_LEAST32_WIDTH__ 32
// #define __INT_LEAST64_FMTd__ "lld"
// #define __INT_LEAST64_FMTi__ "lli"
// #define __INT_LEAST64_MAX__ 9223372036854775807LL
// #define __INT_LEAST64_TYPE__ long long int
// #define __INT_LEAST64_WIDTH__ 64
// #define __INT_LEAST8_FMTd__ "hhd"
// #define __INT_LEAST8_FMTi__ "hhi"
// #define __INT_LEAST8_MAX__ 127
// #define __INT_LEAST8_TYPE__ signed char
// #define __INT_LEAST8_WIDTH__ 8
// #define __INT_MAX__ 2147483647
// #define __INT_WIDTH__ 32
// #define __LDBL_DECIMAL_DIG__ 17
// #define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
// #define __LDBL_DIG__ 15
// #define __LDBL_EPSILON__ 2.2204460492503131e-16L
// #define __LDBL_HAS_DENORM__ 1
// #define __LDBL_HAS_INFINITY__ 1
// #define __LDBL_HAS_QUIET_NAN__ 1
// #define __LDBL_MANT_DIG__ 53
// #define __LDBL_MAX_10_EXP__ 308
// #define __LDBL_MAX_EXP__ 1024
// #define __LDBL_MAX__ 1.7976931348623157e+308L
// #define __LDBL_MIN_10_EXP__ (-307)
// #define __LDBL_MIN_EXP__ (-1021)
// #define __LDBL_MIN__ 2.2250738585072014e-308L
// #define __LITTLE_ENDIAN__ 1
// #define __LLONG_WIDTH__ 64
// #define __LONG_LONG_MAX__ 9223372036854775807LL
// #define __LONG_MAX__ 2147483647L
// #define __LONG_WIDTH__ 32
// #define __NO_INLINE__ 1
// #define __NO_MATH_ERRNO__ 1
// #define __OBJC_BOOL_IS_BOOL 0
// #define __OPENCL_MEMORY_SCOPE_ALL_SVM_DEVICES 3
// #define __OPENCL_MEMORY_SCOPE_DEVICE 2
// #define __OPENCL_MEMORY_SCOPE_SUB_GROUP 4
// #define __OPENCL_MEMORY_SCOPE_WORK_GROUP 1
// #define __OPENCL_MEMORY_SCOPE_WORK_ITEM 0
// #define __ORDER_BIG_ENDIAN__ 4321
// #define __ORDER_LITTLE_ENDIAN__ 1234
// #define __ORDER_PDP_ENDIAN__ 3412
// #define __PIC__ 2
// #define __PIE__ 2
// #define __POINTER_WIDTH__ 32
// #define __PRAGMA_REDEFINE_EXTNAME 1
// #define __PTRDIFF_FMTd__ "d"
// #define __PTRDIFF_FMTi__ "i"
// #define __PTRDIFF_MAX__ 2147483647
// #define __PTRDIFF_TYPE__ int
// #define __PTRDIFF_WIDTH__ 32
// #define __REGISTER_PREFIX__
// #define __SCHAR_MAX__ 127
// #define __SHRT_MAX__ 32767
// #define __SHRT_WIDTH__ 16
// #define __SIG_ATOMIC_MAX__ 2147483647
// #define __SIG_ATOMIC_WIDTH__ 32
// #define __SIZEOF_DOUBLE__ 8
// #define __SIZEOF_FLOAT__ 4
// #define __SIZEOF_INT__ 4
// #define __SIZEOF_LONG_DOUBLE__ 8
// #define __SIZEOF_LONG_LONG__ 8
// #define __SIZEOF_LONG__ 4
// #define __SIZEOF_POINTER__ 4
// #define __SIZEOF_PTRDIFF_T__ 4
// #define __SIZEOF_SHORT__ 2
// #define __SIZEOF_SIZE_T__ 4
// #define __SIZEOF_WCHAR_T__ 4
// #define __SIZEOF_WINT_T__ 4
// #define __SIZE_FMTX__ "X"
// #define __SIZE_FMTo__ "o"
// #define __SIZE_FMTu__ "u"
// #define __SIZE_FMTx__ "x"
// #define __SIZE_MAX__ 4294967295U
// #define __SIZE_TYPE__ unsigned int
// #define __SIZE_WIDTH__ 32
// #define __STDC_HOSTED__ 1
// #define __STDC_UTF_16__ 1
// #define __STDC_UTF_32__ 1
// #define __STDC_VERSION__ 201710L
// #define __STDC__ 1
// #define __THUMB_INTERWORK__ 1
// #define __UINT16_C_SUFFIX__
// #define __UINT16_FMTX__ "hX"
// #define __UINT16_FMTo__ "ho"
// #define __UINT16_FMTu__ "hu"
// #define __UINT16_FMTx__ "hx"
// #define __UINT16_MAX__ 65535
// #define __UINT16_TYPE__ unsigned short
// #define __UINT32_C_SUFFIX__ U
// #define __UINT32_FMTX__ "X"
// #define __UINT32_FMTo__ "o"
// #define __UINT32_FMTu__ "u"
// #define __UINT32_FMTx__ "x"
// #define __UINT32_MAX__ 4294967295U
// #define __UINT32_TYPE__ unsigned int
// #define __UINT64_C_SUFFIX__ ULL
// #define __UINT64_FMTX__ "llX"
// #define __UINT64_FMTo__ "llo"
// #define __UINT64_FMTu__ "llu"
// #define __UINT64_FMTx__ "llx"
// #define __UINT64_MAX__ 18446744073709551615ULL
// #define __UINT64_TYPE__ long long unsigned int
// #define __UINT8_C_SUFFIX__
// #define __UINT8_FMTX__ "hhX"
// #define __UINT8_FMTo__ "hho"
// #define __UINT8_FMTu__ "hhu"
// #define __UINT8_FMTx__ "hhx"
// #define __UINT8_MAX__ 255
// #define __UINT8_TYPE__ unsigned char
// #define __UINTMAX_C_SUFFIX__ ULL
// #define __UINTMAX_FMTX__ "llX"
// #define __UINTMAX_FMTo__ "llo"
// #define __UINTMAX_FMTu__ "llu"
// #define __UINTMAX_FMTx__ "llx"
// #define __UINTMAX_MAX__ 18446744073709551615ULL
// #define __UINTMAX_TYPE__ long long unsigned int
// #define __UINTMAX_WIDTH__ 64
// #define __UINTPTR_FMTX__ "X"
// #define __UINTPTR_FMTo__ "o"
// #define __UINTPTR_FMTu__ "u"
// #define __UINTPTR_FMTx__ "x"
// #define __UINTPTR_MAX__ 4294967295U
// #define __UINTPTR_TYPE__ unsigned int
// #define __UINTPTR_WIDTH__ 32
// #define __UINT_FAST16_FMTX__ "hX"
// #define __UINT_FAST16_FMTo__ "ho"
// #define __UINT_FAST16_FMTu__ "hu"
// #define __UINT_FAST16_FMTx__ "hx"
// #define __UINT_FAST16_MAX__ 65535
// #define __UINT_FAST16_TYPE__ unsigned short
// #define __UINT_FAST32_FMTX__ "X"
// #define __UINT_FAST32_FMTo__ "o"
// #define __UINT_FAST32_FMTu__ "u"
// #define __UINT_FAST32_FMTx__ "x"
// #define __UINT_FAST32_MAX__ 4294967295U
// #define __UINT_FAST32_TYPE__ unsigned int
// #define __UINT_FAST64_FMTX__ "llX"
// #define __UINT_FAST64_FMTo__ "llo"
// #define __UINT_FAST64_FMTu__ "llu"
// #define __UINT_FAST64_FMTx__ "llx"
// #define __UINT_FAST64_MAX__ 18446744073709551615ULL
// #define __UINT_FAST64_TYPE__ long long unsigned int
// #define __UINT_FAST8_FMTX__ "hhX"
// #define __UINT_FAST8_FMTo__ "hho"
// #define __UINT_FAST8_FMTu__ "hhu"
// #define __UINT_FAST8_FMTx__ "hhx"
// #define __UINT_FAST8_MAX__ 255
// #define __UINT_FAST8_TYPE__ unsigned char
// #define __UINT_LEAST16_FMTX__ "hX"
// #define __UINT_LEAST16_FMTo__ "ho"
// #define __UINT_LEAST16_FMTu__ "hu"
// #define __UINT_LEAST16_FMTx__ "hx"
// #define __UINT_LEAST16_MAX__ 65535
// #define __UINT_LEAST16_TYPE__ unsigned short
// #define __UINT_LEAST32_FMTX__ "X"
// #define __UINT_LEAST32_FMTo__ "o"
// #define __UINT_LEAST32_FMTu__ "u"
// #define __UINT_LEAST32_FMTx__ "x"
// #define __UINT_LEAST32_MAX__ 4294967295U
// #define __UINT_LEAST32_TYPE__ unsigned int
// #define __UINT_LEAST64_FMTX__ "llX"
// #define __UINT_LEAST64_FMTo__ "llo"
// #define __UINT_LEAST64_FMTu__ "llu"
// #define __UINT_LEAST64_FMTx__ "llx"
// #define __UINT_LEAST64_MAX__ 18446744073709551615ULL
// #define __UINT_LEAST64_TYPE__ long long unsigned int
// #define __UINT_LEAST8_FMTX__ "hhX"
// #define __UINT_LEAST8_FMTo__ "hho"
// #define __UINT_LEAST8_FMTu__ "hhu"
// #define __UINT_LEAST8_FMTx__ "hhx"
// #define __UINT_LEAST8_MAX__ 255
// #define __UINT_LEAST8_TYPE__ unsigned char
// #define __USER_LABEL_PREFIX__
// #define __VERSION__ "Clang 14.0.3"
// #define __VFP_FP__ 1
// #define __WCHAR_MAX__ 4294967295U
// #define __WCHAR_TYPE__ unsigned int
// #define __WCHAR_UNSIGNED__ 1
// #define __WCHAR_WIDTH__ 32
// #define __WINT_MAX__ 4294967295U
// #define __WINT_TYPE__ unsigned int
// #define __WINT_UNSIGNED__ 1
// #define __WINT_WIDTH__ 32
// #define __arm 1
// #define __arm__ 1
// #define __clang__ 1
// #define __clang_literal_encoding__ "UTF-8"
// #define __clang_major__ 14
// #define __clang_minor__ 0
// #define __clang_patchlevel__ 3
// #define __clang_version__ "14.0.3 "
// #define __clang_wide_literal_encoding__ "UTF-32"
// #define __linux 1
// #define __linux__ 1
// #define __llvm__ 1
// #define __pic__ 2
// #define __pie__ 2
// #define __unix 1
// #define __unix__ 1
// #define linux 1
// #define unix 1
