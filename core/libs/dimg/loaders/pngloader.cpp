/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : a PNG image loader for DImg framework.
 *
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

#define PNG_BYTES_TO_CHECK 4

#include "pngloader.h"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// C++ includes

#include <cstdlib>
#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>
#include <QSysInfo>

// Local includes

#include "metaengine.h"
#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"
#include "dimg.h"
#include "dimgloaderobserver.h"

// libPNG includes

extern "C"
{
#include <png.h>
}

#ifdef Q_OS_WIN
void _ReadProc(struct png_struct_def* png_ptr, png_bytep data, png_size_t size)
{
    FILE* const file_handle = (FILE*)png_get_io_ptr(png_ptr);
    fread(data, size, 1, file_handle);
}
#endif

namespace Digikam
{

#if PNG_LIBPNG_VER_MAJOR >= 1 && PNG_LIBPNG_VER_MINOR >= 5
typedef png_bytep iCCP_data;
#else
typedef png_charp iCCP_data;
#endif

PNGLoader::PNGLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

bool PNGLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    png_uint_32  w32, h32;
    int          width, height;
    FILE*        f          = 0;
    int          bit_depth, color_type, interlace_type;
    png_structp  png_ptr    = NULL;
    png_infop    info_ptr   = NULL;

    readMetadata(filePath, DImg::PNG);

    // -------------------------------------------------------------------
    // Open the file

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Opening file" << filePath;

    f = fopen(QFile::encodeName(filePath).constData(), "rb");

    if (!f)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot open image file.";
        loadingFailed();
        return false;
    }

    unsigned char buf[PNG_BYTES_TO_CHECK];

    size_t membersRead = fread(buf, 1, PNG_BYTES_TO_CHECK, f);

#if PNG_LIBPNG_VER >= 10400
    if ((membersRead != PNG_BYTES_TO_CHECK) || png_sig_cmp(buf, 0, PNG_BYTES_TO_CHECK))
#else
    if ((membersRead != PNG_BYTES_TO_CHECK) || !png_check_sig(buf, PNG_BYTES_TO_CHECK))
#endif
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Not a PNG image file.";
        fclose(f);
        loadingFailed();
        return false;
    }

    rewind(f);

    // -------------------------------------------------------------------
    // Initialize the internal structures

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Invalid PNG image file structure.";
        fclose(f);
        loadingFailed();
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot reading PNG image file structure.";
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(f);
        loadingFailed();
        return false;
    }

    // -------------------------------------------------------------------
    // PNG error handling. If an error occurs during reading, libpng
    // will jump here

    // setjmp-save cleanup
    class CleanupData
    {
    public:

        CleanupData():
            data(0),
            lines(0),
            f(0)
        {
        }

        ~CleanupData()
        {
            delete [] data;
            freeLines();

            if (f)
            {
                fclose(f);
            }
        }

        void setData(uchar* const d)
        {
            data = d;
        }

        void setLines(uchar** const l)
        {
            lines = l;
        }

        void setFile(FILE* const file)
        {
            f = file;
        }

        void takeData()
        {
            data = 0;
        }

        void freeLines()
        {
            if (lines)
            {
                free(lines);
            }

            lines = 0;
        }

        uchar*  data;
        uchar** lines;
        FILE*   f;
    };

    CleanupData* const cleanupData = new CleanupData;
    cleanupData->setFile(f);

#if PNG_LIBPNG_VER >= 10400
    if (setjmp(png_jmpbuf(png_ptr)))
#else
    if (setjmp(png_ptr->jmpbuf))
#endif
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Internal libPNG error during reading file. Process aborted!";
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        delete cleanupData;
        loadingFailed();
        return false;
    }

#ifdef Q_OS_WIN
    png_set_read_fn(png_ptr, f, _ReadProc);
#else
    png_init_io(png_ptr, f);
