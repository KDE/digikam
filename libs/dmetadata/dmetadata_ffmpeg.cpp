/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-02-26
 * Description : metadata extraction with ffmpeg (libav)
 * 
 * References  :
 *
 * FFMpeg metadata review: https://wiki.multimedia.cx/index.php?title=FFmpeg_Metadata
 * FFMpeg MP4 parser     : https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/mov.c#L298
 * Exiv2 XMP video       : https://github.com/Exiv2/exiv2/blob/master/src/properties.cpp#L1331
 * Apple metadata desc   : https://developer.apple.com/library/content/documentation/QuickTime/QTFF/Metadata/Metadata.html
 * FFMpeg metadata writer: https://github.com/kritzikratzi/ofxAvCodec/blob/master/src/ofxAvUtils.cpp#L61
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

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QStringList>

// Local incudes

#include "captionvalues.h"
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

QStringList s_keywordsSeparation(const QString& data)
{
    QStringList keywords = data.split(QLatin1String("/"));

    if (keywords.isEmpty())
    {
        keywords = data.split(QLatin1String(","));

        if (keywords.isEmpty())
        {
            keywords = data.split(QLatin1String(" "));
        }
    }
    
    return keywords;
}
    
qint64 s_secondsSinceJanuary1904(const QDateTime dt)
{
    QDateTime dt1904(QDate(1904, 1, 1), QTime(0, 0, 0));
    return dt1904.secsTo(dt);
}

#ifdef HAVE_MEDIAPLAYER

