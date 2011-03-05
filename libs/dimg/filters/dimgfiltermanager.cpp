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

#include "dimgfiltermanager.h"

// Qt includes

#include <QMap>
#include <QMutex>
#include <QHash>

// KDE includes

#include <kdebug.h>
#include <kglobal.h>
#include <KLocale>

// Local includes

#include "config-digikam.h"
#include "dimgfiltergenerator.h"
#include "dimgbuiltinfilter.h"
#include "filteraction.h"
#include "rawprocessingfilter.h"

// Filter includes

#include "antivignettingfilter.h"
#include "autoexpofilter.h"
#include "autolevelsfilter.h"
#include "bcgfilter.h"
#include "blurfilter.h"
#include "blurfxfilter.h"
#include "borderfilter.h"
#include "bwsepiafilter.h"
#include "cbfilter.h"
#include "charcoalfilter.h"
#include "colorfxfilter.h"
#include "contentawarefilter.h"
#include "curvesfilter.h"
#include "distortionfxfilter.h"
#include "embossfilter.h"
#include "equalizefilter.h"
#include "filmgrainfilter.h"
#include "freerotationfilter.h"
#include "greycstorationfilter.h"
#include "hslfilter.h"
#include "icctransformfilter.h"
#include "infraredfilter.h"
#include "invertfilter.h"
#include "lensdistortionfilter.h"
#include "lensfunfilter.h"
#include "levelsfilter.h"
#include "localcontrastfilter.h"
#include "mixerfilter.h"
#include "normalizefilter.h"
#include "nrfilter.h"
#include "oilpaintfilter.h"
#include "raindropfilter.h"
#include "refocusfilter.h"
#include "sharpenfilter.h"
#include "shearfilter.h"
#include "stretchfilter.h"
#include "texturefilter.h"
#include "tonalityfilter.h"
#include "unsharpmaskfilter.h"
#include "wbfilter.h"

namespace Digikam
{

class DImgFilterManager::DImgFilterManagerPriv
{
public:

    DImgFilterManagerPriv()
        : mutex(QMutex::Recursive)
    {
    }

    ~DImgFilterManagerPriv()
    {
        qDeleteAll(coreGenerators);
    }

    QMap<QString, DImgFilterGenerator*> filterMap;

    QList<DImgFilterGenerator*>         coreGenerators;

    QHash<QString, QString>             filterIcons;
    QHash<QString, QString>             i18nFilterNames;

    QMutex                              mutex;

