/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizing tool.
 *
 * Copyright (C) 2009      by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class ContentAwareResizeTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit ContentAwareResizeTool(QObject* const parent);
    ~ContentAwareResizeTool();

private:

    void writeSettings();
    void readSettings();
    void preparePreview();
    void prepareFinal();
    void setPreviewImage();
    void setFinalImage();
    void renderingFinished();
    void blockWidgetSignals(bool b);
    void disableSettings();
    void contentAwareResizeCore(DImg* const image, int target_width, int target_height, const QImage& mask);
    void enableMaskSettings(bool b);
    void enableContentAwareSettings(bool b);
    bool eventFilter(QObject*, QEvent*);

private Q_SLOTS:

    void slotResetSettings();
    void slotValuesChanged();
    void slotMixedRescaleValueChanged();
    void slotMaskColorChanged(int);
    void slotWeightMaskBoxStateChanged(int);
    void slotMaskPenSizeChanged(int);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /*CONTENT_AWARE_RESIZE_TOOL_H*/
