#!/usr/bin/env python3
import argparse
import os
import re
import json

def main():
    parser = argparse.ArgumentParser(
        description="Convert JSON gcov files to test case to function map")
    parser.add_argument("json_dir", metavar="DIR", type=str,
                        help="Path to directory containing the json files."
                        " Each subdirectory should be of the form"
                        " <test_script_name>/<executable_name>.gcov.json")
    parser.add_argument(
        "output_file", type=str, help="path to the file to process")

    args = parser.parse_args()
    collected_info = list()

    for subdir in os.listdir(args.json_dir):
        match = re.match(r'(\w+)_([\w-]+)_([\w-]+)', subdir)
        if not match:
            continue
        test_script_path = (
            f"{match.group(1)}/{match.group(2)}/{match.group(3)}.sh")
        subdir_path = os.path.join(args.json_dir, subdir)
        for entry in os.listdir(subdir_path):
            entry_path = os.path.join(subdir_path, entry)
            if (not os.path.isfile(entry_path)
                    or not entry.endswith(".gcov.json")):
                continue
            with open(entry_path, "r") as json_file:
                json_contents = json.load(json_file)
            for source_file in json_contents["files"]:
                for function in source_file["functions"]:
                    if not function["blocks_executed"]:
                        continue  # Skip unexecuted functions
                    info = {
                        "executable_name": json_contents["data_file"],
                        "source_file": source_file["file"],
                        "function_name": function["name"],
                        "basic_block_coverage": (
                            function["blocks_executed"] / function["blocks"]
                            if function["blocks"]
                            else 0
                        ),
                        "test_script": test_script_path
                    }
                    collected_info.append(info)

    with open(args.output_file, 'w') as output_file:
        json.dump(collected_info, output_file)

if __name__ == "__main__":
    main()
