/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-06
 * Description : Raw decoding settings for digiKam:
 *               standard libkdcraw parameters plus
 *               few customized for post processing.
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "drawdecoding.h"

// KDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "filteraction.h"

namespace Digikam
{

class RawDecodingSettingsWriter
{
public:

    RawDecodingSettingsWriter(const RawDecodingSettings& settings, FilterAction& action, const QString& prefix = QString())
        : settings(settings), action(action), prefix(prefix)
    {
        timeOptimizedSettings.optimizeTimeLoading();
    }

    const RawDecodingSettings& settings;
    FilterAction&              action;
    QString                    prefix;

    RawDecodingSettings        defaultSettings;
    RawDecodingSettings        timeOptimizedSettings;

    inline QString combinedKey(const QString& key)
    {
        return prefix + key;
    }

    template <typename T>
    void addParameterIfNotDefault(const QString& key, const T& value, const T& defaultValue)
    {
        if (value != defaultValue)
        {
            action.addParameter(key, value);
        }
    }
#define AddParameterIfNotDefault(name) AddParameterIfNotDefaultWithValue(name, name)
#define AddParameterIfNotDefaultWithValue(name, value) \
        addParameterIfNotDefault(prefix + #name, settings.value, defaultSettings.value)

#define AddParameterIfNotDefaultEnum(name) AddParameterIfNotDefaultEnumWithValue(name, name)
#define AddParameterIfNotDefaultEnumWithValue(name, value) \
        addParameterIfNotDefault<int>(prefix + #name, settings.value, defaultSettings.value)

#define AddParameter(name) action.addParameter(prefix + #name, settings.name)
#define AddParameterEnum(name) action.addParameter(prefix + #name, static_cast<int>(settings.name))

    void write();
};

// --------------------------------------------------------------------------------------------

class RawDecodingSettingsReader
{
public:

    RawDecodingSettingsReader(const FilterAction& action, const QString& prefix = QString())
        : action(action), prefix(prefix)
    {
    }

    const FilterAction& action;
    QString             prefix;
    RawDecodingSettings settings;

    template <typename enumType, typename variantType>
    void readParameter(const QString& key, enumType& setting, const variantType& defaultValue)
    {
        setting = static_cast<enumType>(action.parameter(key, defaultValue));
    }

#define ReadParameter(name) ReadParameterWithValue(name, name)
#define ReadParameterWithValue(name, value) \
        settings.value = action.parameter(prefix + #name, settings.value)

#define ReadParameterEnum(name) ReadParameterEnumWithValue(name, name)
#define ReadParameterEnumWithValue(name, value) \
        readParameter(prefix + #name, settings.value, static_cast<int>(settings.value))

    void read();
};

// --------------------------------------------------------------------------------------------

DRawDecoding::DRawDecoding()
{
    resetPostProcessingSettings();
}

DRawDecoding::DRawDecoding(const RawDecodingSettings& prm)
{
    rawPrm = prm;

    resetPostProcessingSettings();
}

DRawDecoding::~DRawDecoding()
{
}

void DRawDecoding::optimizeTimeLoading()
{
    rawPrm.optimizeTimeLoading();
    resetPostProcessingSettings();
}

void DRawDecoding::resetPostProcessingSettings()
{
    bcg          = BCGContainer();
    wb           = WBContainer();
    curvesAdjust = CurvesContainer();
}

bool DRawDecoding::postProcessingSettingsIsDirty() const
{
    return !(bcg == BCGContainer() &&
             wb  == WBContainer()  &&
             curvesAdjust.isEmpty());
}

bool DRawDecoding::operator==(const DRawDecoding& other) const
{
    return rawPrm       == other.rawPrm       &&
           bcg          == other.bcg          &&
           wb           == other.wb           &&
           curvesAdjust == other.curvesAdjust;
}

void DRawDecoding::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    RawDecodingSettingsWriter writer(rawPrm, action, prefix);
    writer.write();

    if (!bcg.isDefault())
    {
        bcg.writeToFilterAction(action, prefix);
    }
    if (!wb.isDefault())
    {
        wb.writeToFilterAction(action, prefix);
    }
    if (!curvesAdjust.isEmpty())
    {
        curvesAdjust.writeToFilterAction(action, prefix);
    }
}

DRawDecoding DRawDecoding::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    DRawDecoding settings;

    RawDecodingSettingsReader reader(action, prefix);
    reader.read();
    settings.rawPrm       = reader.settings;

    settings.bcg          = BCGContainer::fromFilterAction(action);
    settings.wb           = WBContainer::fromFilterAction(action);
    settings.curvesAdjust = CurvesContainer::fromFilterAction(action);

    return settings;
}

void RawDecodingSettingsWriter::write()
{
    action.addParameter("kdcraw-version", KDcrawIface::KDcraw::version());

    if (settings == defaultSettings)
    {
        action.addParameter("RawDefaultSettings", true);
        return;
    }
    if (settings == timeOptimizedSettings)
    {
        action.addParameter("RawTimeOptimizedSettings", true);
        return;
    }

    AddParameter(sixteenBitsImage);

    AddParameter(fixColorsHighlights);
    AddParameter(autoBrightness);

    AddParameterEnum(whiteBalance);
    if (settings.whiteBalance == KDcrawIface::RawDecodingSettings::CUSTOM)
    {
        AddParameter(customWhiteBalance);
        AddParameter(customWhiteBalanceGreen);
    }

    AddParameterIfNotDefault(RGBInterpolate4Colors);
    AddParameterIfNotDefault(DontStretchPixels);
    AddParameterIfNotDefault(unclipColors);

    AddParameterEnum(RAWQuality);

    AddParameterIfNotDefault(medianFilterPasses);
    AddParameterIfNotDefaultEnumWithValue(noiseReductionType, NRType);
    AddParameterIfNotDefaultWithValue(noiseReductionThreshold, NRThreshold);
    AddParameterIfNotDefaultWithValue(enableChromaticAberrationCorrection, enableCACorrection);
    AddParameterIfNotDefaultWithValue(redChromaticAberrationMultiplier, caMultiplier[0]);
    AddParameterIfNotDefaultWithValue(blueChromaticAberrationMultiplier, caMultiplier[1]);
    AddParameterIfNotDefault(brightness);

    AddParameter(enableBlackPoint);
    if (settings.enableBlackPoint)
    {
        AddParameter(blackPoint);
    }

    AddParameter(enableWhitePoint);
    if (settings.enableWhitePoint)
    {
        AddParameter(whitePoint);
    }

    AddParameterEnum(inputColorSpace);
    if (settings.inputColorSpace == KDcrawIface::RawDecodingSettings::CUSTOMINPUTCS)
    {
        AddParameter(inputProfile);
    }

    AddParameterEnum(outputColorSpace);
    if (settings.outputColorSpace == KDcrawIface::RawDecodingSettings::CUSTOMOUTPUTCS)
    {
        AddParameter(outputProfile);
    }

    AddParameterIfNotDefault(deadPixelMap);

    if (settings.whiteBalance == KDcrawIface::RawDecodingSettings::AERA /*sic*/)
    {
        if (!settings.whiteBalanceArea.isNull())
        {
            action.addParameter(prefix + "whiteBalanceAreaX",      settings.whiteBalanceArea.x());
            action.addParameter(prefix + "whiteBalanceAreaY",      settings.whiteBalanceArea.y());
            action.addParameter(prefix + "whiteBalanceAreaWidth",  settings.whiteBalanceArea.width());
            action.addParameter(prefix + "whiteBalanceAreaHeight", settings.whiteBalanceArea.height());
        }
    }

    AddParameterIfNotDefault(dcbIterations);
    AddParameterIfNotDefault(dcbEnhanceFl);
    AddParameterIfNotDefault(eeciRefine);
    AddParameterIfNotDefault(esMedPasses);
    AddParameterIfNotDefaultWithValue(noiseReductionChrominanceThreshold, NRChroThreshold);
    AddParameterIfNotDefault(expoCorrection);
    AddParameterIfNotDefaultWithValue(exposureCorrectionShift,     expoCorrectionShift);
    AddParameterIfNotDefaultWithValue(exposureCorrectionHighlight, expoCorrectionHighlight);
}

void RawDecodingSettingsReader::read()
{
    if (action.parameter("RawDefaultSettings").toBool())
    {
        return;
    }
    if (action.parameter("RawTimeOptimizedSettings").toBool())
    {
        settings.optimizeTimeLoading();
        return;
    }

    ReadParameter(sixteenBitsImage);

    ReadParameter(fixColorsHighlights);
    ReadParameter(autoBrightness);

    ReadParameterEnum(whiteBalance);
    if (settings.whiteBalance == KDcrawIface::RawDecodingSettings::CUSTOM)
    {
        ReadParameter(customWhiteBalance);
        ReadParameter(customWhiteBalanceGreen);
    }

    ReadParameter(RGBInterpolate4Colors);
    ReadParameter(DontStretchPixels);
    ReadParameter(unclipColors);

    ReadParameterEnum(RAWQuality);

    ReadParameter(medianFilterPasses);
    ReadParameterEnumWithValue(noiseReductionType, NRType);
    ReadParameterWithValue(noiseReductionThreshold, NRThreshold);
    ReadParameterWithValue(enableChromaticAberrationCorrection, enableCACorrection);
    ReadParameterWithValue(redChromaticAberrationMultiplier, caMultiplier[0]);
    ReadParameterWithValue(blueChromaticAberrationMultiplier, caMultiplier[1]);
    ReadParameter(brightness);

    ReadParameter(enableBlackPoint);
    if (settings.enableBlackPoint)
    {
        ReadParameter(blackPoint);
    }

    ReadParameter(enableWhitePoint);
    if (settings.enableWhitePoint)
    {
        ReadParameter(whitePoint);
    }

    ReadParameterEnum(inputColorSpace);
    if (settings.inputColorSpace == KDcrawIface::RawDecodingSettings::CUSTOMINPUTCS)
    {
        ReadParameter(inputProfile);
    }

    ReadParameterEnum(outputColorSpace);
    if (settings.outputColorSpace == KDcrawIface::RawDecodingSettings::CUSTOMOUTPUTCS)
    {
        ReadParameter(outputProfile);
    }

    ReadParameter(deadPixelMap);

    if (!action.hasParameter("whiteBalanceAreaX"))
    {
        int x      = action.parameter(prefix + "whiteBalanceAreaX", 0);
        int y      = action.parameter(prefix + "whiteBalanceAreaY", 0);
        int width  = action.parameter(prefix + "whiteBalanceAreaWidth", 0);
        int height = action.parameter(prefix + "whiteBalanceAreaHeight", 0);
        QRect rect(x, y, width, height);
        if (rect.isValid())
        {
            settings.whiteBalanceArea = rect;
        }
    }

    ReadParameter(dcbIterations);
    ReadParameter(dcbEnhanceFl);
    ReadParameter(eeciRefine);
    ReadParameter(esMedPasses);
    ReadParameterWithValue(noiseReductionChrominanceThreshold, NRChroThreshold);
    ReadParameter(expoCorrection);
    ReadParameterWithValue(exposureCorrectionShift,     expoCorrectionShift);
    ReadParameterWithValue(exposureCorrectionHighlight, expoCorrectionHighlight);
}

}  // namespace Digikam
