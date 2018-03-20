/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-29
 * Description : perform lossless rotation/flip to JPEG file
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "jpegutils.h"

// C++ includes

#include <cstdio>
#include <cstdlib>

// Digikam includes

#include <dimg.h>
#include <dmetadata.h>

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

// Pragma directives to reduce warnings from libjpeg transupp header file.
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

extern "C"
{
#include "transupp.h"
}

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

// Qt includes

#include <QImageReader>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dmetadata.h"
#include "metadatasettings.h"
#include "filereadwritelock.h"

#ifdef Q_OS_WIN
#include "windows.h"
#include "jpegwin.h"
#endif

namespace Digikam
{

namespace JPEGUtils
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
    jpegutils_jpeg_error_mgr* myerr = static_cast<jpegutils_jpeg_error_mgr*>(cinfo->err);

    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCDebug(DIGIKAM_GENERAL_LOG) << "Jpegutils error, aborting operation:" << buffer;

    longjmp(myerr->setjmp_buffer, 1);
}

static void jpegutils_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
    Q_UNUSED(msg_level)
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    // TODO this was behind the ifdef guard for dimg imageloaders, should this class be moved to dimg?
    //qCDebug(DIGIKAM_GENERAL_LOG) << buffer << " (" << msg_level << ")";
}

static void jpegutils_jpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    // TODO this was behind the ifdef guard for dimg imageloaders, should this class be moved to dimg?
    //qCDebug(DIGIKAM_GENERAL_LOG) << buffer;
}

