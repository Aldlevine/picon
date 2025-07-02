import argparse
import os
import pathlib
import typing

import PIL.Image

from dataclasses import dataclass
from textwrap import dedent


ImageFormat = typing.Literal["GS4", "GS4A1", "R5G6B5", "R5G5B5A1"]

PICON_HPP_INCLUDES = [
    "graphics/image.hpp",
]

PICON_IMAGE_NAMESPACE = "picon::graphics"
PICON_COLOR_NAMESPACE = f"{PICON_IMAGE_NAMESPACE}::color"

@dataclass
class ImportOptions:
    input_dir: pathlib.PurePath
    output_dir: pathlib.PurePath
    format: ImageFormat
    images_namespace: str


def main() -> None:
    parser = argparse.ArgumentParser("picon image importer")
    _ = parser.add_argument("-i", "--input-dir", required=True)
    _ = parser.add_argument("-o", "--output-dir", required=True)
    _ = parser.add_argument(
        "-f", "--format",
        choices=typing.get_args(ImageFormat),
        default="GS4")
    _ = parser.add_argument("--images-namespace", default="assets::images")
    args = parser.parse_args()
    # exits if parser cannot parse

    options = ImportOptions(
        input_dir=pathlib.PurePath(typing.cast(str, args.input_dir)),
        output_dir=pathlib.PurePath(typing.cast(str, args.output_dir)),
        format=typing.cast(ImageFormat, args.format),
        images_namespace=typing.cast(str, args.images_namespace)
    )

    os.makedirs(options.output_dir, exist_ok=True)

    generate_images_source(options)


def generate_images_source(options: ImportOptions) -> None:
    with open(options.output_dir.joinpath("images.hpp"), "w") as hpp_file:
        with open(file=options.output_dir.joinpath("images.cpp"), mode="w") as cpp_file:
            print(make_images_hpp_prefix(options), file=hpp_file)
            print(make_images_cpp_prefix(options), file=cpp_file)

            image_paths = [pathlib.PurePath(options.input_dir).joinpath(s) for s in os.listdir(options.input_dir)]
            for image_path in image_paths:
                if image_path.suffix.lower() != ".png":
                    continue
                image = PIL.Image.open(image_path)
                match options.format:
                    case "GS4": image = image.convert("L")
                    case "GS4A1": image = image.convert("LA")
                    case "R5G6B5": image = image.convert("RGB")
                    case "R5G5B5A1": image = image.convert("RGBA")
                name = image_path.stem

                image_data_decl = make_image_data_declaration(options.format, name, image)
                image_data_defn = make_image_data_definition(options.format, name, image)
                print("extern " + image_data_decl + ";", file=hpp_file)
                print(image_data_decl + " = " + image_data_defn + ";", file=cpp_file)

                image_decl = make_image_definition(options.format, name, image)
                print(image_decl + ";", file=hpp_file)

            print(make_images_hpp_suffix(options), file=hpp_file)
            print(make_images_cpp_suffix(options), file=cpp_file)


def make_images_hpp_prefix(options: ImportOptions) -> str:
    return dedent(f"""
    #pragma once

    { "\n".join(f"#include \"{f}\"" for f in PICON_HPP_INCLUDES) }

    namespace {options.images_namespace}
    {{
    """).strip()


def make_images_hpp_suffix(_options: ImportOptions) -> str:
    return "}"


def make_images_cpp_prefix(options: ImportOptions) -> str:
    return dedent(f"""
    #include "images.hpp"

    namespace {options.images_namespace}
    {{
    """).strip()


def make_images_cpp_suffix(_options: ImportOptions) -> str:
    return "}"


def make_image_data_definition(format: ImageFormat, _name: str, image: PIL.Image.Image) -> str:
    image_data = ""
    for y in range(image.height):
        image_data += "\n    "
        for x in range(image.width):
            image_data += make_color_str(format, typing.cast(tuple[int], image.getpixel((x,y)))) + ", "

    return f"{{ std::array<const {PICON_COLOR_NAMESPACE}::{format}, {image.width * image.height}>{{ {{ {image_data} }} }} }}"
    

def make_image_data_declaration(format: ImageFormat, name: str, image: PIL.Image.Image) -> str:
    image_data = ""
    for y in range(image.height):
        image_data += "\n    "
        for x in range(image.width):
            image_data += make_color_str(format, typing.cast(tuple[int], image.getpixel((x,y)))) + ", "
    return f"const {PICON_IMAGE_NAMESPACE}::ImageData<const {PICON_COLOR_NAMESPACE}::{format}, {image.width}, {image.height}> {name}_data"


def make_image_definition(format: ImageFormat, name: str, _image: PIL.Image.Image) -> str:
    return f"constexpr {PICON_IMAGE_NAMESPACE}::Image<const {PICON_COLOR_NAMESPACE}::{format}> {name} {{{name}_data}}"


def make_color_str(format: ImageFormat, value: tuple[int, ...]) -> str:
    if isinstance(value, int):
        value = tuple([value])
    match format:
        case "GS4": return f"{{{value[0] >> 4}}}"
        case "GS4A1": return f"{{{value[0] >> 4}, {value[1] >> 7}}}"
        case "R5G6B5": return f"{{{value[0] >> 3}, {value[1] >> 2}, {value[2] >> 3}}}"
        case "R5G5B5A1": return f"{{{value[0] >> 3}, {value[1] >> 3}, {value[2] >> 3}, {value[3] >> 7}}}"


if __name__ == "__main__":
    main()
