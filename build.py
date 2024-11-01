# coding: UTF-8
# NEACビルドスクリプト
# このスクリプトを実行することで、NEACのすべてのプロジェクトを一括でビルドできます。
# gcc（Windowsならmingw-w64のgccを想定）があれば動作します。また、COMPILERの中身を"clang"に置き換えると、
# clangを使用してビルドすることができます。
# Pythonは、少なくとも3.13.0以降をインストールすることを推奨します。あまり古いと動作しません。
# makeやcmakeがインストールされている必要はありません。
#
# 【実行方法】
# 端末で、python build.py を入力してください
# Windowsで、.pyファイルの関連付けを行っている場合、ダブルクリックでも実行可能です。
#
# 【ビルド結果の出力先】
# オブジェクトファイルや実行可能ファイルは、以下の場所に生成されます。
#
#       ../bin          実行可能ファイル
#       ../obj          オブジェクトファイル

import subprocess
import glob
import os
import platform

COMPILER = "gcc"        # 使用するコンパイラ
OPTIMIZE = "-O3"        # 最適化オプション

# 指定されたディレクトリから数えて、cnt個前のディレクトリを取得する。
def get_previous_directory(source_path, cnt):
    # パスを区切り文字で分割し、スライスでcnt個前まで取得
    separated = source_path.split(os.sep)
    result = os.sep.join(separated[:-cnt])
    return result

# 指定されたパスのディレクトリが存在しなければ作成する。
def mkdir(folder_path):
    if (os.path.exists(folder_path) == False):
        os.mkdir(folder_path)

# コマンドを実行する
def execute_command(cmd):
    print(f">>{cmd}")
    subprocess.run(cmd, shell=True)

# プロジェクトをコンパイルする
def compile_project(project_dir, output_dir, include_list):
    os.makedirs(output_dir, exist_ok=True)  # ディレクトリが存在しない場合に作成

    # インクルードオプションを生成
    include_options = " ".join(f'-I "{include_file}"' for include_file in include_list or [])

    # ソースファイルをコンパイル
    for source_file in glob.glob(os.path.join(project_dir, "*.c")):
        base_name = os.path.splitext(os.path.basename(source_file))[0]
        
        # コマンドを生成し実行
        compile_command = (
            f"{COMPILER} {OPTIMIZE} -c \"{source_file}\" "
            f"-o \"{os.path.join(output_dir, base_name + '.o')}\" {include_options}"
        )
        execute_command(compile_command)

# link_dirsに指定されたディレクトリに存在する.oファイルをすべてリンクし、out_pathに指定されたファイルに保存する。
def link_files(output_path, additional_options, link_dirs):
    # オブジェクトファイルのパスを収集
    links = " ".join(
        f'"{file}"' 
        for dirname in (link_dirs or []) 
        for file in glob.glob(os.path.join(dirname, "*.o"))
    )
    
    # リンクコマンドを生成して実行
    link_command = f"{COMPILER} {links} -o \"{output_path}\" {additional_options}"
    execute_command(link_command)

# link_dirsに指定されたディレクトリに存在する.oファイルをすべてリンクし、実行可能ファイルを生成する。
def make_exec(proj_name, out_dir, link_dirs):
    system = platform.system()

    if system == "Linux":
        link_files(f"{out_dir}{os.sep}{proj_name}", "", link_dirs)
    elif system == "Windows":
        link_files(f"{out_dir}{os.sep}{proj_name}.exe", "", link_dirs)

# link_dirsに指定されたディレクトリに存在する.oファイルをすべてリンクし、共有ライブラリファイルを生成する。
def make_shared_lib(proj_name, out_dir, link_dirs):
    system = platform.system()
    
    if system == "Linux":
        link_files(f"{out_dir}{os.sep}{proj_name}.so", "-shared", link_dirs)
    elif system == "Windows":
        link_files(f"{out_dir}{os.sep}{proj_name}.dll", "-shared", link_dirs)

# すべてのプロジェクトをビルドする
def build():
    src_dir = os.path.dirname(__file__)                         # build.py（このスクリプト）の場所
    libneac_dir = f"{src_dir}{os.sep}libneac"                   # libneacのソースファイルの配置場所
    libneacdll_dir = f"{src_dir}{os.sep}libneacdll"             # libneacdllのソースファイルの配置場所
    libwavefile_dir = f"{src_dir}{os.sep}libwavefile"           # libwavefileのソースファイルの配置場所
    neac_dir = f"{src_dir}{os.sep}neac"                         # neacのソースファイルの配置場所

    project_dir = get_previous_directory(src_dir, 1)              # プロジェクトの配置場所
    object_dir = f"{project_dir}{os.sep}obj"                    # オブジェクトファイル(.oファイル)の配置場所の親ディレクトリ
    libneac_obj_path = f"{object_dir}{os.sep}libneac"           # libneacのオブジェクトファイルの配置場所
    libneacdll_obj_path = f"{object_dir}{os.sep}libneacdll"     # libneacdllのオブジェクトファイルの配置場所
    libwavefile_obj_path = f"{object_dir}{os.sep}libwavefile"   # libwavefileのオブジェクトファイルの配置場所
    neac_obj_path = f"{object_dir}{os.sep}neac"                 # neacのオブジェクトファイルの配置場所

    bin_dir = f"{project_dir}{os.sep}bin"                       # 実行ファイルの配置場所

    # 古い出力ファイルがあれば削除し、出力用ディレクトリを作り直す。
    mkdir(object_dir)
    mkdir(bin_dir)

    # libneacをコンパイル
    compile_project(libneac_dir, libneac_obj_path, None)

    # libwavefileをコンパイル
    compile_project(libwavefile_dir, libwavefile_obj_path, None)

    # neacをコンパイル
    compile_project(neac_dir, neac_obj_path, [f"{libneac_dir}{os.sep}include", f"{libwavefile_dir}{os.sep}include"])
    make_exec("neac", bin_dir, [libneac_obj_path, libwavefile_obj_path, neac_obj_path])

    # DLL版NEACのコンパイル
    compile_project(libneacdll_dir, libneacdll_obj_path, [f"{libneac_dir}{os.sep}include"])
    make_shared_lib("libneacdll", bin_dir, [libneac_obj_path, libneacdll_obj_path])

print(f"NEACのコンパイルを開始します。コンパイラは {COMPILER} が、最適化オプションは {OPTIMIZE} が使用されます。")
build()
print("コンパイルが終了しました。")