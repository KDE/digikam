/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * date        : 2006-09-13
 * Description : Raw Decoder settings widgets
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DRAW_DECODER_WIDGET_H
#define DRAW_DECODER_WIDGET_H

// Qt includes

#include <QString>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "drawdecodersettings.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dfileselector.h"
#include "digikam_export.h"



namespace Digikam
{

class DIGIKAM_EXPORT DRawDecoderWidget : public DExpanderBox
{
    Q_OBJECT

public:

    enum AdvancedSettingsOptions
    {
        SIXTEENBITS      = 0x00000001,
        COLORSPACE       = 0x00000002,
        POSTPROCESSING   = 0x00000004,
        BLACKWHITEPOINTS = 0x00000008
    };

    enum SettingsTabs
    {
        DEMOSAICING = 0,
        WHITEBALANCE,
        CORRECTIONS,
        COLORMANAGEMENT
    };

public:

    /**
     * @param advSettings the default value is COLORSPACE
     */
    explicit DRawDecoderWidget(QWidget* const parent, int advSettings = COLORSPACE);
    virtual ~DRawDecoderWidget();

    DFileSelector* inputProfileUrlEdit()  const;
    DFileSelector* outputProfileUrlEdit() const;

    void setup(int advSettings);

    void setEnabledBrightnessSettings(bool b);
    bool brightnessSettingsIsEnabled() const;

    void updateMinimumWidth();

    void resetToDefault();

    void setSettings(const DRawDecoderSettings& settings);
    DRawDecoderSettings settings() const;

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    static void readSettings(DRawDecoderSettings& setting, KConfigGroup& group);
    static void writeSettings(const DRawDecoderSettings& setting, KConfigGroup& group);

Q_SIGNALS:

    void signalSixteenBitsImageToggled(bool);
    void signalSettingsChanged();

private Q_SLOTS:

    void slotWhiteBalanceToggled(int);
    void slotsixteenBitsImageToggled(bool);
    void slotUnclipColorActivated(int);
    void slotNoiseReductionChanged(int);
    void slotCACorrectionToggled(bool);
    void slotExposureCorrectionToggled(bool);
    void slotAutoCAToggled(bool);
    void slotInputColorSpaceChanged(int);
    void slotOutputColorSpaceChanged(int);
    void slotRAWQualityChanged(int);
    void slotExpoCorrectionShiftChanged(double);

private:

    class Private;
    Private* const d;
};

} // NameSpace Digikam

#endif // DRAW_DECODER_WIDGET_H
