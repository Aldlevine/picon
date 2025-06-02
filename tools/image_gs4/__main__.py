# pyright: basic

import os
import json
import pathlib
import sys
from io import TextIOWrapper
from typing import Sequence, cast, TypedDict

from PIL import Image


class FontJSON_rect(TypedDict):
    x: int
    y: int
    w: int
    h: int


class FontJSON_size(TypedDict):
    w: int
    h: int


class FontJSON_frame(TypedDict):
    filename: str
    frame: FontJSON_rect
    rotated: bool
    trimmed: bool
    spriteSourceSize: FontJSON_rect
    sourceSize: FontJSON_size
    duration: int


class FontJSON_cel(TypedDict):
    frame: int
    data: str


class FontJSON_layer(TypedDict):
    name: str
    opacity: int
    blendMode: str
    cels: Sequence[FontJSON_cel]


class FontJSON_meta(TypedDict):
    app: str
    version: str
    image: str
    format: str
    size: FontJSON_size
    scale: str
    layers: Sequence[FontJSON_layer]


class FontJSON(TypedDict):
    font_path: str
    frames: Sequence[FontJSON_frame]
    meta: FontJSON_meta


def make_image_dims_even(img: Image.Image) -> Image.Image:
    width = img.width
    height = img.height

    # if width % 2 == 1:
    #     width += 1

    # if height % 2 == 1:
    #     height += 1

    return img.crop((0, 0, width, height))


def load_image(image_path: str) -> Image.Image:
    img = Image.open(image_path).convert("L")
    img = make_image_dims_even(img)
    return img


def make_image_decl(
    decl_file: TextIOWrapper, image_path: str, font_json: FontJSON | None = None
) -> None:
    img = load_image(image_path)
    name = pathlib.PurePath(image_path).stem
    if font_json:
        name += "_image"
    decl_file.write(
        # f"extern const Image<ImageFormat::GS4_HMSB, {img.width}, {img.height}> {name};\n"
        f"extern const Image<ImageFormat::GS4, {img.width}, {img.height}> {name};\n"
    )


def make_image_impl(
    impl_file: TextIOWrapper, image_path: str, font_json: FontJSON | None = None
) -> None:
    img = load_image(image_path)

    name = pathlib.PurePath(image_path).stem
    if font_json:
        name += "_image"

    pixels = ((px // 16) & 0x0F for px in cast(Sequence[int], img.getdata()))
    pixel_data = bytearray()

    for px0 in pixels:
        # px1 = next(pixels, 0)
        # byte = (px0 << 4) | px1
        # pixel_data.append(byte)
        pixel_data.append(px0)

    impl_file.write(
        # f"const Image<ImageFormat::GS4_HMSB, {img.width}, {img.height}> {name}{{{{"
        f"const Image<ImageFormat::GS4, {img.width}, {img.height}> {name}{{{{"
    )
    for i, byte in enumerate(pixel_data):
        # if i % (img.width // 2) == 0:
        if i % (img.width) == 0:
            impl_file.write("\n    ")
        impl_file.write(f"(uint8_t)(0x{byte:02X}), ")
    impl_file.write("\n}};\n")


def make_images_source(assets_dir: str, output_decl: str, output_impl: str) -> None:
    image_paths = os.listdir(assets_dir)

    os.makedirs(os.path.dirname(output_decl), 511, True)
    os.makedirs(os.path.dirname(output_impl), 511, True)

    with open(output_decl, "w") as decl_file:
        with open(output_impl, "w") as impl_file:
            decl_file.write(f"#pragma once\n\n")
            decl_file.write('#include "graphics/graphics.hpp"\n\n')
            decl_file.write("namespace pg::assets\n{\n\n")

            impl_file.write(f'#include "{output_decl}"\n\n')
            impl_file.write("namespace pg::assets\n{\n\n")

            for image_path in image_paths:
                if not image_path.endswith(".png"):
                    continue
                make_image_decl(decl_file, assets_dir + "/" + image_path)
                make_image_impl(impl_file, assets_dir + "/" + image_path)

            decl_file.write("\n}")
            impl_file.write("\n}")


def get_font_json(font_path: str) -> FontJSON:
    with open(font_path, "r") as f:
        result = json.load(f)
    result["font_path"] = font_path
    return result


def get_font_image_path(font_json: FontJSON) -> str:
    return os.path.join(
        os.path.dirname(font_json["font_path"]), font_json["meta"]["image"]
    )


def make_font_decl(decl_file: TextIOWrapper, font_path: str) -> None:
    font_json = get_font_json(font_path)
    image_path = get_font_image_path(font_json)

    make_image_decl(decl_file, image_path, font_json)

    img = load_image(image_path)
    name = pathlib.PurePath(font_path).stem
    decl_file.write(
        # f"extern const Font<Image<ImageFormat::GS4_HMSB, {img.width}, {img.height}>> {name};\n"
        f"extern const Font<Image<ImageFormat::GS4, {img.width}, {img.height}>> {name};\n"
    )


def make_font_impl(impl_file: TextIOWrapper, font_path: str) -> None:
    font_json = get_font_json(font_path)
    image_path = get_font_image_path(font_json)

    make_image_impl(impl_file, image_path, font_json)

    img = load_image(image_path)
    name = pathlib.PurePath(font_path).stem

    impl_file.write(
        # f"const Font<Image<ImageFormat::GS4_HMSB, {img.width}, {img.height}>> {name} {{\n"
        f"const Font<Image<ImageFormat::GS4, {img.width}, {img.height}>> {name} {{\n"
    )

    impl_file.write(f"    .image = {name}_image,\n")
    impl_file.write( "    .glyph_rect_map = {\n")
    for frame in font_json["frames"]:
        impl_file.write(
            f"        GlyphRect{{ {frame['frame']['x']}, {frame['frame']['y']}, {frame['frame']['w']}, {frame['frame']['h']}, 0, {frame['spriteSourceSize']['y']} }},\n"
        )

    impl_file.write("\n    }};\n")


def make_fonts_source(assets_dir: str, output_decl: str, output_impl: str) -> None:
    font_paths = os.listdir(assets_dir)

    os.makedirs(os.path.dirname(output_decl), 511, True)
    os.makedirs(os.path.dirname(output_impl), 511, True)

    with open(output_decl, "w") as decl_file:
        with open(output_impl, "w") as impl_file:
            decl_file.write(f"#pragma once\n\n")
            decl_file.write('#include "graphics/graphics.hpp"\n\n')
            decl_file.write("namespace pg::assets\n{\n\n")

            impl_file.write(f'#include "{output_decl}"\n\n')
            impl_file.write("namespace pg::assets\n{\n\n")

            for font_path in font_paths:
                if not font_path.endswith(".json"):
                    continue
                make_font_decl(decl_file, assets_dir + "/" + font_path)
                make_font_impl(impl_file, assets_dir + "/" + font_path)

            decl_file.write("\n}")
            impl_file.write("\n}")


def main() -> None:
    if len(sys.argv) != 3:
        print(f"Usage {sys.argv[0]} <assets_dir> <output_dir>")
    else:
        assets_dir = sys.argv[1]
        output_dir = sys.argv[2]
        make_images_source(
            os.path.join(assets_dir, "images"),
            os.path.join(output_dir, "images/images.hpp"),
            os.path.join(output_dir, "images/images.cpp"),
        )

        make_fonts_source(
            os.path.join(assets_dir, "fonts"),
            os.path.join(output_dir, "fonts/fonts.hpp"),
            os.path.join(output_dir, "fonts/fonts.cpp"),
        )


if __name__ == "__main__":
    main()