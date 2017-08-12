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
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "BlurPhotoEffect.h"
#include "StandardEffectsFactory.h"

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

BlurPhotoEffect::BlurPhotoEffect(StandardEffectsFactory * factory, QObject * parent) :
    AbstractPhotoEffectInterface(factory, parent),
    m_radius(10)
{
}

QImage BlurPhotoEffect::apply(const QImage & image) const
{
    int tempRadius = radius();
    if (!tempRadius)
        return image;
    QImage result = image;
    QPainter p(&result);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0,0,AbstractPhotoEffectInterface::apply(blurred(image, image.rect(), tempRadius)));
    return result;
}

QString BlurPhotoEffect::name() const
{
    return i18n("Blur effect");
}

QString BlurPhotoEffect::toString() const
{
    return this->name() + QLatin1String(" [") + QString::number(this->radius()) + QLatin1Char(']');
}

BlurPhotoEffect::operator QString() const
{
    return toString();
}
