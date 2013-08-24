/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 09-08-2013
 * Description : Showfoto icon view tool tip
 *
 *Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

// KDE includes

#include <klocale.h>
#include <kfileitem.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes
#include "ditemtooltip.h"
#include "showfotoiteminfo.h"

using namespace Digikam;

namespace ShowFoto
{

QString ShowfotoToolTipFiller::ShowfotoItemInfoTipContents(const ShowfotoItemInfo& info)
{
    QString str;
    KSharedConfigPtr config         = KGlobal::config();
    KConfigGroup group              = config->group("ImageViewer Settings");
    DToolTipStyleSheet cnt(group.readEntry("ToolTips Font"));

    PhotoInfoContainer photoInfo   = info.photoInfo;
    QString tip                    = cnt.tipHeader;

    // -- File properties ----------------------------------------------

    if (group.readEntry("ToolTips Show File Name",         true)   ||
        group.readEntry("ToolTips Show File Date",         false)  ||
        group.readEntry("ToolTips Show File Size",         false)  ||
        group.readEntry("ToolTips Show File Type",        false)  ||
        group.readEntry("ToolTips Show File Dim",         true) )
    {
        tip += cnt.headBeg + i18n("File Properties") + cnt.headEnd;

        if (group.readEntry("ToolTips Show File Name",         true))
        {
            tip += cnt.cellBeg + i18nc("filename",
                                       "Name:") + cnt.cellMid;
            tip += info.name + cnt.cellEnd;
        }

        if (group.readEntry("ToolTips Show File Date",         false))
        {
            QDateTime createdDate  = info.dtime;
            str                     = KGlobal::locale()->formatDateTime(createdDate, KLocale::ShortDate, true);
            tip                    += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (group.readEntry("ToolTips Show File Size",         false))
        {
            tip += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            QString localeFileSize = KGlobal::locale()->formatNumber(info.size, 0);
            str = i18n("%1 (%2)", KIO::convertSize(info.size), localeFileSize);
            tip += str + cnt.cellEnd;
        }

        if(group.readEntry("ToolTips Show Image Type",         true))
        {
            tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + info.mime + cnt.cellEnd;
        }

        if(group.readEntry("ToolTips Show Image Dim",         false))
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
    // NOTE: these info require \"Use File Metadata\" option from Camera Setup Behavior page.

    if (group.readEntry("ToolTips Show Photo Make",  true)  ||
        group.readEntry("ToolTips Show Photo Focal", true) ||
        group.readEntry("ToolTips Show Photo Expo",  true)  ||
        group.readEntry("ToolTips Show Photo Flash", false) ||
        group.readEntry("ToolTips Show Photo WB",    false) ||
        group.readEntry("ToolTips Show Photo Date",  true))
    {
        if (!photoInfo.isNull())
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (group.readEntry("ToolTips Show Photo Make",  true))
            {
                str = QString("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                      .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (group.readEntry("ToolTips Show Photo Date",  true))
            {
                if (info.ctime.isValid())
                {
                    QDateTime createdDate  = info.ctime;
                    str                     = KGlobal::locale()->formatDateTime(createdDate, KLocale::ShortDate, true);
                    tip                    += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
                }
                else
                {
                    metaStr += cnt.cellBeg + i18nc("creation date of the image",
                                                   "Created:") + cnt.cellMid + Qt::escape(cnt.unavailable) + cnt.cellEnd;
                }
            }

            if (group.readEntry("ToolTips Show Photo Focal", true))
            {
                str = photoInfo.aperture.isEmpty() ? cnt.unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                {
                    str += QString(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                }
                else
                {
                    str += QString(" / %1").arg(i18n("%1 (35mm: %2)",photoInfo.focalLength, photoInfo.focalLength35mm));
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (group.readEntry("ToolTips Show Photo Expo",  true))
            {
                str = QString("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable : photoInfo.exposureTime)
                      .arg(photoInfo.sensitivity.isEmpty() ? cnt.unavailable : i18n("%1 ISO",photoInfo.sensitivity));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("Exposure/Sensitivity:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (group.readEntry("ToolTips Show Photo Flash", false))
            {
                str = photoInfo.flash.isEmpty() ? cnt.unavailable : photoInfo.flash;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18nc("camera flash settings",
                                               "Flash:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            if (group.readEntry("ToolTips Show Photo WB",    false))
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + "...";
                }

                metaStr += cnt.cellBeg + i18n("White Balance:") + cnt.cellMid + Qt::escape(str) + cnt.cellEnd;
            }

            tip += metaStr;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

}  // namespace ShowFoto


