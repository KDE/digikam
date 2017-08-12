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

#include "SceneBorderLoader.h"
#include "SceneBorder.h"

#include <QDebug>

using namespace PhotoLayoutsEditor;

SceneBorderLoader::SceneBorderLoader(SceneBorder * border, QDomElement & element, QObject * parent) :
    QThread(parent),
    m_border(border),
    m_element(element)
{
    connect(this, SIGNAL(finished()), border, SLOT(render()));
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void SceneBorderLoader::run()
{
    if (!m_border || m_element.isNull())
        this->exit(1);

    QDomElement border = m_element.firstChildElement();
    while (!border.isNull() && border.attribute(QLatin1String("id")) != QLatin1String("border"))
        border = border.nextSiblingElement();
    if (border.isNull())
        this->exit(1);

    QDomElement defs = border.firstChildElement(QLatin1String("defs"));
    if (defs.isNull())
        this->exit(1);

    QDomElement pattern = defs.firstChildElement(QLatin1String("pattern"));
    if (pattern.isNull())
        this->exit(1);

    QDomElement image = pattern.firstChildElement(QLatin1String("image"));
    if (image.isNull())
        this->exit(1);
    m_border->m_image = QImage::fromData( QByteArray::fromBase64(image.attributeNS(QLatin1String("http://www.w3.org/1999/xlink"), QLatin1String("href")).remove(QLatin1String("data:image/png;base64,")).toLatin1()) );

    this->exit(0);
}
