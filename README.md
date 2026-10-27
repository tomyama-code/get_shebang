# Get Shebang Project

## 概要
- スクリプトからシバンを読み取りインタプリタを特定する

## 役割・目的

- `get_shebang_interpreter` というコマンドとしてビルド
  - EmEditorの外部ツールでスクリプトか否かの判定で使用

![Image of the get_shebang_interpreter command execution](https://tomyama-code.github.io/get_shebang/get_shebang_interpreter.png)

- `shebang.h`, `shebang.c`, `config.h` は、EmEditorのプラグイン`SelectConfFromShebang.dll`でも使用
  - `config.h`はMSVC環境で作成したものを用いる
    - 正しい情報は、`plugin_select_config_from_shebang/collect_file.sh` を参照

## 構成

- CMakeを使用したマルチプラットフォームのプロジェクト
- ビルドはユーザが作成した`build/*環境用のディレクトリ*` でおこなう

## サイト

- Github（リポジトリ）
  - [https://github.com/tomyama-code/get_shebang](https://github.com/tomyama-code/get_shebang)
- [Github Pages](https://tomyama-code.github.io/get_shebang/)
  - ドキュメント（Doxygen）
    - [https://tomyama-code.github.io/get_shebang/doc/](https://tomyama-code.github.io/get_shebang/doc/)
  - テスト結果（lcov）
    - [https://tomyama-code.github.io/get_shebang/cov/](https://tomyama-code.github.io/get_shebang/cov/)

* * *

### ビルド方法

#### 事前準備

- 他プロジェクトから流用しているソースファイルをかき集める（スクリプトを実行）

```sh
./build/collect_file.sh
```

#### ビルド手順

- `build`配下に*環境用のディレクトリ*を作成する
  - ディレクトリ名を決めるのが面倒であれば、`./build/create_arch_dir.pl` を実行する
- 作成したディレクトリに移動してCMakeを実行する（各環境用のスクリプトを用意している）
- 上記手順を投入コマンドレベルで書き直すと次のようになる

##### GNU, もしくは Clang 環境の場合

- 動作確認環境
  - GNU環境: Cygwin, CentOS
  - Clang環境: FreeBSD, Android Termux

- 手順

```sh
./build/create_arch_dir.pl
cd "build/*環境用ディレクトリ*"
../build_unix.sh
```

##### MSVC 環境の場合

- 動作確認環境
  - Microsoft Visual Studio (MSVC環境)
    - 「Visual Studio 開発者コマンドプロンプト」で実行
      - 「x64 Native Tools Command Prompt」
      - 「x86 Native Tools Command Prompt」

- 手順

```bat
perl build/create_arch_dir.pl
cd "build/*環境用ディレクトリ*"
..\build_msvc.bat
```

##### 各環境用のビルドスクリプトについて

- スクリプトにしているが、特殊な手順はなくて通常のCMakeの作法通り（のはず）
- コードカバレッジの計測は（現時点では）GNU環境とClang環境で実施するようになっている

* * *

### その他の手順

#### `get_shebang_interpreter`のインストール

##### GNU, もしくは Clang 環境の場合

- 動作確認環境
  - GNU環境: Cygwin, CentOS
  - Clang環境: FreeBSD, Android Termux

- 手順
  - `build/*環境用ディレクトリ*`に移動してMakefileを使用

```sh
cd "build/*環境用ディレクトリ*"
make install
```

##### MSVC 環境の場合

- 動作確認環境
  - MSVC環境: Microsoft Visual Studio
    - 「Visual Studio 開発者コマンドプロンプト」で実行
      - 「x64 Native Tools Command Prompt」
      - 「x86 Native Tools Command Prompt」

- 手順
  - `build/*環境用ディレクトリ*`に移動してCMakeを使用
    - 手順的には下記のようになるはずだが、インストールしても使わないだろうから試していない（Cygwinでインストールしておけば充分）
      - 上記の理由で、インストール先（`CMAKE_INSTALL_PREFIX`）もデフォルトのまま変更していない（恐らく `%ProgramFiles%`, もしくは `%ProgramFiles(x86)%` のまま）
      - 将来的にMSVCでもインストールをおこないたくなった場合は、インストール先の指定は CMakeLists.txt ではなく、引数で与える方法を用いること（`../build_unix.sh`を参考に）

```sh
cd "build/*環境用ディレクトリ*"
cmake --install . --config Release
```


* * *


以上
