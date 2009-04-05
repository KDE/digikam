/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef AUTOCORRECTIONTOOL_H
#define AUTOCORRECTIONTOOL_H

// Local includes

#include "dimg.h"
#include "editortool.h"

class QCheckBox;
class QLabel;
class QWidget;


namespace Digikam
{
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamAutoCorrectionImagesPlugin
{

class KLFDeviceSelector;

class AutoCorrectionTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    AutoCorrectionTool(QObject *parent);
    ~AutoCorrectionTool();

private Q_SLOTS:

    void slotSetFilters();
    void slotLensChanged();
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

    QLabel                      *m_maskPreviewLabel;

    QCheckBox                   *m_showGrid;
    QCheckBox                   *m_filterCCA;
    QCheckBox                   *m_filterVig;
    QCheckBox                   *m_filterCCI;
    QCheckBox                   *m_filterDist;
    QCheckBox                   *m_filterGeom;

    KLFDeviceSelector           *m_cameraSelector;

    Digikam::ImageWidget        *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // namespace DigikamAutoCorrectionImagesPlugin

#endif /* AUTOCORRECTIONTOOL_H */
