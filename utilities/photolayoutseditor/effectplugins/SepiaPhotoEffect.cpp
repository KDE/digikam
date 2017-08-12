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

#include "SepiaPhotoEffect.h"
#include "StandardEffectsFactory.h"

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

SepiaPhotoEffect::SepiaPhotoEffect(StandardEffectsFactory * factory, QObject * parent) :
    AbstractPhotoEffectInterface(factory, parent)
{
}

QImage SepiaPhotoEffect::apply(const QImage & image) const
{
    if (!strength())
        return image;
    QImage result = image;
    QPainter p(&result);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0,0,AbstractPhotoEffectInterface::apply(sepia_converted(image)));
    return result;
}

QString SepiaPhotoEffect::name() const
{
    return i18n("Sepia effect");
}

QString SepiaPhotoEffect::toString() const
{
    return this->name();
}

SepiaPhotoEffect::operator QString() const
{
    return toString();
}
