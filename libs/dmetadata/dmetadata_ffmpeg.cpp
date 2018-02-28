/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-02-26
 * Description : metadata extraction with ffmpeg (libav)
 * References  :
 *
 * FFMpeg metadata review: https://wiki.multimedia.cx/index.php?title=FFmpeg_Metadata
 * FFMpeg MP4 parser     : https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/mov.c#L298
 * Exiv2 XMP video       : https://github.com/Exiv2/exiv2/blob/master/src/properties.cpp#L1331
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
#include <libavutil/pixdesc.h>
#include <libavcodec/avcodec.h>
}

#endif

namespace Digikam
{

qint64 s_secondsSinceJanuary1904(const QDateTime dt)
{
    QDateTime dt1904(QDate(1904, 1, 1), QTime(0, 0, 0));
    return dt1904.secsTo(dt);
}

#ifdef HAVE_MEDIAPLAYER

QStringList s_extractFFMpegMetadataEntriesFromDictionary(AVDictionary* const dict)
{
    AVDictionaryEntry* entry = 0;
    QStringList meta;

    do
    {
        entry = av_dict_get(dict, "", entry, AV_DICT_IGNORE_SUFFIX);

        if (entry)
            meta.append(QString::fromUtf8("%1 = %2").arg(QString::fromUtf8(entry->key))
                                                    .arg(QString::fromUtf8(entry->value)));
    }
    while (entry);

    return meta;
}

#endif

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

    int totalSecs            = fmt_ctx->duration / AV_TIME_BASE;
    int bitrate              = fmt_ctx->bit_rate;
    AVDictionaryEntry* entry = 0;
    AVDictionary* dict       = 0;

    setXmpTagString("Xmp.video.Duration",    QString::number(totalSecs), false);
    setXmpTagString("Xmp.video.MaxBitRate",  QString::number(bitrate), false);
    setXmpTagString("Xmp.video.StreamCount", QString::number(fmt_ctx->nb_streams), false);

    // To only register one video and one audio stream in XMP metadata.
    bool vstream = false;
    bool astream = false;

    for (uint i = 0 ; i < fmt_ctx->nb_streams ; i++)
    {
        const AVStream* const stream   = fmt_ctx->streams[i];
        AVCodecParameters* const codec = stream->codecpar;

        if (!astream && codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            astream           = true;
            const char* cname = avcodec_get_name(codec->codec_id);

            setXmpTagString("Xmp.audio.Codec", QString::fromUtf8(cname), false);
            setXmpTagString("Xmp.audio.CodecDescription", QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name), false);
            setXmpTagString("Xmp.audio.SampleRate", QString::number(codec->sample_rate), false);
            setXmpTagString("Xmp.audio.ChannelType", QString::number(codec->channels), false);
            setXmpTagString("Xmp.audio.Format",     QString::fromUtf8(av_get_sample_fmt_name((AVSampleFormat)codec->format)), false);

            // --------------

            dict  = stream->metadata;

            qCDebug(DIGIKAM_METAENGINE_LOG) << "FFMpeg audio stream metadata entries:";
            qCDebug(DIGIKAM_METAENGINE_LOG) << s_extractFFMpegMetadataEntriesFromDictionary(dict);

            // --------------

            entry = av_dict_get(dict, "language", NULL, 0);

            if (entry)
            {
                setXmpTagString("Xmp.audio.TrackLang", QString::fromUtf8(entry->value), false);
            }
            
            // --------------
            
            entry = av_dict_get(dict, "creation_time", NULL, 0);

            if (entry)
            {
                QDateTime dt = QDateTime::fromString(QString::fromUtf8(entry->value), Qt::ISODate);
                setXmpTagString("Xmp.audio.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)), false);
            }
        }

        if (!vstream && codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vstream           = true;
            const char* cname = avcodec_get_name(codec->codec_id); 

            setXmpTagString("Xmp.video.Codec", QString::fromUtf8(cname), false);
            setXmpTagString("Xmp.video.CodecDescription", QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name), false);
            setXmpTagString("Xmp.video.Format",     QString::fromUtf8(av_get_pix_fmt_name((AVPixelFormat)codec->format)), false);
            setXmpTagString("Xmp.video.ColorSpace", QString::fromUtf8(av_color_space_name(codec->color_space)), false);

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

            // ----------------------------

            dict = stream->metadata;

            qCDebug(DIGIKAM_METAENGINE_LOG) << "FFMpeg video stream metadata entries:";
            qCDebug(DIGIKAM_METAENGINE_LOG) << s_extractFFMpegMetadataEntriesFromDictionary(dict);

            // --------------

            entry = av_dict_get(dict, "rotate", NULL, 0);

            if (entry)
            {
                bool b               = false;
                int val              = QString::fromUtf8(entry->value).toInt(&b);
                ImageOrientation ori = ORIENTATION_UNSPECIFIED;

                if (b)
                {
                    switch (val)
                    {
                        case 0:
                            ori = ORIENTATION_NORMAL;
                            break;
                        case 90:
                            ori = ORIENTATION_ROT_90;
                            break;
                        case 180:
                            ori = ORIENTATION_ROT_180;
                            break;
                        case 270:
                            ori = ORIENTATION_ROT_270;
                            break;
                        default:
                            break;
                    }

                    setXmpTagString("Xmp.video.Orientation", QString::number(ori), false);
                    // Backport orientation in Exif
                    setImageOrientation(ori, false);
                }
            }
           
            // --------------

            entry = av_dict_get(dict, "language", NULL, 0);

            if (entry)
            {
                setXmpTagString("Xmp.video.Language", QString::fromUtf8(entry->value), false);
            }

            // --------------
            
            entry = av_dict_get(dict, "creation_time", NULL, 0);

            if (entry)
            {
                QDateTime dt = QDateTime::fromString(QString::fromUtf8(entry->value), Qt::ISODate);
                setXmpTagString("Xmp.video.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)), false);
            }
        }
    }

