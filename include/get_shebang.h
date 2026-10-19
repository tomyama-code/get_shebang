/****************************************************************//**
 * @file  get_shebang.h
 *
 * @brief  スクリプトファイルのシバンを取得する
 *
 * @details
 *
 * - 文字コード・改行コード
 *     - 文字コードは UTF-8(BOM無し)
 *     - 改行コードは LF
 *
 * ## 外部から指定可能なマクロ（ビルドオプション）一覧
 * ビルド時に以下のマクロを指定することで挙動を切り替える。
 *
 * - **EE_PLUGIN_DEV**:
 *   EmEditorのプラグイン開発でソースを使いまわす時に指定。
 *   - **影響範囲**: 少なくとも `"get_shebang.h"` をインクルードしているソースのコンパイル時に、このマクロの指定が必須となる。
 *   - **推奨設定**: 意図しないビルドエラーを防ぐため、プロジェクト全体（ビルドオプションの全体設定など）で常時定義しておくことを推奨。
 *
 * @author   tomyama  2026
 * @version  $Revision: 1.4 $
 * @date     $Date: 2026-10-18 16:19:25+09 $
 *
 * @example test/testcase_shebang_function.c
 ********************************************************************/

#ifndef _GET_SHEBANG_H_
#define _GET_SHEBANG_H_

//#define EE_PLUGIN_DEV

#include <stdio.h>  /* size_t */


/********************************************//**
 * \brief  単体テストの時に使用する変数
 ************************************************/

extern int stub_trigger_fread_error;


/********************************************************************/
/*        function prototype                                        */
/********************************************************************/

/********************************************//**
 * \brief  スクリプトのシバン（1行目）を取得する
 *
 * ```
 * "#! /usr/bin/env  -S  perl  -w  " -> "/usr/bin/env  -S  perl  -w"
 * ```
 *
 * \return  関数の成否を返す
 * \retval  0     正常終了
 * \retval  負値  失敗
 * \param[in]   pFilename  スクリプトファイルのパス
 * \param[out]  pBuff      シバンを格納するバッファ
 * \param[in]   size       bBuffの大きさ
 * \author  tomyama
 * \callgraph
 * \callergraph
 ************************************************/

#ifdef EE_PLUGIN_DEV
int get_shebang( const wchar_t *const pFilename, char *const pBuff, const size_t size );
#else
int get_shebang( const char *const pFilename, char *const pBuff, const size_t size );
#endif


/********************************************//**
 * \brief  シバンからトークンを切り出す
 *
 * ```
 * "/usr/bin/env  -S  perl  -w"
 *   -> "/usr/bin/env"
 *   -> "-S"
 *   -> "perl"
 *   -> "-w"
 *   -> NULL
 * ```
 *
 * 2回目以降は pSheBang にNULLを指定すること。
 *
 * \return  null終端されたトークンの文字列を返す
 * \retval  NULL以外  正常終了
 * \retval  NULL      一巡した、もしくは一巡するまでにトークンが見つからなかった
 * \param[in]   pSheBang  シバン
 * \param[out]  pBuff     トークンを格納するバッファ
 * \param[in]   size      bBuffの大きさ
 * \author  tomyama
 * \callgraph
 * \callergraph
 ************************************************/

char *get_token_from_shebang( const char *const pSheBang, char *const pBuff, const size_t size );


/********************************************//**
 * \brief  シバンからインタプリタを取得
 *
 * ```
 * "#! /usr/bin/env  -S  perl  -w" -> "perl"
 * "#! /usr/bin/perl -w  " -> "/usr/bin/perl"
 * ```
 *
 * \return  null終端されたインタプリタの文字列を返す
 * \retval  NULL以外  正常終了
 * \retval  NULL      失敗
 * \param[in]   pFilename  スクリプトファイルのパス
 * \param[out]  pBuff      トークンを格納するバッファ
 * \param[in]   size       bBuffの大きさ
 * \author  tomyama
 * \callgraph
 * \callergraph
 ************************************************/

#ifdef EE_PLUGIN_DEV
char *get_shebang_interpreter( const wchar_t *const pFilename, char *const pBuff, const size_t size );
#else
char *get_shebang_interpreter( const char *const pFilename, char *const pBuff, const size_t size );
#endif


/********************************************//**
 * \brief  インタプリタ（パス付）からインタプリタ名を取り出す
 *
 * ```
 * "/usr/bin/perl" -> "perl"
 * "perl" -> "perl"
 * ```
 *
 * \return  null終端されたインタプリタの文字列を返す
 * \retval  NULL以外  正常終了（常に正常終了を返す）
 * \param[in]   pInterpreterPath  null終端されたインタプリタの文字列
 * \author  tomyama
 * \callgraph
 * \callergraph
 ************************************************/

char *extract_interpreter_name( char *const pInterpreterPath );

#endif /* _GET_SHEBANG_H_ */