bool loadJPEGScaled(QImage& image, const QString& path, int maximumSize)
{
    FileReadLocker lock(path);

    if (!isJpegImage(path))
    {
        return false;
    }

    FILE* const inputFile = fopen(QFile::encodeName(path).constData(), "rb");

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

#ifdef Q_OS_WIN

    QFile inFile(path);
    QByteArray buffer;

    if (inFile.open(QIODevice::ReadOnly))
    {
        buffer = inFile.readAll();
        inFile.close();
    }

    jpeg_memory_src(&cinfo, (JOCTET*)buffer.data(), buffer.size());

#else  // Q_OS_WIN

    jpeg_stdio_src(&cinfo, inputFile);

#endif // Q_OS_WIN

    jpeg_read_header(&cinfo, true);

    int imgSize = qMax(cinfo.image_width, cinfo.image_height);

    // libjpeg supports 1/1, 1/2, 1/4, 1/8
    int scale=1;

    while(maximumSize*scale*2 <= imgSize)
    {
        scale *= 2;
    }

    if (scale > 8)
    {
        scale = 8;
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
            (cinfo.out_color_space == JCS_RGB  && (cinfo.output_components == 3 || cinfo.output_components == 1)) ||
            (cinfo.out_color_space == JCS_CMYK &&  cinfo.output_components == 4)
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
            img = QImage(cinfo.output_width, cinfo.output_height, QImage::Format_RGB32);
            break;
        case 1: // B&W image
            img = QImage(cinfo.output_width, cinfo.output_height, QImage::Format_Indexed8);
            img.setColorCount(256);

            for (int i = 0 ; i < 256 ; ++i)
            {
                img.setColor(i, qRgb(i, i, i));
            }

            break;
    }

    uchar* const data = img.bits();
    int bpl           = img.bytesPerLine();

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
            uchar* in       = img.scanLine(j) + cinfo.output_width * 3;
            QRgb* const out = reinterpret_cast<QRgb*>(img.scanLine(j));

            for (uint i = cinfo.output_width; --i; )
            {
                in     -= 3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }
    else if (cinfo.out_color_space == JCS_CMYK)
    {
        for (uint j = 0; j < cinfo.output_height; ++j)
        {
            uchar* in       = img.scanLine(j) + cinfo.output_width * 4;
            QRgb* const out = reinterpret_cast<QRgb*>(img.scanLine(j));

            for (uint i = cinfo.output_width; --i; )
            {
                in     -= 4;
                int k  = in[3];
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
    : m_file(file),
      m_destFile(file)
{
    m_metadata.load(file);
    m_orientation  = m_metadata.getImageOrientation();
    QFileInfo info(file);
    m_documentName = info.fileName();
}

// -----------------------------------------------------------------------------

void JpegRotator::setCurrentOrientation(MetaEngine::ImageOrientation orientation)
{
    m_orientation = orientation;
}

void JpegRotator::setDocumentName(const QString& documentName)
{
    m_documentName = documentName;
}

void JpegRotator::setDestinationFile(const QString& dest)
{
    m_destFile = dest;
}

bool JpegRotator::autoExifTransform()
{
    return exifTransform(MetaEngineRotation::NoTransformation);
}

bool JpegRotator::exifTransform(TransformAction action)
{
    MetaEngineRotation matrix;
    matrix *= m_orientation;
    matrix *= action;
    return exifTransform(matrix);
}

bool JpegRotator::exifTransform(const MetaEngineRotation& matrix)
{
    FileWriteLocker lock(m_destFile);

    QFileInfo fi(m_file);

    if (!fi.exists())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "ExifRotate: file does not exist: " << m_file;
        return false;
    }

    if (!isJpegImage(m_file))
    {
        // Not a jpeg image.
        qCDebug(DIGIKAM_GENERAL_LOG) << "ExifRotate: not a JPEG file: " << m_file;
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

    QString     dest = m_destFile;
    QString     src  = m_file;
    QString     dir  = fi.absolutePath();
    QStringList removeLater;

    for (int i = 0 ; i < actions.size() ; i++)
    {
        SafeTemporaryFile* const temp = new SafeTemporaryFile(dir + QLatin1String("/JpegRotator-XXXXXX.digikamtempfile.jpg"));
        temp->setAutoRemove(false);
        temp->open();
        QString tempFile = temp->fileName();
        // Crash fix: a QTemporaryFile is not properly closed until its destructor is called.
        delete temp;

        if (!performJpegTransform(actions[i], src, tempFile))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "JPEG lossless transform failed for" << src;

             // See bug 320107 : if lossless transform cannot be achieve, do lossy transform.
            DImg srcImg;

            qCDebug(DIGIKAM_GENERAL_LOG) << "Trying lossy transform for " << src;

            if (!srcImg.load(src))
            {
                QFile::remove(tempFile);
                return false;
            }

            if (actions[i] != MetaEngineRotation::NoTransformation)
            {
                srcImg.transform(actions[i]);
            }

            srcImg.setAttribute(QLatin1String("quality"), getJpegQuality(src));

            if (!srcImg.save(tempFile, DImg::JPEG))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Lossy transform failed for" << src;

                QFile::remove(tempFile);
                return false;
            }

            qCDebug(DIGIKAM_GENERAL_LOG) << "Lossy transform done for " << src;
        }

        if (i+1 != actions.size())
        {
            // another round
            src = tempFile;
            removeLater << tempFile;
            continue;
        }

        // finalize
        updateMetadata(tempFile, matrix);

        // atomic rename

        if (DMetadata::hasSidecar(tempFile))
        {
            QString sidecarTemp = DMetadata::sidecarPath(tempFile);
            QString sidecarDest = DMetadata::sidecarPath(dest);

            if (sidecarTemp != sidecarDest && QFile::exists(sidecarTemp) && QFile::exists(sidecarDest))
            {
                QFile::remove(sidecarDest);
            }

            if (!QFile::rename(sidecarTemp, sidecarDest))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Renaming sidecar file" << sidecarTemp << "to" << sidecarDest << "failed";
                removeLater << sidecarTemp;
                break;
            }
        }

        if (tempFile != dest && QFile::exists(tempFile) && QFile::exists(dest))
        {
            QFile::remove(dest);
        }

        if (!QFile::rename(tempFile, dest))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Renaming" << tempFile << "to" << dest << "failed";
            removeLater << tempFile;
            break;
        }
    }

    foreach (const QString& tempFile, removeLater)
    {
        QFile::remove(tempFile);
    }

    return true;
}

