/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-29
 * Description : perform lossless rotation/flip to JPEG file
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of the loading code is taken from qjpeghandler.cpp, copyright follows:
 * Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
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

/*
 * Define libjpeg_EXPORTS: kde-win emerged jpeg lib uses this define to
 * decide wether to make dllimport (by default) or dllexport. We need to
 * export.
 */

#define libjpeg_EXPORTS

#include "jpegutils.h"

// C++ includes

#include <cstdio>
#include <cstdlib>

// C ANSI includes

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <setjmp.h>
#include <jpeglib.h>
}

// KDE includes

#include <kdebug.h>
#include <ktemporaryfile.h>

// Qt includes

#include <QImageReader>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>

// Local includes

#include "config-digikam.h"
#include "dmetadata.h"
#include "filereadwritelock.h"
#include "transupp.h"

#ifdef Q_CC_MSVC
#include "jpegwin.h"
#endif

namespace Digikam
{

// To manage Errors/Warnings handling provide by libjpeg

struct jpegutils_jpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

static void jpegutils_jpeg_error_exit(j_common_ptr cinfo);
static void jpegutils_jpeg_emit_message(j_common_ptr cinfo, int msg_level);
static void jpegutils_jpeg_output_message(j_common_ptr cinfo);

static void jpegutils_jpeg_error_exit(j_common_ptr cinfo)
{
    jpegutils_jpeg_error_mgr* myerr = (jpegutils_jpeg_error_mgr*) cinfo->err;

    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    kDebug() << "Jpegutils error, aborting operation:" << buffer;

    longjmp(myerr->setjmp_buffer, 1);
}

static void jpegutils_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
    Q_UNUSED(msg_level)
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef USE_ADVANCEDDEBUGMSG
    kDebug() << buffer << " (" << msg_level << ")";
#endif
}

static void jpegutils_jpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

#ifdef USE_ADVANCEDDEBUGMSG
    kDebug() << buffer;
#endif
}

bool loadJPEGScaled(QImage& image, const QString& path, int maximumSize)
{
    FileReadLocker lock(path);

    if (!isJpegImage(path))
    {
        return false;
    }

    FILE* inputFile = fopen(QFile::encodeName(path), "rb");

    if (!inputFile)
    {
        return false;
    }

    struct jpeg_decompress_struct   cinfo;

    struct jpegutils_jpeg_error_mgr jerr;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err                 = jpeg_std_error(&jerr);

    cinfo.err->error_exit     = jpegutils_jpeg_error_exit;

    cinfo.err->emit_message   = jpegutils_jpeg_emit_message;

    cinfo.err->output_message = jpegutils_jpeg_output_message;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(inputFile);
        return false;
    }

    jpeg_create_decompress(&cinfo);
#ifdef Q_CC_MSVC
    QFile inFile(path);
    QByteArray buffer;

    if (inFile.open(QIODevice::ReadOnly))
    {
        buffer = inFile.readAll();
        inFile.close();
    }

    jpeg_memory_src(&cinfo, (JOCTET*)buffer.data(), buffer.size());
#else
    jpeg_stdio_src(&cinfo, inputFile);
