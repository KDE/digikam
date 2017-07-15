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

#ifndef ADV_PRINT_SETTING_H
#define ADV_PRINT_SETTING_H

// Qt includes

#include <QtGlobal>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMap>
#include <QColor>
#include <QPrinter>
#include <QFont>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintphoto.h"
#include "filesaveconflictbox.h"

class KConfigGroup;

namespace Digikam
{

class AdvPrintSettings
{

public:

    // Images selection mode
    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    // Print output destination, outside real printers configured
    enum Output
    {
        PDF = 0,
        FILES,
        GIMP
    };

    // Image format for Output::FILES
    enum ImageFormat
    {
        JPEG = 0,
        PNG,
        TIFF
    };

public:

    explicit AdvPrintSettings();
    ~AdvPrintSettings();

    // Read and write settings in config file between sessions.
    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    QString format()               const;
    QString outputName(Output out) const;

    // Helper methods to fill combobox from GUI.
    static QMap<Output,      QString> outputNames();
    static QMap<ImageFormat, QString> imageFormatNames();

public:

    Selection                         selMode;       // Items selection mode

    QList<QUrl>                       inputImages;

    QString                           printerName;
    QSize                             iconSize;

    QSizeF                            pageSize;      // Page Size in mm

    QList<AdvPrintPhoto*>             photos;
    QList<AdvPrintPhotoSize*>         photosizes;

    // Caption management.
    int                               captions;
    QColor                            captionColor;
    QFont                             captionFont;
    int                               captionSize;
    QString                           captionTxt;

    // Crop management
    int                               currentPreviewPage;
    int                               currentCropPhoto;
    bool                              disableCrop;

    // For Print to Gimp only
    QString                           tempPath;
    QStringList                       gimpFiles;
    QString                           gimpPath;
    QString                           savedPhotoSize;

    // For print to image files only.
    ImageFormat                       imageFormat;
    FileSaveConflictBox::ConflictRule conflictRule;  // Rule if output image files already exists.
    QUrl                              outputDir;     // Directory where to store output images.
    bool                              openInFileBrowser;

    // Generic data used by printing thread.
    AdvPrintPhotoSize*                outputLayouts;
    QPrinter*                         outputPrinter;
    QString                           outputPath;
};

} // namespace Digikam

#endif // ADV_PRINT_SETTING_H
