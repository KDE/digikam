/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
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

#ifndef RAINDROPTOOL_H
#define RAINDROPTOOL_H

// Digikam includes.

#include "editortool.h"

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class ImageWidget;
}

namespace DigikamRainDropImagesPlugin
{

class RainDropTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    RainDropTool(QObject *parent);
    ~RainDropTool();

private slots:

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

    KDcrawIface::RIntNumInput   *m_dropInput;
    KDcrawIface::RIntNumInput   *m_amountInput;
    KDcrawIface::RIntNumInput   *m_coeffInput;

    Digikam::ImageWidget        *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamRainDropImagesPlugin

#endif /* RAINDROPTOOL_H */