#endif
    jpeg_read_header(&cinfo, true);

    int imgSize = qMax(cinfo.image_width, cinfo.image_height);

    // libjpeg supports 1/1, 1/2, 1/4, 1/8
    int scale=1;

    while (maximumSize* scale*2<=imgSize)
    {
        scale*=2;
    }

    if (scale>8)
    {
        scale=8;
    }

    //cinfo.scale_num = 1;
    //cinfo.scale_denom = scale;
    cinfo.scale_denom *= scale;

    switch (cinfo.jpeg_color_space)
    {
        case JCS_UNKNOWN:
            break;
        case JCS_GRAYSCALE:
        case JCS_RGB:
        case JCS_YCbCr:
            cinfo.out_color_space = JCS_RGB;
            break;
        case JCS_CMYK:
        case JCS_YCCK:
            cinfo.out_color_space = JCS_CMYK;
            break;
        default:
            break;
    }

    jpeg_start_decompress(&cinfo);

    QImage img;

    // We only take RGB with 1 or 3 components, or CMYK with 4 components
    if (!(
            (cinfo.out_color_space == JCS_RGB  && (cinfo.output_components == 3 || cinfo.output_components == 1))
            || (cinfo.out_color_space == JCS_CMYK &&  cinfo.output_components == 4)
        ))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(inputFile);
        return false;
    }

    switch (cinfo.output_components)
    {
        case 3:
        case 4:
            img = QImage( cinfo.output_width, cinfo.output_height, QImage::Format_RGB32 );
            break;
        case 1: // B&W image
            img = QImage( cinfo.output_width, cinfo.output_height, QImage::Format_Indexed8);
            img.setNumColors(256);

            for (int i = 0 ; i < 256 ; ++i)
            {
                img.setColor(i, qRgb(i, i, i));
            }

            break;
    }

    uchar* data = img.bits();
    int bpl = img.bytesPerLine();

    while (cinfo.output_scanline < cinfo.output_height)
    {
        uchar* d = data + cinfo.output_scanline * bpl;
        jpeg_read_scanlines(&cinfo, &d, 1);
    }

    jpeg_finish_decompress(&cinfo);

    if (cinfo.output_components == 3)
    {
        // Expand 24->32 bpp.
        for (uint j=0; j<cinfo.output_height; ++j)
        {
            uchar* in = img.scanLine(j) + cinfo.output_width * 3;
            QRgb* out = (QRgb*)img.scanLine(j);

            for (uint i = cinfo.output_width; --i; )
            {
                in -= 3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }
    else if (cinfo.out_color_space == JCS_CMYK)
    {
        for (uint j = 0; j < cinfo.output_height; ++j)
        {
            uchar* in = img.scanLine(j) + cinfo.output_width * 4;
            QRgb* out = (QRgb*)img.scanLine(j);

            for (uint i = cinfo.output_width; --i; )
            {
                in -= 4;
                int k = in[3];
                out[i] = qRgb(k * in[0] / 255, k * in[1] / 255, k * in[2] / 255);
            }
        }
    }

    if (cinfo.density_unit == 1)
    {
        img.setDotsPerMeterX(int(100. * cinfo.X_density / 2.54));
        img.setDotsPerMeterY(int(100. * cinfo.Y_density / 2.54));
    }
    else if (cinfo.density_unit == 2)
    {
        img.setDotsPerMeterX(int(100. * cinfo.X_density));
        img.setDotsPerMeterY(int(100. * cinfo.Y_density));
    }

    //int newMax = qMax(cinfo.output_width, cinfo.output_height);
    //int newx = maximumSize*cinfo.output_width / newMax;
    //int newy = maximumSize*cinfo.output_height / newMax;

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    image = img;

    return true;
}

JpegRotator::JpegRotator(const QString& file)
    : m_file(file), m_destFile(file)
{
    m_metadata.load(file);
    m_orientation = m_metadata.getImageOrientation();
    QFileInfo info(file);
    m_documentName = info.fileName();
}

void JpegRotator::setCurrentOrientation(KExiv2Iface::KExiv2::ImageOrientation orientation)
{
    m_orientation = orientation;
}

void JpegRotator::setDocumentName(const QString& documentName)
{
    m_documentName = documentName;
}

void JpegRotator::setDestinationFile(const QString dest)
{
    m_destFile = dest;
}

bool JpegRotator::autoExifTransform()
{
    return exifTransform(KExiv2Iface::RotationMatrix::NoTransformation);
}

bool JpegRotator::exifTransform(TransformAction action)
{
    KExiv2Iface::RotationMatrix matrix;
    matrix *= m_orientation;
    matrix *= action;
    return exifTransform(matrix);
}

bool JpegRotator::exifTransform(const KExiv2Iface::RotationMatrix &matrix)
{
    FileWriteLocker lock(m_destFile);

    QFileInfo fi(m_file);

    if (!fi.exists())
    {
        kError() << "ExifRotate: file does not exist: " << m_file;
        return false;
    }

    if (!isJpegImage(m_file))
    {
        // Not a jpeg image.
        kError() << "ExifRotate: not a JPEG file: " << m_file;
        return false;
    }

    QList<TransformAction> actions = matrix.transformations();

    if (actions.isEmpty())
    {
        if (m_file != m_destFile)
        {
            copyFile(m_file, m_destFile);
        }
        return true;
    }

    QString dest = m_destFile;
    QString src  = m_file;
    QString dir  = fi.absolutePath();
    QStringList unlinkLater;
    for (int i=0; i<actions.size(); i++)
    {
        KTemporaryFile temp;
        temp.setPrefix(dir + "/");
        temp.setSuffix(".digikamtempfile.jpg");
        temp.setAutoRemove(false);
        temp.open();

        if (!performJpegTransform(actions[i], src, temp))
        {
            kError() << "JPEG transform of" << src << "failed";
            return false;
        }

        if (i+1 != actions.size())
        {
            // another round
            src = temp.fileName();
            unlinkLater << temp.fileName();
            continue;
        }

        // finalize
        updateMetadata(temp.fileName(), matrix);

        // atomic rename
        if (::rename(QFile::encodeName(temp.fileName()), QFile::encodeName(m_destFile)) != 0)
        {
            unlinkLater << temp.fileName();
            kError() << "Renaming" << temp.fileName() << "to" << m_destFile << "failed";
            break;
        }
    }

    foreach (const QString tempFile, unlinkLater)
    {
        ::unlink(QFile::encodeName(tempFile));
    }

    return true;
}


void JpegRotator::updateMetadata(const QString& fileName, const KExiv2Iface::RotationMatrix &matrix)
{
    // Reset the Exif orientation tag of the temp image to normal
    m_metadata.setImageOrientation(DMetadata::ORIENTATION_NORMAL);

    QMatrix qmatrix = matrix.toMatrix();
    QRect r(QPoint(0,0), m_originalSize);
    QSize newSize = qmatrix.mapRect(r).size();

    // Get the new image dimension of the temp image. Using a dummy QImage object here
    // has a sense because the Exif dimension information can be missing from original image.
    // Get new dimensions with QImage will always work...
    m_metadata.setImageDimensions(newSize);

    // Update the image thumbnail.
    QImage exifThumb = m_metadata.getExifThumbnail(true);
    if (!exifThumb.isNull())
    {
        m_metadata.setExifThumbnail(exifThumb.transformed(qmatrix));
    }
    QImage imagePreview;
    if (m_metadata.getImagePreview(imagePreview))
    {
        m_metadata.setImagePreview(imagePreview.transformed(qmatrix));
    }

    // Update Exif Document Name tag (the original file name from camera for example).
    m_metadata.setExifTagString("Exif.Image.DocumentName", m_documentName);

    // We update all new metadata now...
    m_metadata.save(fileName);

    // set the file modification time of the temp file to that
    // of the original file
    struct stat st;
    ::stat(QFile::encodeName(m_file), &st);

    struct utimbuf ut;
    ut.modtime = st.st_mtime;
    ut.actime  = st.st_atime;

    ::utime(QFile::encodeName(fileName), &ut);
}



bool JpegRotator::performJpegTransform(TransformAction action, const QString& src, QFile& dest)
{
    QByteArray in  = QFile::encodeName(src);
    QByteArray out = QFile::encodeName(dest.fileName());

    JCOPY_OPTION copyoption = JCOPYOPT_ALL;
    jpeg_transform_info transformoption;

    transformoption.force_grayscale = false;
    transformoption.trim            = false;
    transformoption.transform       = JXFORM_NONE;
    #if (JPEG_LIB_VERSION >= 80)
    // we need to initialize a few more parameters, see bug 274947
    transformoption.perfect         = false;
    transformoption.crop            = false;
    #endif

    transformoption.transform = (Digikam::JXFORM_CODE)action;


    if (transformoption.transform == JXFORM_NONE)
    {
        return true;
    }
    // A transformation must be done.

    struct jpeg_decompress_struct srcinfo;
    struct jpeg_compress_struct   dstinfo;
    struct jpegutils_jpeg_error_mgr jsrcerr, jdsterr;
    jvirt_barray_ptr* src_coef_arrays;
    jvirt_barray_ptr* dst_coef_arrays;

    // Initialize the JPEG decompression object with default error handling
    srcinfo.err                 = jpeg_std_error(&jsrcerr);
    srcinfo.err->error_exit     = jpegutils_jpeg_error_exit;
    srcinfo.err->emit_message   = jpegutils_jpeg_emit_message;
    srcinfo.err->output_message = jpegutils_jpeg_output_message;

    // Initialize the JPEG compression object with default error handling
    dstinfo.err                 = jpeg_std_error(&jdsterr);
    dstinfo.err->error_exit     = jpegutils_jpeg_error_exit;
    dstinfo.err->emit_message   = jpegutils_jpeg_emit_message;
    dstinfo.err->output_message = jpegutils_jpeg_output_message;

    FILE* input_file;
    FILE* output_file;

    input_file = fopen(in, "rb");

    if (!input_file)
    {
        kWarning() << "ExifRotate: Error in opening input file: " << input_file;
        return false;
    }

    output_file = fdopen(dest.handle(), "wb");

    if (!output_file)
    {
        fclose(input_file);
        kWarning() << "ExifRotate: Error in opening output file: " << output_file;
        return false;
    }

    if (setjmp(jsrcerr.setjmp_buffer) || setjmp(jdsterr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&srcinfo);
        jpeg_destroy_compress(&dstinfo);
        fclose(input_file);
        fclose(output_file);
        return false;
    }

    jpeg_create_decompress(&srcinfo);
    jpeg_create_compress(&dstinfo);

    jpeg_stdio_src(&srcinfo, input_file);
    jcopy_markers_setup(&srcinfo, copyoption);

    (void) jpeg_read_header(&srcinfo, true);

    // Read original size initially
    if (!m_originalSize.isValid())
    {
        m_originalSize = QSize(srcinfo.image_width, srcinfo.image_height);
    }

    jtransform_request_workspace(&srcinfo, &transformoption);

    // Read source file as DCT coefficients
    src_coef_arrays = jpeg_read_coefficients(&srcinfo);

    // Initialize destination compression parameters from source values
    jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

    dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);

    // Specify data destination for compression
    jpeg_stdio_dest(&dstinfo, output_file);

    // Start compressor (note no image data is actually written here)
    dstinfo.optimize_coding = true;
    jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

    // Copy to the output file any extra markers that we want to preserve
    jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

    jtransform_execute_transformation(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);

    // Finish compression and release memory
    jpeg_finish_compress(&dstinfo);
    jpeg_destroy_compress(&dstinfo);
    (void) jpeg_finish_decompress(&srcinfo);
    jpeg_destroy_decompress(&srcinfo);

    fclose(input_file);
    fclose(output_file);

    return true;
}

