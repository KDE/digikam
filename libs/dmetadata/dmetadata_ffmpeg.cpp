/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-02-26
 * Description : metadata extraction with ffmpeg (libav)
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmetadata.h"

// C Ansi includes

#include <stdint.h>

// Qt includes

#include <QDateTime>

// Local incudes

#include "digikam_debug.h"
#include "digikam_config.h"

#ifdef HAVE_MEDIAPLAYER

// Libav includes

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>
}

#endif

namespace Digikam
{

bool DMetadata::loadUsingFFmpeg(const QString& filePath)
{
#ifdef HAVE_MEDIAPLAYER

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Parse metadada with FFMpeg:" << filePath;

    av_register_all();

    AVFormatContext* fmt_ctx = avformat_alloc_context();
    int ret                  = avformat_open_input(&fmt_ctx, filePath.toUtf8().data(), NULL, NULL);

    if (ret < 0)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "avformat_open_input error: " << ret;
        return false;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);

    if (ret < 0)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "avform_find_stream_info error: " << ret;
        return false;
    }

    int totalSecs = fmt_ctx->duration / AV_TIME_BASE;
    int bitrate   = fmt_ctx->bit_rate;

    setXmpTagString("Xmp.video.Duration", QString::number(totalSecs), false);
    setXmpTagString("Xmp.video.MaxBitRate", QString::number(bitrate), false);

    for (uint i = 0 ; i < fmt_ctx->nb_streams ; i++)
    {
        const AVStream* const stream   = fmt_ctx->streams[i];
        AVCodecParameters* const codec = stream->codecpar;

        if (codec->codec_type == AVMEDIA_TYPE_AUDIO || codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
/*
            if (codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                subRes.addType( NFO::Audio() );
                subRes.addProperty( NFO::sampleRate(), codec->sample_rate );
                subRes.addProperty( NFO::channels(), codec->channels );
            }
*/

            if (codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                int aspectRatio = codec->sample_aspect_ratio.num;
                int frameRate   = stream->avg_frame_rate.num;

                if (codec->sample_aspect_ratio.den)
                    aspectRatio /= codec->sample_aspect_ratio.den;

                if (stream->avg_frame_rate.den)
                    frameRate /= stream->avg_frame_rate.den;

                setXmpTagString("Xmp.video.SourceImageWidth",  QString::number(codec->width),  false);
                setXmpTagString("Xmp.video.SourceImageHeight", QString::number(codec->height), false);

                // Backport size in Exif and Iptc
                setImageDimensions(QSize(codec->width, codec->height), false);

                if (aspectRatio)
                    setXmpTagString("Xmp.video.AspectRatio", QString::number(aspectRatio), false);

                if (frameRate)
                    setXmpTagString("Xmp.video.FrameRate", QString::number(frameRate), false);

                setXmpTagString("Xmp.video.PixelDepth",  QString::number(codec->bits_per_coded_sample), false);

                //TODO: codec->format      => "Xmp.video.Format"     (the pixel format, the value corresponds to enum AVPixelFormat).
                //TODO: codec->color_space => "Xmp.video.ColorSpace" (the YUV colorspace type, the value corresponds to enum AVColorSpace).
            }
        }
    }

    AVDictionary* const dict = fmt_ctx->metadata;
    AVDictionaryEntry* entry = av_dict_get(dict, "title", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Title", QString::fromUtf8(entry->value), false);
    }

    entry = av_dict_get(dict, "author", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Artist", QString::fromUtf8(entry->value), false);
    }

    entry = av_dict_get(dict, "copyright", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Copyright", QString::fromUtf8(entry->value), false);
    }

    entry = av_dict_get(dict, "comment", NULL, 0);

    if (entry)
    {
        QString data = QString::fromUtf8(entry->value);
        setXmpTagString("Xmp.video.Comment", data, false);
        // Backport comment in Exif
        setExifComment(data, false);
    }

    entry = av_dict_get(dict, "album", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Album", QString::fromUtf8(entry->value), false);
    }

    entry = av_dict_get(dict, "genre", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Genre", QString::fromUtf8(entry->value), false);
    }

    entry = av_dict_get(dict, "track", NULL, 0);

    if (entry)
    {
        QString value = QString::fromUtf8(entry->value);

        bool ok   = false;
        int track = value.toInt(&ok);

        if (ok && track)
            setXmpTagString("Xmp.video.TrackNumber", QString::number(track), false);
    }

    entry = av_dict_get(dict, "year", NULL, 0);

    if (entry)
    {
        int year = QString::fromUtf8(entry->value).toInt();
        setXmpTagString("Xmp.video.Year", QString::number(year), false);
    }

    entry = av_dict_get(dict, "creation_time", NULL, 0);

    if (entry)
    {
        QString data = QString::fromUtf8(entry->value);
        setXmpTagString("Xmp.video.CreationDate", data, false);
        // Backport date in Exif and Iptc.
        QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
        setImageDateTime(dt, true, false);
    }

    // GPS info as string. ex: "+44.8511-000.6229/"
    entry = av_dict_get(dict, "location", NULL, 0);

    if (entry)
    {
        QString data     = QString::fromUtf8(entry->value);
        setXmpTagString("Xmp.video.LocationInfo", data, false);

        // Backport location in Exif.
        data.remove(QLatin1Char('/'));
        QLatin1Char sep  = QLatin1Char('+');

        int index        = data.indexOf(sep, 1);

        if (index == -1)
        {
            sep   = QLatin1Char('-');
            index = data.indexOf(sep, 1);
        }

        QString lng      = data.right(data.length() - index);
        QString lat      = data.left(index);

        qCDebug(DIGIKAM_METAENGINE_LOG) << lat << lng;

        bool b1          = false;
        bool b2          = false;
        double lattitude = lat.toDouble(&b1);
        double longitude = lng.toDouble(&b2);
        double* alt      = 0;

        if (b1 && b2)
        {
            setGPSInfo(alt, lattitude, longitude, false);
        }
    }

    avformat_close_input(&fmt_ctx);

    return true;

#else
    Q_UNUSED(filePath);
    return false;
#endif
}

} // namespace Digikam
