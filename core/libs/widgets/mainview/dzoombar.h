/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : a zoom bar used in status bar.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DZOOM_BAR_H
#define DZOOM_BAR_H

// Qt includes

#include <QString>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

class QAction;


namespace Digikam
{

class DIGIKAM_EXPORT DZoomBar : public DHBox
{
    Q_OBJECT

public:

    enum BarMode
    {
        PreviewZoomCtrl=0,      // Preview Zoom controller.
        ThumbsSizeCtrl,         // Thumb Size controller. Preview zoom controller still visible but disabled.
        NoPreviewZoomCtrl       // Thumb Size controller alone. Preview Zoom controller is hidden.
    };

public:

    explicit DZoomBar(QWidget* const parent=0);
    ~DZoomBar();

    void setBarMode(BarMode mode);
    void setZoom(double zoom, double zmin, double zmax);
    void setThumbsSize(int size);

    void setZoomToFitAction(QAction* const action);
    void setZoomTo100Action(QAction* const action);
    void setZoomPlusAction(QAction* const action);
    void setZoomMinusAction(QAction* const action);

    void triggerZoomTrackerToolTip();

    static int    sizeFromZoom(double zoom, double zmin, double zmax);
    static double zoomFromSize(int size, double zmin, double zmax);

Q_SIGNALS:

    void signalZoomSliderChanged(int);
    void signalDelayedZoomSliderChanged(int);
    void signalZoomSliderReleased(int);
    void signalZoomValueEdited(double);

public Q_SLOTS:

    void slotUpdateTrackerPos();

private Q_SLOTS:

    void slotZoomSliderChanged(int);
    void slotDelayedZoomSliderChanged();
    void slotZoomSliderReleased();
    void slotZoomSelected(int);
    void slotZoomTextChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DZOOM_BAR_H
