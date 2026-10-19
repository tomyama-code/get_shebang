/****************************************************************//**
 * \file  main.c
 *
 * \brief  シバンからインタプリタ名を取得する
 *
 * - get_shebang_interpreter というコマンドとしてビルド
 *   - EmEditorの外部コマンド「exec_script.sh」などで使用している
 *
 * - 文字コード・改行コード
 *     - 文字コードは UTF-8(BOM無し)
 *     - 改行コードは LF
 *
 * \author   tomyama  2026
 * \version  $Revision$
 * \date     $Date$
 ********************************************************************/

#include <stdio.h>
#include "get_shebang.h"


#define BUFF_SIZE 1024


/**
 * @brief プログラムのエントリポイント
 *
 * @sa get_shebang_interpreter(), extract_interpreter_name()
 */
int main( int argc, char *argv[] )
{
    int ret_val = 0;
    static char buff[ BUFF_SIZE ];

    //! - 1. 引数の数だけループ
    if( argc > 1 ){
        for( int idx=1; idx<argc; idx++ ){
            char *pFilePath = argv[ idx ];

            //! - 2. インタプリタ（のパス）を取得
            char *pInterpreter = get_shebang_interpreter( pFilePath, buff, BUFF_SIZE );
            if( pInterpreter == NULL ){
                fprintf( stderr, "error: get_shebang_interpreter(): failure\n" );
                ret_val = 1;
            }else{
                //! - 3. パスからインタプリタ名を抽出して出力
                char *pInterpreterName = extract_interpreter_name( pInterpreter );
                printf( "%s\n", pInterpreterName );
            }
        }
    }

    return ret_val;
}

