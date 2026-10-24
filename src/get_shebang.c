/****************************************************************//**
 * \file  get_shebang.c
 *
 * \brief  スクリプトファイルのシバンを取得する
 *
 * - 文字コード・改行コード
 *     - 文字コードは UTF-8(BOM無し)
 *     - 改行コードは LF
 *
 * \author   tomyama  2026
 * \version  $Revision: 1.9 $
 * \date     $Date: 2026-10-24 12:52:23+09 $
 *
 * \example test/testcase_shebang_function.c
 ********************************************************************/

#include "get_shebang.h"
#include "config.h"
#include <errno.h>          /* errno */
#include <string.h>         /* strerror(), memset() */
#include <stdlib.h>         /* malloc(),free() */


/********************************************************************/
/*        defines                                                   */
/********************************************************************/

//#define GSI_DEBUG
#undef  GSI_DEBUG
#ifdef  GSI_DEBUG
 #define    MSG_PREFIX  "[GSI]"
 #define    DBGP(fmt,...)   \
 fprintf(stderr,MSG_PREFIX" %s[%d]: %s(): "fmt,__FILE__,__LINE__,__func__,__VA_ARGS__)
#else   /* GSI_DEBUG */
 #define    DBGP(fmt,...)
#endif /* GSI_DEBUG */


/********************************************************************/
/*        types                                                     */
/********************************************************************/


/********************************************************************/
/*        function prototype (static)                               */
/********************************************************************/

static void *_gsi_malloc( size_t size );
static size_t _gsi_fread( void *ptr, size_t size, size_t nmemb, FILE *stream );
static int _gsi_ferror( FILE *stream );
static void _safe_copy( char *dest, size_t dest_size, const char *src );
#ifdef EE_PLUGIN_DEV
static size_t _read_first_line( const wchar_t *const pFilename, char *const pBuff, const size_t size );
#else
static size_t _read_first_line( const char *const pFilename, char *const pBuff, const size_t size );
#endif

/********************************************************************/
/*        static value                                              */
/********************************************************************/


/********************************************************************/
/*        globals variable                                          */
/********************************************************************/

int stub_trigger_fread_error = 0;


/********************************************************************/
/*        function (global)                                         */
/********************************************************************/