bool jpegConvert(const QString& src, const QString& dest, const QString& documentName, const QString& format)
{
    QFileInfo fi(src);

    if (!fi.exists())
    {
        kDebug() << "JpegConvert: file do not exist: " << src;
        return false;
    }

    if (isJpegImage(src))
    {
        DImg image(src);

        // Get image Exif/IPTC data.
        DMetadata meta(image.getMetadata());

        // Update IPTC preview.
        QImage preview = image.smoothScale(1280, 1024, Qt::KeepAspectRatio).copyQImage();

        // TODO: see B.K.O #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
        // bigger than 64K duing of image preview tag size, the target JPEG image will be
        // broken. Note that IPTC image preview tag is limited to 256K!!!
        // Temp. solution to disable IPTC preview record in JPEG file until a right solution
        // will be found into Exiv2.
        // Note : There is no limitation with TIFF and PNG about IPTC byte array size.

        if (format.toUpper() != QString("JPG") && format.toUpper() != QString("JPEG") &&
            format.toUpper() != QString("JPE"))
        {
            meta.setImagePreview(preview);
        }

        // Update Exif thumbnail.
        QImage thumb = preview.scaled(160, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        meta.setExifThumbnail(thumb);

        // Update Exif Document Name tag (the original file name from camera for example).
        meta.setExifTagString("Exif.Image.DocumentName", documentName);

        // Store new Exif/IPTC data into image.
        image.setMetadata(meta.data());

        // And now save the image to a new file format.

        if ( format.toUpper() == QString("PNG") )
        {
            image.setAttribute("quality", 9);
        }

        if ( format.toUpper() == QString("TIFF") || format.toUpper() == QString("TIF") )
        {
            image.setAttribute("compress", true);
        }

        if ( format.toUpper() == QString("JP2") || format.toUpper() == QString("JPX") ||
             format.toUpper() == QString("JPC") || format.toUpper() == QString("PGX") ||
             format.toUpper() == QString("J2K") )
        {
            image.setAttribute("quality", 100);    // LossLess
        }

        if ( format.toUpper() == QString("PGF") )
        {
            image.setAttribute("quality", 0);    // LossLess
        }

        return (image.save(dest, format));
    }

    return false;
}

bool isJpegImage(const QString& file)
{
    // Check if the file is an JPEG image
    QString format = QString(QImageReader::imageFormat(file)).toUpper();
    kDebug() << "mimetype = " << format;

    if (format !="JPEG")
    {
        return false;
    }

    return true;
}

bool copyFile(const QString& src, const QString& dst)
{
    QFile sFile(src);
    QFile dFile(dst);

    if ( !sFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    if ( !dFile.open(QIODevice::WriteOnly) )
    {
        sFile.close();
        return false;
    }

    const int MAX_IPC_SIZE = (1024*32);

    char buffer[MAX_IPC_SIZE];

    qint64 len;

    while ((len = sFile.read(buffer, MAX_IPC_SIZE)) != 0)
    {
        if (len == -1 || dFile.write(buffer, (qint64)len) == -1)
        {
            sFile.close();
            dFile.close();
            return false;
        }
    }

    sFile.close();
    dFile.close();

    return true;
}

} // namespace Digikam
