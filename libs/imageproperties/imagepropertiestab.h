/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPROPERTIESTAB_H
#define IMAGEPROPERTIESTAB_H

// Qt includes

#include <QString>
#include <QColor>

// KDE includes

#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ImagePropertiesTab : public KDcrawIface::RExpanderBox
{
    Q_OBJECT

public:

    ImagePropertiesTab(QWidget* parent);
    ~ImagePropertiesTab();

    void setCurrentURL(const KUrl& url=KUrl());

    void setPhotoInfoDisable(const bool b);
    void showOrHideCaptionAndTags();

    void setFileModifiedDate(const QString& str);
    void setFileSize(const QString& str);
    void setFileOwner(const QString& str);
    void setFilePermissions(const QString& str);

    void setImageDimensions(const QString& str);
    void setImageMime(const QString& str);
    void setImageCompression(const QString& str);
    void setImageBitDepth(const QString& str);
    void setImageColorMode(const QString& str);
    void hideImageCompression();

    void setPhotoMake(const QString& str);
    void setPhotoModel(const QString& str);
    void setPhotoDateTime(const QString& str);
    void setPhotoLens(const QString& str);
    void setPhotoAperture(const QString& str);
    void setPhotoFocalLength(const QString& str);
    void setPhotoExposureTime(const QString& str);
    void setPhotoSensitivity(const QString& str);
    void setPhotoExposureMode(const QString& str);
    void setPhotoFlash(const QString& str);
    void setPhotoWhiteBalance(const QString& str);

    void setCaption(const QString& str);
    void setPickLabel(int pickId);
    void setColorLabel(int colorId);
    void setRating(int rating);
    void setTags(const QStringList& tagPaths, const QStringList& tagNames = QStringList());

private:

    class ImagePropertiesTabPriv;
    ImagePropertiesTabPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEPROPERTIESTAB_H */
