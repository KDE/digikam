/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : Qt Multimedia based video thumbnailer 
 *
 * Copyright (C) 2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "videothumbnailer_p.h"

// Qt includes

#include <QDebug>

namespace Digikam
{

/**
 * Private helper conversion methods from QVideoFrame to QImage taken from Qt 5.7
 * Copyright (C) 2016 The Qt Company Ltd.
 * For future update, see file qvideoframeconversionhelper.cpp from
 * https://github.com/qtproject/qtmultimedia/tree/dev/src/multimedia/video
 */

#define FETCH_INFO_PACKED(frame) \
    const uchar *src = frame.bits(); \
    int stride = frame.bytesPerLine(); \
    int width = frame.width(); \
    int height = frame.height();

#define FETCH_INFO_BIPLANAR(frame) \
    const uchar *plane1 = frame.bits(0); \
    const uchar *plane2 = frame.bits(1); \
    int plane1Stride = frame.bytesPerLine(0); \
    int plane2Stride = frame.bytesPerLine(1); \
    int width = frame.width(); \
    int height = frame.height();

#define FETCH_INFO_TRIPLANAR(frame) \
    const uchar *plane1 = frame.bits(0); \
    const uchar *plane2 = frame.bits(1); \
    const uchar *plane3 = frame.bits(2); \
    int plane1Stride = frame.bytesPerLine(0); \
    int plane2Stride = frame.bytesPerLine(1); \
    int plane3Stride = frame.bytesPerLine(2); \
    int width = frame.width(); \
    int height = frame.height(); \

#define MERGE_LOOPS(width, height, stride, bpp) \
    if (stride == width * bpp) { \
        width *= height; \
        height = 1; \
        stride = 0; \
    }

#define ALIGN(boundary, ptr, x, length) \
    for (; ((reinterpret_cast<qintptr>(ptr) & (boundary - 1)) != 0) && x < length; ++x)

#define CLAMP(n) (n > 255 ? 255 : (n < 0 ? 0 : n))

#define EXPAND_UV(u, v) \
    int uu = u - 128; \
    int vv = v - 128; \
    int rv = 409 * vv + 128; \
    int guv = 100 * uu + 208 * vv + 128; \
    int bu = 516 * uu + 128; \

inline quint32 qConvertBGRA32ToARGB32(quint32 bgra)
{
    return (((bgra & 0xFF000000) >> 24)
            | ((bgra & 0x00FF0000) >> 8)
            | ((bgra & 0x0000FF00) << 8)
            | ((bgra & 0x000000FF) << 24));
}

inline quint32 qConvertBGR24ToARGB32(const uchar *bgr)
{
    return 0xFF000000 | bgr[0] | bgr[1] << 8 | bgr[2] << 16;
}

inline quint32 qConvertBGR565ToARGB32(quint16 bgr)
{
    return 0xff000000
            | ((((bgr) >> 8) & 0xf8) | (((bgr) >> 13) & 0x7))
            | ((((bgr) << 5) & 0xfc00) | (((bgr) >> 1) & 0x300))
            | ((((bgr) << 19) & 0xf80000) | (((bgr) << 14) & 0x70000));
}

inline quint32 qConvertBGR555ToARGB32(quint16 bgr)
{
    return 0xff000000
            | ((((bgr) >> 7) & 0xf8) | (((bgr) >> 12) & 0x7))
            | ((((bgr) << 6) & 0xf800) | (((bgr) << 1) & 0x700))
            | ((((bgr) << 19) & 0xf80000) | (((bgr) << 11) & 0x70000));
}

inline quint32 qYUVToARGB32(int y, int rv, int guv, int bu, int a = 0xff)
{
    int yy = (y - 16) * 298;
    return (a << 24)
            | CLAMP((yy + rv) >> 8) << 16
            | CLAMP((yy - guv) >> 8) << 8
            | CLAMP((yy + bu) >> 8);
}

inline void planarYUV420_to_ARGB32(const uchar *y, int yStride,
                                   const uchar *u, int uStride,
                                   const uchar *v, int vStride,
                                   int uvPixelStride,
                                   quint32 *rgb,
                                   int width, int height)
{
    quint32 *rgb0 = rgb;
    quint32 *rgb1 = rgb + width;

    for (int j = 0; j < height; j += 2) {
        const uchar *lineY0 = y;
        const uchar *lineY1 = y + yStride;
        const uchar *lineU = u;
        const uchar *lineV = v;

        for (int i = 0; i < width; i += 2) {
            EXPAND_UV(*lineU, *lineV);
            lineU += uvPixelStride;
            lineV += uvPixelStride;

            *rgb0++ = qYUVToARGB32(*lineY0++, rv, guv, bu);
            *rgb0++ = qYUVToARGB32(*lineY0++, rv, guv, bu);
            *rgb1++ = qYUVToARGB32(*lineY1++, rv, guv, bu);
            *rgb1++ = qYUVToARGB32(*lineY1++, rv, guv, bu);
        }

        y += yStride << 1; // stride * 2
        u += uStride;
        v += vStride;
        rgb0 += width;
        rgb1 += width;
    }
}

void QT_FASTCALL qt_convert_YUV420P_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_TRIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane2, plane2Stride,
                           plane3, plane3Stride,
                           1,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}