void JpegRotator::updateMetadata(const QString& fileName, const MetaEngineRotation &matrix)
{
    // Reset the Exif orientation tag of the temp image to normal
    m_metadata.setImageOrientation(DMetadata::ORIENTATION_NORMAL);

    QMatrix qmatrix = matrix.toMatrix();
    QRect r(QPoint(0, 0), m_originalSize);
    QSize newSize   = qmatrix.mapRect(r).size();

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

    // We update all new metadata now...
    m_metadata.save(fileName);

    // File properties restoration.

    struct stat st;

    if (::stat(QFile::encodeName(m_file).constData(), &st) == 0)
    {
        // See bug #329608: Restore file modification time from original file only if updateFileTimeStamp for Setup/Metadata is turned off.

        if (!MetadataSettings::instance()->settings().updateFileTimeStamp)
        {
            struct utimbuf ut;
            ut.modtime = st.st_mtime;
            ut.actime  = st.st_atime;

            if (::utime(QFile::encodeName(fileName).constData(), &ut) != 0)
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to restore modification time for file " << fileName;
            }
        }

        // Restore permissions in all cases

#ifndef Q_OS_WIN

        if (::chmod(QFile::encodeName(fileName).constData(), st.st_mode) != 0)
        {
            qCWarning(DIGIKAM_GENERAL_LOG) << "Failed to restore file permissions for file " << fileName;
        }

#else  // Q_OS_WIN

        QFile::Permissions permissions = QFile::permissions(m_file);
        QFile::setPermissions(fileName, permissions);

#endif //Q_OS_WIN

    }
}

bool JpegRotator::performJpegTransform(TransformAction action, const QString& src, const QString& dest)
{
    QByteArray in                   = QFile::encodeName(src).constData();
    QByteArray out                  = QFile::encodeName(dest).constData();

    JCOPY_OPTION copyoption         = JCOPYOPT_ALL;
    jpeg_transform_info transformoption;

    transformoption.force_grayscale = false;
    transformoption.trim            = false;

#if (JPEG_LIB_VERSION >= 80)

    // we need to initialize a few more parameters, see bug 274947
    transformoption.perfect         = true;   // See bug 320107 : we need perfect transform here.
    transformoption.crop            = false;

#endif // (JPEG_LIB_VERSION >= 80)

    // NOTE : Cast is fine here. See metaengine_rotation.h for details.
    transformoption.transform       = (JXFORM_CODE)action;

    if (transformoption.transform == JXFORM_NONE)
    {
        return true;
    }

    // A transformation must be done.

    struct jpeg_decompress_struct srcinfo;
    struct jpeg_compress_struct   dstinfo;
    struct jpegutils_jpeg_error_mgr jsrcerr, jdsterr;
    jvirt_barray_ptr* src_coef_arrays = 0;
    jvirt_barray_ptr* dst_coef_arrays = 0;

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

    FILE* input_file  = 0;
    FILE* output_file = 0;

    input_file = fopen(in.constData(), "rb");

    if (!input_file)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "ExifRotate: Error in opening input file: " << input_file;
        return false;
    }

    output_file = fopen(out.constData(), "wb");

    if (!output_file)
    {
        fclose(input_file);
        qCWarning(DIGIKAM_GENERAL_LOG) << "ExifRotate: Error in opening output file: " << output_file;
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
    qCDebug(DIGIKAM_GENERAL_LOG) << "Converting " << src << " to " << dest << " format: " << format << " documentName: " << documentName;
    QFileInfo fi(src);

    if (!fi.exists())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "JpegConvert: file do not exist: " << src;
        return false;
    }

    if (isJpegImage(src))
    {
        DImg image(src);

        // Get image Exif/IPTC data.
        DMetadata meta(image.getMetadata());

        // Update IPTC preview.
        QImage preview = image.smoothScale(1280, 1024, Qt::KeepAspectRatio).copyQImage();

        // TODO: see bug #130525. a JPEG segment is limited to 64K. If the IPTC byte array is
        // bigger than 64K duing of image preview tag size, the target JPEG image will be
        // broken. Note that IPTC image preview tag is limited to 256K!!!
        // Temp. solution to disable IPTC preview record in JPEG file until a right solution
        // will be found into Exiv2.
        // Note : There is no limitation with TIFF and PNG about IPTC byte array size.

        if (format.toUpper() != QLatin1String("JPG") && format.toUpper() != QLatin1String("JPEG") &&
            format.toUpper() != QLatin1String("JPE"))
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

        if ( format.toUpper() == QLatin1String("PNG") )
        {
            image.setAttribute(QLatin1String("quality"), 9);
        }

        if ( format.toUpper() == QLatin1String("TIFF") || format.toUpper() == QLatin1String("TIF") )
        {
            image.setAttribute(QLatin1String("compress"), true);
        }

        if ( format.toUpper() == QLatin1String("JP2") || format.toUpper() == QLatin1String("JPX") ||
             format.toUpper() == QLatin1String("JPC") || format.toUpper() == QLatin1String("PGX") ||
             format.toUpper() == QLatin1String("J2K") )
        {
            image.setAttribute(QLatin1String("quality"), 100);    // LossLess
        }

        if ( format.toUpper() == QLatin1String("PGF") )
        {
            image.setAttribute(QLatin1String("quality"), 0);    // LossLess
        }

        return (image.save(dest, format));
    }

    return false;
}