#endif

    // -------------------------------------------------------------------
    // Read all PNG info up to image data

    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)(&w32),
                 (png_uint_32*)(&h32), &bit_depth, &color_type,
                 &interlace_type, NULL, NULL);

    width  = (int)w32;
    height = (int)h32;

    int colorModel = DImg::COLORMODELUNKNOWN;
    m_sixteenBit   = (bit_depth == 16);

    switch (color_type)
    {
        case PNG_COLOR_TYPE_RGB:            // RGB
            m_hasAlpha = false;
            colorModel = DImg::RGB;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:     // RGBA
            m_hasAlpha = true;
            colorModel = DImg::RGB;
            break;

        case PNG_COLOR_TYPE_GRAY:          // Grayscale
            m_hasAlpha = false;
            colorModel = DImg::GRAYSCALE;
            break;

        case PNG_COLOR_TYPE_GRAY_ALPHA:    // Grayscale + Alpha
            m_hasAlpha = true;
            colorModel = DImg::GRAYSCALE;
            break;

        case PNG_COLOR_TYPE_PALETTE:       // Indexed
            m_hasAlpha = false;
            colorModel = DImg::INDEXED;
            break;
    }

    uchar* data  = 0;

    if (m_loadFlags & LoadImageData)
    {
        // TODO: Endianness:
        // You may notice that the code for little and big endian
        // below is now identical. This was found to work by PPC users.
        // If this proves right, all the conditional clauses can be removed.

        if (bit_depth == 16)
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in 16 bits/color/pixel.";

            switch (color_type)
            {
                case PNG_COLOR_TYPE_RGB :            // RGB
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB";

                    png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
                    break;

                case PNG_COLOR_TYPE_RGB_ALPHA :     // RGBA
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB_ALPHA";
                    break;

                case PNG_COLOR_TYPE_GRAY :          // Grayscale
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY";

                    png_set_gray_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);

                    break;

                case PNG_COLOR_TYPE_GRAY_ALPHA :    // Grayscale + Alpha
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY_ALPHA";

                    png_set_gray_to_rgb(png_ptr);

                    break;

                case PNG_COLOR_TYPE_PALETTE :       // Indexed
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_PALETTE";

                    png_set_palette_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFFFF, PNG_FILLER_AFTER);

                    break;

                default:
                    qCWarning(DIGIKAM_DIMG_LOG_PNG) << "PNG color type unknown.";

                    delete cleanupData;
                    loadingFailed();
                    return false;
            }
        }
        else
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in >=8 bits/color/pixel.";

            png_set_packing(png_ptr);

            switch (color_type)
            {
                case PNG_COLOR_TYPE_RGB :           // RGB
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB";

                    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

                    break;

                case PNG_COLOR_TYPE_RGB_ALPHA :     // RGBA
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_RGB_ALPHA";

                    break;

                case PNG_COLOR_TYPE_GRAY :          // Grayscale
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY";

#if PNG_LIBPNG_VER >= 10400
                    png_set_expand_gray_1_2_4_to_8(png_ptr);
#else
                    png_set_gray_1_2_4_to_8(png_ptr);
#endif
                    png_set_gray_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

                    break;

                case PNG_COLOR_TYPE_GRAY_ALPHA :    // Grayscale + alpha
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_GRAY_ALPHA";

                    png_set_gray_to_rgb(png_ptr);
                    break;

                case PNG_COLOR_TYPE_PALETTE :       // Indexed
                    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG in PNG_COLOR_TYPE_PALETTE";

                    png_set_packing(png_ptr);
                    png_set_palette_to_rgb(png_ptr);
                    png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

                    break;

                default:
                    qCWarning(DIGIKAM_DIMG_LOG_PNG) << "PNG color type unknown." << color_type;

                    delete cleanupData;
                    loadingFailed();
                    return false;
            }
        }

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(png_ptr);
        }

        png_set_bgr(png_ptr);

        //png_set_swap_alpha(png_ptr);

        if (observer)
        {
            observer->progressInfo(m_image, 0.1F);
        }

        // -------------------------------------------------------------------
        // Get image data.

        // Call before png_read_update_info and png_start_read_image()
        // for non-interlaced images number_passes will be 1
        int number_passes = png_set_interlace_handling(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        if (m_sixteenBit)
        {
            data = new_failureTolerant(width, height, 8); // 16 bits/color/pixel
        }
        else
        {
            data = new_failureTolerant(width, height, 4); // 8 bits/color/pixel
        }

        cleanupData->setData(data);

        uchar** lines = 0;
        lines         = (uchar**)malloc(height * sizeof(uchar*));
        cleanupData->setLines(lines);

        if (!data || !lines)
        {
            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Cannot allocate memory to load PNG image data.";
            png_read_end(png_ptr, info_ptr);
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
            delete cleanupData;
            loadingFailed();
            return false;
        }

        for (int i = 0; i < height; ++i)
        {
            if (m_sixteenBit)
            {
                lines[i] = data + (i * width * 8);
            }
            else
            {
                lines[i] = data + (i * width * 4);
            }
        }

        // The easy way to read the whole image
        // png_read_image(png_ptr, lines);
        // The other way to read images is row by row. Necessary for observer.
        // Now we need to deal with interlacing.

        for (int pass = 0; pass < number_passes; ++pass)
        {
            int y;
            int checkPoint = 0;

            for (y = 0; y < height; ++y)
            {
                if (observer && y == checkPoint)
                {
                    checkPoint += granularity(observer, height, 0.7F);

                    if (!observer->continueQuery(m_image))
                    {
                        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
                        delete cleanupData;
                        loadingFailed();
                        return false;
                    }

                    // use 10% - 80% for progress while reading rows
                    observer->progressInfo(m_image, 0.1 + (0.7 * (((float)y) / ((float)height))));
                }

                png_read_rows(png_ptr, lines + y, NULL, 1);
            }
        }

        cleanupData->freeLines();

        if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        {
            // Swap bytes in 16 bits/color/pixel for DImg

            if (m_sixteenBit)
            {
                uchar ptr[8];   // One pixel to swap

                for (int p = 0; p < width * height * 8; p += 8)
                {
                    memcpy(&ptr[0], &data[p], 8);   // Current pixel

                    data[ p ] = ptr[1];  // Blue
                    data[p + 1] = ptr[0];
                    data[p + 2] = ptr[3]; // Green
                    data[p + 3] = ptr[2];
                    data[p + 4] = ptr[5]; // Red
                    data[p + 5] = ptr[4];
                    data[p + 6] = ptr[7]; // Alpha
                    data[p + 7] = ptr[6];
                }
            }
        }
    }

    if (observer)
    {
        observer->progressInfo(m_image, 0.9F);
    }

    // -------------------------------------------------------------------
    // Read image ICC profile

    if (m_loadFlags & LoadICCData)
    {
        png_charp   profile_name;
        iCCP_data   profile_data = NULL;
        png_uint_32 profile_size;
        int         compression_type;

        png_get_iCCP(png_ptr, info_ptr, &profile_name, &compression_type, &profile_data, &profile_size);

        if (profile_data != NULL)
        {
            QByteArray profile_rawdata;
            profile_rawdata.resize(profile_size);
            memcpy(profile_rawdata.data(), profile_data, profile_size);
            imageSetIccProfile(profile_rawdata);
        }
        else
        {
            // If ICC profile is null, check Exif metadata.
            checkExifWorkingColorSpace();
        }
    }

    // -------------------------------------------------------------------
    // Get embedded text data.

    png_text* text_ptr = 0;
    int num_comments   = png_get_text(png_ptr, info_ptr, &text_ptr, NULL);

    /*
    Standard Embedded text includes in PNG :

    Title            Short (one line) title or caption for image
    Author           Name of image's creator
    Description      Description of image (possibly long)
    Copyright        Copyright notice
    Creation Time    Time of original image creation
    Software         Software used to create the image
    Disclaimer       Legal disclaimer
    Warning          Warning of nature of content
    Source           Device used to create the image
    Comment          Miscellaneous comment; conversion from GIF comment

    Extra Raw profiles tag are used by ImageMagick and defines at this URL :
    http://search.cpan.org/src/EXIFTOOL/Image-ExifTool-5.87/html/TagNames/PNG.html#TextualData
    */

    if (m_loadFlags & LoadICCData)
    {
        for (int i = 0; i < num_comments; ++i)
        {
            // Check if we have a Raw profile embedded using ImageMagick technique.

            if (memcmp(text_ptr[i].key, "Raw profile type exif", 21) != 0 ||
                memcmp(text_ptr[i].key, "Raw profile type APP1", 21) != 0 ||
                memcmp(text_ptr[i].key, "Raw profile type iptc", 21) != 0)
            {
                imageSetEmbbededText(QLatin1String(text_ptr[i].key), QLatin1String(text_ptr[i].text));

                qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Reading PNG Embedded text: key=" << text_ptr[i].key
                         << " text=" << text_ptr[i].text;
            }
        }
    }

    // -------------------------------------------------------------------

    if (m_loadFlags & LoadImageData)
    {
        png_read_end(png_ptr, info_ptr);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    cleanupData->takeData();
    delete cleanupData;

    if (observer)
    {
        observer->progressInfo(m_image, 1.0);
    }

    imageWidth()  = width;
    imageHeight() = height;
    imageData()   = data;
    imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
    imageSetAttribute(QLatin1String("originalColorModel"), colorModel);
    imageSetAttribute(QLatin1String("originalBitDepth"),   bit_depth);
    imageSetAttribute(QLatin1String("originalSize"),       QSize(width, height));

    return true;
}

