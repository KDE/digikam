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
 * Exiv2 RIFF tags       : https://github.com/Exiv2/exiv2/blob/master/src/riffvideo.cpp#L83
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
 *  If 'xmpTags' is not empty, register XMP tags value with string.
 */
QString s_setXmpTagStringFromEntry(DMetadata* const meta,
                                   const QStringList& lst,
                                   const DMetadata::MetaDataMap& map,
                                   const QStringList& xmpTags=QStringList())
{
    foreach (QString tag, lst)
    {
        DMetadata::MetaDataMap::const_iterator it = map.find(tag);

        if (it != map.end())
        {
            if (meta &&                     // Protection.
                !xmpTags.isEmpty())         // If xmpTags is empty, we only return the matching value from the map.
            {
                foreach (const QString& tag, xmpTags)
                {
                    // Only register the tag value if it doesn't exists yet.

                    if (meta->getXmpTagString(tag.toLatin1().data()).isNull()) 
                    {
                        meta->setXmpTagString(tag.toLatin1().data(), it.value());
                    }
                }
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

    if (fmt_ctx->bit_rate > 0)
        setXmpTagString("Xmp.video.MaxBitRate", QString::number(fmt_ctx->bit_rate));

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
                                       QStringList() << QLatin1String("language"),              // MOV files.
                                       ameta,
                                       QStringList() << QLatin1String("Xmp.audio.TrackLang"));

            // --------------

            data = s_setXmpTagStringFromEntry(this,
                                              QStringList() << QLatin1String("creation_time"),  // MOV files.
                                              ameta);

            if (!data.isEmpty())
            {
                QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
                setXmpTagString("Xmp.audio.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)));
            }

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("handler_name"),          // MOV files.
                                       ameta,
                                       QStringList() << QLatin1String("Xmp.audio.HandlerDescription"));
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
                                              QStringList() << QLatin1String("rotate"),   // MOV files.
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
                                       QStringList() << QLatin1String("language")   // MOV files.
                                                     << QLatin1String("ILNG")       // Riff files.
                                                     << QLatin1String("LANG"),      // Riff files.
                                       vmeta,
                                       QStringList() << QLatin1String("Xmp.video.Language"));

            // --------------

            data = s_setXmpTagStringFromEntry(this,
                                              QStringList() << QLatin1String("creation_time")                 // MOV files.
                                                            << QLatin1String("_STATISTICS_WRITING_DATE_UTC"), // MKV files.
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
                                       QStringList() << QLatin1String("handler_name"),            // MOV files.
                                       vmeta,
                                       QStringList() << QLatin1String("Xmp.video.HandlerDescription"));

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("TVER")                     // Riff files.
                                                     << QLatin1String("_STATISTICS_WRITING_APP"), // MKV files.
                                       vmeta,
                                       QStringList() << QLatin1String("Xmp.video.SoftwareVersion"));
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
                                       QStringList() << QLatin1String("subtitle")           // MOV files.
                                                     << QLatin1String("title"),             // MOV files.
                                       smeta,
                                       QStringList() << QLatin1String("Xmp.video.Subtitle"));

            // --------------

            s_setXmpTagStringFromEntry(this,
                                       QStringList() << QLatin1String("language"),          // MOV files.
                                       smeta,
                                       QStringList() << QLatin1String("Xmp.video.SubTLang"));
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
                               QStringList() << QLatin1String("major_brand"),               // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.MajorBrand"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("compatible_brands"),         // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.CompatibleBrands"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("minor_version"),             // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.MinorVersion"));

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("keywords")                   // MOV files.
                                             << QLatin1String("IMIT")                       // Riff files.
                                             << QLatin1String("KEYWORDS")                   // MKV files.
                                             << QLatin1String("com.apple.quicktime.keywords"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.InfoText"));

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
                               QStringList() << QLatin1String("category")                   // MOV files.
                                             << QLatin1String("ISBJ")                       // RIFF files.
                                             << QLatin1String("SUBJECT"),                   // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Subject"));

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
                               QStringList() << QLatin1String("premiere_version")           // MOV files.
                                             << QLatin1String("quicktime_version")          // MOV files.
                                             << QLatin1String("ISFT")                       // Riff files
                                             << QLatin1String("com.apple.quicktime.software"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.SoftwareVersion"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("firmware")                   // MOV files.
                                             << QLatin1String("com.apple.proapps.serialno"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.FirmwareVersion"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("composer")                   // MOV files.
                                             << QLatin1String("COMPOSER"),                  // MKV files
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Composer")
                                             << QLatin1String("Xmp.xmpDM.composer"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("com.apple.quicktime.displayname"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Name"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("playback_requirements"),     // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Requirements"));

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("lyrics"),                    // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Lyrics"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("performers"),                // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Performers"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("producer")                   // MOV files.
                                             << QLatin1String("PRODUCER")                   // MKV files.
                                             << QLatin1String("com.apple.quicktime.producer"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Producer"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("artist")                     // MOV files.
                                             << QLatin1String("album_artist")               // MOV files.
                                             << QLatin1String("original_artist")            // MOV files.
                                             << QLatin1String("com.apple.quicktime.artist")
                                             << QLatin1String("IART")                       // Riff files.
                                             << QLatin1String("ARTIST")                     // MKV files.
                                             << QLatin1String("author")                     // MOV files.
                                             << QLatin1String("com.apple.quicktime.author"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Artist")
                                             << QLatin1String("Xmp.xmpDM.artist"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("director")                   // MOV files.
                                             << QLatin1String("DIRC")                       // Riff files.
                                             << QLatin1String("DIRECTOR")                   // MKV files.
                                             << QLatin1String("com.apple.quicktime.director"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Director")
                                             << QLatin1String("Xmp.xmpDM.director"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("media_type")                 // MOV files.
                                             << QLatin1String("IMED")                       // Riff files.
                                             << QLatin1String("ORIGINAL_MEDIA_TYPE"),       // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Medium"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("grouping"),                  // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Grouping"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("BPS"),                       // MKV files
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.MaxBitRate"));
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISRC"),                      // MKV files
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ISRCCode"));

    // --------------
    
    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("CONTENT_TYPE"),              // MKV files
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ExtendedContentDescription"));
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("FPS"),                       // MKV files.                                                    
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.videoFrameRate")
                                             << QLatin1String("Xmp.xmpDM.FrameRate"));
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("encoder")                    // MOV files.
                                             << QLatin1String("ENCODER"),                   // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Encoder"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("com.apple.proapps.clipID"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.FileID"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("original_source")            // MOV files.
                                             << QLatin1String("ISRC")                       // Riff files
                                             << QLatin1String("com.apple.proapps.cameraName"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Source"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("original_format")            // MOV files.
                                             << QLatin1String("com.apple.proapps.originalFormat"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Format"));

    // --------------

    data = s_setXmpTagStringFromEntry(this,
               QStringList() << QLatin1String("rating")                                     // MOV files.
                             << QLatin1String("IRTD")                                       // Riff files.
                             << QLatin1String("RATE")                                       // Riff files.
                             << QLatin1String("RATING")                                     // MKV files.
                             << QLatin1String("com.apple.quicktime.rating.user"),
               rmeta,
               QStringList() << QLatin1String("Xmp.video.Rating")
                             << QLatin1String("Xmp.video.Rate"));

    if (!data.isEmpty())
    {
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
                               QStringList() << QLatin1String("make")           // MOV files.
                                             << QLatin1String("com.apple.quicktime.make"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Make"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("model")          // MOV files.
                                             << QLatin1String("com.apple.quicktime.model"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Model")
                                             << QLatin1String("Xmp.xmpDM.cameraModel"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("URL")            // MOV files.
                                             << QLatin1String("TURL"),          // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.URL"));


    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("title")           // MOV files.
                                             << QLatin1String("INAM")            // Riff files.
                                             << QLatin1String("TITL")            // Riff files.
                                             << QLatin1String("TITLE")           // MKV files.
                                             << QLatin1String("com.apple.quicktime.title"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Title")
                                             << QLatin1String("Xmp.xmpDM.shotName"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("copyright")          // MOV files.
                                             << QLatin1String("ICOP")               // Riff files.
                                             << QLatin1String("COPYRIGHT")          // MKV files.
                                             << QLatin1String("com.apple.quicktime.copyright"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Copyright")
                                             << QLatin1String("Xmp.xmpDM.copyright"));

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("comment")            // MOV files.
                                             << QLatin1String("description")        // MOV files.
                                             << QLatin1String("CMNT")               // Riff Files.
                                             << QLatin1String("COMN")               // Riff Files.
                                             << QLatin1String("ICMT")               // Riff Files.
                                             << QLatin1String("COMMENT")            // MKV Files.
                                             << QLatin1String("DESCRIPTION")        // MKV Files.
                                             << QLatin1String("com.apple.quicktime.description"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Comment")
                                             << QLatin1String("Xmp.xmpDM.logComment"));

    if (!data.isEmpty())
    {
        // Backport comment in Exif and Iptc

        CaptionsMap capMap;
        MetaEngine::AltLangMap comMap;
        comMap.insert(QLatin1String("x-default"), data);
        capMap.setData(comMap, MetaEngine::AltLangMap(), QString(), MetaEngine::AltLangMap());

        setImageComments(capMap);
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("synopsis")           // MOV files.
                                             << QLatin1String("SUMMARY")            // MKV files.
                                             << QLatin1String("SYNOPSIS"),          // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Information"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("lyrics")             // MOV files.
                                             << QLatin1String("LYRICS"),            // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.xmpDM.lyrics"));
        
    // --------------

    for (int i = 1 ; i <= 9 ; i++)
    {    
        s_setXmpTagStringFromEntry(this,
                               QStringList() << QString::fromLatin1("IAS%1").arg(i), // Riff files.
                               rmeta,
                               QStringList() << QString::fromLatin1("Xmp.video.Edit%1").arg(i));
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("CODE")               // Riff files.
                                             << QLatin1String("IECN")               // Riff files.
                                             << QLatin1String("ENCODED_BY"),        // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.EncodedBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("DISP"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.SchemeTitle"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("AGES")               // Riff files.
                                             << QLatin1String("ICRA")               // MKV files.
                                             << QLatin1String("LAW_RATING"),        // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Rated"));
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IBSU"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.BaseURL"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICAS"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.DefaultStream"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICDS"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.CostumeDesigner"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICMS"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Commissioned"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICNM"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Cinematographer"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICNT"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Country"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IARL"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ArchivalLocation"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICRP"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Cropped"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IDIM"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Dimensions"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IDPI"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.DotsPerInch"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IDST")               // Riff files.
                                             << QLatin1String("DISTRIBUTED_BY"),    // MKV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.DistributedBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IEDT"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.EditedBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IENG"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Engineer")
                                             << QLatin1String("Xmp.xmpDM.engineer"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IKEY"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.PerformerKeywords"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ILGT"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Lightness"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ILGU"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.LogoURL"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ILIU"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.LogoIconURL"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMBI"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.InfoBannerImage"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMBU"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.InfoBannerURL"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMIU"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.InfoURL"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IMUS"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.MusicBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPDS"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ProductionDesigner"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPLT"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.NumOfColors"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPRD"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Product"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IPRO"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ProducedBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IRIP"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.RippedBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISGN"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.SecondaryGenre"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISHP"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Sharpness"));
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISRF"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.SourceForm"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISTD"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ProductionStudio"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ISTR")               // Riff files.
                                             << QLatin1String("STAR"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Starring"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ITCH"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Technician"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IWMU"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.WatermarkURL"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("IWRI"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.WrittenBy"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("PRT1"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Part"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("PRT2"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.NumOfParts"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("STAT"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Statistics"));
    
    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TAPE"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.TapeName"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TCDO"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.EndTimecode"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TCOD"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.StartTimecode"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TLEN"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Length"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("TORG"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Organization"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("VMAJ"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.VegasVersionMajor"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("VMIN"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.VegasVersionMinor"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("LOCA"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.LocationInfo")
                                             << QLatin1String("Xmp.xmpDM.shotLocation"));

    // --------------
    
    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("album")              // MOV files.
                                             << QLatin1String("com.apple.quicktime.album"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Album")
                                             << QLatin1String("Xmp.xmpDM.album"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("genre")              // MOV files.
                                             << QLatin1String("GENR")               // Riff files.
                                             << QLatin1String("IGNR")               // Riff files.
                                             << QLatin1String("GENRE")              // MKV files.
                                             << QLatin1String("com.apple.quicktime.genre"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Genre")
                                             << QLatin1String("Xmp.xmpDM.genre"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("track")              // MOV files.
                                             << QLatin1String("TRCK"),              // Riff files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.TrackNumber")
                                             << QLatin1String("Xmp.xmpDM.trackNumber"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("year")               // MOV files.
                                             << QLatin1String("YEAR")               // Riff files.
                                             << QLatin1String("com.apple.quicktime.year"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.Year"));

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("ICRD")               // Riff files
                                             << QLatin1String("DATE_DIGITIZED"),    // MKV files
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.DateTimeDigitized"));

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("creation_time")      // MOV files.
                                             << QLatin1String("DTIM")               // Riff files.
                                             << QLatin1String("DATE_RECORDED")      // MKV files.
                                             << QLatin1String("com.apple.quicktime.creationdate"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.DateTimeOriginal"));

    if (!data.isEmpty())
    {
        // Backport date in Exif and Iptc.
        QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
        setImageDateTime(dt, true);
    }

    // --------------

    s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("edit_date"),         // MOV files.
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.ModificationDate")
                                             << QLatin1String("Xmp.xmpDM.videoModDate"));

    // --------------

    data = s_setXmpTagStringFromEntry(this,
                               QStringList() << QLatin1String("date")               // MOV files.
                                             << QLatin1String("DATE_RELEASED"),     // MKV files.
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
                               QStringList() << QLatin1String("location")           // MOV files.
                                             << QLatin1String("RECORDING_LOCATION") // MKV files.
                                             << QLatin1String("com.apple.quicktime.location.ISO6709"),
                               rmeta,
                               QStringList() << QLatin1String("Xmp.video.GPSCoordinates")
                                             << QLatin1String("Xmp.xmpDM.shotLocation"));

    if (!data.isEmpty())
    {
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
