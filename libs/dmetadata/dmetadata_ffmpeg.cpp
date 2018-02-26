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

// Libav includes

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavcodec/avcodec.h>
}

// Qt includes

#include <QDateTime>

// Local incudes

#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::loadUsingFFmpeg(const QString& filePath) const
{
    av_register_all();

    AVFormatContext* const fmt_ctx = avformat_alloc_context();
    int ret                        = avformat_open_input(&fmt_ctx, filePath..utf8().data(), NULL, NULL)

    if (ret < 0)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "avformat_open_input error: " << ret;
        return;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);

    if (ret < 0)
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "avform_find_stream_info error: " << ret;
        return;
    }

    result->addType(Type::Video);

    int totalSecs = fmt_ctx->duration / AV_TIME_BASE;
    int bitrate   = fmt_ctx->bit_rate;

    setXmpTagString("Xmp.video.Duration", totalSecs, false);
    setXmpTagString("Xmp.video.MaxBitRate", bitrate, false);
    
    for (uint i = 0 ; i < fmt_ctx->nb_streams ; i++)
    {
        const AVStream* const stream      = fmt_ctx->streams[i];
        const AVCodecContext* const codec = stream->codec;

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

                setXmpTagString("Xmp.video.SourceImageWidth",  codec->width,  false);
                setXmpTagString("Xmp.video.SourceImageHeight", codec->height, false);

                if (aspectRatio)
                    setXmpTagString("Xmp.video.AspectRatio", aspectRatio, false);

                if (frameRate)
                    setXmpTagString("Xmp.video.FrameRate",   frameRate,   false);
            }
        }
    }

    AVDictionary* const dict       = fmt_ctx->metadata;
    AVDictionaryEntry* const entry = av_dict_get(dict, "title", NULL, 0);

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
        setXmpTagString("Xmp.video.Comment", QString::fromUtf8(entry->value), false);
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
            setXmpTagString("Xmp.video.TrackNumber", track, false);
    }

    entry = av_dict_get(dict, "year", NULL, 0);
    
    if (entry)
    {
        int year = QString::fromUtf8(entry->value).toInt();
        setXmpTagString("Xmp.video.Year", year, false);
    }

    entry = av_dict_get(dict, "creation_time", NULL, 0);
    
    if (entry)
    {
        QDateTime dt = QDateTime::fromString(entry->value, Qt::ISODate);
        setXmpTagString("Xmp.video.CreationDate", year, false);
    }

    // GPS info as string. ex: "+44.8511-000.6229/"
    entry = av_dict_get(dict, "location", NULL, 0);
    
    if (entry)
    {
        setXmpTagString("Xmp.video.LocationInfo", QString::fromUtf8(entry->value), false);
    }

    creation_time
    
    avformat_close_input(&fmt_ctx);
}

} // namespace Digikam
