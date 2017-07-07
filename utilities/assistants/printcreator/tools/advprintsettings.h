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

// Local includes

#include "advprintphoto.h"

class KConfigGroup;

namespace Digikam
{

static const char* const INTRO_PAGE_NAME         = I18N_NOOP("Welcome to Print Creator");
static const char* const PHOTO_PAGE_NAME         = I18N_NOOP("Select page layout");
static const char* const CAPTION_PAGE_NAME       = I18N_NOOP("Caption settings");
static const char* const CROP_PAGE_NAME          = I18N_NOOP("Crop and rotate photos");
static const char* const CUSTOM_PAGE_LAYOUT_NAME = I18N_NOOP("Custom");

class AdvPrintSettings
{

public:

    // Images selection mode
    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

public:

    explicit AdvPrintSettings();
    ~AdvPrintSettings();

public:

    Selection                 selMode;             // Items selection mode

    QList<QUrl>               inputImages;

    // Page Size in mm
    QSizeF                    pageSize;

    QList<AdvPrintPhoto*>     photos;
    QList<AdvPrintPhotoSize*> photosizes;

    QString                   tempPath;
    QStringList               gimpFiles;
    QString                   savedPhotoSize;

    int                       currentPreviewPage;
    int                       currentCropPhoto;
};

} // namespace Digikam

#endif // ADV_PRINT_SETTING_H
