/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * Copyright (C) 2007-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "advprintsettings.h"

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

AdvPrintSettings::AdvPrintSettings()
{
    selMode            = IMAGES;

    // Select a different page to force a refresh in initPhotoSizes.
    pageSize           = QSizeF(-1, -1);

    iconSize           = QSize(24, 24);
    currentPreviewPage = 0;
    currentCropPhoto   = 0;
    disableCrop        = false;
    imageFormat        = JPEG;
    printerName        = outputName(PDF);
    captions           = 0;
    captionColor       = QColor(Qt::yellow);
    captionFont        = QFont(QLatin1String("Sans Serif"));
    captionSize        = 4;
    outputLayouts      = 0;
    outputPrinter      = 0;
}

AdvPrintSettings::~AdvPrintSettings()
{
    for (int i = 0 ; i < photos.count() ; ++i)
    {
        delete photos.at(i);
    }

    photos.clear();
}

void AdvPrintSettings::readSettings(KConfigGroup& group)
{
    selMode           = (Selection)group.readEntry("SelMode",
                        (int)IMAGES);
    imageFormat       = (ImageFormat)group.readEntry("ImageFormat",
                        (int)JPEG);
    savedPhotoSize    = group.readEntry("PhotoSize",
                        QString());
    iconSize          = group.readEntry("IconSize",
                        QSize(24, 24));
    printerName       = group.readEntry("Printer",
                        outputName(PDF));
    captions          = group.readEntry(QLatin1String("Captions"),
                        0);
    captionColor      = group.readEntry(QLatin1String("CaptionColor"),
                        QColor(Qt::yellow));
    captionFont       = group.readEntry(QLatin1String("CaptionFont"),
                        QFont(QLatin1String("Sans Serif")));
    captionSize       = group.readEntry(QLatin1String("CaptionSize"),
                        4);
    captionTxt        = group.readEntry(QLatin1String("CustomCaption"),
                        QString());
    outputDir         = group.readEntry("OutputPath",
                        QUrl::fromLocalFile(QStandardPaths::writableLocation
                            (QStandardPaths::DocumentsLocation)));
    conflictRule      = (FileSaveConflictBox::ConflictRule)group.readEntry("ConflictRule",
                        (int)FileSaveConflictBox::OVERWRITE);
    openInFileBrowser = group.readEntry("OpenInFileBrowser",
                        true);
    imageFormat       = (ImageFormat)group.readEntry("ImageFormat",
                        (int)JPEG);
}

void AdvPrintSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry("SelMode",           (int)selMode);
    group.writeEntry("ImageFormat",       (int)imageFormat);
    group.writeEntry("PhotoSize",         savedPhotoSize);
    group.writeEntry("IconSize",          iconSize);
    group.writeEntry("Printer",           printerName);
    group.writeEntry("Captions",          captions);
    group.writeEntry("CaptionColor",      captionColor);
    group.writeEntry("CaptionFont",       captionFont);
    group.writeEntry("CaptionSize",       captionSize);
    group.writeEntry("CustomCaption",     captionTxt);
    group.writeEntry("OutputPath",        outputDir);
    group.writeEntry("ConflictRule",      (int)conflictRule);
    group.writeEntry("OpenInFileBrowser", openInFileBrowser);
    group.writeEntry("ImageFormat",       (int)imageFormat);
}

QString AdvPrintSettings::outputName(Output out) const
{
    QMap<Output, QString> outputs = outputNames();

    if (outputs.contains(out))
    {
        return outputs[out];
    }

    return QString();
}

QMap<AdvPrintSettings::Output, QString> AdvPrintSettings::outputNames()
{
    QMap<Output, QString> out;

    out[PDF]   = i18nc("Output: PDF",  "Print to PDF");
    out[FILES] = i18nc("Output: FILE", "Print to Image File");
    out[GIMP]  = i18nc("Output: GIMP", "Print with Gimp");

    return out;
}

QString AdvPrintSettings::format() const
{
    if (imageFormat == JPEG)
    {
        return QLatin1String("JPEG");
    }
    else if (imageFormat == TIFF)
    {
        return QLatin1String("TIF");
    }

    return QLatin1String("PNG");
}

QMap<AdvPrintSettings::ImageFormat, QString> AdvPrintSettings::imageFormatNames()
{
    QMap<ImageFormat, QString> frms;

    frms[JPEG] = i18nc("Image format: JPEG", "JPEG");
    frms[PNG]  = i18nc("Image format: PNG",  "PNG");
    frms[TIFF] = i18nc("Image format: TIFF", "TIFF");

    return frms;
}

} // namespace Digikam
