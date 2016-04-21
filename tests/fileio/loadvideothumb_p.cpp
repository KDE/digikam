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

#include "loadvideothumb_p.h"

// Qt includes

#include <QStandardPaths>
#include <QDebug>

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

VideoThumbnailer::Private::Private(VideoThumbnailer* const parent)
    : QObject(parent),
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

    strip = QImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/sprocket-large.png")));
}

QString VideoThumbnailer::Private::fileName() const
{
    return media.canonicalUrl().fileName();
}

QImage VideoThumbnailer::Private::imageFromVideoFrame(const QVideoFrame& f) const
{
    QVideoFrame& frame = const_cast<QVideoFrame&>(f);
    QImage result;

    if (!frame.isValid() || !frame.map(QAbstractVideoBuffer::ReadOnly))
    {
        return result;
    }

    // Formats supported by QImage don't need conversion
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());

    if (imageFormat != QImage::Format_Invalid)
    {
        result = QImage(frame.bits(), frame.width(), frame.height(), imageFormat).copy();
    }
    else if (frame.pixelFormat() == QVideoFrame::Format_Jpeg)
    {
        // Load from JPG

        result.loadFromData(frame.bits(), frame.mappedBytes(), "JPG");
    }
    else
    {
        // Need conversion

        VideoFrameConvertFunc convert = qConvertFuncs[frame.pixelFormat()];

        if (!convert)
        {
            qDebug() << "Unsupported frame pixel format" << frame.pixelFormat();
        }
        else
        {
            result = QImage(frame.width(), frame.height(), QImage::Format_ARGB32);
            convert(frame, result.bits());
        }
    }

    frame.unmap();

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
                dd->emit signalVideoThumbDone();
            }

            if (player->duration() <= 0)
            {
                qDebug() << "Video has no valid duration for " << fileName();
                dd->emit signalVideoThumbDone();
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
            dd->emit signalVideoThumbDone();
        }
        default:
            break;
    }
}

void VideoThumbnailer::Private::slotHandlePlayerError()
{
    qDebug() << "Problem while video data extraction from " << fileName();
    qDebug() << "Error : " << player->errorString();

    dd->emit signalVideoThumbDone();
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
        dd->emit signalVideoThumbDone();
    }

    QImage img = imageFromVideoFrame(frm);

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

        img.save(QString::fromUtf8("%1-thumb.png").arg(fileName()), "PNG");

        qDebug() << "Video frame extracted with size " << img.size();
    }
    else
    {
        qDebug() << "Video frame format is not supported: " << frm;
        qDebug() << "Video frame is not extracted.";
    }

    dd->emit signalVideoThumbDone();
}
