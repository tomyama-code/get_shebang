# ====================================================================
#  テストの設定
# ====================================================================

# テスト用の実行ファイルを定義
add_executable(get_shebang_test
    testcase_shebang_function.c
    tmym_common.c
    unit_test.c
    $<TARGET_OBJECTS:get_shebang_ut_obj>)

# テストプログラムに config.h のパスを通す
target_include_directories(get_shebang_test
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_BINARY_DIR}")

# スレッドをサポートする設定を読み込む
# （通常はルートのCMakeLists.txtに1度書けばOK）
# テストでしか使わない "tmym_common.c" が pthread_create() を
# 使っている為、ここで定義している
find_package(Threads REQUIRED)

# ターゲットにスレッドライブラリをリンク
target_link_libraries(get_shebang_test PRIVATE Threads::Threads)

if(ENABLE_COVERAGE)
    message(STATUS "Coverage is enabled in subdirectory!")
    # 明示的に gcov ライブラリをリンクする
    target_link_libraries(get_shebang_test PRIVATE gcov)
    # サブディレクトリ内のターゲットにフラグを適用
    target_link_options(get_shebang_test PRIVATE --coverage)
endif()
