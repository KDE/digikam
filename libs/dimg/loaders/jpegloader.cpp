/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A JPEG IO file for DImg framework
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#define XMD_H

#include "jpegloader.h"

// C ANSI includes

extern "C"
{
#include "iccjpeg.h"
}

// Qt includes

#include <QFile>
#include <QByteArray>

// Local includes

#include "digikam_config.h"
#include "dimg.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"

#ifdef Q_OS_WIN
#include "jpegwin.h"
#endif

namespace Digikam
{

// To manage Errors/Warnings handling provide by libjpeg

void JPEGLoader::dimg_jpeg_error_exit(j_common_ptr cinfo)
{
    dimg_jpeg_error_mgr* const myerr = static_cast<dimg_jpeg_error_mgr*>(cinfo->err);

    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCDebug(DIGIKAM_DIMG_LOG_JPEG) << buffer;

    longjmp(myerr->setjmp_buffer, 1);
}

void JPEGLoader::dimg_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCDebug(DIGIKAM_DIMG_LOG_JPEG) << buffer << " (" << msg_level << ")";
}

void JPEGLoader::dimg_jpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCDebug(DIGIKAM_DIMG_LOG_JPEG) << buffer;
}

JPEGLoader::JPEGLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool JPEGLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    readMetadata(filePath, DImg::JPEG);

    FILE* const file = fopen(QFile::encodeName(filePath).constData(), "rb");

    if (!file)
    {
        loadingFailed();
        return false;
    }

    unsigned char header[2];

    if (fread(&header, 2, 1, file) != 1)
    {
        fclose(file);
        loadingFailed();
        return false;
    }

    unsigned char jpegID[] = { 0xFF, 0xD8 };

    if (memcmp(header, jpegID, 2) != 0)
    {
        // not a jpeg file
        fclose(file);
        loadingFailed();
        return false;
    }

    rewind(file);

    struct jpeg_decompress_struct cinfo;
    struct dimg_jpeg_error_mgr    jerr;

    // -------------------------------------------------------------------
    // JPEG error handling.

    cinfo.err                 = jpeg_std_error(&jerr);
    cinfo.err->error_exit     = dimg_jpeg_error_exit;
    cinfo.err->emit_message   = dimg_jpeg_emit_message;
    cinfo.err->output_message = dimg_jpeg_output_message;

    // setjmp-save cleanup
    class CleanupData
    {
    public:

        CleanupData()
        {
            data  = 0;
            dest  = 0;
            f     = 0;
        }

        ~CleanupData()
        {
            delete [] data;
            delete [] dest;

            if (f)
            {
                fclose(f);
            }
        }

        void setData(uchar* const d)
        {
            data = d;
        }

        void setDest(uchar* const d)
        {
            dest = d;
        }

        void setFile(FILE* const file)
        {
            f = file;
        }

        void deleteData()
        {
            delete [] data;
            data = 0;
        }

        void takeDest()
        {
            dest = 0;
        }

    public:

        uchar* data;
        uchar* dest;
        FILE*  f;
    };

    CleanupData* const cleanupData = new CleanupData;
    cleanupData->setFile(file);

    // If an error occurs during reading, libjpeg will jump here

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        delete cleanupData;
        loadingFailed();
        return false;
    }

    // -------------------------------------------------------------------
    // Find out if we do the fast-track loading with reduced size. Jpeg specific.
    int scaledLoadingSize = 0;
    QVariant attribute    = imageGetAttribute(QLatin1String("scaledLoadingSize"));

    if (attribute.isValid())
    {
        scaledLoadingSize = attribute.toInt();
    }

    // -------------------------------------------------------------------
    // Set JPEG decompressor instance

    jpeg_create_decompress(&cinfo);
    bool startedDecompress = false;

#ifdef Q_OS_WIN
    QFile inFile(filePath);
    QByteArray buffer;

    if (inFile.open(QIODevice::ReadOnly))
    {
        buffer = inFile.readAll();
        inFile.close();
    }

    JPEGUtils::jpeg_memory_src(&cinfo, (JOCTET*)buffer.data(), buffer.size());
#else
    jpeg_stdio_src(&cinfo, file);
