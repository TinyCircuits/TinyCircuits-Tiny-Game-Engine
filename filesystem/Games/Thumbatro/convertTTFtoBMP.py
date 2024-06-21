import os
from PIL import Image, ImageDraw, ImageFont, ImageOps
from fontTools.ttLib import TTFont

# Define the characters to be used (note the space at the start and the double quote at the end)
characters = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}\""

# Function to create BMP image from TTF/OTF font
def create_bmp_from_ttf(font_path, font_size, output_path, use_given_size=True):
    
    # Load the TTF/OTF font
    font = ImageFont.truetype(font_path, font_size)

    # Calculate the maximum character height
    max_char_height = max(font.getbbox(char)[3] for char in characters)

    # Calculate image dimensions
    char_widths = [font.getbbox(char)[2] for char in characters]
    image_width = sum(char_widths)
    image_height = max_char_height + 1

    # Create an image with black background
    image = Image.new('RGB', (image_width, image_height), color='black')
    draw = ImageDraw.Draw(image)

    # Draw each character
    x_position = 0
    for i, char in enumerate(characters):
        char_width = char_widths[i]
        y_position = 0  # Set y_position to 0 to remove any top margin
        draw.text((x_position, y_position), char, font=font, fill='white')

        x_position += char_width

    # Convert to grayscale
    gray_image = image.convert('L')
    # Apply threshold
    threshold_image = gray_image.point(lambda p: 255 if p > 128 else 0)
    # Convert back to RGB
    bw_image = threshold_image.convert('RGB')

    draw = ImageDraw.Draw(bw_image)

    x_position = 0
    for i, char in enumerate(characters):
        char_width = char_widths[i]
        y_position = 0  # Set y_position to 0 to remove any top margin
        
        # Draw alternating red and blue lines just below the text
        if i % 2 == 0:
            draw.line([(x_position, y_position + max_char_height), (x_position + char_width, y_position + max_char_height)], fill='red')
        else:
            draw.line([(x_position, y_position + max_char_height), (x_position + char_width, y_position + max_char_height)], fill='blue')

        x_position += char_width

    # Save the image as BMP
    bw_image.save(output_path)

# Process all TTF and OTF files in the current directory
for font_file in os.listdir('.'):
    if font_file.lower().endswith(('.ttf', '.otf')):
        for size in range(5, 17, 1):
            output_file = f"{os.path.splitext(font_file)[0]}_{size}.bmp"
            create_bmp_from_ttf(font_file, size, output_file, use_given_size=False)


#for f in *.bmp; do convert "$f" -depth 16 -define bmp:subtype=RGB565 "output/${f}"; done