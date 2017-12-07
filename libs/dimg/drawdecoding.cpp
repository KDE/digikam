/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-06
 * Description : Raw decoding settings for digiKam:
 *               standard RawEngine parameters plus
 *               few customized for post processing.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QDomDocument>

// Local includes

#include "drawdecoder.h"
#include "filteraction.h"
#include "digikam_version.h"

namespace Digikam
{

class DRawDecoderSettingsWriter
{
public:

    DRawDecoderSettingsWriter(const DRawDecoderSettings& settings, FilterAction& action, const QString& prefix = QString())
        : settings(settings),
          action(action),
          prefix(prefix)
    {
        timeOptimizedSettings.optimizeTimeLoading();
    }

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
        addParameterIfNotDefault(prefix + QLatin1String(#name), settings.value, defaultSettings.value)

#define AddParameterIfNotDefaultEnum(name) AddParameterIfNotDefaultEnumWithValue(name, name)
#define AddParameterIfNotDefaultEnumWithValue(name, value) \
        addParameterIfNotDefault<int>(prefix + QLatin1String(#name), settings.value, defaultSettings.value)

#define AddParameter(name) action.addParameter(prefix + QLatin1String(#name), settings.name)
#define AddParameterEnum(name) action.addParameter(prefix + QLatin1String(#name), static_cast<int>(settings.name))

    void write();

public:

    const DRawDecoderSettings& settings;
    FilterAction&              action;
    QString                    prefix;

    DRawDecoderSettings        defaultSettings;
    DRawDecoderSettings        timeOptimizedSettings;
};

void DRawDecoderSettingsWriter::write()
{
    action.addParameter(QLatin1String("RawDecoder"), QLatin1String(digikam_version_short));

    if (settings == defaultSettings)
    {
        action.addParameter(QLatin1String("RawDefaultSettings"), true);
        return;
    }

    if (settings == timeOptimizedSettings)
    {
        action.addParameter(QLatin1String("RawTimeOptimizedSettings"), true);
        return;
    }

    AddParameter(sixteenBitsImage);

    AddParameter(fixColorsHighlights);
    AddParameter(autoBrightness);

    AddParameterEnum(whiteBalance);

    if (settings.whiteBalance == DRawDecoderSettings::CUSTOM)
    {
        AddParameter(customWhiteBalance);
        AddParameter(customWhiteBalanceGreen);
    }

    AddParameterIfNotDefault(RGBInterpolate4Colors);
    AddParameterIfNotDefault(DontStretchPixels);
    AddParameterIfNotDefault(unclipColors);

    AddParameterEnum(RAWQuality);

    AddParameterIfNotDefault(medianFilterPasses);
    AddParameterIfNotDefaultEnumWithValue(noiseReductionType,              NRType);
    AddParameterIfNotDefaultWithValue(noiseReductionThreshold,             NRThreshold);
    AddParameterIfNotDefaultWithValue(enableChromaticAberrationCorrection, enableCACorrection);
    AddParameterIfNotDefaultWithValue(redChromaticAberrationMultiplier,    caMultiplier[0]);
    AddParameterIfNotDefaultWithValue(blueChromaticAberrationMultiplier,   caMultiplier[1]);
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

    if (settings.inputColorSpace == DRawDecoderSettings::CUSTOMINPUTCS)
    {
        AddParameter(inputProfile);
    }

    AddParameterEnum(outputColorSpace);

    if (settings.outputColorSpace == DRawDecoderSettings::CUSTOMOUTPUTCS)
    {
        AddParameter(outputProfile);
    }

    AddParameterIfNotDefault(deadPixelMap);

    if (settings.whiteBalance == DRawDecoderSettings::AERA /*sic*/)
    {
        if (!settings.whiteBalanceArea.isNull())
        {
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaX"),      settings.whiteBalanceArea.x());
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaY"),      settings.whiteBalanceArea.y());
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaWidth"),  settings.whiteBalanceArea.width());
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaHeight"), settings.whiteBalanceArea.height());
        }
    }

    AddParameterIfNotDefault(dcbIterations);
    AddParameterIfNotDefault(dcbEnhanceFl);
    AddParameterIfNotDefault(eeciRefine);
    AddParameterIfNotDefault(esMedPasses);
    AddParameterIfNotDefaultWithValue(noiseReductionChrominanceThreshold, NRChroThreshold);
    AddParameterIfNotDefault(expoCorrection);
    AddParameterIfNotDefaultWithValue(exposureCorrectionShift,            expoCorrectionShift);
    AddParameterIfNotDefaultWithValue(exposureCorrectionHighlight,        expoCorrectionHighlight);
}

// --------------------------------------------------------------------------------------------

class DRawDecoderSettingsReader
{
public:

    DRawDecoderSettingsReader(const FilterAction& action, const QString& prefix = QString())
        : action(action), prefix(prefix)
    {
    }

    template <typename enumType, typename variantType>
    void readParameter(const QString& key, enumType& setting, const variantType& defaultValue)
    {
        setting = static_cast<enumType>(action.parameter(key, defaultValue));
    }

#define ReadParameter(name) ReadParameterWithValue(name, name)
#define ReadParameterWithValue(name, value) \
        settings.value = action.parameter(prefix + QLatin1String(#name), settings.value)

#define ReadParameterEnum(name) ReadParameterEnumWithValue(name, name)
#define ReadParameterEnumWithValue(name, value) \
        readParameter(prefix + QLatin1String(#name), settings.value, static_cast<int>(settings.value))

    void read();

public:

    const FilterAction& action;
    QString             prefix;
    DRawDecoderSettings settings;
};

void DRawDecoderSettingsReader::read()
{
    if (action.parameter(QLatin1String("RawDefaultSettings")).toBool())
    {
        return;
    }

    if (action.parameter(QLatin1String("RawTimeOptimizedSettings")).toBool())
    {
        settings.optimizeTimeLoading();
        return;
    }

    ReadParameter(sixteenBitsImage);

    ReadParameter(fixColorsHighlights);
    ReadParameter(autoBrightness);

    ReadParameterEnum(whiteBalance);

    if (settings.whiteBalance == DRawDecoderSettings::CUSTOM)
    {
        ReadParameter(customWhiteBalance);
        ReadParameter(customWhiteBalanceGreen);
    }

    ReadParameter(RGBInterpolate4Colors);
    ReadParameter(DontStretchPixels);
    ReadParameter(unclipColors);

    ReadParameterEnum(RAWQuality);

    ReadParameter(medianFilterPasses);
    ReadParameterEnumWithValue(noiseReductionType,              NRType);
    ReadParameterWithValue(noiseReductionThreshold,             NRThreshold);
    ReadParameterWithValue(enableChromaticAberrationCorrection, enableCACorrection);
    ReadParameterWithValue(redChromaticAberrationMultiplier,    caMultiplier[0]);
    ReadParameterWithValue(blueChromaticAberrationMultiplier,   caMultiplier[1]);
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

    if (settings.inputColorSpace == DRawDecoderSettings::CUSTOMINPUTCS)
    {
        ReadParameter(inputProfile);
    }

    ReadParameterEnum(outputColorSpace);

    if (settings.outputColorSpace == DRawDecoderSettings::CUSTOMOUTPUTCS)
    {
        ReadParameter(outputProfile);
    }

    ReadParameter(deadPixelMap);

    if (!action.hasParameter(QLatin1String("whiteBalanceAreaX")))
    {
        int x      = action.parameter(prefix + QLatin1String("whiteBalanceAreaX"),      0);
        int y      = action.parameter(prefix + QLatin1String("whiteBalanceAreaY"),      0);
        int width  = action.parameter(prefix + QLatin1String("whiteBalanceAreaWidth"),  0);
        int height = action.parameter(prefix + QLatin1String("whiteBalanceAreaHeight"), 0);
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

// --------------------------------------------------------------------------------------------

DRawDecoding::DRawDecoding()
{
    resetPostProcessingSettings();
}

DRawDecoding::DRawDecoding(const DRawDecoderSettings& prm)
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
    return (rawPrm       == other.rawPrm       &&
            bcg          == other.bcg          &&
            wb           == other.wb           &&
            curvesAdjust == other.curvesAdjust);
}

DRawDecoding DRawDecoding::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    DRawDecoding settings;

    DRawDecoderSettingsReader reader(action, prefix);
    reader.read();
    settings.rawPrm       = reader.settings;

    settings.bcg          = BCGContainer::fromFilterAction(action);
    settings.wb           = WBContainer::fromFilterAction(action);
    settings.curvesAdjust = CurvesContainer::fromFilterAction(action);

    return settings;
}

void DRawDecoding::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    DRawDecoderSettingsWriter writer(rawPrm, action, prefix);
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

void DRawDecoding::decodingSettingsToXml(const DRawDecoderSettings& prm, QDomElement& elm)
{
    QDomDocument doc = elm.ownerDocument();
    QDomElement  data;

    data = doc.createElement(QString::fromLatin1("autobrightness"));
    data.setAttribute(QString::fromLatin1("value"), prm.autoBrightness);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("fixcolorshighlights"));
    data.setAttribute(QString::fromLatin1("value"), prm.fixColorsHighlights);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("sixteenbitsimage"));
    data.setAttribute(QString::fromLatin1("value"), prm.sixteenBitsImage);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("brightness"));
    data.setAttribute(QString::fromLatin1("value"), prm.brightness);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("rawquality"));
    data.setAttribute(QString::fromLatin1("value"), prm.RAWQuality);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("inputcolorspace"));
    data.setAttribute(QString::fromLatin1("value"), prm.inputColorSpace);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("outputcolorspace"));
    data.setAttribute(QString::fromLatin1("value"), prm.outputColorSpace);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("rgbinterpolate4colors"));
    data.setAttribute(QString::fromLatin1("value"), prm.RGBInterpolate4Colors);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("dontstretchpixels"));
    data.setAttribute(QString::fromLatin1("value"), prm.DontStretchPixels);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("unclipcolors"));
    data.setAttribute(QString::fromLatin1("value"), prm.unclipColors);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("whitebalance"));
    data.setAttribute(QString::fromLatin1("value"), prm.whiteBalance);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("customwhitebalance"));
    data.setAttribute(QString::fromLatin1("value"), prm.customWhiteBalance);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("customwhitebalancegreen"));
    data.setAttribute(QString::fromLatin1("value"), prm.customWhiteBalanceGreen);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("halfsizecolorimage"));
    data.setAttribute(QString::fromLatin1("value"), prm.halfSizeColorImage);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("enableblackpoint"));
    data.setAttribute(QString::fromLatin1("value"), prm.enableBlackPoint);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("blackpoint"));
    data.setAttribute(QString::fromLatin1("value"), prm.blackPoint);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("enablewhitepoint"));
    data.setAttribute(QString::fromLatin1("value"), prm.enableWhitePoint);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("whitepoint"));
    data.setAttribute(QString::fromLatin1("value"), prm.whitePoint);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("noisereductiontype"));
    data.setAttribute(QString::fromLatin1("value"), prm.NRType);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("noisereductionthreshold"));
    data.setAttribute(QString::fromLatin1("value"), prm.NRThreshold);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("enablecacorrection"));
    data.setAttribute(QString::fromLatin1("value"), prm.enableCACorrection);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("redchromaticaberrationmultiplier"));
    data.setAttribute(QString::fromLatin1("value"), prm.caMultiplier[0]);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("bluechromaticaberrationmultiplier"));
    data.setAttribute(QString::fromLatin1("value"), prm.caMultiplier[1]);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("medianfilterpasses"));
    data.setAttribute(QString::fromLatin1("value"), prm.medianFilterPasses);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("inputprofile"));
    data.setAttribute(QString::fromLatin1("value"), prm.inputProfile);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("outputprofile"));
    data.setAttribute(QString::fromLatin1("value"), prm.outputProfile);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("deadpixelmap"));
    data.setAttribute(QString::fromLatin1("value"), prm.deadPixelMap);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("whitebalanceareax"));
    data.setAttribute(QString::fromLatin1("value"), prm.whiteBalanceArea.x());
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("whitebalanceareay"));
    data.setAttribute(QString::fromLatin1("value"), prm.whiteBalanceArea.y());
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("whitebalanceareawidth"));
    data.setAttribute(QString::fromLatin1("value"), prm.whiteBalanceArea.width());
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("whitebalanceareaheight"));
    data.setAttribute(QString::fromLatin1("value"), prm.whiteBalanceArea.height());
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("dcbiterations"));
    data.setAttribute(QString::fromLatin1("value"), prm.dcbIterations);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("dcbenhancefl"));
    data.setAttribute(QString::fromLatin1("value"), prm.dcbEnhanceFl);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("eecirefine"));
    data.setAttribute(QString::fromLatin1("value"), prm.eeciRefine);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("esmedpasses"));
    data.setAttribute(QString::fromLatin1("value"), prm.esMedPasses);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("nrchrominancethreshold"));
    data.setAttribute(QString::fromLatin1("value"), prm.NRChroThreshold);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("expocorrection"));
    data.setAttribute(QString::fromLatin1("value"), prm.expoCorrection);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("expocorrectionshift"));
    data.setAttribute(QString::fromLatin1("value"), prm.expoCorrectionShift);
    elm.appendChild(data);

    data = doc.createElement(QString::fromLatin1("expocorrectionhighlight"));
    data.setAttribute(QString::fromLatin1("value"), prm.expoCorrectionHighlight);
    elm.appendChild(data);
}

