/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef BORDERTOOL_H
#define BORDERTOOL_H

// Qt includes

#include <QString>

// Local includes

#include "editortool.h"

class QColor;

namespace DigikamBorderImagesPlugin
{

class BorderToolPriv;

class BorderTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    BorderTool(QObject *parent);
    ~BorderTool();

private Q_SLOTS:

    void slotResetSettings();
    void slotPreserveAspectRatioToggled(bool);
    void slotBorderTypeChanged(int borderType);
    void slotColorForegroundChanged(const QColor& color);
    void slotColorBackgroundChanged(const QColor& color);

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();
    void toggleBorderSlider(bool b);
    void blockWidgetSignals(bool b);

    QString getBorderPath(int border);

private:

    BorderToolPriv* const d;
};

}  // namespace DigikamBorderImagesPlugin

#endif /* BORDERTOOL_H */
