# Requires: Pillow, fontTools, libraqm
import os
import subprocess
import sys

import struct
from PIL import Image, ImageColor, ImageChops, ImageDraw, ImageFont

from fontTools.ttLib import TTFont
from fontTools.unicode import Unicode

font_a = TTFont("NineteenNinetyThree-L1Ay.ttf")
font_b = TTFont("LanaPixel.ttf")
font_c = TTFont("Arabic Pixel 2.ttf")

def has_glyph(font, glyph):
    for table in font['cmap'].tables:
        if ord(glyph) in table.cmap.keys():
            return True
    return False



def _convert_str_to_bool(argument: str, argument_name : str):
    if argument.lower() == "true":
        return True
    elif argument.lower() == "false":
        return False
    else:
        print(f"Argument {argument_name} must be either true or false (case insensitive)")
        sys.exit(2)


def main() -> None:
    if len(sys.argv) > 6:
        if len(sys.argv) > 10:
            font_size = int(sys.argv[10])
        else:
            font_size = 16

        font = ImageFont.truetype(sys.argv[1], font_size, encoding="unic")
        font_alt = ImageFont.truetype("LanaPixel.ttf", 11, encoding="unic")
        font_alt_ar = ImageFont.truetype("Arabic Pixel 2.ttf", 8, encoding="unic")
        font_600_6FF = Image.open("0600-06ff.png")
        make_1d = _convert_str_to_bool(sys.argv[5], "make_1d")
        #make_4bpp = _convert_str_to_bool(sys.argv[3], "make_4bpp")
        size = [128, 0]
        use_utf8 = _convert_str_to_bool(sys.argv[6], "use_utf8")

        try:
            font_bpp = int(sys.argv[3])

            if font_bpp != 8 and font_bpp != 4 and font_bpp != 1:
                raise ValueError
        except ValueError:
            print(f"font_bpp argument must be either 8, 4, or 1 instead of {font_bpp}")
            sys.exit(2)

            return

        if font_size % 8 > 0:
            rounded_font_size = font_size + (8 - (font_size % 8))
        else:
            rounded_font_size = font_size

        print(f"Original font size is {font_size} Rounded font size is {rounded_font_size}")


        characters = []
        for i in range(0x21,0x10000):
            if i >= 0xD800 and i < 0xE000:
                character = ' '
            else:
                character = struct.pack("<L", i).decode("utf-32le")
            characters += [character]

        if len(characters) * (rounded_font_size / 8) < 128 / rounded_font_size:
            size[0] = int((len(characters) * (rounded_font_size / 8)) * rounded_font_size)

        size[1] = int((((len(characters) + 1) + (rounded_font_size / 4)) / (128 / rounded_font_size)) *
                      rounded_font_size)


        if size[1] % 8 != 0:
            size[1] += 8 - (size[1] % 8)

        if font_bpp == 1:
            image_mode = "1"

            background_color = 1
        else:
            image_mode = "RGB"

            if len(sys.argv) > 9:
                background_color = (int(sys.argv[7]), int(sys.argv[8]), int(sys.argv[9]))
            else:
                background_color = (0, 0, 0)

        image = Image.new(image_mode, (size[0], size[1]), background_color)
        image_drawing = ImageDraw.Draw(image)
        image_path = f"{sys.argv[4]}.png"

        print(f"image size: ({size[0]}, {size[1]})")

        print_position = [rounded_font_size, 0]

        character_width = {}

        for character in characters:
            if print_position[1] >= size[1] - (rounded_font_size - 1):
                break

            if print_position[0] >= size[0] - (rounded_font_size - 1):
                print_position[0] = 0
                print_position[1] += rounded_font_size

            print_adjust = (0,0)
            ordval = struct.unpack('<L',character.encode('utf-32le'))[0]

            feats = ["-kern", "-liga"]
            drawdir = "ltr"
            draw_font = font
            if not has_glyph(font_a, character) and has_glyph(font_b, character):
                draw_font = font_alt
                print_adjust = (0,2)
            if (not has_glyph(font_a, character) and has_glyph(font_c, character)) or ordval >= 0x600 and ordval < 0x700:
                draw_font = font_alt_ar
                print_adjust = (0,0)
                #drawdir = "rtl"
                feats = []

            if ordval >= 0x600 and ordval < 0x700:
                crop_x = (ordval & 0xF) * 12
                crop_y = ((ordval & 0xF0) >> 4) * 12
                char = font_600_6FF.crop((crop_x, crop_y, crop_x+12, crop_y+12))
                image.paste(char, (print_position[0]+print_adjust[0], print_position[1]+print_adjust[1]))
                character_width[character] = 0
            else:
                image_drawing.text((print_position[0]+print_adjust[0], print_position[1]+print_adjust[1]), character, features=feats, font=draw_font, direction=drawdir, language="en")
                character_width[character] = image_drawing.textsize(character, features=feats, font=draw_font)[0] + 1

            check_w = 0
            for x in range(16):
                for y in range(16):
                    pos = (print_position[0]+print_adjust[0]+x, print_position[1]+print_adjust[1]+y)
                    if (not image.getpixel(pos)) and x > check_w:
                        check_w = x

            
            check_w += 1
            if check_w > character_width[character] and character_width[character] < 4:
                character_width[character] = check_w
            #print(f"{character} at ({print_position[0]}, {print_position[1]})")

            print_position[0] += rounded_font_size

        #image_drawing.multiline_text((0, 0), string_to_print, fill=background_color, font=font)

        if font_bpp == 1:
            for x in range(image.size[0]):
                for y in range(image.size[1]):
                    image.putpixel((x, y), not image.getpixel((x, y)))

        if make_1d:
            working_coords = [0, 0]
            temp_1d_image = Image.new("1", (size[0], size[1]), background_color)
            temp_1d_image_draw = ImageDraw.Draw(temp_1d_image)

            for image_y in range(0, size[1]-16, 16):
                for image_x in range(0, size[0], 16):
                    for font_x in range(int(rounded_font_size // 8)):
                        for font_y in range(int(rounded_font_size // 8)):
                            for x in range(8):
                                for y in range(8):
                                    old_image_coords = (image_x + (font_x * 8) + x, image_y + (font_y * 8) + y)
                                    #print (old_image_coords)
                                    
                                    temp_1d_image.putpixel((working_coords[0] + x, working_coords[1] + y), image.getpixel(old_image_coords))

                            if working_coords[0] + 8 >= size[0]:
                                working_coords[1] += 8
                                working_coords[0] = 0
                            else:
                                working_coords[0] += 8

            #temp_1d_image.save(image_path + ".idk.png", "PNG")
            #image.save(image_path, "PNG")
            temp_1d_image.save(image_path, "PNG")
            image.save(image_path + ".orig.png", "PNG")
        else:
            image.save(image_path, "PNG")

        base_image_file_name = os.path.splitext(os.path.basename(sys.argv[4]))[0].replace("-", "_")
        grit_subprocess = ["grit", image_path]

        if font_bpp == 4:
            grit_subprocess.append("-gB4")
        elif font_bpp == 8:
            grit_subprocess.append("-gB8")
            grit_subprocess.append("-MRtf")
        elif font_bpp == 1:
            grit_subprocess.append("-gB1")

        #grit_subprocess.append("-tc")
        grit_subprocess.append("-ftc")

        print(subprocess.run(grit_subprocess, capture_output=True))


        grit_subprocess_bin = ["grit", image_path]

        if font_bpp == 4:
            grit_subprocess_bin.append("-gB4")
        elif font_bpp == 8:
            grit_subprocess_bin.append("-gB8")
            grit_subprocess_bin.append("-MRtf")
        elif font_bpp == 1:
            grit_subprocess_bin.append("-gB1")

        grit_subprocess_bin.append("-ftb")

        subprocess.run(grit_subprocess_bin, capture_output=True)

        if use_utf8:
            with open(f"{base_image_file_name}_utf_map.hpp", 'w') as grit_hpp_file:
                grit_hpp_file.write("#include <map>\n")

                grit_hpp_file.write(f"const std::map<unsigned int, unsigned int> {base_image_file_name}UtfMap=\n{{")

                for i in range(len(characters)):
                    if i % 4 == 0:
                        grit_hpp_file.write("\n    ")

                    if make_1d:
                        # + 1 to account for blank starting space
                        tile_id = int(4 * (i + 1))
                    else:
                        tile_id = (int(rounded_font_size / 8) * int((i + 1) % int(128 / rounded_font_size))) + \
                                  int(int(16 * int(rounded_font_size / 8)) * int((i + 1) / 8))

                    #print (characters[i].encode('utf-16le'))
                    ordval = struct.unpack('<L',characters[i].encode('utf-32le'))[0]
                    grit_hpp_file.write(f"{{ {hex(ordval)}, {tile_id} }}, ")

                grit_hpp_file.write("\n};\n")

        if True:
            with open(f"{base_image_file_name}.c", 'a') as grit_hpp_file:

                grit_hpp_file.write(f"const unsigned char {base_image_file_name}Widths[{len(characters)}] __attribute__((aligned(4))) =\n{{\n4,")

                for i in range(len(characters)):
                    if i % 4 == 0:
                        grit_hpp_file.write("\n    ")

                    if make_1d:
                        # + 1 to account for blank starting space
                        tile_id = int(4 * (i + 1))
                    else:
                        tile_id = (int(rounded_font_size / 8) * int((i + 1) % int(128 / rounded_font_size))) + \
                                  int(int(16 * int(rounded_font_size / 8)) * int((i + 1) / 8))

                    #character_width[characters[i]] = 16
                    grit_hpp_file.write(f"{character_width[characters[i]]}, ")

                grit_hpp_file.write("\n};\n")

                grit_hpp_file.write("#include <tonc_tte.h>\n")

                grit_hpp_file.write(f"const TFont {base_image_file_name}Font __attribute__((aligned(4))) =\n{{\n")

                grit_hpp_file.write(f"{base_image_file_name}Tiles,\n")
                grit_hpp_file.write(f"{base_image_file_name}Widths,\n")
                grit_hpp_file.write("0,\n")
                grit_hpp_file.write("0x20,\n") # charoffset
                grit_hpp_file.write(f"{len(characters)},\n") # charcount
                grit_hpp_file.write("8,\n") # charwidth
                grit_hpp_file.write("12,\n") # charheight
                grit_hpp_file.write("16,\n") # cell width
                grit_hpp_file.write("16,\n") # cell height
                grit_hpp_file.write("32,\n") # cell size bytes
                grit_hpp_file.write("1,\n") # bpp
                grit_hpp_file.write("0\n") # extra

                grit_hpp_file.write("\n};\n")

            with open(f"{base_image_file_name}.vwflen.bin", 'wb') as out_file:
                out_file.write(struct.pack("<B", 4))
                for i in range(len(characters)):
                    out_file.write(struct.pack("<B", character_width[characters[i]]))

    else:
        print("Invalid syntax:")
        print(f"{os.path.basename(__file__)} ttf_font_file font_character_list_file font_bpp destination_file make_1d "
              f"use_utf8 [background_color_r] [background_color_g] [background_color_b] [font_size]")

        sys.exit(2)


if __name__ == "__main__":
    main()