#endif

    // Recording ICC profile marker (from iccjpeg.c)
    if (m_loadFlags & LoadICCData)
    {
        setup_read_icc_profile(&cinfo);
    }

    // read image information
    jpeg_read_header(&cinfo, boolean(true));

    // read dimension (nominal values from header)
    int w = cinfo.image_width;
    int h = cinfo.image_height;
    QSize originalSize(w, h);

    // Libjpeg handles the following conversions:
    // YCbCr => GRAYSCALE, YCbCr => RGB, GRAYSCALE => RGB, YCCK => CMYK
    // So we cannot get RGB from CMYK or YCCK, CMYK conversion is handled below
    int colorModel = DImg::COLORMODELUNKNOWN;

    switch (cinfo.jpeg_color_space)
    {
        case JCS_UNKNOWN:
            // perhaps jpeg_read_header did some guessing, leave value unchanged
            colorModel            = DImg::COLORMODELUNKNOWN;
            break;

        case JCS_GRAYSCALE:
            cinfo.out_color_space = JCS_RGB;
            colorModel            = DImg::GRAYSCALE;
            break;

        case JCS_RGB:
            cinfo.out_color_space = JCS_RGB;
            colorModel            = DImg::RGB;
            break;

        case JCS_YCbCr:
            cinfo.out_color_space = JCS_RGB;
            colorModel            = DImg::YCBCR;
            break;

        case JCS_CMYK:
        case JCS_YCCK:
            cinfo.out_color_space = JCS_CMYK;
            colorModel            = DImg::CMYK;
            break;

        default:
            break;
    }

    // -------------------------------------------------------------------
    // Load image data.

    uchar* dest = 0;

    if (m_loadFlags & LoadImageData)
    {
        // set decompression parameters
        cinfo.do_fancy_upsampling = boolean(true);
        cinfo.do_block_smoothing  = boolean(false);

        // handle scaled loading
        if (scaledLoadingSize)
        {
            int imgSize = qMax(cinfo.image_width, cinfo.image_height);

            // libjpeg supports 1/1, 1/2, 1/4, 1/8
            int scale = 1;

            while (scaledLoadingSize* scale * 2 <= imgSize)
            {
                scale *= 2;
            }

            if (scale > 8)
            {
                scale = 8;
            }

            //cinfo.scale_num   = 1;
            //cinfo.scale_denom = scale;
            cinfo.scale_denom *= scale;
        }

        // initialize decompression
        if (!startedDecompress)
        {
            jpeg_start_decompress(&cinfo);
            startedDecompress = true;
        }

        // some pseudo-progress
        if (observer)
        {
            observer->progressInfo(m_image, 0.1F);
        }

        // re-read dimension (scaling included)
        w = cinfo.output_width;
        h = cinfo.output_height;

        // -------------------------------------------------------------------
        // Get scanlines

        uchar* ptr  = 0, *data = 0, *line[16];
        uchar* ptr2 = 0;
        int    x, y, l, i, scans;
        //        int count;
        //        int prevy;

        if (cinfo.rec_outbuf_height > 16)
        {
            jpeg_destroy_decompress(&cinfo);
            qCWarning(DIGIKAM_DIMG_LOG_JPEG) << "Height of JPEG scanline buffer out of range!";
            delete cleanupData;
            loadingFailed();
            return false;
        }

        // We only take RGB with 1 or 3 components, or CMYK with 4 components
        if (!(
                (cinfo.out_color_space == JCS_RGB  && (cinfo.output_components == 3 || cinfo.output_components == 1)) ||
                (cinfo.out_color_space == JCS_CMYK &&  cinfo.output_components == 4)
            ))
        {
            jpeg_destroy_decompress(&cinfo);
            qCWarning(DIGIKAM_DIMG_LOG_JPEG)
                    << "JPEG colorspace ("
                    << cinfo.out_color_space
                    << ") or Number of JPEG color components ("
                    << cinfo.output_components
                    << ") unsupported!";
            delete cleanupData;
            loadingFailed();
            return false;
        }

        data = new_failureTolerant(w * 16 * cinfo.output_components);
        cleanupData->setData(data);

        if (!data)
        {
            jpeg_destroy_decompress(&cinfo);
            qCWarning(DIGIKAM_DIMG_LOG_JPEG) << "Cannot allocate memory!";
            delete cleanupData;
            loadingFailed();
            return false;
        }

        dest = new_failureTolerant(w, h, 4);
        cleanupData->setDest(dest);

        if (!dest)
        {
            jpeg_destroy_decompress(&cinfo);
            qCWarning(DIGIKAM_DIMG_LOG_JPEG) << "Cannot allocate memory!";
            delete cleanupData;
            loadingFailed();
            return false;
        }

        ptr2  = dest;
        //        count = 0;
        //        prevy = 0;

        if (cinfo.output_components == 3)
        {
            for (i = 0; i < cinfo.rec_outbuf_height; ++i)
            {
                line[i] = data + (i * w * 3);
            }

            int checkPoint = 0;

            for (l = 0; l < h; l += cinfo.rec_outbuf_height)
            {
                // use 0-10% and 90-100% for pseudo-progress
                if (observer && l >= checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery(m_image))
                    {
                        jpeg_destroy_decompress(&cinfo);
                        delete cleanupData;
                        loadingFailed();
                        return false;
                    }

                    observer->progressInfo(m_image, 0.1 + (0.8 * (((float)l) / ((float)h))));
                }

                jpeg_read_scanlines(&cinfo, &line[0], cinfo.rec_outbuf_height);
                scans = cinfo.rec_outbuf_height;

                if ((h - l) < scans)
                {
                    scans = h - l;
                }

                ptr = data;

                for (y = 0; y < scans; ++y)
                {
                    for (x = 0; x < w; ++x)
                    {
                        ptr2[3] = 0xFF;
                        ptr2[2] = ptr[0];
                        ptr2[1] = ptr[1];
                        ptr2[0] = ptr[2];

                        ptr    += 3;
                        ptr2   += 4;
                    }
                }
            }
        }
        else if (cinfo.output_components == 1)
        {
            for (i = 0; i < cinfo.rec_outbuf_height; ++i)
            {
                line[i] = data + (i * w);
            }

            int checkPoint = 0;

            for (l = 0; l < h; l += cinfo.rec_outbuf_height)
            {
                if (observer && l >= checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery(m_image))
                    {
                        jpeg_destroy_decompress(&cinfo);
                        delete cleanupData;
                        loadingFailed();
                        return false;
                    }

                    observer->progressInfo(m_image, 0.1 + (0.8 * (((float)l) / ((float)h))));
                }

                jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
                scans = cinfo.rec_outbuf_height;

                if ((h - l) < scans)
                {
                    scans = h - l;
                }

                ptr = data;

                for (y = 0; y < scans; ++y)
                {
                    for (x = 0; x < w; ++x)
                    {
                        ptr2[3] = 0xFF;
                        ptr2[2] = ptr[0];
                        ptr2[1] = ptr[0];
                        ptr2[0] = ptr[0];

                        ptr    ++;
                        ptr2   += 4;
                    }
                }
            }
        }
        else // CMYK
        {
            for (i = 0; i < cinfo.rec_outbuf_height; ++i)
            {
                line[i] = data + (i * w * 4);
            }

            int checkPoint = 0;

            for (l = 0; l < h; l += cinfo.rec_outbuf_height)
            {
                // use 0-10% and 90-100% for pseudo-progress
                if (observer && l >= checkPoint)
                {
                    checkPoint += granularity(observer, h, 0.8F);

                    if (!observer->continueQuery(m_image))
                    {
                        jpeg_destroy_decompress(&cinfo);
                        delete cleanupData;
                        loadingFailed();
                        return false;
                    }

                    observer->progressInfo(m_image, 0.1 + (0.8 * (((float)l) / ((float)h))));
                }

                jpeg_read_scanlines(&cinfo, &line[0], cinfo.rec_outbuf_height);
                scans = cinfo.rec_outbuf_height;

                if ((h - l) < scans)
                {
                    scans = h - l;
                }

                ptr   = data;

                for (y = 0; y < scans; ++y)
                {
                    for (x = 0; x < w; ++x)
                    {
                        // Inspired by Qt's JPEG loader

                        int k = ptr[3];

                        ptr2[3] = 0xFF;
                        ptr2[2] = k * ptr[0] / 255;
                        ptr2[1] = k * ptr[1] / 255;
                        ptr2[0] = k * ptr[2] / 255;

                        ptr    += 4;
                        ptr2   += 4;
                    }
                }
            }
        }

        // clean up
        cleanupData->deleteData();
    }

    // -------------------------------------------------------------------
    // Read image ICC profile

    if (m_loadFlags & LoadICCData)
    {
        if (!startedDecompress)
        {
            jpeg_start_decompress(&cinfo);
            startedDecompress = true;
        }

        JOCTET* profile_data = NULL;
        uint    profile_size = 0;

        read_icc_profile(&cinfo, &profile_data, &profile_size);

        if (profile_data != NULL)
        {
            QByteArray profile_rawdata;
            profile_rawdata.resize(profile_size);
            memcpy(profile_rawdata.data(), profile_data, profile_size);
            imageSetIccProfile(profile_rawdata);
            free(profile_data);
        }
        else
        {
            // If ICC profile is null, check Exif metadata.
            checkExifWorkingColorSpace();
        }
    }

    // -------------------------------------------------------------------

    if (startedDecompress)
    {
        jpeg_finish_decompress(&cinfo);
    }

    jpeg_destroy_decompress(&cinfo);

    // -------------------------------------------------------------------

    cleanupData->takeDest();
    delete cleanupData;

    if (observer)
    {
        observer->progressInfo(m_image, 1.0);
    }

    imageWidth()  = w;
    imageHeight() = h;
    imageData()   = dest;
    imageSetAttribute(QLatin1String("format"),             QLatin1String("JPG"));
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   8);
    imageSetAttribute(QLatin1String("originalSize"),       originalSize);

    return true;
}

