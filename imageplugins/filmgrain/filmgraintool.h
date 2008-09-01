/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film 
 *               grain on an image.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FILMGRAINTOOL_H
#define FILMGRAINTOOL_H

// Digikam includes.

#include "editortool.h"

class QSlider;
class QLCDNumber;

namespace Digikam
{
class EditorToolSettings;
class ImagePanelWidget;
}

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrainTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    FilmGrainTool(QObject* parent);
    ~FilmGrainTool();

private slots:

    void slotSliderMoved(int);
    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QSlider                     *m_sensibilitySlider;

    QLCDNumber                  *m_sensibilityLCDValue;

    Digikam::ImagePanelWidget   *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamFilmGrainImagesPlugin

#endif /* FILMGRAINTOOL_H */
