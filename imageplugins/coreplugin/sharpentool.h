/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to sharp an image
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHARPENTOOL_H
#define SHARPENTOOL_H

// Local includes

#include "editortool.h"

namespace DigikamImagesPluginCore
{

class SharpenToolPriv;

class SharpenTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    SharpenTool(QObject *parent);
    ~SharpenTool();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotSharpMethodActived(int);

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void abortPreview();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();
    void blockWidgetSignals(bool b);

private:

    enum SharpingMethods
    {
        SimpleSharp = 0,
        UnsharpMask,
        Refocus
    };

private:

    SharpenToolPriv* const d;
};

}  // namespace DigikamImagesPluginCore

#endif /* SHARPENTOOL_H */
