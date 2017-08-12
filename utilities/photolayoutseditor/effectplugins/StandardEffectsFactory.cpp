/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "StandardEffectsFactory.h"

#include "BlurPhotoEffect.h"
#include "ColorizePhotoEffect.h"
#include "GrayscalePhotoEffect.h"
#include "SepiaPhotoEffect.h"
#include "NegativePhotoEffect.h"

using namespace PhotoLayoutsEditor;

StandardEffectsFactory::StandardEffectsFactory(QObject* parent) :
    AbstractPhotoEffectFactory(parent)
{}

AbstractPhotoEffectInterface * StandardEffectsFactory::getEffectInstance(const QString& name)
{
    if (name == i18n("Blur effect"))
        return new BlurPhotoEffect(this);
    if (name == i18n("Colorize effect"))
        return new ColorizePhotoEffect(this);
    if (name == i18n("Grayscale effect"))
        return new GrayscalePhotoEffect(this);
    if (name == i18n("Sepia effect"))
        return new SepiaPhotoEffect(this);
    if (name == i18n("Negative effect"))
        return new NegativePhotoEffect(this);
    return 0;
}

QString StandardEffectsFactory::effectName() const
{
    return i18n("Blur effect") + QLatin1String(";") +
           i18n("Colorize effect") + QLatin1String(";") +
           i18n("Grayscale effect") + QLatin1String(";") +
           i18n("Sepia effect") + QLatin1String(";") +
           i18n("Negative effect");
}
