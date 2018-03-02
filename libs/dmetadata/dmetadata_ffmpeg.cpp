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
#include <QRegExp>

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
            if (xmpTag)
                meta->setXmpTagString(xmpTag, it.value());

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

    int totalSecs = fmt_ctx->duration / AV_TIME_BASE;
    int bitrate   = fmt_ctx->bit_rate;
    QString data;

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

        // -----------------------------------------
        // Audio stream parsing
        // -----------------------------------------
        
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
                                       QStringList() << QLatin1String("language"),
                                       vmeta,
                                       "Xmp.video.Language");

            // --------------

            data = s_setXmpTagStringFromEntry(this, 
                                              QStringList() << QLatin1String("creation_time"),
                                              vmeta);

            if (!data.isEmpty())
            {
                QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
                setXmpTagString("Xmp.video.TrackCreateDate",
                                QString::number(s_secondsSinceJanuary1904(dt)));
            }

            // --------------

            s_setXmpTagStringFromEntry(this, 
                                       QStringList() << QLatin1String("handler_name"),
                                       vmeta,
                                       "Xmp.video.HandlerDescription");
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
               QStringList() << QLatin1String("category"),
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
                                             << QLatin1String("com.apple.quicktime.software"),
                               rmeta,
                               "Xmp.video.SoftwareVersion");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("firmware"),
                               rmeta,
                               "Xmp.video.FirmwareVersion");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("composer"),
                               rmeta,
                               "Xmp.video.Composer");

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

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("artist")
                                             << QLatin1String("album_artist")
                                             << QLatin1String("original_artist")
                                             << QLatin1String("com.apple.quicktime.artist"),
                               rmeta,
                               "Xmp.video.Artist");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("director")
                                             << QLatin1String("com.apple.quicktime.director"),
                               rmeta,
                               "Xmp.video.Director");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("media_type"),
                               rmeta,
                               "Xmp.video.Medium");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("grouping"),
                               rmeta,
                               "Xmp.video.Grouping");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("encoder"),
                               rmeta,
                               "Xmp.video.Encoder");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("subtitle"),
                               rmeta,
                               "Xmp.video.Subtitle");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("original_source"),
                               rmeta,
                               "Xmp.video.SourceCredits");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("original_format"),
                               rmeta,
                               "Xmp.video.Format");

    // --------------

    data = s_setXmpTagStringFromEntry(this, 
               QStringList() << QLatin1String("rating")
                             << QLatin1String("com.apple.quicktime.rating.user"),
               rmeta,
               "Xmp.video.Rating");

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
                               QStringList() << QLatin1String("make"),
                               rmeta,
                               "Xmp.video.Make");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("model"),
                               rmeta,
                               "Xmp.video.Model");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("URL"),
                               rmeta,
                               "Xmp.video.URL");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("title")
                                             << QLatin1String("com.apple.quicktime.title"),
                               rmeta,
                               "Xmp.video.Title");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("author")
                                             << QLatin1String("com.apple.quicktime.author"),
                               rmeta,
                               "Xmp.video.Artist");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("copyright")
                                             << QLatin1String("com.apple.quicktime.copyright"),
                               rmeta,
                               "Xmp.video.Copyright");

    // --------------

    data = s_setXmpTagStringFromEntry(this, 
               QStringList() << QLatin1String("comment")
                             << QLatin1String("description")
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
    }

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("synopsis"),
                               rmeta,
                               "Xmp.video.Information");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("album")
                                             << QLatin1String("com.apple.quicktime.album"),
                               rmeta,
                               "Xmp.video.Album");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("genre")
                                             << QLatin1String("com.apple.quicktime.genre"),
                               rmeta,
                               "Xmp.video.Genre");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("track"),
                               rmeta,
                               "Xmp.video.TrackNumber");

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("year")
                                             << QLatin1String("com.apple.quicktime.year"),
                               rmeta,
                               "Xmp.video.Year");

    // --------------

    data = s_setXmpTagStringFromEntry(this, 
               QStringList() << QLatin1String("creation_time")
                             << QLatin1String("com.apple.quicktime.creationdate"),
               rmeta,
               "Xmp.video.DateTimeOriginal");

    if (!data.isEmpty())
    {
        setXmpTagString("Xmp.video.DateTimeDigitized", data);
        // Backport date in Exif and Iptc.
        QDateTime dt = QDateTime::fromString(data, Qt::ISODate);
        setImageDateTime(dt, true);
    }

    // --------------

    s_setXmpTagStringFromEntry(this, 
                               QStringList() << QLatin1String("edit_date"),
                               rmeta,
                               "Xmp.video.ModificationDate");

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

    data = s_setXmpTagStringFromEntry(this, 
               QStringList() << QLatin1String("location")
                             << QLatin1String("com.apple.quicktime.location.ISO6709"),
               rmeta,
               "Xmp.video.GPSCoordinates");

    if (!data.isEmpty())
    {
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
    }

    avformat_close_input(&fmt_ctx);

    return true;

#else
    Q_UNUSED(filePath);
    return false;
#endif
}

} // namespace Digikam
