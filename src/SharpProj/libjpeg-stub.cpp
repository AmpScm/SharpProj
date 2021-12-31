#include "pch.h"
#include <jpeglib.h>

#pragma unmanaged
#pragma warning(disable: 4100)

#if USE_STUB_LIBJPEG

// PROJ uses geotiff tables for grid transforms to improve accuracy. These tables use the lossless tiff format
// to ease implementation and re-using other tools.
//
// But then PROJ uses libtiff for its processing, which then by default enables jpeg support, which makes SharpProj
// about 500 KByte larger, while the compression would never 
void jpeg_write_tables(j_compress_ptr cinfo)
{
    abort(); // Code can't write
}

void jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables)
{
    abort(); // Code can't write
}

void jpeg_finish_compress(j_compress_ptr cinfo)
{
    abort(); // Code can't write
}

JDIMENSION jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines)
{
    return -1;
}

JDIMENSION jpeg_write_raw_data(j_compress_ptr cinfo, JSAMPIMAGE data, JDIMENSION num_lines)
{
    return -1;
}

void jpeg_suppress_tables(j_compress_ptr cinfo, boolean suppress)
{
    
}

struct jpeg_error_mgr* jpeg_std_error(struct jpeg_error_mgr* err)
{
    memset(err, 0, sizeof(*err));
    err->msg_code = -1;

    return err;
}

boolean jpeg_start_decompress(j_decompress_ptr cinfo)
{
    return false;
}

JDIMENSION jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines)
{
    return -1;
}

JDIMENSION jpeg_read_raw_data(j_decompress_ptr cinfo, JSAMPIMAGE data, JDIMENSION max_lines)
{
    return -1;
}

boolean jpeg_finish_decompress(j_decompress_ptr cinfo)
{
    return -1;
}

void jpeg_abort(j_common_ptr cinfo)
{
}

void jpeg_destroy(j_common_ptr cinfo)
{

}

void jpeg_set_defaults(j_compress_ptr cinfo)
{

}

void jpeg_set_colorspace(j_compress_ptr cinfo, J_COLOR_SPACE colorspace)
{

}

void jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline)
{
}

boolean jpeg_resync_to_restart(j_decompress_ptr cinfo, int desired)
{
    abort(); // Callback passed to us. Ignored
}

void jpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize)
{
    memset(cinfo, 0, structsize);
}

void jpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize)
{
    memset(cinfo, 0, structsize);
}

int jpeg_read_header(j_decompress_ptr cinfo, boolean require_image)
{
    return -1;
}

boolean jpeg_has_multiple_scans(j_decompress_ptr cinfo)
{
    return false;
}
#endif