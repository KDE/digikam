/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Qt includes

#include <QMap>
#include <QHash>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <KLocale>

// Local includes

#include "dimgfiltergenerator.h"
#include "bcgfilter.h"
#include "autoexpofilter.h"
#include "autolevelsfilter.h"
#include "equalizefilter.h"
#include "normalizefilter.h"
#include "stretchfilter.h"
#include "wbfilter.h"
#include "dimgfiltermanager.h"

namespace Digikam
{

class DImgFilterManager::DImgFilterManagerPriv
{
public:

    DImgFilterManagerPriv()
    {
    }

    ~DImgFilterManagerPriv()
    {
        foreach (DImgFilterGenerator* gen, builtinGenerators)
            delete gen;
    }

    QMap<QString, DImgFilterGenerator*> filterMap;

    QList<DImgFilterGenerator*>         builtinGenerators;

    QHash<QString, QString>             filterIcons;
    QHash<QString, QString>             i18nFilterNames;

    void setupBuiltinGenerators();
    void setupFilterIcons();
    void setupI18nStrings();
};

void DImgFilterManager::DImgFilterManagerPriv::setupBuiltinGenerators()
{
    builtinGenerators
        << new BasicDImgFilterGenerator<BCGFilter>()
        << new BasicDImgFilterGenerator<AutoExpoFilter>()
        << new BasicDImgFilterGenerator<AutoLevelsFilter>();
        //<< new BasicDImgFilterGenerator<EqualizeFilter>()
        //<< new BasicDImgFilterGenerator<NormalizeFilter>()
        //<< new BasicDImgFilterGenerator<StretchFilter>();
        //<< new BasicDImgFilterGenerator<WBFilter>();
}

void DImgFilterManager::DImgFilterManagerPriv::setupFilterIcons()
{
    //Please keep this list sorted alphabetically
    filterIcons.insert("digikam:AntiVignettingFilter", "antivignetting");
    filterIcons.insert("digikam:autoExpoFilter",       "autocorrection");
    filterIcons.insert("digikam:autolevelsfilter",     "autocorrection");
    filterIcons.insert("digikam:BCGFilter",            "contrast");
    filterIcons.insert("digikam:BlurFilter",           "blurimage");
    filterIcons.insert("digikam:BlurFXFilter",         "blurfx");
    filterIcons.insert("digikam:BorderFilter",         "bordertool");
    filterIcons.insert("digikam:BWSepiaFilter",        "bwtonal");
    filterIcons.insert("digikam:CBFilter",             "adjustrgb");
    filterIcons.insert("digikam:CharcoalFilter",       "charcoaltool");
    filterIcons.insert("digikam:ColorFX",              "colorfx");
    filterIcons.insert("digikam:ContentAwareFilter",   "unknownapp");         //FIXME
    filterIcons.insert("digikam:CurvesFilter",         "adjustcurves");
    filterIcons.insert("digikam:DistortionFXFilter",   "distortionfx");
    filterIcons.insert("digikam:EmbossFilter",         "embosstool");
    filterIcons.insert("digikam:equalizeFilter",       "autocorrection");
    filterIcons.insert("digikam:FilmGrainFilter",      "filmgrain");
    filterIcons.insert("digikam:FreeRotationFilter",   "freerotation");
    filterIcons.insert("digikam:GreycstorationFilter", "unknownapp");         //FIXME
    filterIcons.insert("digikam:HSLFilter",            "adjusthsl");
    filterIcons.insert("digikam:InfraredFilter",       "unknownapp");         //FIXME
    filterIcons.insert("digikam:InvertFilter",         "invertimage");
    filterIcons.insert("digikam:LensDistortionFilter", "lensdistortion");
    filterIcons.insert("digikam:LensFunFilter",        "unknownapp");         //FIXME
    filterIcons.insert("digikam:LevelsFilter",         "adjustlevels");
    filterIcons.insert("digikam:LocalContrastFilter",  "contrast");
    filterIcons.insert("digikam:MixerFilter",          "channelmixer");
    filterIcons.insert("digikam:NRFilter",             "noisereduction");
    filterIcons.insert("digikam:normalizeFilter",      "autocorrection");
    filterIcons.insert("digikam:OilPaintFilter",       "oilpaint");
    filterIcons.insert("digikam:PixlesAliasFilter",    "unknownapp");         //FIXME
    filterIcons.insert("digikam:RainDropFilter",       "raindrop");
    filterIcons.insert("digikam:ratioCrop",            "ratiocrop");
    filterIcons.insert("digikam:RefocusFilter",        "sharpenimage");
    filterIcons.insert("digikam:rotate90",             "object-rotate-right");
    filterIcons.insert("digikam:rotate270",            "object-rotate-left");
    filterIcons.insert("digikam:SharpenFilter",        "sharpenimage");
    filterIcons.insert("digikam:ShearFilter",          "shear");
    filterIcons.insert("digikam:stretchFilter",        "autocorrection");
    filterIcons.insert("digikam:TextureFilter",        "texture");
    filterIcons.insert("digikam:TonalityFilter",       "tonemap");
    filterIcons.insert("digikam:UnsharpMaskFilter",    "sharpenimage");
    filterIcons.insert("digikam:whiteBalance",         "whitebalance");
}

void DImgFilterManager::DImgFilterManagerPriv::setupI18nStrings() {
    //Please keep this list sorted alphabetically
    i18nFilterNames.insert("digikam:AntiVignettingFilter", i18n("Anti-Vignetting Tool"));
    i18nFilterNames.insert("digikam:autoExpoFilter",       i18n("Auto Exposure"));
    i18nFilterNames.insert("digikam:autolevelsfilter",     i18n("Auto Levels"));
    i18nFilterNames.insert("digikam:BCGFilter",            i18n("Brightness / Contrast / Gamma Filter"));
    i18nFilterNames.insert("digikam:BlurFilter",           i18n("Blur Filter"));
    i18nFilterNames.insert("digikam:BlurFXFilter",         i18n("Blur FX Filter"));
    i18nFilterNames.insert("digikam:BorderFilter",         i18n("Border Tool"));
    i18nFilterNames.insert("digikam:BWSepiaFilter",        i18n("Black & White / Sepia Filter"));
    i18nFilterNames.insert("digikam:CBFilter",             i18n("Color Balance Tool"));
    i18nFilterNames.insert("digikam:CharcoalFilter",       i18n("Charcoal Effect"));
    i18nFilterNames.insert("digikam:ColorFX",              i18n("Color Effects"));
    i18nFilterNames.insert("digikam:ContentAwareFilter",   i18n("Content-Aware Filter"));
    i18nFilterNames.insert("digikam:CurvesFilter",         i18n("Adjust Curves"));
    i18nFilterNames.insert("digikam:DistortionFXFilter",   i18n("Distortion Effect"));
    i18nFilterNames.insert("digikam:EmbossFilter",         i18n("Emboss Effect"));
    i18nFilterNames.insert("digikam:equalizeFilter",       i18n("Auto Equalize"));
    i18nFilterNames.insert("digikam:FilmGrainFilter",      i18n("Film Grain Effect"));
    i18nFilterNames.insert("digikam:FreeRotationFilter",   i18n("Free Rotation"));
    i18nFilterNames.insert("digikam:GreycstorationFilter", i18n("Greycstoration Filter"));
    i18nFilterNames.insert("digikam:HSLFilter",            i18n("Hue / Saturation / Lightness Filter"));
    i18nFilterNames.insert("digikam:InfraredFilter",       i18n("Infrared Filter"));
    i18nFilterNames.insert("digikam:InvertFilter",         i18n("Invert Effect"));
    i18nFilterNames.insert("digikam:LensDistortionFilter", i18n("Lens Distortion Tool"));
    i18nFilterNames.insert("digikam:LensFunFilter",        i18n("Lens Fun Tool"));
    i18nFilterNames.insert("digikam:LevelsFilter",         i18n("Levels Adjust Tool"));
    i18nFilterNames.insert("digikam:LocalContrastFilter",  i18n("Local Contrast Filter"));
    i18nFilterNames.insert("digikam:MixerFilter",          i18n("Channel Mixer Tool"));
    i18nFilterNames.insert("digikam:NRFilter",             i18n("Noise Reduction Filter"));
    i18nFilterNames.insert("digikam:normalizeFilter",      i18n("Auto Normalize"));
    i18nFilterNames.insert("digikam:OilPaintFilter",       i18n("Oil Painter Effect"));
    i18nFilterNames.insert("digikam:PixlesAliasFilter",    i18n("Pixels Alias Filter"));
    i18nFilterNames.insert("digikam:RainDropFilter",       i18n("Rain Drops Effect"));
    i18nFilterNames.insert("digikam:ratioCrop",            i18n("Ratio Crop"));
    i18nFilterNames.insert("digikam:RefocusFilter",        i18n("Refocus"));
    i18nFilterNames.insert("digikam:rotate90",             i18n("Rotate Right"));
    i18nFilterNames.insert("digikam:rotate270",            i18n("Rotate Left"));
    i18nFilterNames.insert("digikam:SharpenFilter",        i18n("Sharpen"));
    i18nFilterNames.insert("digikam:ShearFilter",          i18n("Shear Tool"));
    i18nFilterNames.insert("digikam:stretchFilter",        i18n("Stretch Contrast"));
    i18nFilterNames.insert("digikam:TextureFilter",        i18n("Texture Filter"));
    i18nFilterNames.insert("digikam:TonalityFilter",       i18n("Tonality Filter"));
    i18nFilterNames.insert("digikam:UnsharpMaskFilter",    i18n("Unsharp Mask Tool"));
    i18nFilterNames.insert("digikam:whiteBalance",         i18n("White Balance Tool"));
}

// -----------------------------------------------------------------------------------------

class DImgFilterManagerCreator
{
public:

