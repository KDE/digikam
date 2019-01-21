/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : slide show settings container.
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_SLIDESHOW_SETTINGS_H
#define DIGIKAM_SLIDESHOW_SETTINGS_H

// Qt includes

#include <QMap>
#include <QColor>
#include <QString>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "previewsettings.h"
#include "previewsettings.h"
#include "dinfointerface.h"

namespace Digikam
{

/** This class contain all settings to perform a slide show of a group of pictures
 */
class DIGIKAM_EXPORT SlideShowSettings
{

public:

    explicit SlideShowSettings();
    ~SlideShowSettings();

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

    /** Print color label and pick label while slide
     */
    bool                         printLabels;

    /** Print rating while slide
     */
    bool                         printRating;

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

    /** URL of the first image to show if requested
     */
    QUrl                         imageUrl;
    
    /** Interface to access to host application data
     */
    DInfoInterface*              iface;

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
    static const QString         configSlideShowPrintRatingEntry;
    static const QString         configSlideShowProgressIndicatorEntry;
    static const QString         configSlideScreenEntry;
};

} // namespace Digikam

#endif // DIGIKAM_SLIDESHOW_SETTINGS_H