bool PNGLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    FILE*          f           = 0;
    png_structp    png_ptr;
    png_infop      info_ptr;
    uchar*         ptr         = 0;
    uchar*         data        = 0;
    uint           x, y, j;
    png_bytep      row_ptr;
    png_color_8    sig_bit;
    int            quality     = 75;
    int            compression = 3;

    // -------------------------------------------------------------------
    // Open the file

    f = fopen(QFile::encodeName(filePath).constData(), "wb");

    if (!f)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot open target image file.";
        return false;
    }

    // -------------------------------------------------------------------
    // Initialize the internal structures

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Invalid target PNG image file structure.";
        fclose(f);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL)
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Cannot create PNG image file structure.";
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
        fclose(f);
        return false;
    }

    // -------------------------------------------------------------------
    // PNG error handling. If an error occurs during writing, libpng
    // will jump here

    // setjmp-save cleanup
    class CleanupData
    {
    public:

        CleanupData():
            data(0),
            f(0)
        {
        }

        ~CleanupData()
        {
            delete [] data;

            if (f)
            {
                fclose(f);
            }
        }

        void setData(uchar* const d)
        {
            data = d;
        }

        void setFile(FILE* const file)
        {
            f = file;
        }

        uchar* data;
        FILE*  f;
    };

    CleanupData* const cleanupData = new CleanupData;
    cleanupData->setFile(f);

