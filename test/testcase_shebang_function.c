/****************************************************************//**
 * \file  testcase_shebang_function.c
 *
 * \brief  "get_shebang.c"の内部向け／外部向け関数をテストするモジュール
 *
 * - 文字コード・改行コード
 *     - 文字コードは UTF-8(BOM無し)
 *     - 改行コードは LF
 *
 * \author   tomyama  2026
 * \version  $Revision: 1.2 $
 * \date     $Date: 2026-10-23 09:02:28+09 $
 ********************************************************************/


/********************************************************************/
/*        include files                                             */
/********************************************************************/

#include <stdio.h>
#include "get_shebang.h"
#include "../../test/unit_test.h"


/********************************************************************/
/*        defines                                                   */
/********************************************************************/

#define BUFF_SZ 1024


/********************************************************************/
/*        types                                                     */
/********************************************************************/


/********************************************************************/
/*        function prototype (test target)                          */
/********************************************************************/


/********************************************************************/
/*        function prototype (static)                               */
/********************************************************************/

static void unittest_shebang_get_shebang( void );
static void unittest_shebang_get_shebang_interpreter( void );
static void unittest_shebang_extract_interpreter_name( void );


/********************************************************************/
/*        static value                                              */
/********************************************************************/

static UNTE_TESTFUNC_S testfuncs[] = {
    UNTE_DEF_FUNC(unittest_shebang_get_shebang),
    UNTE_DEF_FUNC(unittest_shebang_get_shebang_interpreter),
    UNTE_DEF_FUNC(unittest_shebang_extract_interpreter_name)
};


/********************************************************************/
/*        globals variable                                          */
/********************************************************************/

UNTE_MODULE_TESTS_S shebang_fn_tests = {
    "get_shebang.c",
    ( sizeof testfuncs ) / ( sizeof testfuncs[ 0 ] ),
    testfuncs
};


/********************************************************************/
/*        function (global)                                         */
/********************************************************************/


/********************************************************************/
/*        function (static)                                         */
/********************************************************************/

struct shebang_testfile_defined{
    char *pTestFile;
    char *pShebang;
    int iExpect_get_shebang;
    char *pExpect_get_shebang;
    char *pExpect_get_shebang_interpreter;
} shebang_testfiles[] = {
    {
        "../../test/testfiles/NON_EXISTENT_FILE",
        NULL,
        -2, "",
        NULL
    },
    {
        "../../test/testfiles/perl_script_pl",
        "#!/usr/bin/perl -w",
        0, "/usr/bin/perl -w",
        "/usr/bin/perl"
    },
    {
        "../../test/testfiles/perl_script_ws_is_tab_pl",
        "	#!/usr/bin/perl	-w	",
        0, "/usr/bin/perl	-w",
        "/usr/bin/perl"
    },
    {
        "../../test/testfiles/perl_script_crlf_pl",
        "#!/usr/bin/perl -w",
        0, "/usr/bin/perl -w",
        "/usr/bin/perl"
    },
    {
        "../../test/testfiles/perl_script_cr_pl",
        "#!/usr/bin/perl -w",
        0, "/usr/bin/perl -w",
        "/usr/bin/perl"
    },
    {
        "../../test/testfiles/perl_script_bom_pl",
        "#!/usr/bin/perl -w",
        -3, "",
        NULL
    },
    {
        "../../test/testfiles/perl_script_utf16be_bom_pl",
        "#!/usr/bin/perl -w",
        -3, "",
        NULL
    },
    {
        "../../test/testfiles/perl_script_utf16le_bom_pl",
        "#!/usr/bin/perl -w",
        -3, "",
        NULL
    },
    {
        "../../test/testfiles/perl_script_utf32be_bom_pl",
        "#!/usr/bin/perl -w",
        -3, "",
        NULL
    },
    {
        "../../test/testfiles/perl_script_utf32le_bom_pl",
        "#!/usr/bin/perl -w",
        -3, "",
        NULL
    },
    {
        "../../test/testfiles/env_go_script",
        "#!/usr/bin/env gorun",
        0, "/usr/bin/env gorun",
        "gorun"
    },
    {
        "../../test/testfiles/env_java_11",
        "#!/usr/bin/env -S java --source 11",
        0, "/usr/bin/env -S java --source 11",
        "java"
    },
    {
        "../../test/testfiles/env_java_25",
        "#!/usr/bin/env -S java --source 25",
        0, "/usr/bin/env -S java --source 25",
        "java"
    },
    {
        "../../test/testfiles/env_perl_pl",
        "#!  /usr/bin/env  -S  perl  -w  ",
        0, "/usr/bin/env  -S  perl  -w",
        "perl"
    },
    {
        "../../test/testfiles/env_python_py",
        "#!/usr/bin/env python",
        0, "/usr/bin/env python",
        "python"
    },
    {
        "../../test/testfiles/env_python2_py",
        "#!/usr/bin/env python2",
        0, "/usr/bin/env python2",
        "python2"
    },
    {
        "../../test/testfiles/env_python3_py",
        "#!/usr/bin/env python3",
        0, "/usr/bin/env python3",
        "python3"
    },
    {
        "../../test/testfiles/shell_script_bash",
        "#! /bin/bash",
        0, "/bin/bash",
        "/bin/bash"
    },
    {
        "../../test/testfiles/shell_script_sh",
        "#! /bin/sh",
        0, "/bin/sh",
        "/bin/sh"
    },
    {
        "../../test/testfiles/text_txt",
        "これは普通のテキスト。",
        -4, "",
        NULL
    },
};
#define SHEBANG_TESTFILES_NUM ( ( sizeof shebang_testfiles ) / ( sizeof shebang_testfiles[ 0 ] ) )

