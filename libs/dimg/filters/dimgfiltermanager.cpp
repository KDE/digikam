/* ============================================================
 * 
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

    void setupBuiltinGenerators();
    void setupFilterIcons();
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
    filterIcons.insert("digikam:ContentAwareFilter",   "unknownapp");         //FIXME
    filterIcons.insert("digikam:CurvesFilter",         "adjustcurves");
    filterIcons.insert("digikam:DistortionFXFilter",   "distortionfx");
    filterIcons.insert("digikam:EmbossFilter",         "embosstool");
    filterIcons.insert("digikam:equalizeFilter",       "autocorrection");
    filterIcons.insert("digikam:FilmGrainFilter",      "filmgrain");
    filterIcons.insert("digikam:FreeRotationFilter",   "freerotation");
    filterIcons.insert("digikam:GreycstorationFilter", "unknownapp");         //FIXME
    filterIcons.insert("digikam:HSLFIlter",            "adjusthsl");
    filterIcons.insert("digikam:InfraredFilter",       "unknownapp");         //FIXME
    filterIcons.insert("digikam:InvertFilter",         "invertimage");
    filterIcons.insert("digikam:LensDistortionFilter", "lensdistortion");
    filterIcons.insert("digikam:LensFunFilter",        "unknownapp");         //FIXME
    filterIcons.insert("digikam:LevelsFilter",         "adjustlevels");
    filterIcons.insert("digikam:LocalContrastFilter",  "constrast");          //FIXME?
    filterIcons.insert("digikam:MixerFilter",          "channelmixer");
    filterIcons.insert("digikam:NRFilter",             "noisereduction");
    filterIcons.insert("digikam:normalizeFilter",      "autocorrection");
    filterIcons.insert("digikam:OilPaintFilter",       "oilpaint");
    filterIcons.insert("digikam:PixlesAliasFilter",    "unknownapp");         //FIXME
    filterIcons.insert("digikam:RainDropFilter",       "raindrop");
    filterIcons.insert("digikam:RefocusFilter",        "sharpenimage");
    filterIcons.insert("digikam:SharpenFilter",        "sharpenimage");
    filterIcons.insert("digikam:ShearFilter",          "shear");
    filterIcons.insert("digikam:stretchFilter",        "autocorrection");
    filterIcons.insert("digikam:TextureFilter",        "texture");
    filterIcons.insert("digikam:TonalityFilter",       "tonemap");
    filterIcons.insert("digikam:UnsharpMaskFilter",    "sharpenimage");
    filterIcons.insert("digikam:whiteBalance",         "whitebalance");
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
