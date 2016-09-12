/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-24
 * Description : manager for filters (registering, creating etc)
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
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
#include "levelsfilter.h"
#include "localcontrastfilter.h"
#include "mixerfilter.h"
#include "normalizefilter.h"
#include "nrfilter.h"
#include "oilpaintfilter.h"
#include "redeyecorrectionfilter.h"
#include "raindropfilter.h"
#include "sharpenfilter.h"
#include "shearfilter.h"
#include "stretchfilter.h"
#include "texturefilter.h"
#include "tonalityfilter.h"
#include "unsharpmaskfilter.h"
#include "wbfilter.h"
#include "filmfilter_p.h"

#ifdef HAVE_LIBLQR_1
#   include "contentawarefilter.h"
#endif /* HAVE_LIBLQR_1 */

#ifdef HAVE_LENSFUN
#   include "lensfunfilter.h"
#endif // HAVE_LENSFUN

#ifdef HAVE_EIGEN3
#   include "refocusfilter.h"
#endif // HAVE_EIGEN3

namespace Digikam
{
typedef QSharedPointer<DImgFilterGenerator> ImgFilterPtr;

class DImgFilterManager::Private
{
public:

    Private()
        : mutex(QMutex::Recursive)
    {
    }

    ~Private()
    {
    }

    void setupCoreGenerators();
    void setupFilterIcons();
    void setupI18nStrings();
    void addGenerator(const ImgFilterPtr& generator);

public:

    QMap<QString, ImgFilterPtr> filterMap;
    QList<ImgFilterPtr>         coreGenerators;

    QHash<QString, QString>     filterIcons;
    QHash<QString, QString>     i18nFilterNames;

    QMutex                      mutex;
};

void DImgFilterManager::Private::setupCoreGenerators()
{
    //Please keep this list sorted alphabetically
    coreGenerators
            << ImgFilterPtr(new BasicDImgFilterGenerator<AntiVignettingFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<AutoExpoFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<AutoLevelsFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<BCGFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<BlurFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<BlurFXFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<BorderFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<BWSepiaFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<CBFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<CharcoalFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<ColorFXFilter>())
#ifdef HAVE_LIBLQR_1
            << ImgFilterPtr(new BasicDImgFilterGenerator<ContentAwareFilter>())
#endif // HAVE_LIBLQR_1
            << ImgFilterPtr(new BasicDImgFilterGenerator<CurvesFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<DistortionFXFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<EmbossFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<EqualizeFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<FilmFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<FilmGrainFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<FreeRotationFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<GreycstorationFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<HSLFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<IccTransformFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<InfraredFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<InvertFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<LensDistortionFilter>())
#ifdef HAVE_LENSFUN
            << ImgFilterPtr(new BasicDImgFilterGenerator<LensFunFilter>())
#endif // HAVE_LENSFUN
            << ImgFilterPtr(new BasicDImgFilterGenerator<LevelsFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<LocalContrastFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<MixerFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<NormalizeFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<NRFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<OilPaintFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<RainDropFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<RedEyeCorrectionFilter>())
#ifdef HAVE_EIGEN3
            << ImgFilterPtr(new BasicDImgFilterGenerator<RefocusFilter>())
#endif // HAVE_EIGEN3
            << ImgFilterPtr(new BasicDImgFilterGenerator<SharpenFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<ShearFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<StretchFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<TextureFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<TonalityFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<UnsharpMaskFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<WBFilter>())
            << ImgFilterPtr(new BasicDImgFilterGenerator<RawProcessingFilter>());
}

void DImgFilterManager::Private::setupFilterIcons()
{
    //Please keep this list sorted alphabetically
    filterIcons.insert(QLatin1String("digikam:AntiVignettingFilter"),   QLatin1String("antivignetting"));
    filterIcons.insert(QLatin1String("digikam:AutoExpoFilter"),         QLatin1String("autocorrection"));
    filterIcons.insert(QLatin1String("digikam:AutoLevelsfilter"),       QLatin1String("autocorrection"));
    filterIcons.insert(QLatin1String("digikam:BCGFilter"),              QLatin1String("contrast"));
    filterIcons.insert(QLatin1String("digikam:BlurFilter"),             QLatin1String("blurimage"));
    filterIcons.insert(QLatin1String("digikam:BlurFXFilter"),           QLatin1String("blurfx"));
    filterIcons.insert(QLatin1String("digikam:BorderFilter"),           QLatin1String("bordertool"));
    filterIcons.insert(QLatin1String("digikam:BWSepiaFilter"),          QLatin1String("bwtonal"));
    filterIcons.insert(QLatin1String("digikam:ColorBalanceFilter"),     QLatin1String("adjustrgb"));
    filterIcons.insert(QLatin1String("digikam:CharcoalFilter"),         QLatin1String("charcoaltool"));
    filterIcons.insert(QLatin1String("digikam:ColorFX"),                QLatin1String("colorfx"));
    filterIcons.insert(QLatin1String("digikam:ContentAwareFilter"),     QLatin1String("transform-scale"));
    filterIcons.insert(QLatin1String("digikam:CurvesFilter"),           QLatin1String("adjustcurves"));
    filterIcons.insert(QLatin1String("digikam:DistortionFXFilter"),     QLatin1String("draw-spiral"));
    filterIcons.insert(QLatin1String("digikam:EmbossFilter"),           QLatin1String("embosstool"));
    filterIcons.insert(QLatin1String("digikam:EqualizeFilter"),         QLatin1String("autocorrection"));
    filterIcons.insert(QLatin1String("digikam:FilmFilter"),             QLatin1String("colorneg"));
    filterIcons.insert(QLatin1String("digikam:FilmGrainFilter"),        QLatin1String("filmgrain"));
    filterIcons.insert(QLatin1String("digikam:FreeRotationFilter"),     QLatin1String("transform-rotate"));
    filterIcons.insert(QLatin1String("digikam:GreycstorationFilter"),   QLatin1String("restoration"));
    filterIcons.insert(QLatin1String("digikam:HSLFilter"),              QLatin1String("adjusthsl"));
    filterIcons.insert(QLatin1String("digikam:InvertFilter"),           QLatin1String("edit-select-invert"));
    filterIcons.insert(QLatin1String("digikam:LensDistortionFilter"),   QLatin1String("lensdistortion"));
    filterIcons.insert(QLatin1String("digikam:LensFunFilter"),          QLatin1String("lensautofix"));
    filterIcons.insert(QLatin1String("digikam:LevelsFilter"),           QLatin1String("adjustlevels"));
    filterIcons.insert(QLatin1String("digikam:LocalContrastFilter"),    QLatin1String("contrast"));
    filterIcons.insert(QLatin1String("digikam:MixerFilter"),            QLatin1String("channelmixer"));
    filterIcons.insert(QLatin1String("digikam:NoiseReductionFilter"),   QLatin1String("noisereduction"));
    filterIcons.insert(QLatin1String("digikam:NormalizeFilter"),        QLatin1String("autocorrection"));
    filterIcons.insert(QLatin1String("digikam:OilPaintFilter"),         QLatin1String("oilpaint"));
    filterIcons.insert(QLatin1String("digikam:RainDropFilter"),         QLatin1String("raindrop"));
    filterIcons.insert(QLatin1String("digikam:RatioCrop"),              QLatin1String("transform-crop"));
    filterIcons.insert(QLatin1String("digikam:RedEyeCorrectionFilter"), QLatin1String("redeyes"));
    filterIcons.insert(QLatin1String("digikam:RefocusFilter"),          QLatin1String("sharpenimage"));
    filterIcons.insert(QLatin1String("digikam:Rotate90"),               QLatin1String("object-rotate-right"));
    filterIcons.insert(QLatin1String("digikam:Rotate270"),              QLatin1String("object-rotate-left"));
    filterIcons.insert(QLatin1String("digikam:SharpenFilter"),          QLatin1String("sharpenimage"));
    filterIcons.insert(QLatin1String("digikam:ShearFilter"),            QLatin1String("transform-shear-left"));
    filterIcons.insert(QLatin1String("digikam:StretchFilter"),          QLatin1String("autocorrection"));
    filterIcons.insert(QLatin1String("digikam:TextureFilter"),          QLatin1String("texture"));
    filterIcons.insert(QLatin1String("digikam:TonalityFilter"),         QLatin1String("contrast"));
    filterIcons.insert(QLatin1String("digikam:UnsharpMaskFilter"),      QLatin1String("sharpenimage"));
    filterIcons.insert(QLatin1String("digikam:WhiteBalanceFilter"),     QLatin1String("bordertool"));
    filterIcons.insert(QLatin1String("digikam:RawConverter"),           QLatin1String("image-x-adobe-dng"));
}

void DImgFilterManager::Private::setupI18nStrings()
{
    // i18nFilterNames.insert("someIdentifier", i18n("display name"));
}

void DImgFilterManager::Private::addGenerator(const ImgFilterPtr& generator)
{
    QMutexLocker lock(&mutex);

    foreach(const QString& id, generator->supportedFilters())
    {
        if (filterMap.contains(id))
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Attempt to register filter identifier" << id << "twice. Ignoring.";
            continue;
        }

        filterMap[id] = generator;
    }
}