bool JPEGLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    FILE* const file = fopen(QFile::encodeName(filePath).constData(), "wb");

    if (!file)
    {
        return false;
    }

    struct jpeg_compress_struct  cinfo;

    struct dimg_jpeg_error_mgr jerr;

    // -------------------------------------------------------------------
    // JPEG error handling.

    cinfo.err                 = jpeg_std_error(&jerr);
    cinfo.err->error_exit     = dimg_jpeg_error_exit;
    cinfo.err->emit_message   = dimg_jpeg_emit_message;
    cinfo.err->output_message = dimg_jpeg_output_message;

    // setjmp-save cleanup
    class CleanupData
    {
    public:

        CleanupData() : line(0), f(0)
        {
        }

        ~CleanupData()
        {
            deleteLine();

            if (f)
            {
                fclose(f);
            }
        }

        void setLine(uchar* const l)
        {
            line = l;
        }

        void setFile(FILE* const file)
        {
            f = file;
        }

        void deleteLine()
        {
            delete [] line;
            line = 0;
        }

    public:

        uchar* line;
        FILE*  f;
    };

    CleanupData* const cleanupData = new CleanupData;
    cleanupData->setFile(file);

    // If an error occurs during writing, libjpeg will jump here

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_compress(&cinfo);
        delete cleanupData;
        return false;
    }

    // -------------------------------------------------------------------
    // Set JPEG compressor instance

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, file);

    uint&              w   = imageWidth();
    uint&              h   = imageHeight();
    unsigned char*& data   = imageData();

    // Size of image.
    cinfo.image_width      = w;
    cinfo.image_height     = h;

    // Color components of image in RGB.
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    QVariant qualityAttr   = imageGetAttribute(QLatin1String("quality"));
    int quality            = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    if (quality < 0)
    {
        quality = 90;
    }

    if (quality > 100)
    {
        quality = 100;
    }

    QVariant subSamplingAttr = imageGetAttribute(QLatin1String("subsampling"));
    int subsampling          = subSamplingAttr.isValid() ? subSamplingAttr.toInt() : 1;  // Medium

    jpeg_set_defaults(&cinfo);

    // bug #149578: set horizontal and vertical chroma subsampling factor to encoder.
    // See this page for details: http://en.wikipedia.org/wiki/Chroma_subsampling

    switch (subsampling)
    {
        case 1:  // 2x1, 1x1, 1x1 (4:2:2)
        {
            qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:2:2";
            cinfo.comp_info[0].h_samp_factor = 2;
            cinfo.comp_info[0].v_samp_factor = 1;
            cinfo.comp_info[1].h_samp_factor = 1;
            cinfo.comp_info[1].v_samp_factor = 1;
            cinfo.comp_info[2].h_samp_factor = 1;
            cinfo.comp_info[2].v_samp_factor = 1;
            break;
        }

        case 2:  // 2x2, 1x1, 1x1 (4:2:0)
        {
            qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:2:0";
            cinfo.comp_info[0].h_samp_factor = 2;
            cinfo.comp_info[0].v_samp_factor = 2;
            cinfo.comp_info[1].h_samp_factor = 1;
            cinfo.comp_info[1].v_samp_factor = 1;
            cinfo.comp_info[2].h_samp_factor = 1;
            cinfo.comp_info[2].v_samp_factor = 1;
            break;
        }

        case 3:  // 4x1, 1x1, 1x1 (4:1:1)
        {
            qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:1:1";
            cinfo.comp_info[0].h_samp_factor = 4;
            cinfo.comp_info[0].v_samp_factor = 1;
            cinfo.comp_info[1].h_samp_factor = 1;
            cinfo.comp_info[1].v_samp_factor = 1;
            cinfo.comp_info[2].h_samp_factor = 1;
            cinfo.comp_info[2].v_samp_factor = 1;
            break;
        }

        default: // 1x1, 1x1, 1x1 (4:4:4)
        {
            qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG chroma-subsampling 4:4:4";
            cinfo.comp_info[0].h_samp_factor = 1;
            cinfo.comp_info[0].v_samp_factor = 1;
            cinfo.comp_info[1].h_samp_factor = 1;
            cinfo.comp_info[1].v_samp_factor = 1;
            cinfo.comp_info[2].h_samp_factor = 1;
            cinfo.comp_info[2].v_samp_factor = 1;
            break;
        }
    }

    jpeg_set_quality(&cinfo, quality, boolean(true));
    jpeg_start_compress(&cinfo, boolean(true));

    qCDebug(DIGIKAM_DIMG_LOG_JPEG) << "Using LibJPEG quality compression value: " << quality;

    if (observer)
    {
        observer->progressInfo(m_image, 0.1F);
    }

    // -------------------------------------------------------------------
    // Write ICC profile.

    QByteArray profile_rawdata = m_image->getIccProfile().data();

    if (!profile_rawdata.isEmpty())
    {
        purgeExifWorkingColorSpace();
        write_icc_profile(&cinfo, (JOCTET*)profile_rawdata.data(), profile_rawdata.size());
    }

    if (observer)
    {
        observer->progressInfo(m_image, 0.2F);
    }

    // -------------------------------------------------------------------
    // Write Image data.

    uchar* line       = new uchar[w * 3];
    uchar* dstPtr     = 0;
    uint   checkPoint = 0;
    cleanupData->setLine(line);

    if (!imageSixteenBit())     // 8 bits image.
    {

        uchar* srcPtr = data;

        for (uint j = 0; j < h; ++j)
        {

            if (observer && j == checkPoint)
            {
                checkPoint += granularity(observer, h, 0.8F);

                if (!observer->continueQuery(m_image))
                {
                    jpeg_destroy_compress(&cinfo);
                    delete cleanupData;
                    return false;
                }

                // use 0-20% for pseudo-progress, now fill 20-100%
                observer->progressInfo(m_image, 0.2 + (0.8 * (((float)j) / ((float)h))));
            }

            dstPtr = line;

            for (uint i = 0; i < w; ++i)
            {
                dstPtr[2] = srcPtr[0];  // Blue
                dstPtr[1] = srcPtr[1];  // Green
                dstPtr[0] = srcPtr[2];  // Red

                srcPtr   += 4;
                dstPtr   += 3;
            }

            jpeg_write_scanlines(&cinfo, &line, 1);
        }
    }
    else
    {
        unsigned short* srcPtr = reinterpret_cast<unsigned short*>(data);

        for (uint j = 0; j < h; ++j)
        {

            if (observer && j == checkPoint)
            {
                checkPoint += granularity(observer, h, 0.8F);

                if (!observer->continueQuery(m_image))
                {
                    jpeg_destroy_compress(&cinfo);
                    delete cleanupData;
                    return false;
                }

                // use 0-20% for pseudo-progress, now fill 20-100%
                observer->progressInfo(m_image, 0.2 + (0.8 * (((float)j) / ((float)h))));
            }

            dstPtr = line;

            for (uint i = 0; i < w; ++i)
            {
                dstPtr[2] = (srcPtr[0] * 255UL) / 65535UL;  // Blue
                dstPtr[1] = (srcPtr[1] * 255UL) / 65535UL;  // Green
                dstPtr[0] = (srcPtr[2] * 255UL) / 65535UL;  // Red

                srcPtr   += 4;
                dstPtr   += 3;
            }

            jpeg_write_scanlines(&cinfo, &line, 1);
        }
    }

    cleanupData->deleteLine();

    // -------------------------------------------------------------------

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    delete cleanupData;

    imageSetAttribute(QLatin1String("savedformat"), QLatin1String("JPG"));

    saveMetadata(filePath);

    return true;
}

bool JPEGLoader::hasAlpha() const
{
    return false;
}

bool JPEGLoader::sixteenBit() const
{
    return false;
}

bool JPEGLoader::isReadOnly() const
{
    return false;
}

}  // namespace Digikam