#ifdef EE_PLUGIN_DEV
int get_shebang( const wchar_t *const pFilename, char *const pBuff, const size_t size )
#else
int get_shebang( const char *const pFilename, char *const pBuff, const size_t size )
#endif
{
    int ret_val = 0;
    char *buf;
    size_t first_line_bytes;
    const char BOM_UTF8[ 4 ] = { 0xEF, 0xBB, 0xBF, '\0'};
    const char BOM_UTF16BE[ 3 ] = { 0xFE, 0xFF, '\0'};
    const char BOM_UTF16LE[ 3 ] = { 0xFF, 0xFE, '\0'};
    const char BOM_UTF32BE[ 5 ] = { 0x00, 0x00, 0xFE, 0xFF, '\0'};
    const char BOM_UTF32LE[ 5 ] = { 0xFF, 0xFE, 0x00, 0x00, '\0'};
    const char *const SHEBANG_SIGNATURE = "#!";
    int offset;

    memset( pBuff, 0x00, size );

    buf = _gsi_malloc( size );
    if( buf == NULL ){
#ifndef EE_PLUGIN_DEV
        fprintf( stderr, "error: could not get buf memory\n" );
#endif
        goto FAIL_MALLOC;
    }

    first_line_bytes = _read_first_line( pFilename, buf, size );
    //printf( "first_line_bytes=%u, str=\"%s\"\n", first_line_bytes, buf );
    if( first_line_bytes == 0 ){
        goto FAIL_OPEN;
    }

    /* 「BOM があればシバンは動かないもの」と考える。
     * そもそもBOM付きファイルに実行権限は付与できない。
     */

    /*
     * Unicode BOM
     *
     *   Encoding    Byte Order Mark
     *   ----------  -------------------
     *   UTF-8       0xEF 0xBB 0xBF
     *   UTF-16  BE  0xFE 0xFF
     *           LE  0xFF 0xFE
     *   UTF-32  BE  0x00 0x00 0xFE 0xFF
     *           LE  0xFF 0xFE 0x00 0x00
     */

    offset = 0;
    if( strncmp( buf, BOM_UTF32LE, 4 ) == 0 ){
        //offset += 2;
        goto FAIL_BOM_CHECK;
    }else if( strncmp( buf, BOM_UTF32BE, 4 ) == 0 ){
        //offset += 2;
        goto FAIL_BOM_CHECK;
    }else if( strncmp( buf, BOM_UTF16LE, 2 ) == 0 ){
        //offset += 2;
        goto FAIL_BOM_CHECK;
    }else if( strncmp( buf, BOM_UTF16BE, 2 ) == 0 ){
        //offset += 2;
        goto FAIL_BOM_CHECK;
    }else if( strncmp( buf, BOM_UTF8, 3 ) == 0 ){
        //offset += 3;
        goto FAIL_BOM_CHECK;
    }
    //printf( "offset=%d\n", offset );

    /* Is it a "she-bang" or not? */
    if( strncmp( buf + offset, SHEBANG_SIGNATURE, 2 ) != 0 ){
        goto FAIL_SHEBANG_CHECK;
    }
    offset += 2;

    for( ; offset<first_line_bytes; offset++ ){
        char ch_curr = *( buf + offset );
        if( ch_curr == ' ' || ch_curr == '\t' ){
        }else{
            break;
        }
    }
    for( ; ( first_line_bytes - 1 ) > offset; first_line_bytes-- ){
        char ch_curr = *( buf + first_line_bytes - 1 );
        if( ch_curr == ' ' || ch_curr == '\t' ){
            *( buf + first_line_bytes - 1 ) = '\0';
        }else{
            break;
        }
    }

    //strncpy( pBuff, buf + offset, first_line_bytes - offset );
    _safe_copy( pBuff, first_line_bytes - offset + 1, buf + offset );

    free( buf );

    return 0;

FAIL_SHEBANG_CHECK:
    ret_val--;
FAIL_BOM_CHECK:
    ret_val--;
    free( buf );
FAIL_OPEN:
    ret_val--;
FAIL_MALLOC:
    ret_val--;
    return ret_val;
}

char *get_token_from_shebang( const char *const pSheBang, char *const pBuff, const size_t size )
{
    static char *pch = NULL;
    static int idx_next = 0;
    static size_t shebang_len = 0;
    char *pch_curr;
    unsigned int idx;

    if( pSheBang != NULL ){
        memset( pBuff, 0x00, size );

        shebang_len = strlen( pSheBang );
        size_t copy_len = ( shebang_len + 1 );
        if( copy_len > size ){
#ifndef EE_PLUGIN_DEV
            fprintf( stderr,
                "error: copy_len=%zu, size=%zu: The size of pBuff is insufficient.\n",
                copy_len, size );
#endif
            return NULL;
        }
        memcpy( pBuff, pSheBang, copy_len );
        pch = NULL;
        idx_next = 0;
        int seq = 0;
        for( idx=0, pch_curr=pBuff; idx<shebang_len; idx++, pch_curr++ ){
            switch( *pch_curr ){
                case ' ':
                case '\t':
                    *pch_curr = '\0';
                    if( seq == 1 ){
                        seq++;
                    }
                    break;
                default:
                    if( pch == NULL ){
                        seq++;
                        pch = pch_curr;
                    }else if( seq == 2 ){
                        seq++;
                        idx_next = idx;
                    }
                    break;
            }
        }
    }else{
        pch = NULL;
        int seq = 0;
        for( idx=idx_next, pch_curr=( pBuff + idx_next ); idx<=shebang_len; idx++, pch_curr++ ){
            switch( *pch_curr ){
                case '\0':
                    if( seq == 1 ){
                        seq++;
                        idx_next = idx;
                    }
                    break;
                default:
                    if( seq == 0 ){
                        seq++;
                        pch = ( pBuff + idx_next );
                    }else if( seq == 2 ){
                        seq++;
                        idx_next = idx;
                    }
                    break;
            }
            if( seq == 3 ){
                break;
            }
        }
        if( pch == NULL ){
            // 最初からトークンを返せるように準備しておく
            idx_next = 0;
        }
    }

    return pch;
}