void QT_FASTCALL qt_convert_YV12_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_TRIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane3, plane3Stride,
                           plane2, plane2Stride,
                           1,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}

void QT_FASTCALL qt_convert_AYUV444_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 4)

    quint32 *rgb = reinterpret_cast<quint32*>(output);

    for (int i = 0; i < height; ++i) {
        const uchar *lineSrc = src;

        for (int j = 0; j < width; ++j) {
            int a = *lineSrc++;
            int y = *lineSrc++;
            int u = *lineSrc++;
            int v = *lineSrc++;

            EXPAND_UV(u, v);

            *rgb++ = qYUVToARGB32(y, rv, guv, bu, a);
        }

        src += stride;
    }
}

void QT_FASTCALL qt_convert_YUV444_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 3)

    quint32 *rgb = reinterpret_cast<quint32*>(output);

    for (int i = 0; i < height; ++i) {
        const uchar *lineSrc = src;

        for (int j = 0; j < width; ++j) {
            int y = *lineSrc++;
            int u = *lineSrc++;
            int v = *lineSrc++;

            EXPAND_UV(u, v);

            *rgb++ = qYUVToARGB32(y, rv, guv, bu);
        }

        src += stride;
    }
}

void QT_FASTCALL qt_convert_UYVY_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 2)

    quint32 *rgb = reinterpret_cast<quint32*>(output);

    for (int i = 0; i < height; ++i) {
        const uchar *lineSrc = src;

        for (int j = 0; j < width; j += 2) {
            int u = *lineSrc++;
            int y0 = *lineSrc++;
            int v = *lineSrc++;
            int y1 = *lineSrc++;

            EXPAND_UV(u, v);

            *rgb++ = qYUVToARGB32(y0, rv, guv, bu);
            *rgb++ = qYUVToARGB32(y1, rv, guv, bu);
        }

        src += stride;
    }
}

void QT_FASTCALL qt_convert_YUYV_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 2)

    quint32 *rgb = reinterpret_cast<quint32*>(output);

    for (int i = 0; i < height; ++i) {
        const uchar *lineSrc = src;

        for (int j = 0; j < width; j += 2) {
            int y0 = *lineSrc++;
            int u = *lineSrc++;
            int y1 = *lineSrc++;
            int v = *lineSrc++;

            EXPAND_UV(u, v);

            *rgb++ = qYUVToARGB32(y0, rv, guv, bu);
            *rgb++ = qYUVToARGB32(y1, rv, guv, bu);
        }

        src += stride;
    }
}

void QT_FASTCALL qt_convert_NV12_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_BIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane2, plane2Stride,
                           plane2 + 1, plane2Stride,
                           2,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}

void QT_FASTCALL qt_convert_NV21_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_BIPLANAR(frame)
    planarYUV420_to_ARGB32(plane1, plane1Stride,
                           plane2 + 1, plane2Stride,
                           plane2, plane2Stride,
                           2,
                           reinterpret_cast<quint32*>(output),
                           width, height);
}