// -----------------------------------------------------------------------------------------

class DImgFilterManagerCreator
{
public:

    DImgFilterManager object;
};

Q_GLOBAL_STATIC(DImgFilterManagerCreator, creator)

DImgFilterManager* DImgFilterManager::instance()
{
    return &creator->object;
}

DImgFilterManager::DImgFilterManager()
    : d(new Private)
{
    QMutexLocker lock(&d->mutex);
    d->setupCoreGenerators();
    d->setupFilterIcons();
    d->setupI18nStrings();

    foreach(const ImgFilterPtr& gen, d->coreGenerators)
    {
        d->addGenerator(gen);
    }
}

DImgFilterManager::~DImgFilterManager()
{
    delete d;
}

void DImgFilterManager::addGenerator(DImgFilterGenerator* const generator)
{
    ImgFilterPtr shared(generator);
    d->addGenerator(shared);
}

void DImgFilterManager::removeGenerator(DImgFilterGenerator* const /*generator*/)
{
/*
    QMutexLocker lock(&d->mutex);
    QMap<QString, DImgFilterGenerator*>::iterator it;

    for (it = d->filterMap.begin(); it != d->filterMap.end();)
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
*/
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
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier).data();

    if (gen)
    {
        return gen->supportedVersions(filterIdentifier);
    }

    return QList<int>();
}

QString DImgFilterManager::displayableName(const QString& filterIdentifier)
{
    QMutexLocker lock(&d->mutex);
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier).data();

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

    return QLatin1String("document-edit");
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
        QByteArray latin1  = name.toLatin1();
        QString translated = i18n(latin1.constData());

        if (translated != name)
        {
            return translated;
        }

        return name;
    }

    QString digikamNamespace = QLatin1String("digikam:");

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
        QString i18nDispName     = i18nDisplayableName(action.identifier());
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

    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier).data();

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
    qCDebug(DIGIKAM_DIMG_LOG) << "Creating filter " << filterIdentifier;
    DImgFilterGenerator* gen = d->filterMap.value(filterIdentifier).data();

    if (gen)
    {
        return gen->createFilter(filterIdentifier, version);
    }

    return 0;
}

} // namespace Digikam
