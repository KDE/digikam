/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RESTORATIONTOOL_H
#define RESTORATIONTOOL_H

// Qt includes.

#include <qstring.h>

// Digikam includes.

#include "editortool.h"

class QComboBox;
class QTabWidget;

namespace Digikam
{
class GreycstorationWidget;
class EditorToolSettings;
class ImagePanelWidget;
}

namespace DigikamRestorationImagesPlugin
{

class RestorationTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    RestorationTool(QObject* parent);
    ~RestorationTool();

private slots:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void processCImgURL(const QString&);
    void slotResetValues(int);

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    enum RestorationFilteringPreset
    {
        NoPreset=0,
        ReduceUniformNoise,
        ReduceJPEGArtefacts,
        ReduceTexturing
    };

    QTabWidget                    *m_mainTab;

    QComboBox                     *m_restorationTypeCB;

    Digikam::GreycstorationWidget *m_settingsWidget;

    Digikam::ImagePanelWidget     *m_previewWidget;

    Digikam::EditorToolSettings   *m_gboxSettings;
};

}  // NameSpace DigikamRestorationImagesPlugin

#endif /* RESTORATIONTOOL_H */