bool isJpegImage(const QString& file)
{
    QFileInfo fileInfo(file);

    // Check if the file is an JPEG image
    QString format = QString::fromUtf8(QImageReader::imageFormat(file)).toUpper();
    // Check if its not MPO format (See bug #307277).
    QString ext    = fileInfo.suffix().toUpper();

    qCDebug(DIGIKAM_GENERAL_LOG) << "mimetype = " << format << " ext = " << ext;

    if (format != QLatin1String("JPEG") || ext == QLatin1String("MPO"))
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

    const  int MAX_IPC_SIZE = (1024*32);
    char   buffer[MAX_IPC_SIZE];
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

int getJpegQuality(const QString& file)
{
    // Set a good default quality
    volatile int quality = 90;

    if (!isJpegImage(file))
    {
        return quality;
    }

    FILE* const inFile = fopen(QFile::encodeName(file).constData(), "rb");

    if (!inFile)
    {
        return quality;
    }

    struct jpeg_decompress_struct   jpeg_info;
    struct jpegutils_jpeg_error_mgr jerr;

    // Initialize the JPEG decompression object with default error handling
    jpeg_info.err                 = jpeg_std_error(&jerr);
    jpeg_info.err->error_exit     = jpegutils_jpeg_error_exit;
    jpeg_info.err->emit_message   = jpegutils_jpeg_emit_message;
    jpeg_info.err->output_message = jpegutils_jpeg_output_message;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&jpeg_info);
        fclose(inFile);
        return quality;
    }

    jpeg_create_decompress(&jpeg_info);
    jpeg_stdio_src(&jpeg_info, inFile);
    jpeg_read_header(&jpeg_info, true);
    jpeg_start_decompress(&jpeg_info);

    // https://subversion.imagemagick.org/subversion/ImageMagick/trunk/coders/jpeg.c
    // Determine the JPEG compression quality from the quantization tables.

    long value;
    long i, j;
    long sum = 0;

    for (i = 0; i < NUM_QUANT_TBLS; ++i)
    {
        if (jpeg_info.quant_tbl_ptrs[i] != NULL)
        {
            for (j = 0; j < DCTSIZE2; ++j)
            {
                sum += jpeg_info.quant_tbl_ptrs[i]->quantval[j];
            }
        }
     }

    if ((jpeg_info.quant_tbl_ptrs[0] != NULL) &&
        (jpeg_info.quant_tbl_ptrs[1] != NULL))
    {
        long hash[101] =
             {
                 1020, 1015,  932,  848,  780,  735,  702,  679,  660,  645,
                  632,  623,  613,  607,  600,  594,  589,  585,  581,  571,
                  555,  542,  529,  514,  494,  474,  457,  439,  424,  410,
                  397,  386,  373,  364,  351,  341,  334,  324,  317,  309,
                  299,  294,  287,  279,  274,  267,  262,  257,  251,  247,
                  243,  237,  232,  227,  222,  217,  213,  207,  202,  198,
                  192,  188,  183,  177,  173,  168,  163,  157,  153,  148,
                  143,  139,  132,  128,  125,  119,  115,  108,  104,   99,
                   94,   90,   84,   79,   74,   70,   64,   59,   55,   49,
                   45,   40,   34,   30,   25,   20,   15,   11,    6,    4,
                    0
             },
             sums[101] =
             {
                 32640, 32635, 32266, 31495, 30665, 29804, 29146, 28599, 28104,
                 27670, 27225, 26725, 26210, 25716, 25240, 24789, 24373, 23946,
                 23572, 22846, 21801, 20842, 19949, 19121, 18386, 17651, 16998,
                 16349, 15800, 15247, 14783, 14321, 13859, 13535, 13081, 12702,
                 12423, 12056, 11779, 11513, 11135, 10955, 10676, 10392, 10208,
                  9928,  9747,  9564,  9369,  9193,  9017,  8822,  8639,  8458,
                  8270,  8084,  7896,  7710,  7527,  7347,  7156,  6977,  6788,
                  6607,  6422,  6236,  6054,  5867,  5684,  5495,  5305,  5128,
                  4945,  4751,  4638,  4442,  4248,  4065,  3888,  3698,  3509,
                  3326,  3139,  2957,  2775,  2586,  2405,  2216,  2037,  1846,
                  1666,  1483,  1297,  1109,   927,   735,   554,   375,   201,
                   128,     0
             };

        value = (long)(jpeg_info.quant_tbl_ptrs[0]->quantval[2]+
                       jpeg_info.quant_tbl_ptrs[0]->quantval[53]+
                       jpeg_info.quant_tbl_ptrs[1]->quantval[0]+
                       jpeg_info.quant_tbl_ptrs[1]->quantval[DCTSIZE2 - 1]);

        for (i = 0; i < 100; ++i)
        {
            if ((value < hash[i]) && (sum < sums[i]))
            {
                continue;
            }

            if (((value <= hash[i]) && (sum <= sums[i])) || (i >= 50))
            {
                quality = i + 1;
            }

            break;
        }
    }
    else if (jpeg_info.quant_tbl_ptrs[0] != NULL)
    {
        long hash[101] =
             {
                 510,  505,  422,  380,  355,  338,  326,  318,  311,  305,
                 300,  297,  293,  291,  288,  286,  284,  283,  281,  280,
                 279,  278,  277,  273,  262,  251,  243,  233,  225,  218,
                 211,  205,  198,  193,  186,  181,  177,  172,  168,  164,
                 158,  156,  152,  148,  145,  142,  139,  136,  133,  131,
                 129,  126,  123,  120,  118,  115,  113,  110,  107,  105,
                 102,  100,   97,   94,   92,   89,   87,   83,   81,   79,
                  76,   74,   70,   68,   66,   63,   61,   57,   55,   52,
                  50,   48,   44,   42,   39,   37,   34,   31,   29,   26,
                  24,   21,   18,   16,   13,   11,    8,    6,    3,    2,
                   0
             },
             sums[101] =
             {
                 16320, 16315, 15946, 15277, 14655, 14073, 13623, 13230, 12859,
                 12560, 12240, 11861, 11456, 11081, 10714, 10360, 10027,  9679,
                  9368,  9056,  8680,  8331,  7995,  7668,  7376,  7084,  6823,
                  6562,  6345,  6125,  5939,  5756,  5571,  5421,  5240,  5086,
                  4976,  4829,  4719,  4616,  4463,  4393,  4280,  4166,  4092,
                  3980,  3909,  3835,  3755,  3688,  3621,  3541,  3467,  3396,
                  3323,  3247,  3170,  3096,  3021,  2952,  2874,  2804,  2727,
                  2657,  2583,  2509,  2437,  2362,  2290,  2211,  2136,  2068,
                  1996,  1915,  1858,  1773,  1692,  1620,  1552,  1477,  1398,
                  1326,  1251,  1179,  1109,  1031,   961,   884,   814,   736,
                   667,   592,   518,   441,   369,   292,   221,   151,    86,
                    64,     0
             };

        value = (long)(jpeg_info.quant_tbl_ptrs[0]->quantval[2]+
                       jpeg_info.quant_tbl_ptrs[0]->quantval[53]);

        for (i = 0; i < 100; ++i)
        {
            if ((value < hash[i]) && (sum < sums[i]))
            {
                continue;
            }

            if (((value <= hash[i]) && (sum <= sums[i])) || (i >= 50))
            {
                quality = i + 1;
            }

            break;
        }
    }

    jpeg_destroy_decompress(&jpeg_info);
    fclose(inFile);

    qCDebug(DIGIKAM_GENERAL_LOG) << "JPEG Quality: " << quality << " File: " << file;
    return quality;
}

} // namespace JPEGUtils

} // namespace Digikam
