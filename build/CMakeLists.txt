cmake_minimum_required(VERSION 3.10)

# 最小限の言語有効化を行ってコンパイラ情報を強制ロード
project(EnvCheck LANGUAGES C)

# OS名とバージョンの取得
#  https://cmake.org/cmake/help/latest/command/cmake_host_system_information.html
cmake_host_system_information(RESULT HOSTNAME QUERY HOSTNAME)
cmake_host_system_information(RESULT PROCESSOR_NAME QUERY PROCESSOR_NAME)
cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)
cmake_host_system_information(RESULT OS_RELEASE QUERY OS_RELEASE)
cmake_host_system_information(RESULT OS_PLATFORM QUERY OS_PLATFORM)
cmake_host_system_information(RESULT DISTRIB_PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)

# コンパイラ名とバージョンの取得
set(COMPILER_ID ${CMAKE_C_COMPILER_ID})
set(COMPILER_VER ${CMAKE_C_COMPILER_VERSION})

# 「小文字化」や「スペースをアンダースコアに置換」して安全なディレクトリ名にする
#  https://cmake.org/cmake/help/latest/command/string.html
# 前後の空白を除去
string(REGEX REPLACE "^ *(.+) *$" "\\1" HOSTNAME "${HOSTNAME}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" PROCESSOR_NAME "${PROCESSOR_NAME}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" OS_NAME "${OS_NAME}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" OS_RELEASE "${OS_RELEASE}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" OS_PLATFORM "${OS_PLATFORM}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" DISTRIB_PRETTY_NAME "${DISTRIB_PRETTY_NAME}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" COMPILER_ID "${COMPILER_ID}")
string(REGEX REPLACE "^ *(.+) *$" "\\1" COMPILER_VER "${COMPILER_VER}")
# スペースをアンダースコアに置き換える
string(REPLACE " " "_" HOSTNAME "${HOSTNAME}")
string(REPLACE " " "_" PROCESSOR_NAME "${PROCESSOR_NAME}")
string(REPLACE " " "_" OS_NAME "${OS_NAME}")
string(REPLACE " " "_" OS_RELEASE "${OS_RELEASE}")
string(REPLACE " " "_" OS_PLATFORM "${OS_PLATFORM}")
string(REPLACE " " "_" DISTRIB_PRETTY_NAME "${DISTRIB_PRETTY_NAME}")
string(REPLACE " " "_" COMPILER_ID "${COMPILER_ID}")
string(REPLACE " " "_" COMPILER_VER "${COMPILER_VER}")

message(STATUS "HOSTNAME='${HOSTNAME}'")
message(STATUS "PROCESSOR_NAME='${PROCESSOR_NAME}'")
message(STATUS "OS_NAME='${OS_NAME}'")
message(STATUS "OS_RELEASE='${OS_RELEASE}'")
message(STATUS "OS_PLATFORM='${OS_PLATFORM}'")
message(STATUS "DISTRIB_PRETTY_NAME='${DISTRIB_PRETTY_NAME}'")
message(STATUS "COMPILER_ID='${COMPILER_ID}'")
message(STATUS "COMPILER_VER='${COMPILER_VER}'")

#                      Win10                Cygwin                              CentOS Stream 9          FreeBSD                    Android Termux
# HOSTNAME             "DESKTOP-I0VU1AC"    "DESKTOP-I0VU1AC"                   "pug"                    "bull"                     "localhost"
# PROCESSOR_NAME       "Unknown_P6_family"  "Unknown_P6_family"                 "Unknown_P6_family"      ""                         "Unknown_family"
# OS_NAME              "Windows"            "CYGWIN_NT-10.0-19045"              "Linux"                  "FreeBSD"                  "Linux"
# OS_RELEASE           "Personal"           "3.7.0-0.602.g8eedf9515282.x86_64"  "5.14.0-754.el9.x86_64"  "15.1-RELEASE-p3"          "4.9.117-g08fe75b-dirty"
# OS_PLATFORM          "AMD64"              "x86_64"                            "x86_64"                 "amd64"                    "armv7l"
# DISTRIB_PRETTY_NAME  ""                   ""                                  "CentOS_Stream_9"        "FreeBSD_15.1-RELEASE-p3"  ""
# COMPILER_ID          "MSVC"               "GNU"                               "GNU"                    "Clang"                    "Clang"
# COMPILER_VER         "19.44.35228.0"      "14.4.0"                            "11.5.0"                 "19.1.7"                   "21.1.8"

# 任意の形式に組み立てる（例: Cygwin_3.7.0-0.602_gcc_14.4.0）
set(DIR_NAME "${OS_NAME}_${OS_RELEASE}_${COMPILER_ID}_${COMPILER_VER}")

# シェルスクリプト側が受け取れるように標準出力に出力する
message(STATUS "OS_AND_COMPILER=${DIR_NAME}")

#-- OS_AND_COMPILER=Windows_Personal_MSVC_19.44.35228.0
#-- OS_AND_COMPILER=CYGWIN_NT-10.0-19045_3.7.0-0.602.g8eedf9515282.x86_64_GNU_14.4.0
#-- OS_AND_COMPILER=Linux_5.14.0-754.el9.x86_64_GNU_11.5.0
#-- OS_AND_COMPILER=FreeBSD_15.1-RELEASE-p3_Clang_19.1.7
#-- OS_AND_COMPILER=Linux_4.9.117-g08fe75b-dirty_Clang_21.1.8
