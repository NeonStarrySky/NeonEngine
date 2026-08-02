import os
import shutil
import argparse

import chardet


def detect_encoding(file_path):
    with open(file_path, "rb") as f:
        data = f.read()

    result = chardet.detect(data)

    return result["encoding"] or "utf-8"



def backup_file(file_path):
    backup = file_path + ".bak"

    if not os.path.exists(backup):
        shutil.copy2(file_path, backup)



def convert_file(file_path, dst_encoding):

    try:
        src_encoding = detect_encoding(file_path)

        with open(
            file_path,
            "r",
            encoding=src_encoding
        ) as f:
            content = f.read()


        backup_file(file_path)


        with open(
            file_path,
            "w",
            encoding=dst_encoding
        ) as f:
            f.write(content)


        print(
            f"[OK] {file_path}"
            f" ({src_encoding} -> {dst_encoding})"
        )


    except Exception as e:
        print(
            f"[ERR] {file_path}: {e}"
        )



def convert_directory(
        directories,
        extensions,
        dst_encoding
):

    extensions = {
        "." + e.lower().lstrip(".")
        for e in extensions
    }


    for directory in directories:

        for root, _, files in os.walk(directory):

            for file in files:

                ext = os.path.splitext(file)[1].lower()

                if ext in extensions:

                    convert_file(
                        os.path.join(root, file),
                        dst_encoding
                    )



def main():

    parser = argparse.ArgumentParser(
        description="代码文件编码转换工具",
        epilog="""
    功能:
      自动检测文件原编码，并转换为指定编码。
      支持单个文件转换和目录批量转换。
      修改前会自动创建 .bak 备份文件。

    示例:
      单文件:
        python convert.py --file main.cpp

      批量目录:
        python convert.py D:\\Project -e cpp h hpp

      指定目标编码:
        python convert.py --file main.cpp --dst utf-8
    """
    )


    parser.add_argument(
        "--file",
        help="转换单个文件"
    )


    parser.add_argument(
        "directories",
        nargs="*",
        help="需要扫描的目录"
    )


    parser.add_argument(
        "-e",
        "--extensions",
        nargs="+",
        default=[],
        help="文件后缀，例如 cpp h hpp"
    )


    parser.add_argument(
        "--dst",
        default="utf-8-sig",
        help="目标编码"
    )


    args = parser.parse_args()


    # 单文件模式
    if args.file:

        if not os.path.exists(args.file):
            print("文件不存在")
            return

        convert_file(
            args.file,
            args.dst
        )

        return



    # 目录模式

    if not args.directories:
        print("没有指定文件或目录")
        return


    if not args.extensions:
        print("目录模式必须指定后缀")
        return


    convert_directory(
        args.directories,
        args.extensions,
        args.dst
    )



if __name__ == "__main__":
    main()
