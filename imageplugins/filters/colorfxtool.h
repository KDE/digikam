/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin for to apply a color
 *               effect to an image.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COLORFXTOOL_H
#define COLORFXTOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace Digikam
{
    class DColor;
}

namespace DigikamFxFiltersImagePlugin
{

class ColorFxTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ColorFxTool(QObject* const parent);
    ~ColorFxTool();

private:

    void readSettings();
    void writeSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();
    void renderingFinished();

private Q_SLOTS:

    void slotResetSettings();
    void slotColorSelectedFromTarget(const Digikam::DColor& color);

private:

    class Private;
    Private* const d;
};

}  // namespace DigikamFxFiltersImagePlugin

#endif /* COLORFXTOOL_H */