#ifdef EE_PLUGIN_DEV
char *get_shebang_interpreter( const wchar_t *const pFilename, char *const pBuff, const size_t size )
#else
char *get_shebang_interpreter( const char *const pFilename, char *const pBuff, const size_t size )
#endif
{
    char *pShebang;
    int ret;
    char *token;
    char *pInterpreter;

    pShebang = _gsi_malloc( size );
    if( pShebang == NULL ){
#ifndef EE_PLUGIN_DEV
        fprintf( stderr, "error: could not get pShebang memory\n" );
#endif
        return NULL;
    }
    ret = get_shebang( pFilename, pShebang, size );
    if( ret != 0 ){
#ifndef EE_PLUGIN_DEV
        fprintf( stderr, "error: get_shebang( \"%s\" ): failure\n", pFilename );
#endif
        free( pShebang );
        return NULL;
    }
    DBGP( "ret=%d, shebang=\"%s\"\n", ret, pShebang );

    char *BIN_ENV = "/usr/bin/env";
    char *interpreters[] = { "java",
                             "node",
                             "perl",
                             "php",
                             "python",
                             "ruby" };
    int interpreters_len = ( sizeof interpreters ) / ( sizeof interpreters[ 0 ] );
    //printf( "interpreters_len=%d\n", interpreters_len );

    token = get_token_from_shebang( pShebang, pBuff, size );
    free( pShebang );

    if( token == NULL ){
#ifndef EE_PLUGIN_DEV
        fprintf( stderr, "error: get_token_from_shebang( \"%s\" ): failure\n", pShebang );
#endif
        return NULL;
    }

    pInterpreter = NULL;
    // 終端'\0'まで完全一致するか否か
    if( strcmp( token, BIN_ENV ) != 0 ){
        pInterpreter = token;
        DBGP( "interpreter is \"%s\"\n", pInterpreter );
    }else{
        while( 1 ){
            token = get_token_from_shebang( NULL, pBuff, size );
            if( token == NULL ){
                break;
            }
            DBGP( "token=\"%s\"\n", token );

            // 絶対にインタプリタではない
            if( strncmp( token, "-", 1 ) == 0 ){
                continue;
            }

            for( int idx=0; idx<interpreters_len; idx++ ){
                char *ip = interpreters[ idx ];
                size_t len = strlen( ip );
                // 途中まで合っていればよし
                if( strncmp( token, ip, len ) == 0 ){
                    pInterpreter = token;
                    break;
                }
            }
            if( pInterpreter != NULL ){
                break;
            }
        }
        if( pInterpreter != NULL ){
            return pInterpreter;
        }

        // BIN_ENV はスルー
        token = get_token_from_shebang( NULL, pBuff, size );
        while( 1 ){
            token = get_token_from_shebang( NULL, pBuff, size );
            if( token == NULL ){
                break;
            }
            if( strncmp( token, "-", 1 ) != 0 ){
                pInterpreter = token;
                break;
            }
        }
    }

    return pInterpreter;
}

char *extract_interpreter_name( char *const pInterpreterPath )
{
    char *pInterpreterName;
    int idx2;
    char ch;

    //printf( "*pInterpreterPath=\"%s\"\n", pInterpreterPath );
    int len = ( int )strlen( pInterpreterPath );
    //printf( "  len=%d\n", len );
    for( idx2=len-1; idx2>0; idx2-- ){
        ch = *( pInterpreterPath + idx2 );
        if( ch == '/' ){
            idx2 += 1;
            //printf( "  ch=%c, idx2=%d\n", ch, idx2 );
            break;
        }
    }
    pInterpreterName = pInterpreterPath + idx2;
    //printf( "  *pInterpreterName=\"%s\"\n", pInterpreterName );

    return pInterpreterName;
}


/********************************************************************/
/*        function (static)                                         */
/********************************************************************/

static void *_gsi_malloc( size_t size )
{
    if( size == 0 ){
        return NULL;
    }
    return malloc( size );
}