    entry = 0;

    // ----------------------------

    dict  = fmt_ctx->metadata;

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FFMpeg lead container metadata entries:";
    qCDebug(DIGIKAM_METAENGINE_LOG) << s_extractFFMpegMetadataEntriesFromDictionary(dict);

    // ----------------------------

/* TODO :
    account_type
    account_id
    compilation
    disc
    episode_uid
    hd_video
    podcast
    gapless_playback
    purchase_date
    sort_album_artist
    sort_album
    sort_artist
    sort_composer
    sort_name
    sort_show
    episode_id
    episode_sort
    network
    show
    season_number
    chapter
    disclaimer
    host_computer
    playback_requirements
    warning
*/

    // --------------

    entry = av_dict_get(dict, "keywords", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.InfoText", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "category", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Subject", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "premiere_version", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.SoftwareVersion", QString::fromUtf8(entry->value), false);
    }
    else
    {
        entry = av_dict_get(dict, "quicktime_version", NULL, 0);

        if (entry)
        {
            setXmpTagString("Xmp.video.SoftwareVersion", QString::fromUtf8(entry->value), false);
        }
    }

    // --------------

    entry = av_dict_get(dict, "firmware", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.FirmwareVersion", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "composer", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Composer", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "lyrics", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Lyrics", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "performers", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Performers", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "producer", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Producer", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "artist", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Artist", QString::fromUtf8(entry->value), false);
    }
    else
    {
        entry = av_dict_get(dict, "album_artist", NULL, 0);

        if (entry)
        {
            setXmpTagString("Xmp.video.Artist", QString::fromUtf8(entry->value), false);
        }
        else
        {
            entry = av_dict_get(dict, "original_artist", NULL, 0);

            if (entry)
            {
                setXmpTagString("Xmp.video.Artist", QString::fromUtf8(entry->value), false);
            }
        }
    }

    // --------------

    entry = av_dict_get(dict, "director", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Director", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "media_type", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Medium", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "grouping", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Grouping", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "encoder", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Encoder", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "subtitle", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Subtitle", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "original_source", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.SourceCredits", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "original_format", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Format", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "rating", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Rating", QString::fromUtf8(entry->value), false);
    }

    // --------------
    
    entry = av_dict_get(dict, "make", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Make", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "model", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Model", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "URL", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.URL", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "title", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Title", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "author", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Artist", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "copyright", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Copyright", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "comment", NULL, 0);

    if (entry)
    {
        QString data = QString::fromUtf8(entry->value);
        setXmpTagString("Xmp.video.Comment",     data, false);
        // Backport comment in Exif
        setExifComment(data, false);
    }

    // --------------

    entry = av_dict_get(dict, "description", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Information", QString::fromUtf8(entry->value), false);
    }
    else
    {
        entry = av_dict_get(dict, "synopsis", NULL, 0);

        if (entry)
        {
            setXmpTagString("Xmp.video.Information", QString::fromUtf8(entry->value), false);
        }
    }

    // --------------

    entry = av_dict_get(dict, "album", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Album", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "genre", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.Genre", QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "track", NULL, 0);

    if (entry)
    {
        QString value = QString::fromUtf8(entry->value);

        bool ok   = false;
        int track = value.toInt(&ok);

        if (ok && track)
            setXmpTagString("Xmp.video.TrackNumber", QString::number(track), false);
    }

    // --------------

    entry = av_dict_get(dict, "year", NULL, 0);

    if (entry)
    {
        int year = QString::fromUtf8(entry->value).toInt();
        setXmpTagString("Xmp.video.Year", QString::number(year), false);
    }

    // --------------

    entry = av_dict_get(dict, "creation_time", NULL, 0);

    if (entry)
    {
        QString data = QString::fromUtf8(entry->value);
        setXmpTagString("Xmp.video.DateTimeOriginal", data, false);
        setXmpTagString("Xmp.video.DateTimeDigitized", data, false);
        // Backport date in Exif and Iptc.
        QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
        setImageDateTime(dt, true, false);
    }

    // --------------

    entry = av_dict_get(dict, "edit_date", NULL, 0);

    if (entry)
    {
        setXmpTagString("Xmp.video.ModificationDate",
                        QString::fromUtf8(entry->value), false);
    }

    // --------------

    entry = av_dict_get(dict, "date", NULL, 0);

    if (entry)
    {
        QDateTime dt = QDateTime::fromString(QString::fromUtf8(entry->value), Qt::ISODate);
        setXmpTagString("Xmp.video.MediaCreateDate",
                        QString::number(s_secondsSinceJanuary1904(dt)), false);
    }
    
    // --------------

    // GPS info as string. ex: "+44.8511-000.6229/"
    entry = av_dict_get(dict, "location", NULL, 0);

    if (entry)
    {
        QString data     = QString::fromUtf8(entry->value);
        setXmpTagString("Xmp.video.GPSCoordinates", data, false);

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

        //qCDebug(DIGIKAM_METAENGINE_LOG) << lat << lng;

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
