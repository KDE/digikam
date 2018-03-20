/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 09-08-2013
 * Description : Showfoto tool tip filler
 *
 * Copyright (C) 2013 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "showfototooltipfiller.h"

// Qt includes

#include <QDateTime>
#include <QTextDocument>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ditemtooltip.h"
#include "imagepropertiestab.h"
#include "showfotoiteminfo.h"
#include "showfotosettings.h"

using namespace Digikam;

namespace ShowFoto
{

QString ShowfotoToolTipFiller::ShowfotoItemInfoTipContents(const ShowfotoItemInfo& info)
{
    QString str;

    ShowfotoSettings* const settings = ShowfotoSettings::instance();

    DToolTipStyleSheet cnt(settings->getToolTipFont());

    PhotoInfoContainer photoInfo     = info.photoInfo;
    QString tip                      = cnt.tipHeader;

    // -- File properties ----------------------------------------------

    if (settings->getShowFileName()  ||
        settings->getShowFileDate()  ||
        settings->getShowFileSize()  ||
        settings->getShowFileType()  ||
        settings->getShowFileDim() )
    {
        tip += cnt.headBeg + i18n("File Properties") + cnt.headEnd;

        if (settings->getShowFileName())
        {
            tip += cnt.cellBeg + i18nc("filename", "Name:") + cnt.cellMid;
            tip += info.name + cnt.cellEnd;
        }

        if (settings->getShowFileDate())
        {
            QDateTime createdDate  = info.dtime;
            str                    = QLocale().toString(createdDate, QLocale::ShortFormat);
            tip                   += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings->getShowFileSize())
        {
            tip                   += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            QString localeFileSize = QLocale().toString(info.size);
            str                    = i18n("%1 (%2)", ImagePropertiesTab::humanReadableBytesCount(info.size), localeFileSize);
            tip                   += str + cnt.cellEnd;
        }

        if(settings->getShowFileType())
        {
            tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + info.mime + cnt.cellEnd;
        }

        if(settings->getShowFileDim())
        {
            if (info.width == 0 || info.height == 0 || info.width == -1 || info.height == -1)
            {
                str = i18nc("unknown / invalid image dimension",
                            "Unknown");
            }
            else
            {
                QString mpixels;
                mpixels.setNum(info.width*info.height/1000000.0, 'f', 2);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            info.width, info.height, mpixels);
            }

            tip += cnt.cellBeg + i18n("Dimensions:") + cnt.cellMid + str + cnt.cellEnd;
        }
    }

    // -- Photograph Info -----------------------------------------------------------------------

    if (settings->getShowPhotoMake()  ||
        settings->getShowPhotoLens()  ||
        settings->getShowPhotoFocal() ||
        settings->getShowPhotoExpo()  ||
        settings->getShowPhotoFlash() ||
        settings->getShowPhotoWB()    ||
        settings->getShowPhotoDate()  ||
        settings->getShowPhotoMode())
    {
        if (!photoInfo.isNull())
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (settings->getShowPhotoMake())
            {
                ImagePropertiesTab::shortenedMakeInfo(photoInfo.make);
                ImagePropertiesTab::shortenedModelInfo(photoInfo.model);

                str = QString::fromUtf8("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                                                  .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getShowPhotoLens())
            {
                str          = photoInfo.lens.isEmpty() ? cnt.unavailable : photoInfo.lens;
                QString lens = i18nc("camera lens", "Lens:");

                if (str.length() > cnt.maxStringLength)
                {
                    int space = str.lastIndexOf(QLatin1Char(' '), cnt.maxStringLength);

                    if (space == -1)
                        space = cnt.maxStringLength;

                    metaStr += cnt.cellBeg + lens + cnt.cellMid + str.left(space).toHtmlEscaped() + cnt.cellEnd;

                    str  = str.mid(space+1);
                    lens = QString();
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + lens + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getShowPhotoDate())
            {
                if (info.ctime.isValid())
                {
                    QDateTime createdDate  = info.ctime;
                    str                    = QLocale().toString(createdDate, QLocale::ShortFormat);
                    tip                   += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
                }
                else
                {
                    metaStr += cnt.cellBeg + i18nc("creation date of the image",
                                                   "Created:") + cnt.cellMid + cnt.unavailable.toHtmlEscaped() + cnt.cellEnd;
                }
            }

            if (settings->getShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? cnt.unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                {
                    str += QString::fromUtf8(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                }
                else
                {
                    str += QString::fromUtf8(" / %1").arg(i18n("%1 (%2)",photoInfo.focalLength, photoInfo.focalLength35mm));
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getShowPhotoExpo())
            {
                str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable : photoInfo.exposureTime)
                                                  .arg(photoInfo.sensitivity.isEmpty()  ? cnt.unavailable : i18n("%1 ISO",photoInfo.sensitivity));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Exposure/Sensitivity:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getShowPhotoMode())
            {
                if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                {
                    str = cnt.unavailable;
                }
                else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
                {
                    str = photoInfo.exposureMode;
                }
                else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
                {
                    str = photoInfo.exposureProgram;
                }
                else
                {
                    str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Mode/Program:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getShowPhotoFlash())
            {
                str = photoInfo.flash.isEmpty() ? cnt.unavailable : photoInfo.flash;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18nc("camera flash settings",
                                               "Flash:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("White Balance:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            tip += metaStr;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

}  // namespace ShowFoto
