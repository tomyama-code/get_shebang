
# # 生成された config.h がある build ディレクトリをインクルードパスに追加
# include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(get_shebang_obj OBJECT get_shebang.c)
add_library(get_shebang_ut_obj OBJECT get_shebang.c)
if(ENABLE_COVERAGE)
    message(STATUS "Coverage is enabled in subdirectory!")
    # サブディレクトリ内のターゲットにフラグを適用
    target_compile_options(get_shebang_ut_obj PRIVATE --coverage -g -O0)
endif()

# get_shebang_objターゲットに対して、config.h が生成されるディレクトリをインクルードパスに追加
target_include_directories(get_shebang_obj
  PUBLIC "${PROJECT_SOURCE_DIR}/include"
  PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(get_shebang_ut_obj
  PUBLIC "${PROJECT_SOURCE_DIR}/include"
  PRIVATE "${CMAKE_BINARY_DIR}")

# 実行ファイルの設定
add_executable(get_shebang_interpreter main.c $<TARGET_OBJECTS:get_shebang_obj>)

target_include_directories(get_shebang_interpreter
  PUBLIC "${PROJECT_SOURCE_DIR}/include")
