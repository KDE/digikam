/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-04
 * Description : a tool to adjust date time stamp of images
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "timeadjust.h"

// C ANSI includes

extern "C"
{
#include <unistd.h>
#include <utime.h>
}

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dlayoutbox.h"
#include "dmetadata.h"
#include "timeadjustsettings.h"

namespace Digikam
{

TimeAdjust::TimeAdjust(QObject* const parent)
    : BatchTool(QLatin1String("TimeAdjust"), MetadataTool, parent)
{
    m_taWidget       = 0;
    m_changeSettings = true;

    setToolTitle(i18n("Time Adjust"));
    setToolDescription(i18n("Adjust Date Time Stamp of Image"));
    setToolIconName(QLatin1String("appointment-new"));
}

TimeAdjust::~TimeAdjust()
{
}

void TimeAdjust::registerSettingsWidget()
{
    DVBox* const vbox = new DVBox;
    m_taWidget        = new TimeAdjustSettings(vbox);
    m_settingsWidget  = vbox;

    connect(m_taWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings TimeAdjust::defaultSettings()
{
    BatchToolSettings settings;
    TimeAdjustContainer prm;

    settings.insert(QLatin1String("Custom Date"),                   prm.customDate);
    settings.insert(QLatin1String("Custom Time"),                   prm.customTime);

    settings.insert(QLatin1String("Adjustment Type"),               prm.adjustmentType);
    settings.insert(QLatin1String("Adjustment Days"),               prm.adjustmentDays);
    settings.insert(QLatin1String("Adjustment Time"),               prm.adjustmentTime);

    settings.insert(QLatin1String("Update File Modification Time"), prm.updFileModDate);
    settings.insert(QLatin1String("Update EXIF Modification Time"), prm.updEXIFModDate);
    settings.insert(QLatin1String("Update EXIF Original Time"),     prm.updEXIFOriDate);
    settings.insert(QLatin1String("Update EXIF Digitization Time"), prm.updEXIFDigDate);
    settings.insert(QLatin1String("Update EXIF Thumbnail Time"),    prm.updEXIFThmDate);
    settings.insert(QLatin1String("Update IPTC Time"),              prm.updIPTCDate);
    settings.insert(QLatin1String("Update XMP Creation Time"),      prm.updXMPDate);

    settings.insert(QLatin1String("Use Timestamp Type"),            prm.dateSource);
    settings.insert(QLatin1String("Meta Timestamp Type"),           prm.metadataSource);
    settings.insert(QLatin1String("File Timestamp Type"),           prm.fileDateSource);

    return settings;
}

void TimeAdjust::slotAssignSettings2Widget()
{
    m_changeSettings   = false;

    TimeAdjustContainer prm;

    prm.customDate     = settings()[QLatin1String("Custom Date")].toDateTime();
    prm.customTime     = settings()[QLatin1String("Custom Time")].toDateTime();

    prm.adjustmentType = settings()[QLatin1String("Adjustment Type")].toInt();
    prm.adjustmentDays = settings()[QLatin1String("Adjustment Days")].toInt();
    prm.adjustmentTime = settings()[QLatin1String("Adjustment Time")].toDateTime();

    prm.updFileModDate = settings()[QLatin1String("Update File Modification Time")].toBool();
    prm.updEXIFModDate = settings()[QLatin1String("Update EXIF Modification Time")].toBool();
    prm.updEXIFOriDate = settings()[QLatin1String("Update EXIF Original Time")].toBool();
    prm.updEXIFDigDate = settings()[QLatin1String("Update EXIF Digitization Time")].toBool();
    prm.updEXIFThmDate = settings()[QLatin1String("Update EXIF Thumbnail Time")].toBool();
    prm.updIPTCDate    = settings()[QLatin1String("Update IPTC Time")].toBool();
    prm.updXMPDate     = settings()[QLatin1String("Update XMP Creation Time")].toBool();

    prm.dateSource     = settings()[QLatin1String("Use Timestamp Type")].toInt();
    prm.metadataSource = settings()[QLatin1String("Meta Timestamp Type")].toInt();
    prm.fileDateSource = settings()[QLatin1String("File Timestamp Type")].toInt();

    m_taWidget->setSettings(prm);

    m_changeSettings   = true;
}

void TimeAdjust::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        TimeAdjustContainer prm = m_taWidget->settings();
        BatchToolSettings settings;

        settings.insert(QLatin1String("Custom Date"),                   prm.customDate);
        settings.insert(QLatin1String("Custom Time"),                   prm.customTime);

        settings.insert(QLatin1String("Adjustment Type"),               prm.adjustmentType);
        settings.insert(QLatin1String("Adjustment Days"),               prm.adjustmentDays);
        settings.insert(QLatin1String("Adjustment Time"),               prm.adjustmentTime);

        settings.insert(QLatin1String("Update File Modification Time"), prm.updFileModDate);
        settings.insert(QLatin1String("Update EXIF Modification Time"), prm.updEXIFModDate);
        settings.insert(QLatin1String("Update EXIF Original Time"),     prm.updEXIFOriDate);
        settings.insert(QLatin1String("Update EXIF Digitization Time"), prm.updEXIFDigDate);
        settings.insert(QLatin1String("Update EXIF Thumbnail Time"),    prm.updEXIFThmDate);
        settings.insert(QLatin1String("Update IPTC Time"),              prm.updIPTCDate);
        settings.insert(QLatin1String("Update XMP Creation Time"),      prm.updXMPDate);

        settings.insert(QLatin1String("Use Timestamp Type"),            prm.dateSource);
        settings.insert(QLatin1String("Meta Timestamp Type"),           prm.metadataSource);
        settings.insert(QLatin1String("File Timestamp Type"),           prm.fileDateSource);

        BatchTool::slotSettingsChanged(settings);
    }
}

bool TimeAdjust::toolOperations()
{
    bool metaLoadState = true;
    DMetadata meta;

    if (image().isNull())
    {
        metaLoadState = meta.load(inputUrl().toLocalFile());
    }
    else
    {
        meta.setData(image().getMetadata());
    }

    TimeAdjustContainer prm;

    prm.customDate       = settings()[QLatin1String("Custom Date")].toDateTime();
    prm.customTime       = settings()[QLatin1String("Custom Time")].toDateTime();

    prm.adjustmentType   = settings()[QLatin1String("Adjustment Type")].toInt();
    prm.adjustmentDays   = settings()[QLatin1String("Adjustment Days")].toInt();
    prm.adjustmentTime   = settings()[QLatin1String("Adjustment Time")].toDateTime();

    prm.updFileModDate   = settings()[QLatin1String("Update File Modification Time")].toBool();
    prm.updEXIFModDate   = settings()[QLatin1String("Update EXIF Modification Time")].toBool();
    prm.updEXIFOriDate   = settings()[QLatin1String("Update EXIF Original Time")].toBool();
    prm.updEXIFDigDate   = settings()[QLatin1String("Update EXIF Digitization Time")].toBool();
    prm.updEXIFThmDate   = settings()[QLatin1String("Update EXIF Thumbnail Time")].toBool();
    prm.updIPTCDate      = settings()[QLatin1String("Update IPTC Time")].toBool();
    prm.updXMPDate       = settings()[QLatin1String("Update XMP Creation Time")].toBool();

    prm.dateSource       = settings()[QLatin1String("Use Timestamp Type")].toInt();
    prm.metadataSource   = settings()[QLatin1String("Meta Timestamp Type")].toInt();
    prm.fileDateSource   = settings()[QLatin1String("File Timestamp Type")].toInt();

    bool metadataChanged = prm.updEXIFModDate || prm.updEXIFOriDate ||
                           prm.updEXIFDigDate || prm.updEXIFThmDate ||
                           prm.updIPTCDate    || prm.updXMPDate;

    QDateTime orgDateTime;

    switch (prm.dateSource)
    {
        case TimeAdjustContainer::METADATADATE:
        {
            switch (prm.metadataSource)
            {
                case TimeAdjustContainer::EXIFIPTCXMP:
                    orgDateTime = meta.getImageDateTime();
                    break;
                case TimeAdjustContainer::EXIFCREATED:
                    orgDateTime = QDateTime::fromString(meta.getExifTagString("Exif.Image.DateTime"),
                                                        QLatin1String("yyyy:MM:dd hh:mm:ss"));
                    break;
                case TimeAdjustContainer::EXIFORIGINAL:
                    orgDateTime = QDateTime::fromString(meta.getExifTagString("Exif.Photo.DateTimeOriginal"),
                                                        QLatin1String("yyyy:MM:dd hh:mm:ss"));
                    break;
                case TimeAdjustContainer::EXIFDIGITIZED:
                    orgDateTime = QDateTime::fromString(meta.getExifTagString("Exif.Photo.DateTimeDigitized"),
                                                        QLatin1String("yyyy:MM:dd hh:mm:ss"));
                    break;
                case TimeAdjustContainer::IPTCCREATED:
                    orgDateTime = QDateTime(QDate::fromString(meta.getIptcTagString("Iptc.Application2.DateCreated"),
                                                              Qt::ISODate),
                                            QTime::fromString(meta.getIptcTagString("Iptc.Application2.TimeCreated"),
                                                              Qt::ISODate));
                    break;
                case TimeAdjustContainer::XMPCREATED:
                    orgDateTime = QDateTime::fromString(meta.getXmpTagString("Xmp.xmp.CreateDate"),
                                                        QLatin1String("yyyy:MM:ddThh:mm:ss"));
                    break;
                default:
                    // orgDateTime stays invalid
                    break;
            };
            break;
        }
        case TimeAdjustContainer::CUSTOMDATE:
        {
            orgDateTime = QDateTime(prm.customDate.date(), prm.customTime.time());
            break;
        }
        case TimeAdjustContainer::FILEDATE:
        {
            orgDateTime = imageInfo().modDateTime();
            break;
        }
        default: // TimeAdjustContainer::APPDATE
        {
            orgDateTime = imageInfo().dateTime();
            break;
        }
    }

    if (!metaLoadState && prm.dateSource != TimeAdjustContainer::CUSTOMDATE)
    {
        orgDateTime = imageInfo().modDateTime();
    }

    QDateTime dt = prm.calculateAdjustedDate(orgDateTime);

    if (!dt.isValid())
    {
        return false;
    }

    if (metadataChanged && metaLoadState)
    {

        if (prm.updEXIFModDate)
        {
            meta.setExifTagString("Exif.Image.DateTime",
                                  dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }

        if (prm.updEXIFOriDate)
        {
            meta.setExifTagString("Exif.Photo.DateTimeOriginal",
                                  dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }

        if (prm.updEXIFDigDate)
        {
            meta.setExifTagString("Exif.Photo.DateTimeDigitized",
                                  dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }

        if (prm.updEXIFThmDate)
        {
            meta.setExifTagString("Exif.Image.PreviewDateTime",
                                  dt.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }

        if (prm.updIPTCDate)
        {
            meta.setIptcTagString("Iptc.Application2.DateCreated",
                                  dt.date().toString(Qt::ISODate));
            meta.setIptcTagString("Iptc.Application2.TimeCreated",
                                  dt.time().toString(Qt::ISODate));
        }

        if (prm.updXMPDate && meta.supportXmp())
        {
            meta.setXmpTagString("Xmp.exif.DateTimeOriginal",
                                 dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.photoshop.DateCreated",
                                 dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.tiff.DateTime",
                                 dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.xmp.CreateDate",
                                 dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.xmp.MetadataDate",
                                 dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
            meta.setXmpTagString("Xmp.xmp.ModifyDate",
                                 dt.toString(QLatin1String("yyyy:MM:ddThh:mm:ss")));
        }
    }

    bool ret = true;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret = QFile::copy(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (ret && metadataChanged && metaLoadState)
        {
            ret = meta.save(outputUrl().toLocalFile());
        }
    }
    else
    {
        if (metadataChanged)
        {
            image().setMetadata(meta.data());
        }

        ret = savefromDImg();
    }

    if (ret && prm.updFileModDate)
    {
        // Since QFileInfo does not support timestamp updates, see Qt suggestion #79427 at
        // http://www.qtsoftware.com/developer/task-tracker/index_html?id=79427&method=entry
        // we have to use the utime() system call.

        int modtime;
        QDateTime unixDate;
        unixDate.setDate(QDate(1970, 1, 1));
        unixDate.setTime(QTime(0, 0, 0, 0));

        if (dt < unixDate)
            modtime = -(dt.secsTo(unixDate) + (60 * 60));
        else
            modtime = dt.toTime_t();

        utimbuf times;
        times.actime  = QDateTime::currentDateTime().toTime_t();
        times.modtime = modtime;

        if (utime(QFile::encodeName(outputUrl().toLocalFile()).constData(), &times) != 0)
        {
            ret = false;
        }
    }

    return ret;
}

} // namespace Digikam
