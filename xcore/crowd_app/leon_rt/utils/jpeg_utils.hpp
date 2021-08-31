#ifndef LEON_RT_UTILS_JPEG_UTILS_HPP
#define LEON_RT_UTILS_JPEG_UTILS_HPP

namespace rt_utils {
#include <jpeglib.h>
int compress_yuv420p_to_jpeg(unsigned char *dest_image, \
                             unsigned char *input_image, \
                            int width, int height, \
                            int quality) {
    int i, j;
  
    JSAMPROW y[16],cb[16],cr[16]; // y[2][5] = color sample of row 2 and pixel column 5; (one plane)
    JSAMPARRAY data[3]; // t[0][2][5] = color sample 0 of row 2 and column 5

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    data[0] = y;
    data[1] = cb; 
    data[2] = cr; 

    cinfo.err = jpeg_std_error(&jerr);  // errors get written to stderr 

    jpeg_create_compress(&cinfo);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    jpeg_set_defaults (&cinfo);

    jpeg_set_colorspace(&cinfo, JCS_YCbCr);

    cinfo.raw_data_in = TRUE;                  // supply downsampled data
    cinfo.do_fancy_downsampling = FALSE;       // fix segfaulst with v7
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 2;
    cinfo.comp_info[1].h_samp_factor = 1;
    cinfo.comp_info[1].v_samp_factor = 1;
    cinfo.comp_info[2].h_samp_factor = 1;
    cinfo.comp_info[2].v_samp_factor = 1;

    jpeg_set_quality(&cinfo, quality, TRUE);
    cinfo.dct_method = JDCT_FASTEST;
    long unsigned int size = 2 * width * height;
    jpeg_mem_dest(&cinfo, &dest_image, &size);    // data written to mem

    jpeg_start_compress (&cinfo, TRUE);

    for (j = 0; j < height; j += 16) {
        for (i = 0; i < 16; i++) {
            y[i] = input_image + width * (i + j);
            if (i % 2 == 0) {
                cb[i / 2] = input_image + width * height + width / 2 * ((i + j) / 2);
                cr[i / 2] = input_image + width * height + width * height / 4 + width / 2 * ((i + j) / 2);
            }
        }
        jpeg_write_raw_data(&cinfo, data, 16);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    return size;
}
}  // namespace rt_utils
#endif  // LEON_RT_UTILS_JPEG_UTILS_HPP
