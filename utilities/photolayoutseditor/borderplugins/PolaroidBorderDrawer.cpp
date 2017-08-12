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

#include "PolaroidBorderDrawer.h"
#include "StandardBordersFactory.h"

#include <klocalizedstring.h>

#include <QPainter>
#include <QPaintEngine>
#include <QMetaProperty>
#include <QDebug>

using namespace PhotoLayoutsEditor;

QMap<const char *,QString> PolaroidBorderDrawer::m_properties;
int PolaroidBorderDrawer::m_default_width = 20;
QString PolaroidBorderDrawer::m_default_text = i18n("Write here some text");
QColor PolaroidBorderDrawer::m_default_color = Qt::black;
QFont PolaroidBorderDrawer::m_default_font(QFont().family(), 24);

PolaroidBorderDrawer::PolaroidBorderDrawer(StandardBordersFactory * factory, QObject * parent) :
    BorderDrawerInterface(factory, parent),
    m_width(m_default_width),
    m_text(m_default_text),
    m_color(m_default_color),
    m_font(m_default_font)
{
    if (m_properties.isEmpty())
    {
        const QMetaObject * meta = this->metaObject();
        int count = meta->propertyCount();

        while (count--)
        {
            QMetaProperty property = meta->property(count);

            if (!QString::fromLatin1("width").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Width"));
            else if (!QString::fromLatin1("text").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Text"));
            else if (!QString::fromLatin1("color").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Color"));
            else if (!QString::fromLatin1("font").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Font"));
        }
    }
}

QPainterPath PolaroidBorderDrawer::path(const QPainterPath & path)
{
    QPainterPath temp;

    QRectF r = path.boundingRect();

    m_text_rect.setTop(r.bottom());

    r.setTop(r.top()-m_width);
    r.setBottom(r.bottom()+m_width*5);
    r.setLeft(r.left()-m_width);
    r.setRight(r.right()+m_width);

    m_text_rect.setBottom(r.bottom());
    m_text_rect.setLeft(r.left());
    m_text_rect.setRight(r.right());

    temp.addRect(r);
    temp -= path;

    m_path = temp;
    return m_path;
}

void PolaroidBorderDrawer::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/)
{
    if (m_path.isEmpty())
        return;
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(m_path, Qt::white);
    painter->setFont(m_font);
    painter->setPen(m_color);
    painter->drawText(m_text_rect, Qt::AlignCenter, m_text);
    painter->restore();
}

QString PolaroidBorderDrawer::propertyName(const QMetaProperty & property) const
{
    return m_properties.value(property.name());
}

QVariant PolaroidBorderDrawer::propertyValue(const QString & propertyName) const
{
    if (!m_properties.key(propertyName))
        return QVariant();
    const QMetaObject * meta = this->metaObject();
    int index = meta->indexOfProperty( m_properties.key(propertyName) );
    if (index >= meta->propertyCount())
        return QVariant();
    return meta->property( index ).read(this);
}

void PolaroidBorderDrawer::setPropertyValue(const QString & propertyName, const QVariant & value)
{
    if (!m_properties.key(propertyName))
        return;
    const QMetaObject * meta = this->metaObject();
    int index = meta->indexOfProperty( m_properties.key(propertyName) );
    if (index >= meta->propertyCount())
        return;
    meta->property( index ).write(this, value);
}

QDomElement PolaroidBorderDrawer::toSvg(QDomDocument & document) const
{
    QDomElement result = document.createElement(QLatin1String("g"));
    QDomElement path  = document.createElement(QLatin1String("path"));
    result.appendChild(path);
    path.setAttribute(QLatin1String("d"),         pathToSvg(m_path));
    path.setAttribute(QLatin1String("fill"),      QLatin1String("#ffffff"));
    path.setAttribute(QLatin1String("fill-rule"), QLatin1String("evenodd"));

    QPainterPath p;
    p.addText(0, 0, m_font, m_text);
    p.translate(m_text_rect.center() - p.boundingRect().center());

    QDomElement text = document.createElement(QLatin1String("path"));
    result.appendChild(text);
    text.setAttribute(QLatin1String("d"), pathToSvg(p));
    text.setAttribute(QLatin1String("fill"), m_color.name());

    return result;
}

QString PolaroidBorderDrawer::name() const
{
    return i18n("Polaroid border");
}

QString PolaroidBorderDrawer::toString() const
{
    return name().append(QLatin1String(" [")) + m_text + QLatin1String("]");
}

PolaroidBorderDrawer::operator QString() const
{
    return this->toString();
}

QVariant PolaroidBorderDrawer::minimumValue(const QMetaProperty & property)
{
    const char * name = property.name();

    if (!QString::fromLatin1("width").compare(QLatin1String(name)))
        return 0;

    return QVariant();
}

QVariant PolaroidBorderDrawer::maximumValue(const QMetaProperty & property)
{
    const char * name = property.name();

    if (!QString::fromLatin1("width").compare(QLatin1String(name)))
        return 100;

    return QVariant();
}

QVariant PolaroidBorderDrawer::stepValue(const QMetaProperty & property)
{
    const char * name = property.name();

    if (!QString::fromLatin1("width").compare(QLatin1String(name)))
        return 1;

    return QVariant();
}

QString PolaroidBorderDrawer::pathToSvg(const QPainterPath & path) const
{
    int count = path.elementCount();
    QString str_path_d;

    for (int i = 0; i < count; ++i)
    {
        QPainterPath::Element e = path.elementAt(i);
        switch (e.type)
        {
            case QPainterPath::LineToElement:
                str_path_d.append(QLatin1String("L ") + QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            case QPainterPath::MoveToElement:
                str_path_d.append(QLatin1String("M ") + QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            case QPainterPath::CurveToElement:
                str_path_d.append(QLatin1String("C ") + QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
            case QPainterPath::CurveToDataElement:
                str_path_d.append(QString::number(e.x) + QLatin1Char(' ') + QString::number(e.y) + QLatin1Char(' '));
                break;
        }
    }

    str_path_d.append(QLatin1String("z"));
    return str_path_d;
}