void QT_FASTCALL qt_convert_BGRA32_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 4)

    quint32 *argb = reinterpret_cast<quint32*>(output);

    for (int y = 0; y < height; ++y) {
        const quint32 *bgra = reinterpret_cast<const quint32*>(src);

        int x = 0;
        for (; x < width - 3; x += 4) {
            *argb++ = qConvertBGRA32ToARGB32(*bgra++);
            *argb++ = qConvertBGRA32ToARGB32(*bgra++);
            *argb++ = qConvertBGRA32ToARGB32(*bgra++);
            *argb++ = qConvertBGRA32ToARGB32(*bgra++);
        }

        // leftovers
        for (; x < width; ++x)
            *argb++ = qConvertBGRA32ToARGB32(*bgra++);

        src += stride;
    }
}

void QT_FASTCALL qt_convert_BGR24_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 3)

    quint32 *argb = reinterpret_cast<quint32*>(output);

    for (int y = 0; y < height; ++y) {
        const uchar *bgr = src;

        int x = 0;
        for (; x < width - 3; x += 4) {
            *argb++ = qConvertBGR24ToARGB32(bgr);
            bgr += 3;
            *argb++ = qConvertBGR24ToARGB32(bgr);
            bgr += 3;
            *argb++ = qConvertBGR24ToARGB32(bgr);
            bgr += 3;
            *argb++ = qConvertBGR24ToARGB32(bgr);
            bgr += 3;
        }

        // leftovers
        for (; x < width; ++x) {
            *argb++ = qConvertBGR24ToARGB32(bgr);
            bgr += 3;
        }

        src += stride;
    }
}

void QT_FASTCALL qt_convert_BGR565_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 2)

    quint32 *argb = reinterpret_cast<quint32*>(output);

    for (int y = 0; y < height; ++y) {
        const quint16 *bgr = reinterpret_cast<const quint16*>(src);

        int x = 0;
        for (; x < width - 3; x += 4) {
            *argb++ = qConvertBGR565ToARGB32(*bgr++);
            *argb++ = qConvertBGR565ToARGB32(*bgr++);
            *argb++ = qConvertBGR565ToARGB32(*bgr++);
            *argb++ = qConvertBGR565ToARGB32(*bgr++);
        }

        // leftovers
        for (; x < width; ++x)
            *argb++ = qConvertBGR565ToARGB32(*bgr++);

        src += stride;
    }
}

void QT_FASTCALL qt_convert_BGR555_to_ARGB32(const QVideoFrame& frame, uchar* output)
{
    FETCH_INFO_PACKED(frame)
    MERGE_LOOPS(width, height, stride, 2)

    quint32 *argb = reinterpret_cast<quint32*>(output);

    for (int y = 0; y < height; ++y) {
        const quint16 *bgr = reinterpret_cast<const quint16*>(src);

        int x = 0;
        for (; x < width - 3; x += 4) {
            *argb++ = qConvertBGR555ToARGB32(*bgr++);
            *argb++ = qConvertBGR555ToARGB32(*bgr++);
            *argb++ = qConvertBGR555ToARGB32(*bgr++);
            *argb++ = qConvertBGR555ToARGB32(*bgr++);
        }

        // leftovers
        for (; x < width; ++x)
            *argb++ = qConvertBGR555ToARGB32(*bgr++);

        src += stride;
    }
}

typedef void (QT_FASTCALL *VideoFrameConvertFunc)(const QVideoFrame& frame, uchar* output);

void QT_FASTCALL qt_convert_BGRA32_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_BGR24_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_BGR565_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_BGR555_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_AYUV444_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_YUV444_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_YUV420P_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_YV12_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_UYVY_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_YUYV_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_NV12_to_ARGB32(const QVideoFrame&, uchar*);
void QT_FASTCALL qt_convert_NV21_to_ARGB32(const QVideoFrame&, uchar*);

