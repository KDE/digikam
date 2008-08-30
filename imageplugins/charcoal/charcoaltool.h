/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digikam image editor plugin to
 *               simulate charcoal drawing.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CHARCOALTOOL_H
#define CHARCOALTOOL_H

// Local includes.

#include "editortool.h"

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class EditorToolSettings;
class ImagePanelWidget;
}

namespace DigikamCharcoalImagesPlugin
{

class CharcoalTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    CharcoalTool(QObject* parent);
    ~CharcoalTool();

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

    KDcrawIface::RIntNumInput   *m_pencilInput;
    KDcrawIface::RIntNumInput   *m_smoothInput;

    Digikam::ImagePanelWidget   *m_previewWidget;

    Digikam::EditorToolSettings *m_gboxSettings;
};

}  // NameSpace DigikamCharcoalImagesPlugin

#endif /* CHARCOALTOOL_H */
