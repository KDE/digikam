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

#include "StandardBordersFactory.h"
#include "PolaroidBorderDrawer.h"
#include "SolidBorderDrawer.h"

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

StandardBordersFactory::StandardBordersFactory(QObject * parent) :
    BorderDrawerFactoryInterface(parent)
{
}

QString StandardBordersFactory::drawersNames() const
{
    return i18n("Polaroid border") + QLatin1String(";") +
           i18n("Solid border");
}

BorderDrawerInterface * StandardBordersFactory::getDrawerInstance(const QString & name)
{
    if (name == i18n("Solid border"))
       return new SolidBorderDrawer(this);
    if (name == i18n("Polaroid border"))
       return new PolaroidBorderDrawer(this);
    return 0;
}