void DRawDecoding::decodingSettingsFromXml(const QDomElement& elm, DRawDecoderSettings& prm)
{
    bool ok = false;

    for (QDomNode node = elm.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        QDomElement echild = node.toElement();

        if (echild.isNull())
        {
            continue;
        }

        QString key        = echild.tagName();
        QString val        = echild.attribute(QString::fromLatin1("value"));

        if (key == QLatin1String("autobrightness"))
        {
            prm.autoBrightness = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("fixcolorshighlights"))
        {
            prm.fixColorsHighlights = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("sixteenbitsimage"))
        {
            prm.sixteenBitsImage = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("brightness"))
        {
            prm.brightness = val.toDouble(&ok);
        }
        else if (key == QLatin1String("rawquality"))
        {
            prm.RAWQuality = (DRawDecoderSettings::DecodingQuality)val.toInt(&ok);
        }
        else if (key == QLatin1String("inputcolorspace"))
        {
            prm.inputColorSpace = (DRawDecoderSettings::InputColorSpace)val.toInt(&ok);
        }
        else if (key == QLatin1String("outputcolorspace"))
        {
            prm.outputColorSpace = (DRawDecoderSettings::OutputColorSpace)val.toInt(&ok);
        }
        else if (key == QLatin1String("rgbinterpolate4colors"))
        {
            prm.RGBInterpolate4Colors = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("dontstretchpixels"))
        {
            prm.DontStretchPixels = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("unclipcolors"))
        {
            prm.unclipColors = (int)val.toInt(&ok);
        }
        else if (key == QLatin1String("whitebalance"))
        {
            prm.whiteBalance = (DRawDecoderSettings::WhiteBalance)val.toInt(&ok);
        }
        else if (key == QLatin1String("customwhitebalance"))
        {
            prm.customWhiteBalance = val.toInt(&ok);
        }
        else if (key == QLatin1String("customwhitebalancegreen"))
        {
            prm.customWhiteBalanceGreen = val.toDouble(&ok);
        }
        else if (key == QLatin1String("halfsizecolorimage"))
        {
            prm.halfSizeColorImage = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("enableblackpoint"))
        {
            prm.enableBlackPoint = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("blackpoint"))
        {
            prm.blackPoint = val.toInt(&ok);
        }
        else if (key == QLatin1String("enablewhitepoint"))
        {
            prm.enableWhitePoint = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("whitepoint"))
        {
            prm.whitePoint = val.toInt(&ok);
        }
        else if (key == QLatin1String("noisereductiontype"))
        {
            prm.NRType = (DRawDecoderSettings::NoiseReduction)val.toInt(&ok);
        }
        else if (key == QLatin1String("noisereductionthreshold"))
        {
            prm.NRThreshold = val.toInt(&ok);
        }
        else if (key == QLatin1String("enablecacorrection"))
        {
            prm.enableCACorrection = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("redchromaticaberrationmultiplier"))
        {
            prm.caMultiplier[0] = val.toDouble(&ok);
        }
        else if (key == QLatin1String("bluechromaticaberrationmultiplier"))
        {
            prm.caMultiplier[1] = val.toDouble(&ok);
        }
        else if (key == QLatin1String("medianfilterpasses"))
        {
            prm.medianFilterPasses = val.toInt(&ok);
        }
        else if (key == QLatin1String("inputprofile"))
        {
            prm.inputProfile = val;
        }
        else if (key == QLatin1String("outputprofile"))
        {
            prm.outputProfile = val;
        }
        else if (key == QLatin1String("deadpixelmap"))
        {
            prm.deadPixelMap = val;
        }
        else if (key == QLatin1String("whitebalanceareax"))
        {
            prm.whiteBalanceArea.setX(val.toInt(&ok));
        }
        else if (key == QLatin1String("whitebalanceareay"))
        {
            prm.whiteBalanceArea.setY(val.toInt(&ok));
        }
        else if (key == QLatin1String("whitebalanceareawidth"))
        {
            prm.whiteBalanceArea.setWidth(val.toInt(&ok));
        }
        else if (key == QLatin1String("whitebalanceareaheight"))
        {
            prm.whiteBalanceArea.setHeight(val.toInt(&ok));
        }
        else if (key == QLatin1String("dcbiterations"))
        {
            prm.dcbIterations = val.toInt(&ok);
        }
        else if (key == QLatin1String("dcbenhancefl"))
        {
            prm.dcbEnhanceFl = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("eecirefine"))
        {
            prm.eeciRefine = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("esmedpasses"))
        {
            prm.esMedPasses = val.toInt(&ok);
        }
        else if (key == QLatin1String("nrchrominancethreshold"))
        {
            prm.NRChroThreshold = val.toInt(&ok);
        }
        else if (key == QLatin1String("expocorrection"))
        {
            prm.expoCorrection = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("expocorrectionshift"))
        {
            prm.expoCorrectionShift = val.toDouble(&ok);
        }
        else if (key == QLatin1String("expocorrectionhighlight"))
        {
            prm.expoCorrectionHighlight = val.toDouble(&ok);
        }
    }
}

}  // namespace Digikam