DMetadata::MetaDataMap s_extractFFMpegMetadataEntriesFromDictionary(AVDictionary* const dict)
{
    AVDictionaryEntry* entry = 0;
    DMetadata::MetaDataMap meta;

    do
    {
        entry = av_dict_get(dict, "", entry, AV_DICT_IGNORE_SUFFIX);

        if (entry)
        {
            meta.insert(QString::fromUtf8(entry->key), QString::fromUtf8(entry->value));
        }   
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

    int totalSecs = fmt_ctx->duration / AV_TIME_BASE;
    int bitrate   = fmt_ctx->bit_rate;
    MetaDataMap::const_iterator it;

    QFileInfo fi(filePath);
    setXmpTagString("Xmp.video.FileName",    fi.fileName());
    setXmpTagString("Xmp.video.FileSize",    QString::number(fi.size() / (1024*1024)));
    setXmpTagString("Xmp.video.FileType",    fi.suffix());
    setXmpTagString("Xmp.video.MimeType",    QMimeDatabase().mimeTypeForFile(filePath).name());
    setXmpTagString("Xmp.video.Duration",    QString::number(totalSecs));
    setXmpTagString("Xmp.video.MaxBitRate",  QString::number(bitrate));
    setXmpTagString("Xmp.video.StreamCount", QString::number(fmt_ctx->nb_streams));

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

            setXmpTagString("Xmp.audio.Codec", QString::fromUtf8(cname));
            setXmpTagString("Xmp.audio.CodecDescription", QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name));
            setXmpTagString("Xmp.audio.SampleRate", QString::number(codec->sample_rate));
            setXmpTagString("Xmp.audio.ChannelType", QString::number(codec->channels));
            setXmpTagString("Xmp.audio.Format",     QString::fromUtf8(av_get_sample_fmt_name((AVSampleFormat)codec->format)));

            // --------------

            MetaDataMap ameta = s_extractFFMpegMetadataEntriesFromDictionary(stream->metadata);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg audio stream metadata entries :";
            qCDebug(DIGIKAM_METAENGINE_LOG) << ameta;
            qCDebug(DIGIKAM_METAENGINE_LOG) << "-----------------------------------------";

            // --------------

            it = ameta.find(QLatin1String("language"));

            if (it != ameta.end())
            {
                setXmpTagString("Xmp.audio.TrackLang", it.value());
            }

            // --------------

            it = ameta.find(QLatin1String("creation_time"));

            if (it != ameta.end())
            {
                QDateTime dt = QDateTime::fromString(it.value(), Qt::ISODate);
                setXmpTagString("Xmp.audio.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)));
            }
        }

        if (!vstream && codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vstream           = true;
            const char* cname = avcodec_get_name(codec->codec_id); 

            setXmpTagString("Xmp.video.Codec", QString::fromUtf8(cname));
            setXmpTagString("Xmp.video.CodecDescription", QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name));
            setXmpTagString("Xmp.video.Format",     QString::fromUtf8(av_get_pix_fmt_name((AVPixelFormat)codec->format)));
            setXmpTagString("Xmp.video.ColorSpace", QString::fromUtf8(av_color_space_name(codec->color_space)));

            int aspectRatio = codec->sample_aspect_ratio.num;
            int frameRate   = stream->avg_frame_rate.num;

            if (codec->sample_aspect_ratio.den)
                aspectRatio /= codec->sample_aspect_ratio.den;

            if (stream->avg_frame_rate.den)
                frameRate /= stream->avg_frame_rate.den;

            setXmpTagString("Xmp.video.Width",             QString::number(codec->width));
            setXmpTagString("Xmp.video.Height",            QString::number(codec->height));
            setXmpTagString("Xmp.video.SourceImageWidth",  QString::number(codec->width));
            setXmpTagString("Xmp.video.SourceImageHeight", QString::number(codec->height));

            // Backport size in Exif and Iptc
            setImageDimensions(QSize(codec->width, codec->height));

            if (aspectRatio)
                setXmpTagString("Xmp.video.AspectRatio", QString::number(aspectRatio));

            if (frameRate)
                setXmpTagString("Xmp.video.FrameRate", QString::number(frameRate));

            setXmpTagString("Xmp.video.PixelDepth",  QString::number(codec->bits_per_coded_sample));

            // ----------------------------

            MetaDataMap vmeta = s_extractFFMpegMetadataEntriesFromDictionary(stream->metadata);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg video stream metadata entries :";
            qCDebug(DIGIKAM_METAENGINE_LOG) << vmeta;
            qCDebug(DIGIKAM_METAENGINE_LOG) << "-----------------------------------------";

            // --------------

            it = vmeta.find(QLatin1String("rotate"));

            if (it != vmeta.end())
            {
                bool b               = false;
                int val              = it.value().toInt(&b);
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

                    setXmpTagString("Xmp.video.Orientation", QString::number(ori));
                    // Backport orientation in Exif
                    setImageOrientation(ori);
                }
            }

            // --------------

            it = vmeta.find(QLatin1String("language"));

            if (it != vmeta.end())
            {
                setXmpTagString("Xmp.video.Language", it.value());
            }

            // --------------

            it = vmeta.find(QLatin1String("creation_time"));

            if (it != vmeta.end())
            {
                QDateTime dt = QDateTime::fromString(it.value(), Qt::ISODate);
                setXmpTagString("Xmp.video.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)));
            }
        }
    }

    // ----------------------------

    MetaDataMap rmeta = s_extractFFMpegMetadataEntriesFromDictionary(fmt_ctx->metadata);

    qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg root container metadata entries :";
    qCDebug(DIGIKAM_METAENGINE_LOG) << rmeta;
    qCDebug(DIGIKAM_METAENGINE_LOG) << "------------------------------------------";

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
    warning
