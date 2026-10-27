cmake_minimum_required(VERSION 3.10)
project(get_shebang VERSION 0.0.11 LANGUAGES C)

# 依存オプション用のモジュールを読み込む
include(CMakeDependentOption)

# Termux環境かどうかの判定フラグを設定
if(DEFINED ENV{TERMUX_VERSION} OR CMAKE_SUBSYSTEM MATCHES "termux")
    set(IS_TERMUX TRUE)
    message(STATUS "IS_TERMUX is ON !")
endif()

if(MSVC)
    set(IS_MSVC TRUE)
    message(STATUS "IS_MSVC is ON !")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(IS_GNU TRUE)
    message(STATUS "IS_GNU is ON !")
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(IS_CLANG TRUE)
    message(STATUS "IS_CLANG is ON !")
else()
    set(UNKNOWN_DEV_ENV TRUE)
    message(STATUS "UNKNOWN DEVELOPMENT ENVIRONMENT !")
endif()

if(IS_MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# Coverageを有効にする条件分岐（例：COVERAGEオプションを定義）
# 消したい時: cmake -U USE_COVERAGE .
option(USE_COVERAGE "Coverage measurement" ON)

set(ALLOW_COVERAGE_COMPILER OFF)
if(IS_GNU OR IS_CLANG)
    set(ALLOW_COVERAGE_COMPILER ON)
endif()

# 条件付きオプションを定義
# 書式: cmake_dependent_option(変数名 "説明文" デフォルト値 "有効になる条件" 無効時の値)
cmake_dependent_option(
    ENABLE_COVERAGE
    "Enable coverage reporting"
    ON
    "USE_COVERAGE;ALLOW_COVERAGE_COMPILER"  # ONの条件
    OFF                                     # 条件を満たさない時は強制的にOFF（キャッシュにも残らない）
)

# システムのヘッダーファイル確認モジュールを読み込む
include(CheckIncludeFile)
check_include_file("unistd.h" HAVE_UNISTD_H)
# 関数チェック用のモジュールを読み込む
include(CheckSymbolExists)
# 必要なヘッダーファイルを指定してシンボル（関数）の存在をチェックする
# check_symbol_exists(関数名 "インクルードするヘッダー" 結果を入れる変数)
# キャッシュを消す方法 → cmake -U HAVE_VSPRINTF_S .
check_symbol_exists(strncpy_s  "string.h" HAVE_STRNCPY_S)
check_symbol_exists(strerror_s "string.h" HAVE_STRERROR_S)
check_symbol_exists(fopen_s    "stdio.h"  HAVE_FOPEN_S)
check_symbol_exists(sprintf_s  "stdio.h"  HAVE_SPRINTF_S)
check_symbol_exists(vsprintf_s "stdio.h"  HAVE_VSPRINTF_S)

# 独自のビルドオプション（例: デバッグ機能の有効化）
option(ENABLE_DEBUG "Enable debug features" OFF)

# config.h.in を元に、build配下に config.h を生成する
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

# サブディレクトリを登録
add_subdirectory(src)
add_subdirectory(test)

# テスト機能を有効化
enable_testing()

# CTestにテストを登録する（テスト名, 実行するプログラム）
add_test(NAME run_get_shebang_test COMMAND get_shebang_test)

install(TARGETS get_shebang_interpreter DESTINATION bin)