static VideoFrameConvertFunc qConvertFuncs[QVideoFrame::Format_AdobeDng + 1] = {
    /* Format_Invalid */                Q_NULLPTR, // Not needed
    /* Format_ARGB32 */                 Q_NULLPTR, // Not needed
    /* Format_ARGB32_Premultiplied */   Q_NULLPTR, // Not needed
    /* Format_RGB32 */                  Q_NULLPTR, // Not needed
    /* Format_RGB24 */                  Q_NULLPTR, // Not needed
    /* Format_RGB565 */                 Q_NULLPTR, // Not needed
    /* Format_RGB555 */                 Q_NULLPTR, // Not needed
    /* Format_ARGB8565_Premultiplied */ Q_NULLPTR, // Not needed
    /* Format_BGRA32 */                 qt_convert_BGRA32_to_ARGB32,
    /* Format_BGRA32_Premultiplied */   qt_convert_BGRA32_to_ARGB32,
    /* Format_BGR32 */                  qt_convert_BGRA32_to_ARGB32,
    /* Format_BGR24 */                  qt_convert_BGR24_to_ARGB32,
    /* Format_BGR565 */                 qt_convert_BGR565_to_ARGB32,
    /* Format_BGR555 */                 qt_convert_BGR555_to_ARGB32,
    /* Format_BGRA5658_Premultiplied */ Q_NULLPTR, // TODO
    /* Format_AYUV444 */                qt_convert_AYUV444_to_ARGB32,
    /* Format_AYUV444_Premultiplied */  Q_NULLPTR, // TODO
    /* Format_YUV444 */                 qt_convert_YUV444_to_ARGB32,
    /* Format_YUV420P */                qt_convert_YUV420P_to_ARGB32,
    /* Format_YV12 */                   qt_convert_YV12_to_ARGB32,
    /* Format_UYVY */                   qt_convert_UYVY_to_ARGB32,
    /* Format_YUYV */                   qt_convert_YUYV_to_ARGB32,
    /* Format_NV12 */                   qt_convert_NV12_to_ARGB32,
    /* Format_NV21 */                   qt_convert_NV21_to_ARGB32,
    /* Format_IMC1 */                   Q_NULLPTR, // TODO
    /* Format_IMC2 */                   Q_NULLPTR, // TODO
    /* Format_IMC3 */                   Q_NULLPTR, // TODO
    /* Format_IMC4 */                   Q_NULLPTR, // TODO
    /* Format_Y8 */                     Q_NULLPTR, // TODO
    /* Format_Y16 */                    Q_NULLPTR, // TODO
    /* Format_Jpeg */                   Q_NULLPTR, // Not needed
    /* Format_CameraRaw */              Q_NULLPTR, // TODO
    /* Format_AdobeDng */               Q_NULLPTR  // TODO
};

// -------------------------------------------------------------

