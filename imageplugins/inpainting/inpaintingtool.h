/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
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

#ifndef INPAINTINGTOOL_H
#define INPAINTINGTOOL_H

// Qt includes.

#include <qimage.h>
#include <qrect.h>
#include <qstring.h>

// KDE includes.

#include <kpassivepopup.h>

// Digikam includes.

#include "dimg.h"
#include "editortool.h"

class QTabWidget;
class QComboBox;

namespace Digikam
{
class GreycstorationWidget;
class ImageWidget;
class EditorToolSettings;
}

namespace DigikamInPaintingImagesPlugin
{

class InPaintingPassivePopup : public KPassivePopup
{
public:

    InPaintingPassivePopup(QWidget* parent) : KPassivePopup(parent), m_parent(parent) {}

protected:

    virtual void positionSelf() { move(m_parent->x() + 30, m_parent->y() + 30); }

private:

    QWidget* m_parent;
};

//-----------------------------------------------------------

class InPaintingTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    InPaintingTool(QObject* parent);
    ~InPaintingTool();

private slots:

    void processCImgURL(const QString&);
    void slotResetValues(int);
    void slotResetSettings();
    void slotSaveAsSettings();
    void slotLoadSettings();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    enum InPaintingFilteringPreset
    {
        NoPreset=0,
        RemoveSmallArtefact,
        RemoveMediumArtefact,
        RemoveLargeArtefact
    };

    bool                           m_isComputed;

    QRect                          m_maskRect;

    QImage                         m_maskImage;

    QComboBox                     *m_inpaintingTypeCB;

    QTabWidget                    *m_mainTab;

    Digikam::DImg                  m_originalImage;
    Digikam::DImg                  m_cropImage;

    Digikam::GreycstorationWidget *m_settingsWidget;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // NameSpace DigikamInPaintingImagesPlugin

#endif /* INPAINTINGTOOL_H */