    void setupCoreGenerators();
    void setupFilterIcons();
    void setupI18nStrings();
};

void DImgFilterManager::DImgFilterManagerPriv::setupCoreGenerators()
{
    //Please keep this list sorted alphabetically
    coreGenerators
            << new BasicDImgFilterGenerator<AntiVignettingFilter>()
            << new BasicDImgFilterGenerator<AutoExpoFilter>()
            << new BasicDImgFilterGenerator<AutoLevelsFilter>()
            << new BasicDImgFilterGenerator<BCGFilter>()
            << new BasicDImgFilterGenerator<BlurFilter>()
            << new BasicDImgFilterGenerator<BlurFXFilter>()
            << new BasicDImgFilterGenerator<BorderFilter>()
            << new BasicDImgFilterGenerator<BWSepiaFilter>()
            << new BasicDImgFilterGenerator<CBFilter>()
            << new BasicDImgFilterGenerator<CharcoalFilter>()
            << new BasicDImgFilterGenerator<ColorFXFilter>()
#ifdef HAVE_GLIB2
            << new BasicDImgFilterGenerator<ContentAwareFilter>()
#endif
            << new BasicDImgFilterGenerator<CurvesFilter>()
            << new BasicDImgFilterGenerator<DistortionFXFilter>()
            << new BasicDImgFilterGenerator<EmbossFilter>()
            << new BasicDImgFilterGenerator<EqualizeFilter>()
            << new BasicDImgFilterGenerator<FilmGrainFilter>()
            << new BasicDImgFilterGenerator<FreeRotationFilter>()
            << new BasicDImgFilterGenerator<GreycstorationFilter>()
            << new BasicDImgFilterGenerator<HSLFilter>()
            << new BasicDImgFilterGenerator<IccTransformFilter>()
            << new BasicDImgFilterGenerator<InfraredFilter>()
            << new BasicDImgFilterGenerator<InvertFilter>()
            << new BasicDImgFilterGenerator<LensDistortionFilter>()
#ifdef HAVE_GLIB2
            << new BasicDImgFilterGenerator<LensFunFilter>()
#endif
            << new BasicDImgFilterGenerator<LevelsFilter>()
            << new BasicDImgFilterGenerator<LocalContrastFilter>()
            << new BasicDImgFilterGenerator<MixerFilter>()
            << new BasicDImgFilterGenerator<NormalizeFilter>()
            << new BasicDImgFilterGenerator<NRFilter>()
            << new BasicDImgFilterGenerator<OilPaintFilter>()
            << new BasicDImgFilterGenerator<RainDropFilter>()
            << new BasicDImgFilterGenerator<RefocusFilter>()
            << new BasicDImgFilterGenerator<SharpenFilter>()
            << new BasicDImgFilterGenerator<ShearFilter>()
            << new BasicDImgFilterGenerator<StretchFilter>()
            << new BasicDImgFilterGenerator<TextureFilter>()
            << new BasicDImgFilterGenerator<TonalityFilter>()
            << new BasicDImgFilterGenerator<UnsharpMaskFilter>()
            << new BasicDImgFilterGenerator<WBFilter>()

            << new BasicDImgFilterGenerator<RawProcessingFilter>();
}

void DImgFilterManager::DImgFilterManagerPriv::setupFilterIcons()
{
    //Please keep this list sorted alphabetically
    filterIcons.insert("digikam:AntiVignettingFilter", "antivignetting");
    filterIcons.insert("digikam:AutoExpoFilter",       "autocorrection");
    filterIcons.insert("digikam:AutoLevelsfilter",     "autocorrection");
    filterIcons.insert("digikam:BCGFilter",            "contrast");
    filterIcons.insert("digikam:BlurFilter",           "blurimage");
    filterIcons.insert("digikam:BlurFXFilter",         "blurfx");
    filterIcons.insert("digikam:BorderFilter",         "bordertool");
    filterIcons.insert("digikam:BWSepiaFilter",        "bwtonal");
    filterIcons.insert("digikam:ColorBalanceFilter",   "adjustrgb");
    filterIcons.insert("digikam:CharcoalFilter",       "charcoaltool");
    filterIcons.insert("digikam:ColorFX",              "colorfx");
    //filterIcons.insert("digikam:ContentAwareFilter",   "");         //FIXME
    filterIcons.insert("digikam:CurvesFilter",         "adjustcurves");
    filterIcons.insert("digikam:DistortionFXFilter",   "distortionfx");
    filterIcons.insert("digikam:EmbossFilter",         "embosstool");
    filterIcons.insert("digikam:EqualizeFilter",       "autocorrection");
    filterIcons.insert("digikam:FilmGrainFilter",      "filmgrain");
    filterIcons.insert("digikam:FreeRotationFilter",   "freerotation");
    //filterIcons.insert("digikam:GreycstorationFilter", "");         //FIXME
    filterIcons.insert("digikam:HSLFilter",            "adjusthsl");
    //filterIcons.insert("digikam:InfraredFilter",       "");         //FIXME
    filterIcons.insert("digikam:InvertFilter",         "invertimage");
    filterIcons.insert("digikam:LensDistortionFilter", "lensdistortion");
    //filterIcons.insert("digikam:LensFunFilter",        "");         //FIXME
    filterIcons.insert("digikam:LevelsFilter",         "adjustlevels");
    filterIcons.insert("digikam:LocalContrastFilter",  "contrast");
    filterIcons.insert("digikam:MixerFilter",          "channelmixer");
    filterIcons.insert("digikam:NoiseReductionFilter", "noisereduction");
    filterIcons.insert("digikam:NormalizeFilter",      "autocorrection");
    filterIcons.insert("digikam:OilPaintFilter",       "oilpaint");
    filterIcons.insert("digikam:RainDropFilter",       "raindrop");
    filterIcons.insert("digikam:RatioCrop",            "ratiocrop");
    filterIcons.insert("digikam:RefocusFilter",        "sharpenimage");
    filterIcons.insert("digikam:Rotate90",             "object-rotate-right");
    filterIcons.insert("digikam:Rotate270",            "object-rotate-left");
    filterIcons.insert("digikam:SharpenFilter",        "sharpenimage");
    filterIcons.insert("digikam:ShearFilter",          "shear");
    filterIcons.insert("digikam:StretchFilter",        "autocorrection");
    filterIcons.insert("digikam:TextureFilter",        "texture");
    filterIcons.insert("digikam:TonalityFilter",       "tonemap");
    filterIcons.insert("digikam:UnsharpMaskFilter",    "sharpenimage");
    filterIcons.insert("digikam:WhiteBalanceFilter",   "whitebalance");

    filterIcons.insert("digikam:RawConverter",         "kdcraw");
}

void DImgFilterManager::DImgFilterManagerPriv::setupI18nStrings()
{
    // i18nFilterNames.insert("someIdentifier", i18n("display name"));
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
    QMutexLocker lock(&d->mutex);
    d->setupCoreGenerators();
    d->setupFilterIcons();
    d->setupI18nStrings();
    foreach (DImgFilterGenerator* gen, d->coreGenerators)
    {
        addGenerator(gen);
    }
}

DImgFilterManager::~DImgFilterManager()
{
    delete d;
}

void DImgFilterManager::addGenerator(DImgFilterGenerator* generator)
{
    QMutexLocker lock(&d->mutex);
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
    QMutexLocker lock(&d->mutex);
    QMap<QString, DImgFilterGenerator*>::iterator it;

    for ( it = d->filterMap.begin(); it != d->filterMap.end(); )
    {
        if (it.value() == generator)
        {
            it = d->filterMap.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

QStringList DImgFilterManager::supportedFilters()
{
    QMutexLocker lock(&d->mutex);
    return DImgBuiltinFilter::supportedFilters() + d->filterMap.keys();
}

QList<int> DImgFilterManager::supportedVersions(const QString& filterIdentifier)
{
    if (DImgBuiltinFilter::isSupported(filterIdentifier))
    {
        return DImgBuiltinFilter::supportedVersions(filterIdentifier);
    }

    QMutexLocker lock(&d->mutex);
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);

    if (gen)
    {
        return gen->supportedVersions(filterIdentifier);
    }

    return QList<int>();
}

QString DImgFilterManager::displayableName(const QString& filterIdentifier)
{
    QMutexLocker lock(&d->mutex);
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);

    if (gen)
    {
        return gen->displayableName(filterIdentifier);
    }

    return QString();
}

QString DImgFilterManager::filterIcon(const QString& filterIdentifier)
{
    if (DImgBuiltinFilter::isSupported(filterIdentifier))
    {
        return DImgBuiltinFilter::filterIcon(filterIdentifier);
    }

    QMutexLocker lock(&d->mutex);
    return d->filterIcons.value(filterIdentifier);
}

QString DImgFilterManager::filterIcon(const FilterAction& action)
{
    QString iconName = filterIcon(action.identifier());

    if (!iconName.isNull())
    {
        return iconName;
    }
    return "document-edit";
}

QString DImgFilterManager::i18nDisplayableName(const QString& filterIdentifier)
{
    QMutexLocker lock(&d->mutex);
    QString name = d->i18nFilterNames.value(filterIdentifier);

    if (!name.isEmpty())
    {
        return name;
    }

    if (DImgBuiltinFilter::isSupported(filterIdentifier))
    {
        return DImgBuiltinFilter::i18nDisplayableName(filterIdentifier);
    }

    name = displayableName(filterIdentifier);

    if (!name.isEmpty())
    {
        QByteArray latin1 = name.toLatin1();
        QString translated = i18n(latin1.constData());

        if (translated != name)
        {
            return translated;
        }

        return name;
    }

    QString digikamNamespace("digikam:");

    if (filterIdentifier.startsWith(digikamNamespace))
    {
        return filterIdentifier.mid(digikamNamespace.length());
    }

    return filterIdentifier;
}

QString DImgFilterManager::i18nDisplayableName(const FilterAction& action)
{
    if (action.displayableName().isEmpty() && action.identifier().isEmpty())
    {
        return i18n("Unknown filter");
    }
    else
    {
        QString i18nDispName = i18nDisplayableName(action.identifier());
        QString metadataDispName = action.displayableName();
        if (!i18nDispName.isEmpty())
        {
            return i18nDispName;
        }
        else if (!metadataDispName.isEmpty())
        {
            return metadataDispName;
        }
        else
        {
            return action.identifier();
        }
    }
}

bool DImgFilterManager::isSupported(const QString& filterIdentifier)
{
    QMutexLocker lock(&d->mutex);

    if (DImgBuiltinFilter::isSupported(filterIdentifier))
    {
        return true;
    }

    return d->filterMap.contains(filterIdentifier);
}

bool DImgFilterManager::isSupported(const QString& filterIdentifier, int version)
{
    QMutexLocker lock(&d->mutex);

    if (DImgBuiltinFilter::isSupported(filterIdentifier, version))
    {
        return true;
    }

    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);

    if (gen)
    {
        return gen->isSupported(filterIdentifier, version);
    }

    return false;
}

bool DImgFilterManager::isRawConversion(const QString& filterIdentifier)
{
    return filterIdentifier == RawProcessingFilter::FilterIdentifier();
}

DImgThreadedFilter* DImgFilterManager::createFilter(const QString& filterIdentifier, int version)
{
    QMutexLocker lock(&d->mutex);
    kDebug() << "Creating filter " << filterIdentifier;
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier);

    if (gen)
    {
        return gen->createFilter(filterIdentifier, version);
    }

    return 0;
}

} // namespace Digikam