static uchar sprocket_large_png[] =
{
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x0f,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x0b, 0x5a, 0x84, 0x6b, 0x00, 0x00, 0x00,
    0x06, 0x62, 0x4b, 0x47, 0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xa0,
    0xbd, 0xa7, 0x93, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00,
    0x00, 0x0b, 0x0e, 0x00, 0x00, 0x0b, 0x0e, 0x01, 0x40, 0xbe, 0xe1, 0x41,
    0x00, 0x00, 0x00, 0x07, 0x74, 0x49, 0x4d, 0x45, 0x07, 0xd6, 0x06, 0x1d,
    0x08, 0x25, 0x03, 0x5a, 0x69, 0xff, 0x95, 0x00, 0x00, 0x02, 0x4a, 0x49,
    0x44, 0x41, 0x54, 0x78, 0xda, 0x45, 0x93, 0x4d, 0x2b, 0xf5, 0x41, 0x18,
    0xc6, 0x7f, 0xc7, 0xb9, 0xbd, 0xbf, 0x53, 0x38, 0x84, 0x28, 0x59, 0x49,
    0x3d, 0x3b, 0x65, 0xc7, 0xea, 0x2c, 0x64, 0x65, 0x23, 0xf9, 0x3a, 0x16,
    0xbe, 0x80, 0x94, 0x4f, 0x20, 0x14, 0x59, 0xe8, 0xf9, 0x06, 0x3c, 0x1b,
    0x2c, 0xc8, 0x42, 0x88, 0xbc, 0xe6, 0xf5, 0x78, 0xe7, 0x3c, 0xae, 0xae,
    0x26, 0x53, 0xd3, 0xcc, 0xfc, 0xff, 0x33, 0xbf, 0xfb, 0xbe, 0xae, 0xb9,
    0x27, 0xc3, 0xc0, 0xc0, 0x3f, 0xf2, 0xf9, 0x3f, 0xb4, 0xb5, 0xc1, 0xc2,
    0x02, 0x64, 0x32, 0x90, 0xcb, 0xc1, 0xf5, 0x35, 0x94, 0x94, 0x40, 0x63,
    0x23, 0x3c, 0x3f, 0xc3, 0xe3, 0x23, 0x34, 0x34, 0x40, 0xb1, 0x08, 0xb7,
    0xb7, 0x50, 0x5e, 0x0e, 0xd9, 0x2c, 0xbc, 0xbc, 0xf8, 0xdb, 0xd8, 0x18,
    0xd4, 0xd7, 0xc3, 0xfa, 0x3a, 0x19, 0xfa, 0xfb, 0xff, 0xfe, 0x80, 0x46,
    0x48, 0x6d, 0x78, 0x18, 0x4a, 0x4b, 0xa1, 0xb9, 0x19, 0xca, 0xca, 0xe0,
    0xe9, 0x09, 0x6a, 0x6a, 0xa0, 0xae, 0x0e, 0x6e, 0x6e, 0xe0, 0xeb, 0x4b,
    0x6b, 0x07, 0x79, 0x78, 0x30, 0x78, 0x63, 0x03, 0x52, 0xeb, 0xe9, 0x21,
    0xa8, 0xad, 0x1d, 0x21, 0x82, 0x9f, 0x11, 0x0e, 0x0f, 0xbd, 0x39, 0x9f,
    0x87, 0xca, 0x4a, 0xc1, 0x3d, 0x7a, 0xee, 0x20, 0x1f, 0x1f, 0xf0, 0xf9,
    0x09, 0xef, 0xef, 0xf0, 0xf6, 0x06, 0xab, 0xab, 0x5e, 0xb7, 0xb6, 0x42,
    0x45, 0x05, 0x54, 0x57, 0x13, 0x34, 0x35, 0x41, 0x77, 0x37, 0x74, 0x74,
    0xc0, 0xfe, 0xbe, 0x01, 0x55, 0x55, 0x3e, 0xf4, 0x7b, 0x58, 0xdf, 0x7d,
    0x28, 0xc2, 0x70, 0x07, 0x91, 0x25, 0xde, 0x33, 0x38, 0x68, 0x35, 0x9b,
    0x9b, 0x04, 0xca, 0x6c, 0x6d, 0x0d, 0x40, 0x1e, 0x6a, 0x93, 0x7e, 0xca,
    0x2b, 0xc1, 0x0c, 0x2e, 0x14, 0x34, 0x37, 0xd4, 0xdd, 0xc0, 0x08, 0xed,
    0xb5, 0x25, 0xcb, 0xcb, 0x66, 0xb4, 0xb4, 0x10, 0xf4, 0xf5, 0xd9, 0x60,
    0x65, 0x2c, 0x6f, 0xe4, 0x93, 0xc0, 0x3a, 0xf4, 0xfa, 0x2a, 0xb8, 0xc0,
    0xee, 0x77, 0x77, 0x52, 0xa1, 0x00, 0xba, 0x28, 0xc3, 0xcf, 0xcf, 0xbd,
    0x1e, 0x1a, 0x42, 0x6a, 0x74, 0x89, 0x81, 0x3e, 0x44, 0x68, 0xb3, 0x0e,
    0x2a, 0x5b, 0x07, 0x31, 0x54, 0x07, 0x75, 0xfb, 0x92, 0xe8, 0x00, 0x56,
    0xa0, 0xff, 0x1e, 0x93, 0xc7, 0x89, 0x71, 0x79, 0x49, 0x70, 0x74, 0x04,
    0x3b, 0x3b, 0xe0, 0x66, 0xc3, 0x2d, 0xcd, 0xa3, 0xa5, 0x1a, 0x16, 0x91,
    0x40, 0x29, 0x90, 0xbc, 0xb5, 0x7c, 0x57, 0x00, 0xba, 0xf0, 0x40, 0x52,
    0x47, 0x47, 0xa1, 0xab, 0x0b, 0xe6, 0xe6, 0x5c, 0x9f, 0xc5, 0xa2, 0x32,
    0xd6, 0x4d, 0xa6, 0x00, 0x92, 0x2b, 0x79, 0x82, 0xb9, 0x67, 0x32, 0x82,
    0xc9, 0x12, 0x07, 0x1a, 0x1f, 0xb7, 0xc2, 0xad, 0x2d, 0x02, 0x6d, 0xdc,
    0xdd, 0x85, 0xd3, 0x53, 0x65, 0xa0, 0xf4, 0x55, 0x56, 0x82, 0x18, 0x98,
    0xcd, 0x0a, 0xee, 0xb5, 0x65, 0x27, 0x05, 0xc9, 0x0e, 0x9f, 0xdb, 0xde,
    0x46, 0xf2, 0xb5, 0x2f, 0x38, 0x39, 0x81, 0x83, 0x03, 0x70, 0x53, 0x61,
    0xab, 0xc8, 0x3d, 0x2a, 0xa0, 0x60, 0xce, 0x54, 0xd9, 0x4b, 0xae, 0xe6,
    0xf2, 0x5b, 0x70, 0xd9, 0x65, 0x65, 0x7b, 0x7b, 0xa8, 0xa9, 0x3c, 0x83,
    0xef, 0x6f, 0x98, 0x98, 0xb0, 0xfc, 0xd9, 0x59, 0x3f, 0xc1, 0x8b, 0x0b,
    0x49, 0x54, 0x86, 0xee, 0x2e, 0xa3, 0xe4, 0x73, 0xaa, 0x53, 0xaf, 0x65,
    0x9f, 0xda, 0xd4, 0x94, 0xe7, 0x2b, 0x2b, 0x04, 0xed, 0xed, 0xb0, 0xb4,
    0xe4, 0x37, 0xae, 0x76, 0x75, 0xa5, 0x2e, 0x59, 0x02, 0xca, 0x78, 0xc9,
    0x52, 0xf7, 0x3a, 0xc2, 0x40, 0x2b, 0xb0, 0xa7, 0x6a, 0x8b, 0x8b, 0x56,
    0xd6, 0xdb, 0x4b, 0x70, 0x76, 0xa6, 0x92, 0x30, 0xc8, 0x0f, 0x40, 0xd9,
    0xca, 0x3f, 0x83, 0x0b, 0x05, 0x59, 0xa1, 0xae, 0xb5, 0xe1, 0xe9, 0xd9,
    0xa6, 0xe7, 0x0d, 0xbf, 0xfe, 0xe6, 0x72, 0x04, 0xc7, 0xc7, 0x30, 0x39,
    0x09, 0x9d, 0x9d, 0x30, 0x33, 0x03, 0xb2, 0xe3, 0xfe, 0x5e, 0x50, 0xf9,
    0x26, 0x80, 0x46, 0xd9, 0x61, 0xb0, 0x2d, 0x49, 0x97, 0xa2, 0x17, 0xe5,
    0x4a, 0x98, 0x9e, 0xb6, 0xda, 0xf9, 0x79, 0xfe, 0x03, 0xe1, 0xc7, 0xff,
    0x96, 0xed, 0xf6, 0x1b, 0x9f, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
    0x44, 0xae, 0x42, 0x60, 0x82
};
static int sprocket_large_png_len = 701;

