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

#include "SceneBackgroundLoader.h"
#include "SceneBackground.h"

#include <QDebug>

using namespace PhotoLayoutsEditor;

SceneBackgroundLoader::SceneBackgroundLoader(SceneBackground * background, QDomElement & element, QObject * parent) :
    QThread(parent),
    m_background(background),
    m_element(element)
{
    connect(this, SIGNAL(finished()), background, SLOT(render()));
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void SceneBackgroundLoader::run()
{
    if (!m_background || m_element.isNull())
        this->exit(1);

    QDomElement background = m_element.firstChildElement();
    while (!background.isNull() && background.attribute(QLatin1String("id")) != QLatin1String("background"))
        background = background.nextSiblingElement();
    if (background.isNull())
        this->exit(1);

    QDomElement defs = background.firstChildElement(QLatin1String("defs"));
    if (defs.isNull())
        this->exit(1);
    QString type = defs.firstChildElement(QLatin1String("type")).text();
    if (type == QLatin1String("color"))
    {
        QDomElement rect = background.firstChildElement(QLatin1String("rect"));
        if (rect.isNull())
            this->exit(1);
        QColor color(rect.attribute(QLatin1String("fill")));
        color.setAlphaF(rect.attribute(QLatin1String("opacity")).toDouble());
        if (!color.isValid())
            this->exit(1);
        m_background->m_first_brush.setColor(color);
    }
    else if (type == QLatin1String("pattern"))
    {
        bool ok = true;
        QDomElement bse = defs.firstChildElement(QLatin1String("brush_style"));
        if (bse.isNull())
            this->exit(1);
        Qt::BrushStyle bs = (Qt::BrushStyle) bse.text().toInt(&ok);

        QDomElement c1e = defs.firstChildElement(QLatin1String("color1"));
        if (c1e.isNull())
            this->exit(1);
        QColor color1(c1e.text());
        color1.setAlphaF(c1e.attribute(QLatin1String("opacity")).toInt());

        QDomElement c2e = defs.firstChildElement(QLatin1String("color2"));
        if (c2e.isNull())
            this->exit(1);
        QColor color2(c2e.text());
        color2.setAlphaF(c2e.attribute(QLatin1String("opacity")).toInt());

        if (!color1.isValid() || !color2.isValid() || !ok || bs <= Qt::SolidPattern || bs >= Qt::LinearGradientPattern)
            this->exit(1);
        m_background->m_first_brush.setStyle(bs);
        m_background->m_first_brush.setColor(color1);
        m_background->m_second_brush.setStyle(Qt::SolidPattern);
        m_background->m_second_brush.setColor(color2);
    }
    else if (type == QLatin1String("image"))
    {
        m_background->m_image_align = (Qt::Alignment) defs.firstChildElement(QLatin1String("align")).text().toInt();
        m_background->m_image_aspect_ratio = (Qt::AspectRatioMode) defs.firstChildElement(QLatin1String("aspect_ratio")).text().toInt();
        m_background->m_image_repeat = (bool) defs.firstChildElement(QLatin1String("repeat")).text().toInt();

        QDomElement pattern = defs.firstChildElement(QLatin1String("pattern"));
        if (pattern.isNull())
            this->exit(1);
        QDomElement image = pattern.firstChildElement(QLatin1String("image"));
        if (image.isNull())
            this->exit(1);
        m_background->m_image_size.setWidth(image.attribute(QLatin1String("width")).remove(QLatin1String("px")).toInt());
        m_background->m_image_size.setHeight(image.attribute(QLatin1String("height")).remove(QLatin1String("px")).toInt());
        m_background->m_image = QImage::fromData( QByteArray::fromBase64(image.attributeNS(QLatin1String("http://www.w3.org/1999/xlink"), QLatin1String("href")).remove(QLatin1String("data:image/png;base64,")).toLatin1()) );
        m_background->m_first_brush.setTextureImage(m_background->m_image.scaled(m_background->m_image_size, m_background->m_image_aspect_ratio));

        QDomElement bColor = defs.firstChildElement(QLatin1String("background_color"));
        QColor backgroundColor(bColor.text());
        backgroundColor.setAlphaF(bColor.attribute(QLatin1String("opacity"), QLatin1String("1.0")).toDouble());
        m_background->m_second_brush.setColor(backgroundColor);
    }
    else if (type == QLatin1String("gradient"))
    {}
    else
        this->exit(1);

    this->exit(0);
}
