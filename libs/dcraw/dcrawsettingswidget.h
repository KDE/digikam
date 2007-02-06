/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-09-13
 * Description : dcraw settings widgets
 *
 * Copyright 2006 by Gilles Caulier
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

#ifndef DCRAWSETTINGSWIDGET_H
#define DCRAWSETTINGSWIDGET_H

// Qt includes.

#include <qgroupbox.h>

// Local includes.

#include "rawdecodingsettings.h"

namespace Digikam
{

class DcrawSettingsWidgetPriv;

class DcrawSettingsWidget : public QGroupBox
{
    Q_OBJECT
    
public:

    DcrawSettingsWidget(QWidget *parent, const QString& dcrawVersion);
    ~DcrawSettingsWidget();

    bool   sixteenBits();
    bool   useCameraWB();
    bool   useAutoColorBalance();
    bool   useFourColor();
    bool   useSecondarySensor();
    bool   useNoiseReduction();
    int    unclipColor();
    double brightness();
    double sigmaDomain();
    double sigmaRange();
    RawDecodingSettings::DecodingQuality quality();
//    RawDecodingSettings::OutputColorSpace outputColorSpace();

    void   setSixteenBits(bool b);
    void   setCameraWB(bool b);
    void   setAutoColorBalance(bool b);
    void   setFourColor(bool b);
    void   setSecondarySensor(bool b);
    void   setNoiseReduction(bool b);
    void   setUnclipColor(int v);
    void   setBrightness(double b);
    void   setSigmaDomain(double b);
    void   setSigmaRange(double b);
    void   setQuality(RawDecodingSettings::DecodingQuality q);
//    void   setOutputColorSpace(RawDecodingSettings::OutputColorSpace c);

    void   setDefaultSettings();

private slots:

    void slotUnclipColorActivated(int);
    void slotNoiseReductionToggled(bool);

private:

    DcrawSettingsWidgetPriv* d;
};

} // NameSpace Digikam

#endif /* DCRAWSETTINGSWIDGET_H */