VideoThumbnailer::Private::Private(VideoThumbnailer* const parent)
    : QThread(parent),
      createStrip(false),
      thumbSize(256),
      player(0),
      probe(0),
      media(0),
      position(0),
      dd(parent)
{
    player = new QMediaPlayer(this);
    probe  = new QVideoProbe(this);

    connect(player, SIGNAL(error(QMediaPlayer::Error)),
            this, SLOT(slotHandlePlayerError()));

    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QMediaPlayer::MediaStatus)));

    connect(probe, SIGNAL(videoFrameProbed(QVideoFrame)),
            this, SLOT(slotProcessframe(QVideoFrame)));

    strip = QImage::fromData(sprocket_large_png, sprocket_large_png_len, "PNG");
}

QString VideoThumbnailer::Private::fileName() const
{
    return media.canonicalUrl().fileName();
}

QImage VideoThumbnailer::Private::imageFromVideoFrame(const QVideoFrame& f) const
{
    QVideoFrame& frm = const_cast<QVideoFrame&>(f);
    QImage result;

    if (!frm.isValid() || !frm.map(QAbstractVideoBuffer::ReadOnly))
    {
        return result;
    }

    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(frm.pixelFormat());

    if (imageFormat != QImage::Format_Invalid)
    {
        // Formats supported by QImage don't need conversion

        result = QImage(frm.bits(), frm.width(), frm.height(), imageFormat).copy();
    }
    else if (frm.pixelFormat() == QVideoFrame::Format_Jpeg)
    {
        // Load from JPG

        result.loadFromData(frm.bits(), frm.mappedBytes(), "JPG");
    }
    else
    {
        // Need conversion

        VideoFrameConvertFunc convert = qConvertFuncs[frm.pixelFormat()];

        if (!convert)
        {
            qDebug() << "Unsupported frame pixel format" << frm.pixelFormat();
        }
        else
        {
            result = QImage(frm.width(), frm.height(), QImage::Format_ARGB32);
            convert(frm, result.bits());
        }
    }

    frm.unmap();

    return result;
}

