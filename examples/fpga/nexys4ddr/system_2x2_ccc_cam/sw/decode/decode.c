#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>

//#define IMGSIZE (640*480) // VGA
//#define IMGSIZE (800*600) // SVGA
//#define IMGSIZE (176*144) // QCIF
#define IMGSIZE (1600*1200) // UXGA
//#define IMGSIZE (352*288) // CIF

int main(int argc, char* argv[])
{
    if (argc < 2)
        exit(1);

    FILE* input_fh;
    FILE* output_fh;

    input_fh = fopen(argv[1], "r");
    output_fh = fopen(argv[2], "w");

    typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb888_t;


//    uint16_t in_rgb565[IMGSIZE];
//    rgb888_t out_rgb888[IMGSIZE];

    uint16_t* in_rgb565 = calloc(sizeof(uint16_t), IMGSIZE);
    rgb888_t* out_rgb888 = calloc(sizeof(rgb888_t), IMGSIZE);

    // read RGB565
    size_t size_read = fread(in_rgb565, sizeof(uint16_t), IMGSIZE, input_fh);
    printf("size read: %d\n", (int)size_read);
    assert(size_read == IMGSIZE);

    fclose(input_fh);

    // convert
    for (int i = 0; i < IMGSIZE; i++) {
        //printf("%x\n", in_rgb565[i]);

        uint8_t r5 = (in_rgb565[i] >> 11) & 0x1f;
        uint8_t g6 = (in_rgb565[i] >> 5) & 0x3f;
        uint8_t b5 = in_rgb565[i] & 0x1f;

        out_rgb888[i].r = r5 << 3;
        out_rgb888[i].g = g6 << 2;
        out_rgb888[i].b = b5 << 3;
    }

    size_t size_written = fwrite(out_rgb888, sizeof(rgb888_t), IMGSIZE, output_fh);
    assert(size_written == IMGSIZE);

    fclose(output_fh);

    /*




    uint32_t rgb565 = atoi(argv[1]);
    printf("%x\n", rgb565);

    uint8_t r5 = (rgb565 >> 11) & 0x1f;
    uint8_t g6 = (rgb565 >> 5) & 0x3f;
    uint8_t b5 = rgb565 & 0x1f;

    uint8_t r8 = r5 << 3;
    uint8_t g8 = g6 << 2;
    uint8_t b8 = b5 << 3;

    printf("r: %d, g: %d, b: %d; #%x%x%x\n", r8, g8, b8, r8, g8, b8);*/
    exit(0);
}
