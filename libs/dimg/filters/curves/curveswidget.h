/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : a widget to draw histogram curves
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CURVESWIDGET_H
#define CURVESWIDGET_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "dcolor.h"
#include "digikam_export.h"
#include "globals.h"

namespace Digikam
{

class ImageHistogram;
class ImageCurves;
class CurvesWidgetPriv;

class DIGIKAM_EXPORT CurvesWidget : public QWidget
{
    Q_OBJECT

public:

    CurvesWidget(int w, int h, QWidget* parent, bool readOnly=false);

    CurvesWidget(int w, int h,                         // Widget size.
                 uchar* i_data, uint i_w, uint i_h,    // Full image info.
                 bool i_sixteenBits,                   // 8 or 16 bits image.
                 QWidget* parent=0,                    // Parent widget instance.
                 bool readOnly=false);                 // If true : widget with full edition mode capabilities.
    // If false : display curve data only without edition.

    ~CurvesWidget();

    void setup(int w, int h, bool readOnly);

    /**
     * Saves the currently created curve to the given group with prefix as a
     * prefix for the curve point config entries.
     *
     * @param group group to save the curve to
     * @param prefix prefix prepended to the point numbers in the config
     */
    void saveCurve(KConfigGroup& group, const QString& prefix);

    /**
     * Restores the curve tfrom the given group with prefix as a
     * prefix for the curve point config entries.
     *
     * @param group group to restore the curve from
     * @param prefix prefix prepended to the point numbers in the config
     */
    void restoreCurve(KConfigGroup& group, const QString& prefix);

    /**
     * Updates the image data the curve should be used for.
     *
     * @param i_data image data
     * @param i_w width of the image
     * @param i_h height of the image
     * @param i_sixteenBits if true, the image is interpreted as having 16 bits
     */
    void updateData(uchar* i_data, uint i_w, uint i_h, bool i_sixteenBits);

    // Stop current histogram computations.
    void stopHistogramComputation();

    void setDataLoading();
    void setLoadingFailed();

    /**
     * Resets the ui including the user specified curve.
     */
    void reset();
    /**
     * Resets only the ui and keeps the curve.
     */
    void resetUI();
    void curveTypeChanged();
    void setCurveGuide(const DColor& color);

    ImageCurves* curves() const;
    bool isSixteenBits();

Q_SIGNALS:

    void signalMouseMoved(int x, int y);
    void signalCurvesChanged();
    void signalHistogramComputationDone();
    void signalHistogramComputationFailed();

public Q_SLOTS:

    void setChannelType(ChannelType channel);
    void setScaleType(HistogramScale scale);

protected Q_SLOTS:

    void slotProgressTimerDone();
    void slotCalculationStarted();
    void slotCalculationFinished(bool success);

protected:

    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void leaveEvent(QEvent*);

private:

    CurvesWidgetPriv* const d;
};

}  // namespace Digikam

#endif /* CURVESWIDGET_H */
