/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-14
 * Description : a curves widget with additional control elements
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CURVESBOX_H
#define CURVESBOX_H

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "digikam_globals.h"

class KConfigGroup;

namespace Digikam
{

class ImageCurves;
class DColor;
class DImg;

class DIGIKAM_EXPORT CurvesBox : public QWidget
{
    Q_OBJECT

public:

    enum ColorPicker
    {
        NoPicker   = -1,
        BlackTonal = 0,
        GrayTonal,
        WhiteTonal
    };

    enum CurvesDrawingType
    {
        SmoothDrawing = 0,
        FreeDrawing
    };

public:

    CurvesBox(int w, int h, QWidget* const parent=0, bool readOnly=false);
    CurvesBox(int w, int h,                            // Widget size.
              const DImg& img,                         // Image data.
              QWidget* const parent=0,                 // Parent widget instance.
              bool readOnly=false);                    // If true : widget with full edition mode capabilities.
                                                       // If false : display curve data only without edition.
    ~CurvesBox();

    void enablePickers(bool enable);
    void enableHGradient(bool enable);
    void enableVGradient(bool enable);
    void enableGradients(bool enable);
    void enableResetButton(bool enable);
    void enableCurveTypes(bool enable);
    void enableControlWidgets(bool enable);

    void setCurveGuide(const DColor& color);

    void resetPickers();
    void resetChannel(int channel);
    void resetChannels();
    void reset();

    void readCurveSettings(KConfigGroup& group, const QString& prefix);
    void writeCurveSettings(KConfigGroup& group, const QString& prefix);

    int  picker()          const;
    ChannelType channel()  const;
    int curvesLeftOffset() const;
    ImageCurves* curves()  const;

Q_SIGNALS:

    void signalPickerChanged(int);
    void signalCurvesChanged();
    void signalChannelReset(int);
    void signalCurveTypeChanged(int);

public Q_SLOTS:

    void setChannel(ChannelType channel);
    void setScale(HistogramScale scale);

private Q_SLOTS:

    void slotCurveTypeChanged(int type);
    void slotResetChannel();
    void slotResetChannels();

private:

    void setup();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* CURVESBOX_H */
