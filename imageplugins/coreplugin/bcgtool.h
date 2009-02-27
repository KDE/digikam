/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-05
 * Description : digiKam image editor to adjust Brightness,
                 Contrast, and Gamma of picture.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BCGTOOL_H
#define BCGTOOL_H

// Local includes.

#include "editortool.h"

namespace KDcrawIface
{
class RIntNumInput;
class RDoubleNumInput;
}

namespace Digikam
{
class DColor;
class EditorToolSettings;
class ImageWidget;
}

namespace DigikamImagesPluginCore
{

class BCGTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    BCGTool(QObject *parent);
    ~BCGTool();

private Q_SLOTS:

    void slotEffect();
    void slotResetSettings();
    void slotColorSelectedFromTarget( const Digikam::DColor &color );

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

private:

    uchar                        *m_destinationPreviewData;

    KDcrawIface::RIntNumInput    *m_bInput;
    KDcrawIface::RIntNumInput    *m_cInput;

    KDcrawIface::RDoubleNumInput *m_gInput;

    Digikam::ImageWidget         *m_previewWidget;

    Digikam::EditorToolSettings  *m_gboxSettings;
};

}  // namespace DigikamImagesPluginCore

#endif /* BCGTOOL_H */
