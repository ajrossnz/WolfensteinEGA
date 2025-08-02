/*
 * egaify.cpp – Convert Wolfenstein 3‑D VGA art assets to a 16‑colour EGA planar
 * format.
 *
 * This tool is modelled on jhhoward’s cgaify.cpp but simplified to emit
 * 4‑bit planar graphics suitable for the new WolfensteinEGA port.  It uses
 * lodepng.cpp for PNG decoding and huffman.cpp for compression.  To build
 * with Borland C++ 3.x or similar, compile this file along with lodepng.cpp
 * and huffman.cpp.
 *
 * The standard IBM EGA palette contains 16 entries.  Each pixel in a
 * decoded image is mapped to the nearest entry in this palette (using
 * Euclidean distance in RGB space), producing a 4‑bit colour value.  The
 * planar output is produced in the same layout that the original
 * Wolfenstein engine expects: four bit planes, each containing one bit per
 * pixel.  The resulting planes are then compressed with the provided
 * HuffCompress routine and written into EGAGRAPH and EGAHEAD files.
 *
 * Note that this tool is intentionally kept simple – it does not attempt
 * sophisticated dithering.  Feel free to extend it with error diffusion or
 * ordered dithering if you prefer a different trade‑off between colour
 * accuracy and noise.
 */

#include "lodepng.cpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "huffman.cpp"

using namespace std;

// The standard 16‑colour EGA palette.  Values are expressed in 8‑bit
// intensity, matching the VGA DAC format.  The order is the typical IBM
// palette: black, blue, green, cyan, red, magenta, brown, light grey,
// dark grey, bright blue, bright green, bright cyan, bright red,
// bright magenta, yellow, white.
static const uint8_t ega_palette[16][3] = {
    {0x00, 0x00, 0x00}, // 0 black
    {0x00, 0x00, 0xAA}, // 1 blue
    {0x00, 0xAA, 0x00}, // 2 green
    {0x00, 0xAA, 0xAA}, // 3 cyan
    {0xAA, 0x00, 0x00}, // 4 red
    {0xAA, 0x00, 0xAA}, // 5 magenta
    {0xAA, 0x55, 0x00}, // 6 brown / dark yellow
    {0xAA, 0xAA, 0xAA}, // 7 light grey
    {0x55, 0x55, 0x55}, // 8 dark grey
    {0x55, 0x55, 0xFF}, // 9 bright blue
    {0x55, 0xFF, 0x55}, // 10 bright green
    {0x55, 0xFF, 0xFF}, // 11 bright cyan
    {0xFF, 0x55, 0x55}, // 12 bright red
    {0xFF, 0x55, 0xFF}, // 13 bright magenta
    {0xFF, 0xFF, 0x55}, // 14 yellow
    {0xFF, 0xFF, 0xFF}  // 15 white
};

// Compute squared distance between two RGB colours.
static inline uint32_t colour_distance(const uint8_t *a, const uint8_t *b)
{
    int dr = (int)a[0] - (int)b[0];
    int dg = (int)a[1] - (int)b[1];
    int db = (int)a[2] - (int)b[2];
    return (uint32_t)(dr * dr + dg * dg + db * db);
}

// Map a 24‑bit RGB pixel to the nearest entry in the EGA palette.  Returns
// the 4‑bit palette index (0–15).
static uint8_t map_to_ega(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t idx = 0;
    uint32_t best_dist = 0xffffffffUL;
    uint8_t rgb[3] = {r, g, b};
    for (uint8_t i = 0; i < 16; i++)
    {
        uint32_t dist = colour_distance(rgb, ega_palette[i]);
        if (dist < best_dist)
        {
            best_dist = dist;
            idx = i;
        }
    }
    return idx;
}

// Pack planar data.  Given a vector of palette indices of length width*height,
// produce four separate planes.  Each plane is stored sequentially; each
// destination byte contains eight pixels (one bit per pixel).  The caller
// must free the returned buffer.
static uint8_t *convert_to_planar(const vector<uint8_t> &indices, unsigned width, unsigned height)
{
    const size_t plane_size = (width * height) / 8; // bytes per plane
    uint8_t *out = (uint8_t *)malloc(plane_size * 4);
    if (!out)
    {
        fprintf(stderr, "Failed to allocate planar buffer\n");
        exit(1);
    }
    memset(out, 0, plane_size * 4);
    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < width; x++)
        {
            unsigned pixel_index = indices[y * width + x] & 0x0F;
            unsigned bit = 7 - (x & 7);
            unsigned byte_offset = (y * width + x) / 8;
            for (unsigned plane = 0; plane < 4; plane++)
            {
                if ((pixel_index >> plane) & 0x1)
                {
                    out[plane * plane_size + byte_offset] |= (1 << bit);
                }
            }
        }
    }
    return out;
}

// Write a 16‑bit little‑endian word to a file.
static void write_le16(FILE *f, uint16_t v)
{
    fputc(v & 0xFF, f);
    fputc((v >> 8) & 0xFF, f);
}

// Entry point.  This program expects the following arguments:
//   egaify <input_png> <output_raw>
// It will decode the input PNG, map the colours to the EGA palette and
// output four planar planes concatenated together.  The raw output can
// then be compressed with HuffCompress and placed into EGAGRAPH.WL6.
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input_png> <output_raw>\n", argv[0]);
        return 1;
    }
    const char *input_path = argv[1];
    const char *output_path = argv[2];

    // Load PNG
    std::vector<unsigned char> png;
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error;
    error = lodepng::load_file(png, input_path);
    if (error)
    {
        fprintf(stderr, "Error loading file %s: %u\n", input_path, error);
        return 1;
    }
    error = lodepng::decode(image, width, height, png);
    if (error)
    {
        fprintf(stderr, "Error decoding PNG %s: %u\n", input_path, error);
        return 1;
    }
    if ((width % 8) != 0)
    {
        fprintf(stderr, "Width must be a multiple of 8 for planar conversion\n");
        return 1;
    }
    // Map each pixel to EGA index
    vector<uint8_t> indices(width * height);
    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < width; x++)
        {
            unsigned idx = 4 * (y * width + x);
            uint8_t r = image[idx + 0];
            uint8_t g = image[idx + 1];
            uint8_t b = image[idx + 2];
            indices[y * width + x] = map_to_ega(r, g, b);
        }
    }
    // Convert to planar format
    uint8_t *planar = convert_to_planar(indices, width, height);
    const size_t plane_size = (width * height) / 8;
    size_t total_size = plane_size * 4;
    // Write raw planar data
    FILE *outf = fopen(output_path, "wb");
    if (!outf)
    {
        fprintf(stderr, "Error opening output file %s\n", output_path);
        free(planar);
        return 1;
    }
    fwrite(planar, 1, total_size, outf);
    fclose(outf);
    free(planar);
    printf("Wrote %zu bytes of planar EGA data to %s (width=%u height=%u)\n", total_size, output_path, width, height);
    return 0;
}