void VideoThumbnailer::Private::slotMediaStatusChanged(QMediaPlayer::MediaStatus state)
{
    if (player->currentMedia() != media)
    {
        return;
    }

    switch (state)
    {
        case QMediaPlayer::LoadedMedia:
        {
            if (!player->isSeekable())
            {
                qDebug() << "Video seek is not available for " << fileName();
                dd->emit signalThumbnailFailed(fileName());
            }

            if (player->duration() <= 0)
            {
                qDebug() << "Video has no valid duration for " << fileName();
                dd->emit signalThumbnailFailed(fileName());
            }

            qDebug() << "Video duration for " << fileName() << "is " << player->duration() << " seconds";

            position = (qint64)(player->duration() * 0.2);

            player->setPosition(position);    // Seek to 20% of the media to take a thumb.
            player->pause();

            qDebug() << "Trying to get thumbnail from " << fileName() << " at position " << position;
            break;
        }
        case QMediaPlayer::InvalidMedia:
        {
            qDebug() << "Video cannot be decoded for " << fileName();
            dd->emit signalThumbnailFailed(fileName());
        }
        default:
            break;
    }
}

void VideoThumbnailer::Private::slotHandlePlayerError()
{
    qDebug() << "Problem while video data extraction from " << fileName();
    qDebug() << "Error : " << player->errorString();

    dd->emit signalThumbnailFailed(fileName());
}

void VideoThumbnailer::Private::slotProcessframe(QVideoFrame frm)
{
    if (player->mediaStatus() != QMediaPlayer::BufferedMedia ||
        player->position()    != position)
    {
        return;
    }

    qDebug() << "Video frame extraction from " << fileName()
             << " at position " << position;

    if (!frm.isValid())
    { 
        qDebug() << "Error : Video frame is not valid.";
        dd->emit signalThumbnailFailed(fileName());
    }

    frame = frm;

    start();
}

void VideoThumbnailer::Private::run()
{
    QImage img = imageFromVideoFrame(frame);

    if (!img.isNull())
    {
        img = img.scaled(thumbSize, thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        if (createStrip && img.width() > strip.width() && img.height() > strip.height())
        {
            // Add a video strip on the left side of video thumb.

            for (int y = 0; y < img.height(); y += strip.height())
            {
                for (int ys = 0 ; ys < strip.height() ; ys++)
                {
                    int pos = y + ys;

                    if (pos < img.height())
                    {
                        memcpy((void*)img.constScanLine(pos), (void*)strip.constScanLine(ys), strip.bytesPerLine());
                    }
                }
            }
        }

        qDebug() << "Video frame extracted with size " << img.size();

        dd->emit signalThumbnailDone(fileName(), img.copy());
    }
    else
    {
        qDebug() << "Video frame format is not supported: " << frame;

        dd->emit signalThumbnailFailed(fileName());
    }
}

}  // namespace Digikam