#if PNG_LIBPNG_VER >= 10400

    if (setjmp(png_jmpbuf(png_ptr)))
#else
    if (setjmp(png_ptr->jmpbuf))
#endif
    {
        qCWarning(DIGIKAM_DIMG_LOG_PNG) << "Internal libPNG error during writing file. Process aborted!";
        png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
        png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
        delete cleanupData;
        return false;
    }

    png_init_io(png_ptr, f);
    png_set_bgr(png_ptr);

    //png_set_swap_alpha(png_ptr);

    if (imageHasAlpha())
    {
        png_set_IHDR(png_ptr, info_ptr, imageWidth(), imageHeight(), imageBitsDepth(),
                     PNG_COLOR_TYPE_RGB_ALPHA,  PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        if (imageSixteenBit())
        {
            data = new uchar[imageWidth() * 8 * sizeof(uchar)];
        }
        else
        {
            data = new uchar[imageWidth() * 4 * sizeof(uchar)];
        }
    }
    else
    {
        png_set_IHDR(png_ptr, info_ptr, imageWidth(), imageHeight(), imageBitsDepth(),
                     PNG_COLOR_TYPE_RGB,        PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        if (imageSixteenBit())
        {
            data = new uchar[imageWidth() * 6 * sizeof(uchar)];
        }
        else
        {
            data = new uchar[imageWidth() * 3 * sizeof(uchar)];
        }
    }

    cleanupData->setData(data);

    sig_bit.red   = imageBitsDepth();
    sig_bit.green = imageBitsDepth();
    sig_bit.blue  = imageBitsDepth();
    sig_bit.alpha = imageBitsDepth();
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);

    // -------------------------------------------------------------------
    // Quality to convert to compression

    QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
    quality              = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "DImg quality level: " << quality;

    if (quality < 1)
    {
        quality = 1;
    }

    if (quality > 99)
    {
        quality = 99;
    }

    quality     = quality / 10;
    compression = 9 - quality;

    if (compression < 0)
    {
        compression = 0;
    }

    if (compression > 9)
    {
        compression = 9;
    }

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "PNG compression level: " << compression;
    png_set_compression_level(png_ptr, compression);

    // -------------------------------------------------------------------
    // Write ICC profile.

    QByteArray profile_rawdata = m_image->getIccProfile().data();

    if (!profile_rawdata.isEmpty())
    {
        purgeExifWorkingColorSpace();
        png_set_iCCP(png_ptr, info_ptr, (png_charp)("icc"), PNG_COMPRESSION_TYPE_BASE, (iCCP_data)profile_rawdata.data(), profile_rawdata.size());
    }

    // -------------------------------------------------------------------
    // Write embedded Text

    typedef QMap<QString, QString> EmbeddedTextMap;
    EmbeddedTextMap map = imageEmbeddedText();

    for (EmbeddedTextMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        if (it.key() != QLatin1String("Software") && it.key() != QLatin1String("Comment"))
        {
            QByteArray key = it.key().toLatin1();
            QByteArray value = it.value().toLatin1();
            png_text text;
            text.key  = key.data();
            text.text = value.data();

            qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Writing PNG Embedded text: key=" << text.key << " text=" << text.text;

            text.compression = PNG_TEXT_COMPRESSION_zTXt;
            png_set_text(png_ptr, info_ptr, &(text), 1);
        }
    }

    // Update 'Software' text tag.
    QString software = QLatin1String("digiKam ");
    software.append(digiKamVersion());
    QString libpngver = QLatin1String(PNG_HEADER_VERSION_STRING);
    libpngver.replace(QLatin1Char('\n'), QLatin1Char(' '));
    software.append(QString::fromLatin1(" (%1)").arg(libpngver));
    QByteArray softwareAsAscii = software.toLatin1();
    png_text text;
    text.key  = (png_charp)("Software");
    text.text = softwareAsAscii.data();

    qCDebug(DIGIKAM_DIMG_LOG_PNG) << "Writing PNG Embedded text: key=" << text.key << " text=" << text.text;

    text.compression = PNG_TEXT_COMPRESSION_zTXt;
    png_set_text(png_ptr, info_ptr, &(text), 1);

    if (observer)
    {
        observer->progressInfo(m_image, 0.2F);
    }

    // -------------------------------------------------------------------
    // Write image data

    png_write_info(png_ptr, info_ptr);
    png_set_shift(png_ptr, &sig_bit);
    png_set_packing(png_ptr);
    ptr = imageData();

    uint checkPoint = 0;

    for (y = 0; y < imageHeight(); ++y)
    {

        if (observer && y == checkPoint)
        {
            checkPoint += granularity(observer, imageHeight(), 0.8F);

            if (!observer->continueQuery(m_image))
            {
                png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
                png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);
                delete cleanupData;
                return false;
            }

            observer->progressInfo(m_image, 0.2 + (0.8 * (((float)y) / ((float)imageHeight()))));
        }

        j = 0;

        if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
        {
            for (x = 0; x < imageWidth()*imageBytesDepth(); x += imageBytesDepth())
            {
                if (imageSixteenBit())
                {
                    if (imageHasAlpha())
                    {
                        data[j++] = ptr[x + 1]; // Blue
                        data[j++] = ptr[ x ];
                        data[j++] = ptr[x + 3]; // Green
                        data[j++] = ptr[x + 2];
                        data[j++] = ptr[x + 5]; // Red
                        data[j++] = ptr[x + 4];
                        data[j++] = ptr[x + 7]; // Alpha
                        data[j++] = ptr[x + 6];
                    }
                    else
                    {
                        data[j++] = ptr[x + 1]; // Blue
                        data[j++] = ptr[ x ];
                        data[j++] = ptr[x + 3]; // Green
                        data[j++] = ptr[x + 2];
                        data[j++] = ptr[x + 5]; // Red
                        data[j++] = ptr[x + 4];
                    }
                }
                else
                {
                    if (imageHasAlpha())
                    {
                        data[j++] = ptr[ x ];  // Blue
                        data[j++] = ptr[x + 1]; // Green
                        data[j++] = ptr[x + 2]; // Red
                        data[j++] = ptr[x + 3]; // Alpha
                    }
                    else
                    {
                        data[j++] = ptr[ x ];  // Blue
                        data[j++] = ptr[x + 1]; // Green
                        data[j++] = ptr[x + 2]; // Red
                    }
                }
            }
        }
        else
        {
            int bytes = (imageSixteenBit() ? 2 : 1) * (imageHasAlpha() ? 4 : 3);

            for (x = 0; x < imageWidth()*imageBytesDepth(); x += imageBytesDepth())
            {
                memcpy(data + j, ptr + x, bytes);
                j += bytes;
            }
        }

        row_ptr = (png_bytep) data;

        png_write_rows(png_ptr, &row_ptr, 1);
        ptr += (imageWidth() * imageBytesDepth());
    }

    // -------------------------------------------------------------------

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, (png_infopp) & info_ptr);
    png_destroy_info_struct(png_ptr, (png_infopp) & info_ptr);

    delete cleanupData;

    imageSetAttribute(QLatin1String("savedformat"), QLatin1String("PNG"));

    saveMetadata(filePath);

    return true;
}

bool PNGLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool PNGLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool PNGLoader::isReadOnly() const
{
    return false;
}

}  // namespace Digikam
