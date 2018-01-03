/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : slide show settings container.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDESHOW_SETTINGS_H
#define SLIDESHOW_SETTINGS_H

// Qt includes

#include <QMap>
#include <QColor>
#include <QString>
#include <QUrl>

// Local includes

#include "infocontainer.h"
#include "digikam_export.h"
#include "previewsettings.h"

namespace Digikam
{

/** This class contains the information of one picture to slide
 */
class DIGIKAM_EXPORT SlidePictureInfo
{

public:

    explicit SlidePictureInfo()
    {
        rating     = 0;
        colorLabel = 0;
        pickLabel  = 0;
    };

    virtual ~SlidePictureInfo()
    {
    };

public:

    int                rating;
    int                colorLabel;
    int                pickLabel;

    /** Picture Comment
     */
    QString            comment;

    /** Picture Title
     */
    QString            title;

    /** Picture Tag names
     */
    QStringList        tags;

    /** Exif photo info of picture
     */
    PhotoInfoContainer photoInfo;
};

// --------------------------------------------------------------------------------

/** This class contain all settings to perform a slide show of a group of pictures
 */
class DIGIKAM_EXPORT SlideShowSettings
{

public:

    explicit SlideShowSettings();
    virtual ~SlideShowSettings();

    void readFromConfig();
    void writeToConfig();

    int indexOf(const QUrl&) const;
    int count() const;

public:

    // Global Slide Show Settings

    /** Start Slide with current selected item
     */
    bool                         startWithCurrent;

    /** Auto-rotate image accordingly with Exif Rotation tag
     */
    bool                         exifRotate;

    /** Print picture file name while slide
     */
    bool                         printName;

    /** Print picture creation date while slide
     */
    bool                         printDate;

    /** Print camera Aperture and Focal while slide
     */
    bool                         printApertureFocal;

    /** Print camera Make and Model while slide
     */
    bool                         printMakeModel;

    /** Print camera Exposure and Sensitivity while slide
     */
    bool                         printExpoSensitivity;

    /** Print picture comment while slide
     */
    bool                         printComment;

    /** Print image title while slide
     */
    bool                         printTitle;

    /** Print image captions if no title available while slide
     */
    bool                         printCapIfNoTitle;

    /** Print tag names while slide
     */
    bool                         printTags;

    /** Print color label, pick label, and rating while slide
     */
    bool                         printLabels;

    /** Slide pictures in loop
     */
    bool                         loop;

    /** Delay in seconds
     */
    int                          delay;

    /** Whether to enable the auto-move feature.
     */
    bool                         autoPlayEnabled;

    /** Screen to use in case of multi-monitor computer.
     */
    int                          slideScreen;

    /** Show progress indicator
     */
    bool                         showProgressIndicator;

    /** Load images (previews) in full size, not reduced version
     */
    PreviewSettings              previewSettings;

    /** List of pictures URL to slide
     */
    QList<QUrl>                  fileList;

    /** Map of pictures information to slide
     */
    QMap<QUrl, SlidePictureInfo> pictInfoMap;

    /** URL of the first image to show if requested
     */
    QUrl                         imageUrl;

private:

    static const QString         configGroupName;
    static const QString         configSlideShowStartCurrentEntry;
    static const QString         configSlideShowDelayEntry;
    static const QString         configSlideShowLoopEntry;
    static const QString         configSlideShowPrintApertureFocalEntry;
    static const QString         configSlideShowPrintCommentEntry;
    static const QString         configSlideShowPrintTitleEntry;
    static const QString         configSlideShowPrintCapIfNoTitleEntry;
    static const QString         configSlideShowPrintDateEntry;
    static const QString         configSlideShowPrintExpoSensitivityEntry;
    static const QString         configSlideShowPrintMakeModelEntry;
    static const QString         configSlideShowPrintNameEntry;
    static const QString         configSlideShowPrintTagsEntry;
    static const QString         configSlideShowPrintLabelsEntry;
    static const QString         configSlideShowProgressIndicatorEntry;
    static const QString         configSlideScreenEntry;
};

}  // namespace Digikam

#endif  // SLIDESHOW_SETTINGS_H
