/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PANO_ACTIONS_H
#define PANO_ACTIONS_H

// Qt includes

#include <QString>
#include <QImage>
#include <QMetaType>
#include <QMap>
#include <QUrl>

namespace Digikam
{

enum PanoAction
{
    PANO_NONE = 0,                   // 0
    PANO_PREPROCESS_INPUT,           // 1
    PANO_CREATEPTO,                  // 2
    PANO_CPFIND,                     // 3
    PANO_CPCLEAN,                    // 4
    PANO_OPTIMIZE,                   // 5
    PANO_AUTOCROP,                   // 6
    PANO_CREATEPREVIEWPTO,           // 7
    PANO_CREATEMK,                   // 8
    PANO_CREATEMKPREVIEW,            // 9
    PANO_CREATEFINALPTO,             // 10
    PANO_NONAFILE,                   // 11
    PANO_NONAFILEPREVIEW,            // 12
    PANO_STITCH,                     // 13
    PANO_STITCHPREVIEW,              // 14
    PANO_HUGINEXECUTOR,              // 15
    PANO_HUGINEXECUTORPREVIEW,       // 16
    PANO_COPY                        // 17
};

typedef enum
{
    JPEG,
    TIFF,
    HDR
}
PanoramaFileType;

struct PanoramaPreprocessedUrls
{
    PanoramaPreprocessedUrls()
    {
    }

    PanoramaPreprocessedUrls(const QUrl& preprocessed, const QUrl& preview)
        : preprocessedUrl(preprocessed),
          previewUrl(preview)
    {
    }

    virtual ~PanoramaPreprocessedUrls()
    {
    }

    QUrl preprocessedUrl;              // Can be an original file or a converted version, depending on the original file type
    QUrl previewUrl;                   // The JPEG preview version, accordingly of preprocessedUrl constent.
};

typedef QMap<QUrl, PanoramaPreprocessedUrls> PanoramaItemUrlsMap;   // Map between original Url and processed temp Urls.

// ----------------------------------------------------------------------------------------------------------

struct PanoActionData
{
    PanoActionData()
        : starting(false),
          success(false),
          id(0),
          action(PANO_NONE)
    {
    }

    bool                starting;
    bool                success;

    QString             message;        // Usually, an error message

    int                 id;

    PanoAction          action;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::PanoActionData)
Q_DECLARE_METATYPE(Digikam::PanoramaPreprocessedUrls)

#endif // PANO_ACTIONS_H
