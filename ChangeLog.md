# Change Log

## [Unreleased]
### Added
- xxx
### Changed
- xxx


## [0.0.8] - 2026-10-25
### Changed
- Create the top page for GitHub Pages.
  - `.github/workflows/doxygen.yml`
  - `_site_index.tar.gz`
- Updated the various procedure scripts located in the `build` directory.

## [0.0.7] - 2026-10-24
### Added
- `build/test_results_for_each_env.pl`: A script that aggregates test results.

### Changed
- Fixed a bug in the handling of shebang lines that specified an interpreter not found in the dictionary.
  - `src/get_shebang.c`
  - `test/testcase_shebang_function.c`
- Added a process to publish a list of test results to GitHub Pages.
  - `.github/workflows/doxygen.yml`
- Updated the various procedure scripts located in the `build` directory.

## [0.0.6] - 2026-10-23
### Added
- `build/utCounter.pl`: A script that aggregates test results.
- `test/testfiles/perl_script_ws_is_tab_pl`: Shebang using a tab character

### Changed
- Updated the various procedure scripts located in the `build` directory.
- Added test cases for detecting tab characters.
  - `test/testcase_shebang_function.c`
- Fixed a bug that prevented the generation of `unittest_Result.tab` in an MSVC environment.
  - `test/unit_test.c`

## [0.0.5] - 2026-10-21
### Added
- `build/parse_config_h.pl`
  - A script to convert "config.h" for use in Perl and shell scripts.

### Changed
- Updated the various procedure scripts located in the `build` directory.

## [0.0.4] - 2026-10-20
### Changed
- Enabled coverage measurement for Clang environments.

## [0.0.3] - 2026-10-19
### Added
- Added a workflow to generate "GitHub Pages".
  - `.github/workflows/doxygen.yml`

## [0.0.2] - 2026-10-19
### Changed
- I forgot to grant execution permissions, so I’ve fixed that.
  - `build/build_unix.sh`
  - `build/collect_file.sh`
  - `build/create_arch_dir.pl`
  - `build/create_archive.pl`

## [0.0.1] - 2026-10-19
### Added
- First commit.
- Environment in which operation was verified
  - amd64, FreeBSD_15.1-RELEASE-p4, Clang
  - AMD64, Windows, MSVC
  - AMD64, Windows, MSVC_x86
  - armv7l, Linux, Clang ( Android Termux )
  - x86_64, CentOS Stream 9, GNU
  - x86_64, CYGWIN_NT-10.0-19045, GNU
- The committed files are as follows:
  - `.clang-format`
  - `.gitignore`
  - `build/build_msvc.bat`
  - `build/build_unix.sh`
  - `build/CMakeLists.txt`
  - `build/collect_file.sh`
  - `build/create_arch_dir.pl`
  - `build/create_archive.pl`
  - `ChangeLog.md`
  - `CMakeLists.txt`
  - `config.h.in`
  - `Doxyfile`
  - `include/get_shebang.h`
  - `LICENSE`
  - `README.md`
  - `src/CMakeLists.txt`
  - `src/get_shebang.c`
  - `src/main.c`
  - `test/CMakeLists.txt`
  - `test/testcase_shebang_function.c`
  - `test/testfiles/broken_shebang`
  - `test/testfiles/broken_shebang_2`
  - `test/testfiles/env_go_script`
  - `test/testfiles/env_java_11`
  - `test/testfiles/env_java_25`
  - `test/testfiles/env_perl_pl`
  - `test/testfiles/env_python2_py`
  - `test/testfiles/env_python3_py`
  - `test/testfiles/env_python_py`
  - `test/testfiles/perl_script_bom_pl`
  - `test/testfiles/perl_script_crlf_pl`
  - `test/testfiles/perl_script_cr_pl`
  - `test/testfiles/perl_script_pl`
  - `test/testfiles/perl_script_utf16be_bom_pl`
  - `test/testfiles/perl_script_utf16le_bom_pl`
  - `test/testfiles/perl_script_utf32be_bom_pl`
  - `test/testfiles/perl_script_utf32le_bom_pl`
  - `test/testfiles/shell_script_bash`
  - `test/testfiles/shell_script_sh`
  - `test/testfiles/text_txt`
  - `test/tmym_common.c`
  - `test/tmym_common.h`
  - `test/unit_test.c`
  - `test/unit_test.h`
  - `test/unit_test_user_define.c`
  - `test/unit_test_user_define.h`