static size_t _gsi_fread( void *ptr, size_t size, size_t nmemb, FILE *stream )
{
    if( stub_trigger_fread_error ){
        fprintf( stderr, "_gsi_fread(): stub_trigger_fread_error=%d\n", stub_trigger_fread_error );
        errno = stub_trigger_fread_error;
        return 0;
    }
    return fread( ptr, size, nmemb, stream );
}

static int _gsi_ferror( FILE *stream )
{
    if( stub_trigger_fread_error ){
        int tmp_stub_trigger_fread_error = stub_trigger_fread_error;
        stub_trigger_fread_error = 0;
        fprintf( stderr, "_gsi_ferror(): stub_trigger_fread_error=%d\n", tmp_stub_trigger_fread_error );
        if( tmp_stub_trigger_fread_error != EINTR ){
            return 1;
        }
    }
    return ferror( stream );
}

static void _safe_copy( char *dest, size_t dest_size, const char *src )
{
#ifdef HAVE_STRNCPY_S
    // Windows(MSVC) などのセキュア関数が使える場合
    strncpy_s( dest, dest_size, src, _TRUNCATE );
#else
    // セキュア関数がない環境（Linux等）の代替処理
    strncpy( dest, src, dest_size - 1 );
    dest[ dest_size - 1 ] = '\0';
#endif
}

#ifdef EE_PLUGIN_DEV
static size_t _read_first_line( const wchar_t *const pFilename, char *const pBuff, const size_t size )
#else
static size_t _read_first_line( const char *const pFilename, char *const pBuff, const size_t size )
#endif
{
    FILE    *pfi;
    size_t  read_num, read_counter, read_sz;
    unsigned int     idx;
    size_t  first_line_sz;

#ifdef HAVE_FOPEN_S
 #ifdef EE_PLUGIN_DEV
    errno_t err = _wfopen_s( &pfi, pFilename, L"r" );
 #else  /* EE_PLUGIN_DEV */
    errno_t err = fopen_s( &pfi, pFilename, "r" );
 #endif /* EE_PLUGIN_DEV */
    if( err != 0 ){
#else   /* HAVE_FOPEN_S */
    pfi = fopen( pFilename, "r" );
    if( pfi == NULL ){
#endif  /* HAVE_FOPEN_S */

#ifndef EE_PLUGIN_DEV
 #ifdef HAVE_STRERROR_S
        char msgbuf[ 1024 ];
        ( void )strerror_s( msgbuf, sizeof msgbuf, err );
 #else  /* HAVE_STRERROR_S */
        char *msgbuf = strerror( errno );
 #endif /* HAVE_STRERROR_S */
        fprintf( stderr, "%s: %s: <%d>%s\n",
            __FUNCTION__, pFilename, errno, msgbuf );
#endif  /* EE_PLUGIN_DEV */
        return 0;
    }

    memset( pBuff, 0x00, size );
    read_num = size - 1;
    read_counter = _gsi_fread( pBuff, 1, read_num, pfi );
    read_sz = read_counter * 1;
    //printf( "read_num=%zu, read_counter=%zu, read_sz=%zu\n", read_num, read_counter, read_sz );
    if( read_sz < read_num ){
        if( feof( pfi ) ){
            //fprintf( stderr, "%s: EOF found\n", __FUNCTION__ );
            clearerr( pfi );    /* EOF指示子をクリアする */
        }else if( _gsi_ferror( pfi ) ){
#ifndef EE_PLUGIN_DEV
            fprintf( stderr, "%s: %d: fread() error\n",
                __FUNCTION__, ferror( pfi ) );
#endif
            clearerr( pfi );    /* エラー指示子をクリアする */
            fclose( pfi );
            return 0;           /* 関数を抜ける */
        }
    }

    fclose( pfi );

    first_line_sz = 0;
    for( idx=0; idx<read_sz; idx++ ){
        char ch = *( pBuff + idx );
        /* LF or CR */
        if( ch == 0x0A || ch == 0x0D ){
            *( pBuff + idx ) = '\0';
            first_line_sz = idx;
            break;
        }
    }

    return first_line_sz;
}