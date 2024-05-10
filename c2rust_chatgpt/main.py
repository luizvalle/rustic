"""
Usage example:
python main.py \
--openai-api-key=abdef-1234 \
--input-function-filepath=~/Desktop/function_src.txt \
--imports-filepath=~/Desktop/imports_src.txt \
--global-vars-filepath=~/Desktop/global_vars_src.txt \
--callsites-filepaths ~/Desktop/callsite1_src.txt ~/Desktop/callsite2_src.txt
"""

from openai import OpenAI
import argparse
import json
import os
import re


_DEFAULT_MODEL = "gpt-3.5-turbo"

_DEFAULT_MODEL_TEMPERATURE = 0.5

_DEFAULT_NUM_CANDIDATES = 1

_INIT_SYS_PROMPT = """\
C2Rust is a transpiler tool that uses unsafe Rust, raw pointers, and external C
API functions and datatypes to convert C code into Rust. However, the Rust code
that it generates is highly unnatural because of the use of features like unsafe
Rust, raw pointers, and the C API.

You are an expert Rust programmer tasked with adapating the C2Rust translations
to safe and idiomatic Rust.
"""

_TRANSLATION_PROMP_TEMPLATE = """\
Here is a transpiled function generated by C2Rust:
```rs
{function}
```

Here are the existing imports:
```rs
{imports}
```

Here are the global variables:
```rs
{global_vars}
```

Here are the places where this function gets called:
{callsites}

Convert the function to idiomatic Rust, meaning Rust code that does not make
use of features like unsafe, raw pointers, and the C API whenever possible.
Provide separate translations of the function, as well as all of its call sites.

Follow the following format for your output: Place the function translation
between the tags <FUNC> and </FUNC>. Place each callsite translation (in the
same order it appears above) between <CALL> and </CALL>. Place any new imports
you use in the translated code between <IMPORT> and </IMPORT>. Do not include
the ```rs and ``` in your output.
"""

_CALLSITE_TEMPLATE = """\
```rs
{callsite}
```
"""

def _parse_args():
    parser = argparse.ArgumentParser(description="Given a c2rust translation,"
                                     " convert it to safe idiomatic Rust")
    parser.add_argument(
        "--openai-api-key",
        type=str,
        dest="openai_api_key",
        required=True,
        help="OpenAI API key for authentication")
    parser.add_argument(
        "--model-version",
        type=str,
        dest="model_version",
        required=False,
        default=_DEFAULT_MODEL,
        help="Model version")
    parser.add_argument(
        "--model-temperature",
        type=float,
        dest="model_temperature",
        required=False,
        default=_DEFAULT_MODEL_TEMPERATURE,
        help="Model temparature ([0, 2])")
    parser.add_argument(
        "--num-candidates",
        type=int,
        dest="num_candidates",
        required=False,
        default=_DEFAULT_NUM_CANDIDATES,
        help="Number of candidate translations (> 0)")
    parser.add_argument(
        "--input-function-filepath",
        type=str,
        dest="input_function_filepath",
        required=True,
        help="Path to the file containing the function to convert")
    parser.add_argument(
        "--imports-filepath",
        type=str,
        dest="imports_filepath",
        required=True,
        help="Path to the file containing the file imports")
    parser.add_argument(
        "--global-vars-filepath",
        type=str,
        dest="global_vars_filepath",
        required=True,
        help="Path to the file containing the global variables")
    parser.add_argument(
        "--callsites-filepaths",
        nargs="+",
        dest="callsites_filepaths",
        required=True,
        help="A list of filepaths, each with a separate callsite")
    args = parser.parse_args()
    return args


def _get_file_content(filepath):
    with open(os.path.expanduser(filepath), "r") as file:
        content = file.read()
    return content

def main():
    args = _parse_args()

    function_src = _get_file_content(args.input_function_filepath)
    imports_src = _get_file_content(args.imports_filepath)
    global_vars_src = _get_file_content(args.global_vars_filepath)
    callsites_src = [_CALLSITE_TEMPLATE.format(
        callsite=_get_file_content(callsite_filepath))
        for callsite_filepath in args.callsites_filepaths]
    
    TRANSLATION_PROMPT = _TRANSLATION_PROMP_TEMPLATE.format(
        function=function_src,
        imports=imports_src,
        global_vars=global_vars_src,
        callsites="\n".join(callsites_src)
    )
    
    openai_client = OpenAI(api_key=args.openai_api_key)

    response = openai_client.chat.completions.create(
        model=args.model_version,
        messages=[
            {"role": "system", "content": _INIT_SYS_PROMPT},
            {"role": "user", "content": TRANSLATION_PROMPT},
        ],
        temperature=args.model_temperature,
        n=args.num_candidates
    )

    response_json = json.loads(response.model_dump_json())

    for i, choice in enumerate(response_json["choices"]):
        print(f"\n## OPTION {i + 1} ##\n")
        print(choice["message"]["content"])


if __name__ == "__main__":
    main()