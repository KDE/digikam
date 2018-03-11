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
 * FFMpeg metadata review: https://wiki.multimedia.cx/index.php/FFmpeg_Metadata
 * FFMpeg MP4 parser     : https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/mov.c#L298
 * Exiv2 XMP video       : https://github.com/Exiv2/exiv2/blob/master/src/properties.cpp#L1331
 * Apple metadata desc   : https://developer.apple.com/library/content/documentation/QuickTime/QTFF/Metadata/Metadata.html
 * Matroska metadata desc: https://matroska.org/technical/specs/tagging/index.html
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

// KDE includes

#include <klocalizedstring.h>

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

/** Search first occurence of string in 'map' with keys given by 'lst'.
 *  Return the string match.
 *  If 'xmpTag' is not null, register XMP tag value with string.
 */
QString s_setXmpTagStringFromEntry(DMetadata* const meta,
                                   const QStringList& lst,
                                   const DMetadata::MetaDataMap& map,
                                   const char* const xmpTag=0)
{
    foreach (QString tag, lst)
    {
        DMetadata::MetaDataMap::const_iterator it = map.find(tag);

        if (it != map.end())
        {
            if (meta   &&                               // Protection.
                xmpTag &&                               // If xmp tag is null, we only return the matching value from the map.
                meta->getXmpTagString(xmpTag).isNull()) // Only register the tag value if it doesn't exists yet.
            {
                meta->setXmpTagString(xmpTag, it.value());
            }

            return it.value();
        }
    }

    return QString();
}

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

    QString   data;
    QFileInfo fi(filePath);

    setXmpTagString("Xmp.video.FileName",
        fi.fileName());

    setXmpTagString("Xmp.video.FileSize",
        QString::number(fi.size() / (1024*1024)));

    setXmpTagString("Xmp.video.FileType",
        fi.suffix());

    setXmpTagString("Xmp.video.MimeType",
        QMimeDatabase().mimeTypeForFile(filePath).name());

    setXmpTagString("Xmp.video.duration",
        QString::number((int)(1000.0 * (double)fmt_ctx->duration / (double)AV_TIME_BASE)));
    setXmpTagString("Xmp.xmpDM.duration",
        QString::number((int)(1000.0 * (double)fmt_ctx->duration / (double)AV_TIME_BASE)));

    setXmpTagString("Xmp.video.MaxBitRate",
        QString::number(fmt_ctx->bit_rate));

    setXmpTagString("Xmp.video.StreamCount",
        QString::number(fmt_ctx->nb_streams));

    // To only register one video, one audio stream, and one subtitle stream in XMP metadata.
    bool vstream = false;
    bool astream = false;
    bool sstream = false;

    for (uint i = 0 ; i < fmt_ctx->nb_streams ; i++)
    {
        const AVStream* const stream   = fmt_ctx->streams[i];
        AVCodecParameters* const codec = stream->codecpar;

        // -----------------------------------------
        // Audio stream parsing
        // -----------------------------------------

        if (!astream && codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            astream           = true;
            const char* cname = avcodec_get_name(codec->codec_id);

            setXmpTagString("Xmp.audio.Codec",
                QString::fromUtf8(cname));

            setXmpTagString("Xmp.audio.CodecDescription",
                QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name));

            setXmpTagString("Xmp.audio.SampleRate",
                QString::number(codec->sample_rate));
            setXmpTagString("Xmp.xmpDM.audioSampleRate",
                QString::number(codec->sample_rate));

            setXmpTagString("Xmp.audio.ChannelType",
                QString::number(codec->channels));
            setXmpTagString("Xmp.xmpDM.audioChannelType",
                QString::number(codec->channels));

            setXmpTagString("Xmp.audio.Format",
                QString::fromUtf8(av_get_sample_fmt_name((AVSampleFormat)codec->format)));

            // --------------

            MetaDataMap ameta = s_extractFFMpegMetadataEntriesFromDictionary(stream->metadata);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg audio stream metadata entries :";
            qCDebug(DIGIKAM_METAENGINE_LOG) << ameta;
            qCDebug(DIGIKAM_METAENGINE_LOG) << "-----------------------------------------";

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("language"),
                                       ameta,
                                       "Xmp.audio.TrackLang");

            // --------------

            data = s_setXmpTagStringFromEntry(this,
                                              QStringList() << QLatin1String("creation_time"),
                                              ameta);

            if (!data.isEmpty())
            {
                QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
                setXmpTagString("Xmp.audio.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)));
            }

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("handler_name"),
                                       ameta,
                                       "Xmp.audio.HandlerDescription");
        }

        // -----------------------------------------
        // Video stream parsing
        // -----------------------------------------

        if (!vstream && codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            vstream           = true;
            const char* cname = avcodec_get_name(codec->codec_id); 

            setXmpTagString("Xmp.video.Codec",
                 QString::fromUtf8(cname));

            setXmpTagString("Xmp.video.CodecDescription",
                 QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name));

            setXmpTagString("Xmp.video.Format",
                 QString::fromUtf8(av_get_pix_fmt_name((AVPixelFormat)codec->format)));

            setXmpTagString("Xmp.video.ColorMode",
                 QString::number(codec->color_space));

            setXmpTagString("Xmp.video.ColorSpace",
                 videoColorModelToString(codec->color_space));
            setXmpTagString("Xmp.xmpDM.videoColorSpace",
                 videoColorModelToString(codec->color_space));

            // ----------

            QString fo;

            switch (codec->field_order)
            {
                case AV_FIELD_PROGRESSIVE:
                    fo = QLatin1String("Progressive");
                    break;
                case AV_FIELD_TT:                       // Top coded first, top displayed first
                case AV_FIELD_BT:                       // Bottom coded first, top displayed first
                    fo = QLatin1String("Upper");
                    break;
                case AV_FIELD_BB:                       // Bottom coded first, bottom displayed first
                case AV_FIELD_TB:                       // Top coded first, bottom displayed first
                    fo = QLatin1String("Lower");
                    break;
                default:
                    break;
            }

            if (!fo.isEmpty())
            {
                setXmpTagString("Xmp.xmpDM.FieldOrder", fo);
            }

            // ----------

            QString aspectRatio;
            int frameRate = -1.0;

            if (codec->sample_aspect_ratio.num != 0)    // Check if undefined by ffmpeg
            {
                AVRational displayAspectRatio;

                av_reduce(&displayAspectRatio.num, &displayAspectRatio.den,
                          codec->width  * (int64_t)codec->sample_aspect_ratio.num,
                          codec->height * (int64_t)codec->sample_aspect_ratio.den,
                          1024 * 1024);

                aspectRatio = QString::fromLatin1("%1/%2").arg(displayAspectRatio.num)
                                                          .arg(displayAspectRatio.den);
            }
            else if (codec->height)
            {
                aspectRatio = QString::fromLatin1("%1/%2").arg(codec->width)
                                                          .arg(codec->height);
            }

            if (stream->avg_frame_rate.den)
            {
                frameRate = (double)stream->avg_frame_rate.num / (double)stream->avg_frame_rate.den;
            }

            setXmpTagString("Xmp.video.Width",
                QString::number(codec->width));
            setXmpTagString("Xmp.video.FrameWidth",
                QString::number(codec->width));
            setXmpTagString("Xmp.video.SourceImageWidth",
                QString::number(codec->width));

            setXmpTagString("Xmp.video.Height",
                QString::number(codec->height));
            setXmpTagString("Xmp.video.FrameHeight",
                QString::number(codec->height));
            setXmpTagString("Xmp.video.SourceImageHeight",
                QString::number(codec->height));

            setXmpTagString("Xmp.video.FrameSize",
                QString::fromLatin1("w:%1, h:%2, unit:pixels").arg(codec->width).arg(codec->height));
            setXmpTagString("Xmp.xmpDM.videoFrameSize",
                QString::fromLatin1("w:%1, h:%2, unit:pixels").arg(codec->width).arg(codec->height));

            // Backport size in Exif and Iptc
            setImageDimensions(QSize(codec->width, codec->height));

            if (!aspectRatio.isEmpty())
            {
                setXmpTagString("Xmp.video.AspectRatio",           aspectRatio);
                setXmpTagString("Xmp.xmpDM.videoPixelAspectRatio", aspectRatio);
            }

            if (frameRate != -1.0)
            {
                setXmpTagString("Xmp.video.FrameRate",      QString::number(frameRate));
                setXmpTagString("Xmp.xmpDM.videoFrameRate", QString::number(frameRate));
            }

            setXmpTagString("Xmp.video.BitDepth",        QString::number(codec->bits_per_coded_sample));
            setXmpTagString("Xmp.xmpDM.videoPixelDepth", QString::number(codec->bits_per_coded_sample));

            // -----------------------------------------

            MetaDataMap vmeta = s_extractFFMpegMetadataEntriesFromDictionary(stream->metadata);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg video stream metadata entries :";
            qCDebug(DIGIKAM_METAENGINE_LOG) << vmeta;
            qCDebug(DIGIKAM_METAENGINE_LOG) << "-----------------------------------------";

            // --------------

            data = s_setXmpTagStringFromEntry(this,
                                              QStringList() << QLatin1String("rotate"),
                                              vmeta);

            if (!data.isEmpty())
            {
                bool b               = false;
                int val              = data.toInt(&b);
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

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("language")
                                                     << QLatin1String("ILNG")       // Riff files.
                                                     << QLatin1String("LANG"),      // Riff files.
                                       vmeta,
                                       "Xmp.video.Language");

            // --------------

            data = s_setXmpTagStringFromEntry(this,
                                              QStringList() << QLatin1String("creation_time")
                                                            << QLatin1String("_STATISTICS_WRITING_DATE_UTC"), // MKV
                                              vmeta);

            if (!data.isEmpty())
            {
                QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
                setXmpTagString("Xmp.video.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)));

                setXmpTagString("Xmp.xmpDM.shotDate", dt.toString());
            }

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("handler_name"),
                                       vmeta,
                                       "Xmp.video.HandlerDescription");

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("TVER")                     // Riff files.
                                                     << QLatin1String("_STATISTICS_WRITING_APP"), // MKV files.
                                       vmeta,
                                       "Xmp.video.SoftwareVersion");
        }

        // -----------------------------------------
        // Subtitle stream parsing
        // -----------------------------------------

        if (!sstream && codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
        {
            sstream           = true;
            const char* cname = avcodec_get_name(codec->codec_id); 

            setXmpTagString("Xmp.video.SubTCodec",
                QString::fromUtf8(cname));
            setXmpTagString("Xmp.video.SubTCodecInfo",
                QString::fromUtf8(avcodec_descriptor_get_by_name(cname)->long_name));

            // -----------------------------------------

            MetaDataMap smeta = s_extractFFMpegMetadataEntriesFromDictionary(stream->metadata);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg subtitle stream metadata entries :";
            qCDebug(DIGIKAM_METAENGINE_LOG) << smeta;
            qCDebug(DIGIKAM_METAENGINE_LOG) << "--------------------------------------------";

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("subtitle")
                                                     << QLatin1String("title"),
                                       smeta,
                                       "Xmp.video.Subtitle");

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("language"),
                                       smeta,
                                       "Xmp.video.SubTLang");
        }
    }

    // -----------------------------------------
    // Root container parsing
    // -----------------------------------------

    MetaDataMap rmeta = s_extractFFMpegMetadataEntriesFromDictionary(fmt_ctx->metadata);

    qCDebug(DIGIKAM_METAENGINE_LOG) << "-- FFMpeg root container metadata entries :";
    qCDebug(DIGIKAM_METAENGINE_LOG) << rmeta;
    qCDebug(DIGIKAM_METAENGINE_LOG) << "------------------------------------------";

    // ----------------------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("major_brand"),
                               rmeta,
                               "Xmp.video.MajorBrand");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("compatible_brands"),
                               rmeta,
                               "Xmp.video.CompatibleBrands");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("minor_version"),
                               rmeta,
                               "Xmp.video.MinorVersion");

    // --------------

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("keywords")
                             << QLatin1String("IMIT")                           // Riff files.
                             << QLatin1String("com.apple.quicktime.keywords"),
               rmeta,
               "Xmp.video.InfoText");

    if (!data.isEmpty())
    {
        QStringList keywords = s_keywordsSeparation(data);

        if (!keywords.isEmpty())
        {
            setXmpKeywords(keywords);
            setIptcKeywords(QStringList(), keywords);
        }
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("category")
                             << QLatin1String("ISBJ"),                              // RIFF files.
               rmeta,
               "Xmp.video.Subject");

    if (!data.isEmpty())
    {
        QStringList categories = s_keywordsSeparation(data);

        if (!categories.isEmpty())
        {
            setXmpSubCategories(categories);
            setIptcSubCategories(QStringList(), categories);
        }
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("premiere_version")
                                             << QLatin1String("quicktime_version")
                                             << QLatin1String("ISFT")               // Riff files
                                             << QLatin1String("com.apple.quicktime.software"),
                               rmeta,
                               "Xmp.video.SoftwareVersion");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("firmware")
                                             << QLatin1String("com.apple.proapps.serialno"),
                               rmeta,
                               "Xmp.video.FirmwareVersion");

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("composer"),
                                      rmeta,
                                      "Xmp.video.Composer");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.composer", data);
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("com.apple.quicktime.displayname"),
                               rmeta,
                               "Xmp.video.Name");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("playback_requirements"),
                               rmeta,
                               "Xmp.video.Requirements");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("lyrics"),
                               rmeta,
                               "Xmp.video.Lyrics");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("performers"),
                               rmeta,
                               "Xmp.video.Performers");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("producer")
                                             << QLatin1String("com.apple.quicktime.producer"),
                               rmeta,
                               "Xmp.video.Producer");

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("artist")
                                                    << QLatin1String("album_artist")
                                                    << QLatin1String("original_artist")
                                                    << QLatin1String("com.apple.quicktime.artist")
                                                    << QLatin1String("IART")                        // Riff files.
                                                    << QLatin1String("author")
                                                    << QLatin1String("com.apple.quicktime.author"),
                                      rmeta,
                                      "Xmp.video.Artist");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.artist", data);
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("director")
                                             << QLatin1String("DIRC")                       // Riff files.
                                             << QLatin1String("com.apple.quicktime.director"),
                               rmeta,
                               "Xmp.video.Director");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("media_type")
                                             << QLatin1String("IMED"),                      // Riff files.
                               rmeta,
                               "Xmp.video.Medium");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("grouping"),
                               rmeta,
                               "Xmp.video.Grouping");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("encoder")
                                             << QLatin1String("ENCODER"),                   // MKV files.
                               rmeta,
                               "Xmp.video.Encoder");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("com.apple.proapps.clipID"),
                               rmeta,
                               "Xmp.video.FileID");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("original_source")
                                             << QLatin1String("ISRC")                       // Riff files
                                             << QLatin1String("com.apple.proapps.cameraName"),
                               rmeta,
                               "Xmp.video.Source");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("original_format")
                                             << QLatin1String("com.apple.proapps.originalFormat"),
                               rmeta,
                               "Xmp.video.Format");

    // --------------

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("rating")
                             << QLatin1String("IRTD")                                       // Riff files.
                             << QLatin1String("RATE")                                       // Riff files.
                             << QLatin1String("com.apple.quicktime.rating.user"),
               rmeta,
               "Xmp.video.Rating");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.video.Rate", data);

        // Backport rating in Exif and Iptc
        bool b     = false;
        int rating = data.toInt(&b);

        if (b)
        {
            setImageRating(rating);
        }
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("make")
                                             << QLatin1String("com.apple.quicktime.make"),
                               rmeta,
                               "Xmp.video.Make");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("model")
                                             << QLatin1String("com.apple.quicktime.model"),
                               rmeta,
                               "Xmp.video.Model");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("URL")
                                             << QLatin1String("TURL"),
                               rmeta,
                               "Xmp.video.URL");


    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("title")
                                                    << QLatin1String("INAM")            // Riff files.
                                                    << QLatin1String("TITL")            // Riff files.
                                                    << QLatin1String("com.apple.quicktime.title"),
                                      rmeta,
                                      "Xmp.video.Title");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.shotName", data);
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("copyright")
                                                    << QLatin1String("ICOP")            // Riff files.
                                                    << QLatin1String("com.apple.quicktime.copyright"),
                                      rmeta,
                                      "Xmp.video.Copyright");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.copyright", data);
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("comment")
                                                    << QLatin1String("description")
                                                    << QLatin1String("CMNT")                             // Riff Files.
                                                    << QLatin1String("COMN")                             // Riff Files.
                                                    << QLatin1String("ICMT")                             // Riff Files.
                                                    << QLatin1String("com.apple.quicktime.description"),
                                      rmeta,
                                      "Xmp.video.Comment");

    if (!data.isEmpty())
    {
        // Backport comment in Exif and Iptc

        CaptionsMap capMap;
        MetaEngine::AltLangMap comMap;
        comMap.insert(QLatin1String("x-default"), data);
        capMap.setData(comMap, MetaEngine::AltLangMap(), QString(), MetaEngine::AltLangMap());

        setImageComments(capMap);

        setXmpTagString("Xmp.xmpDM.logComment", data);
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("synopsis"),
                               rmeta,
                               "Xmp.video.Information");
    
    // --------------

    for (int i = 1 ; i <= 9 ; i++)
    {    
        s_setXmpTagStringFromEntry(this,
                                   QStringList() << QString::fromLatin1("IAS%1").arg(i),          // Riff files.
                                   rmeta,
                                   QString::fromLatin1("Xmp.video.Edit%1").arg(i).toLatin1().data());
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("CODE")    // Riff files.
                                             << QLatin1String("IECN"),   // Riff files.
                               rmeta,
                               "Xmp.video.EncodedBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("DISP"),   // Riff files.
                               rmeta,
                               "Xmp.video.SchemeTitle");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("AGES"),   // Riff files.
                               rmeta,
                               "Xmp.video.Rated");
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IBSU"),   // Riff files.
                               rmeta,
                               "Xmp.video.BaseURL");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICAS"),   // Riff files.
                               rmeta,
                               "Xmp.video.DefaultStream");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICDS"),   // Riff files.
                               rmeta,
                               "Xmp.video.CostumeDesigner");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICMS"),   // Riff files.
                               rmeta,
                               "Xmp.video.Commissioned");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICNM"),   // Riff files.
                               rmeta,
                               "Xmp.video.Cinematographer");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICNT"),   // Riff files.
                               rmeta,
                               "Xmp.video.Country");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IARL"),   // Riff files.
                               rmeta,
                               "Xmp.video.ArchivalLocation");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICRP"),   // Riff files.
                               rmeta,
                               "Xmp.video.Cropped");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICRP"),   // Riff files.
                               rmeta,
                               "Xmp.video.Cropped");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IDIM"),   // Riff files.
                               rmeta,
                               "Xmp.video.Dimensions");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IDPI"),   // Riff files.
                               rmeta,
                               "Xmp.video.DotsPerInch");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IDST"),   // Riff files.
                               rmeta,
                               "Xmp.video.DistributedBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IEDT"),   // Riff files.
                               rmeta,
                               "Xmp.video.EditedBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IENG"),   // Riff files.
                               rmeta,
                               "Xmp.video.Engineer");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IKEY"),   // Riff files.
                               rmeta,
                               "Xmp.video.PerformerKeywords");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ILGT"),   // Riff files.
                               rmeta,
                               "Xmp.video.Lightness");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ILGU"),   // Riff files.
                               rmeta,
                               "Xmp.video.LogoURL");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ILIU"),   // Riff files.
                               rmeta,
                               "Xmp.video.LogoIconURL");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMBI"),   // Riff files.
                               rmeta,
                               "Xmp.video.InfoBannerImage");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMBU"),   // Riff files.
                               rmeta,
                               "Xmp.video.InfoBannerURL");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMIU"),   // Riff files.
                               rmeta,
                               "Xmp.video.InfoURL");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMUS"),   // Riff files.
                               rmeta,
                               "Xmp.video.MusicBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPDS"),   // Riff files.
                               rmeta,
                               "Xmp.video.ProductionDesigner");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPLT"),   // Riff files.
                               rmeta,
                               "Xmp.video.NumOfColors");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPRD"),   // Riff files.
                               rmeta,
                               "Xmp.video.Product");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPRO"),   // Riff files.
                               rmeta,
                               "Xmp.video.ProducedBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IRIP"),   // Riff files.
                               rmeta,
                               "Xmp.video.RippedBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISGN"),   // Riff files.
                               rmeta,
                               "Xmp.video.SecondaryGenre");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISHP"),   // Riff files.
                               rmeta,
                               "Xmp.video.Sharpness");
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISRF"),   // Riff files.
                               rmeta,
                               "Xmp.video.SourceForm");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISTD"),   // Riff files.
                               rmeta,
                               "Xmp.video.ProductionStudio");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISTR")    // Riff files.
                                             << QLatin1String("STAR"),   // Riff files.
                               rmeta,
                               "Xmp.video.Starring");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ITCH"),   // Riff files.
                               rmeta,
                               "Xmp.video.Technician");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IWMU"),   // Riff files.
                               rmeta,
                               "Xmp.video.WatermarkURL");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IWRI"),   // Riff files.
                               rmeta,
                               "Xmp.video.WrittenBy");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("PRT1"),   // Riff files.
                               rmeta,
                               "Xmp.video.Part");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("PRT2"),   // Riff files.
                               rmeta,
                               "Xmp.video.NumOfParts");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("STAT"),   // Riff files.
                               rmeta,
                               "Xmp.video.Statistics");
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TAPE"),   // Riff files.
                               rmeta,
                               "Xmp.video.TapeName");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TCDO"),   // Riff files.
                               rmeta,
                               "Xmp.video.EndTimecode");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TCOD"),   // Riff files.
                               rmeta,
                               "Xmp.video.StartTimecode");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TLEN"),   // Riff files.
                               rmeta,
                               "Xmp.video.Length");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TORG"),   // Riff files.
                               rmeta,
                               "Xmp.video.Organization");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("VMAJ"),   // Riff files.
                               rmeta,
                               "Xmp.video.VegasVersionMajor");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("VMIN"),   // Riff files.
                               rmeta,
                               "Xmp.video.VegasVersionMinor");

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("LOCA"),   // Riff files.
                                      rmeta,
                                      "Xmp.video.LocationInfo");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.shotLocation", data);
    }
    
    // --------------
    
    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("album")
                                                    << QLatin1String("com.apple.quicktime.album"),
                                      rmeta,
                                      "Xmp.video.Album");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.album", data);
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("genre")
                                                    << QLatin1String("GENR")        // Riff files.
                                                    << QLatin1String("IGNR")        // Riff files.
                                                    << QLatin1String("com.apple.quicktime.genre"),
                                      rmeta,
                                      "Xmp.video.Genre");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.genre", data);
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("track")
                                                    << QLatin1String("TRCK"),       // Riff files.
                                      rmeta,
                                      "Xmp.video.TrackNumber");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.trackNumber", data);
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("year")
                                             << QLatin1String("YEAR")               // Riff files.
                                             << QLatin1String("com.apple.quicktime.year"),
                               rmeta,
                               "Xmp.video.Year");

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICRD"),              // Riff files
                               rmeta,
                               "Xmp.video.DateTimeDigitized");

    // --------------

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("creation_time")
                             << QLatin1String("DTIM")                               // Riff files.
                             << QLatin1String("com.apple.quicktime.creationdate"),
               rmeta,
               "Xmp.video.DateTimeOriginal");

    if (!data.isEmpty())
    {
        // Backport date in Exif and Iptc.
        QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
        setImageDateTime(dt, true);
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                                      QStringList() << QLatin1String("edit_date"),
                                      rmeta,
                                      "Xmp.video.ModificationDate");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.videoModDate", data);
    }

    // --------------

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("date"),
               rmeta);

    if (!data.isEmpty())
    {
        QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
        setXmpTagString("Xmp.video.MediaCreateDate",
                        QString::number(s_secondsSinceJanuary1904(dt)));
    }

    // --------------

    // GPS info as string. ex: "+44.8511-000.6229/"
    // Defined in ISO 6709:2008.
    // Notes: altitude can be passed as 3rd values.
    //        each value is separated from others by '-' or '+'.
    //        '/' is always the terminaison character.

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("location")
                             << QLatin1String("com.apple.quicktime.location.ISO6709"),
               rmeta,
               "Xmp.video.GPSCoordinates");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.xmpDM.shotLocation", data);

        // Backport location to Exif.

        QList<int> digits;

        for (int i = 0 ; i < data.length() ; i++)
        {
            QChar c = data[i];

            if (c == QLatin1Char('+') || c == QLatin1Char('-') || c == QLatin1Char('/'))
            {
                digits << i;
            }
        }

        QString coord;
        double lattitude = 0.0;
        double longitude = 0.0;
        double altitude  = 0.0;
        bool b1          = false;
        bool b2          = false;
        bool b3          = false;

        if (digits.size() > 1)
        {
            coord     = data.mid(digits[0], digits[1] - digits[0]);
            lattitude = coord.toDouble(&b1);
        }

        if (digits.size() > 2)
        {
            coord     = data.mid(digits[1], digits[2] - digits[1]);
            longitude = coord.toDouble(&b2);
        }

        if (digits.size() > 3)
        {
            coord    = data.mid(digits[2], digits[3] - digits[2]);
            altitude = coord.toDouble(&b3);
        }

        if (b1 && b2)
        {
            if (b3)
            {
                // All GPS values are available.
                setGPSInfo(altitude, lattitude, longitude);

                setXmpTagString("Xmp.video.GPSAltitude",
                                getXmpTagString("Xmp.exif.GPSAltitude"));
                setXmpTagString("Xmp.exif.GPSAltitude",
                                getXmpTagString("Xmp.exif.GPSAltitude"));
            }
            else
            {
                // No altitude available.
                double* alt = 0;
                setGPSInfo(alt, lattitude, longitude);
            }

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
    }

    avformat_close_input(&fmt_ctx);

    return true;

#else
    Q_UNUSED(filePath);
    return false;
#endif
}

QString DMetadata::videoColorModelToString(int colorSpace)
{
    QString cs = i18n("unknown");

#ifdef HAVE_MEDIAPLAYER
    cs = QString::fromUtf8(av_color_space_name((AVColorSpace)colorSpace));
#else
    Q_UNUSED(colorSpace);
#endif

    return cs;
}

} // namespace Digikam