    DImgFilterManager object;
};

K_GLOBAL_STATIC(DImgFilterManagerCreator, creator)

DImgFilterManager* DImgFilterManager::instance()
{
    return &creator->object;
}

DImgFilterManager::DImgFilterManager()
                 : d(new DImgFilterManagerPriv)
{
    d->setupBuiltinGenerators();
    d->setupFilterIcons();
    d->setupI18nStrings();
    foreach (DImgFilterGenerator* gen, d->builtinGenerators)
        addGenerator(gen);
}

DImgFilterManager::~DImgFilterManager()
{
    delete d;
}

void DImgFilterManager::addGenerator(DImgFilterGenerator* generator)
{
    foreach (const QString& id, generator->supportedFilters())
    {
        if (d->filterMap.contains(id))
        {
            kError() << "Attempt to register filter identifier" << id << "twice. Ignoring.";
            continue;
        }
        d->filterMap[id] = generator;
    }
}

void DImgFilterManager::removeGenerator(DImgFilterGenerator* generator)
{
    QMap<QString, DImgFilterGenerator*>::iterator it;
    for ( it = d->filterMap.begin(); it != d->filterMap.end(); )
    {
        if (it.value() == generator)
            it = d->filterMap.erase(it);
        else
            ++it;
    }
}

QStringList DImgFilterManager::supportedFilters()
{
    return d->filterMap.keys();
}

QList<int> DImgFilterManager::supportedVersions(const QString& filterIdentifier)
{
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);
    if (gen)
        return gen->supportedVersions(filterIdentifier);

    return QList<int>();
}

QString DImgFilterManager::displayableName(const QString& filterIdentifier)
{
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);
    if (gen)
        return gen->displayableName(filterIdentifier);

    return QString();
}

QString DImgFilterManager::getFilterIcon(const QString& filterIdentifier)
{
    return d->filterIcons.value(filterIdentifier, "unknownapp");
}

QString DImgFilterManager::getI18nFilterName(const QString& filterIdentifier)
{
    return d->i18nFilterNames.value(filterIdentifier, i18n("Unknown Filter"));
}

bool DImgFilterManager::isSupported(const QString& filterIdentifier)
{
    return d->filterMap.contains(filterIdentifier);
}

bool DImgFilterManager::isSupported(const QString& filterIdentifier, int version)
{
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);
    if (gen)
        return gen->isSupported(filterIdentifier, version);

    return false;
}

DImgThreadedFilter* DImgFilterManager::createFilter(const QString& filterIdentifier, int version)
{
    kDebug() << "Creating filter " << filterIdentifier;
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);
    if (gen)
        return gen->createFilter(filterIdentifier, version);

    return 0;
}

} // namespace Digikam
