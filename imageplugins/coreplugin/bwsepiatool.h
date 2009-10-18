/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef BWSEPIATOOL_H
#define BWSEPIATOOL_H

// Qt includes

#include <QString>

// Local includes

#include "dimg.h"
#include "editortool.h"

class QListWidget;
class QToolButton;
class QButtonGroup;
class QPushButton;

class KTabWidget;

namespace KDcrawIface
{
class RIntNumInput;
}

namespace Digikam
{
class HistogramWidget;
class ColorGradientWidget;
class ImageWidget;
class DColor;
class CurvesBox;
class EditorToolSettings;
}

namespace DigikamImagesPluginCore
{

class PreviewPixmapFactory;
class BWSepiaToolPriv;

class BWSepiaTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    BWSepiaTool(QObject *parent);
    ~BWSepiaTool();

    friend class PreviewPixmapFactory;

private:

    void readSettings();
    void writeSettings();
    void blackAndWhiteConversion(uchar *data, int w, int h, bool sb, int type);
    void updatePreviews();
    void finalRendering();
    void blockWidgetSignals(bool b);
    QPixmap getThumbnailForEffect(int type);

private Q_SLOTS:

    void slotResetSettings();
    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotEffect();
    void slotScaleChanged();
    void slotSpotColorChanged(const Digikam::DColor& color);
    void slotColorSelectedFromTarget(const Digikam::DColor& color);
    void slotFilterSelected();

private:

    BWSepiaToolPriv* const d;
};

}  // namespace DigikamImagesPluginCore

#endif /* BWSEPIATOOL_H */