*/

    // --------------

    QStringList tagsLst;
    tagsLst << QLatin1String("keywords")
            << QLatin1String("com.apple.quicktime.keywords");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {        
            QString data         = it.value();
            setXmpTagString("Xmp.video.InfoText", data);

            QStringList keywords = s_keywordsSeparation(data);

            if (!keywords.isEmpty())
            {
                setXmpKeywords(keywords);
                setIptcKeywords(QStringList(), keywords);
                break;
            }
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("category");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            QString data           = it.value();
            setXmpTagString("Xmp.video.Subject", data);

            QStringList categories = s_keywordsSeparation(data);

            if (!categories.isEmpty())
            {
                setXmpSubCategories(categories);
                setIptcSubCategories(QStringList(), categories);
            }

            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("premiere_version")
            << QLatin1String("quicktime_version");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.SoftwareVersion", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("firmware");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.FirmwareVersion", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("composer");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Composer", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("playback_requirements");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Requirements", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("lyrics");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Lyrics", it.value());
            break;            
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("performers");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Performers", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("producer");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Producer", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("artist")
            << QLatin1String("album_artist")
            << QLatin1String("original_artist");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Artist", it.value());
            break;            
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("director");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Director", it.value());
            break;            
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("media_type");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Medium", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("grouping");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Grouping", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("encoder");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Encoder", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("subtitle");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Subtitle", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("original_source");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.SourceCredits", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("original_format");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Format", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("rating")
            << QLatin1String("com.apple.quicktime.rating.user");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            QString data = it.value();
            setXmpTagString("Xmp.video.Rating", data);

            // Backport rating in Exif and Iptc
            bool b     = false;
            int rating = data.toInt(&b);

            if (b)
            {
                setImageRating(rating);
                break;
            }
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("make");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Make", it.value());
            break;            
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("model");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Model", it.value());
            break;            
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("URL");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.URL", it.value());
            break;            
        }
    }
        
    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("title");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Title", it.value());
            break;            
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("author");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Artist", it.value());
        }
    }
        
    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("copyright");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Copyright", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("comment")
            << QLatin1String("description")
            << QLatin1String("com.apple.quicktime.description");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            QString data = it.value();
            setXmpTagString("Xmp.video.Comment", data);

            // Backport comment in Exif and Iptc

            CaptionsMap capMap;
            MetaEngine::AltLangMap comMap;
            comMap.insert(QLatin1String("x-default"), data);
            capMap.setData(comMap, MetaEngine::AltLangMap(), QString(), MetaEngine::AltLangMap());

            setImageComments(capMap);
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("synopsis");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Information", it.value());
        }
        
        break;
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("album")
            << QLatin1String("com.apple.quicktime.album");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Album", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("genre")
            << QLatin1String("com.apple.quicktime.genre");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.Genre", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("track");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            bool ok   = false;
            int track = it.value().toInt(&ok);

            if (ok)
            {
                setXmpTagString("Xmp.video.TrackNumber", QString::number(track));
                break;
            }
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("year");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            bool ok  = false;
            int year = it.value().toInt(&ok);
            
            if (ok)
            {
                setXmpTagString("Xmp.video.Year", QString::number(year));
                break;
            }
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("creation_time")
            << QLatin1String("com.apple.quicktime.creationdate");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            QString data = it.value();
            setXmpTagString("Xmp.video.DateTimeOriginal", data);
            setXmpTagString("Xmp.video.DateTimeDigitized", data);
            // Backport date in Exif and Iptc.
            QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
            setImageDateTime(dt, true);
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("edit_date");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            setXmpTagString("Xmp.video.ModificationDate", it.value());
            break;
        }
    }

    // --------------

    tagsLst.clear();
    tagsLst << QLatin1String("date");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            QDateTime dt = QDateTime::fromString(it.value(), Qt::ISODate);
            setXmpTagString("Xmp.video.MediaCreateDate",
                            QString::number(s_secondsSinceJanuary1904(dt)));
            break;
        }
    }

    // --------------

    // GPS info as string. ex: "+44.8511-000.6229/"

    tagsLst.clear();
    tagsLst << QLatin1String("location");

    foreach (QString tags, tagsLst)
    {
        it = rmeta.find(tags);

        if (it != rmeta.end())
        {
            QString data     = it.value();
            setXmpTagString("Xmp.video.GPSCoordinates", data);

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
                setGPSInfo(alt, lattitude, longitude);

                setXmpTagString("Xmp.video.GPSLatitude",
                                getXmpTagString("Xmp.exif.GPSLatitude"));
                setXmpTagString("Xmp.video.GPSLongitude",
                                getXmpTagString("Xmp.exif.GPSLongitude"));
                setXmpTagString("Xmp.video.GPSMapDatum",
                                getXmpTagString("Xmp.exif.GPSMapDatum"));
                setXmpTagString("Xmp.video.GPSVersionID",
                                getXmpTagString("Xmp.exif.GPSVersionID"));

                setXmpTagString("Xmp.exif.GPSLatitude",
                                getXmpTagString("Xmp.exif.GPSLatitude"));
                setXmpTagString("Xmp.exif.GPSLongitude",
                                getXmpTagString("Xmp.exif.GPSLongitude"));
                setXmpTagString("Xmp.exif.GPSMapDatum",
                                getXmpTagString("Xmp.exif.GPSMapDatum"));
                setXmpTagString("Xmp.exif.GPSVersionID",
                                getXmpTagString("Xmp.exif.GPSVersionID"));
            }
            
            break;
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