static void unittest_shebang_get_shebang( void )
{
    static char buff[ BUFF_SZ ];
    int ret;

    ret = get_shebang( "../../test/testfiles/perl_script_pl", buff, 0 );
    UNTE_ASSERT( ret == -1 );

    stub_trigger_fread_error = EBADF;
    ret = get_shebang( "../../test/testfiles/perl_script_pl", buff, BUFF_SZ );
    UNTE_ASSERT( ret == -2 );

    stub_trigger_fread_error = EINTR;
    ret = get_shebang( "../../test/testfiles/perl_script_pl", buff, BUFF_SZ );
    UNTE_ASSERT( ret == -2 );

    for( int idx=0; idx<SHEBANG_TESTFILES_NUM; idx++ ){
        struct shebang_testfile_defined *pTF = &( shebang_testfiles[ idx ] );
        ret = get_shebang( pTF->pTestFile, buff, BUFF_SZ );
        if( UNTE_ASSERT( ret == pTF->iExpect_get_shebang ) ){
            printf( "pTestFile=\"%s\": ret=%d\n", pTF->pTestFile, ret );
        }
        if( UNTE_ASSERT( strcmp( buff, pTF->pExpect_get_shebang ) == 0 ) ){
            printf( "pTestFile=\"%s\": buff=\"%s\"\n", pTF->pTestFile, buff );
        }
    }

    return;
}

static void unittest_shebang_get_shebang_interpreter( void )
{
    static char buff[ BUFF_SZ ];
    char *pInterpreter;

    pInterpreter = get_shebang_interpreter( "../../test/testfiles/perl_script_pl", buff, 0 );
    UNTE_ASSERT( pInterpreter == NULL );

    pInterpreter = get_shebang_interpreter( "../../test/testfiles/broken_shebang", buff, BUFF_SZ );
    if( UNTE_ASSERT( pInterpreter == NULL ) ){
        printf( "pTestFile=\"../../test/testfiles/broken_shebang\": pInterpreter=\"%s\"\n", pInterpreter );
    }

    pInterpreter = get_shebang_interpreter( "../../test/testfiles/broken_shebang_2", buff, BUFF_SZ );
    if( UNTE_ASSERT( pInterpreter == NULL ) ){
        printf( "pTestFile=\"../../test/testfiles/broken_shebang_2\": pInterpreter=\"%s\"\n", pInterpreter );
    }

    for( int idx=0; idx<SHEBANG_TESTFILES_NUM; idx++ ){
        struct shebang_testfile_defined *pTF = &( shebang_testfiles[ idx ] );
        pInterpreter = get_shebang_interpreter( pTF->pTestFile, buff, BUFF_SZ );
        if( pTF->pExpect_get_shebang_interpreter == NULL ){
            if( UNTE_ASSERT( pInterpreter == NULL ) ){
                printf( "pTestFile=\"%s\": pInterpreter=\"%s\"\n", pTF->pTestFile, pInterpreter );
            }
        }else{
            if( UNTE_ASSERT( strcmp( pInterpreter, pTF->pExpect_get_shebang_interpreter ) == 0 ) ){
                printf( "pTestFile=\"%s\": buff=\"%s\"\n", pTF->pTestFile, buff );
            }
        }
    }

    return;
}

static void unittest_shebang_extract_interpreter_name( void )
{
    char *pInterpreterName;

    pInterpreterName = extract_interpreter_name( "/usr/bin/perl" );
    UNTE_ASSERT( strcmp( pInterpreterName, "perl" ) == 0 );

    pInterpreterName = extract_interpreter_name( "perl" );
    UNTE_ASSERT( strcmp( pInterpreterName, "perl" ) == 0 );

    return;
}
