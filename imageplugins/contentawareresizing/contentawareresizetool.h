/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizing tool.
 *
 * Copyright (C) 2009 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONTENT_AWARE_RESIZE_TOOL_H
#define CONTENT_AWARE_RESIZE_TOOL_H

// Local includes

#include "editortool.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamContentAwareResizingImagesPlugin
{

class ContentAwareResizeToolPriv;

class ContentAwareResizeTool : public Digikam::EditorToolThreaded
{
    Q_OBJECT

public:

    ContentAwareResizeTool(QObject *parent);
    ~ContentAwareResizeTool();

private:

    void writeSettings();
    void readSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();
    void blockWidgetSignals(bool b);
    void disableSettings();
    void contentAwareResizeCore(DImg *image, int target_width, int target_height, QImage mask);
    void enableContentAwareSettings(bool b);

private Q_SLOTS:

    void slotResetSettings();
    void slotValuesChanged();
    void slotMixedRescaleValueChanged();
    void slotMaskColorChanged(int);
    void slotWeightMaskBoxStateChanged(int);
    void slotMaskPenSizeChanged(int);

private:

    ContentAwareResizeToolPriv* const d;
};

} // namespace DigikamContentAwareResizingImagesPlugin

#endif /*CONTENT_AWARE_RESIZE_TOOL_H*/
