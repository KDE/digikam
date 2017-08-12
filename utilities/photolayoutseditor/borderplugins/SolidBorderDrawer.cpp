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

#include "SolidBorderDrawer.h"
#include "StandardBordersFactory.h"

#include <klocalizedstring.h>

#include <QPainter>
#include <QPaintEngine>
#include <QMetaProperty>
#include <QDebug>

using namespace PhotoLayoutsEditor;

QMap<const char *,QString> SolidBorderDrawer::m_properties;
QMap<Qt::PenJoinStyle, QString> SolidBorderDrawer::m_corners_style_names;
int SolidBorderDrawer::m_default_width = 1;
QColor SolidBorderDrawer::m_default_color = Qt::red;
int SolidBorderDrawer::m_default_spacing = 0;
Qt::PenJoinStyle SolidBorderDrawer::m_default_corners_style = Qt::MiterJoin;

SolidBorderDrawer::SolidBorderDrawer(StandardBordersFactory * factory, QObject * parent) :
    BorderDrawerInterface(factory, parent),
    m_width(m_default_width),
    m_color(m_default_color),
    m_spacing(m_default_spacing),
    m_corners_style(m_default_corners_style)
{
    if (m_corners_style_names.isEmpty())
    {
        SolidBorderDrawer::m_corners_style_names.insert(Qt::MiterJoin, i18n("Miter"));
        SolidBorderDrawer::m_corners_style_names.insert(Qt::BevelJoin, i18n("Bevel"));
        SolidBorderDrawer::m_corners_style_names.insert(Qt::RoundJoin, i18n("Round"));
    }

    if (m_properties.isEmpty())
    {
        const QMetaObject * meta = this->metaObject();
        int count                = meta->propertyCount();

        while (count--)
        {
            QMetaProperty property = meta->property(count);
            if (!QString::fromLatin1("color").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Color"));
            else if (!QString::fromLatin1("corners_style").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Corners style"));
            else if (!QString::fromLatin1("width").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Width"));
            else if (!QString::fromLatin1("spacing").compare(QLatin1String(property.name())))
                m_properties.insert(property.name(), i18n("Spacing"));
        }
    }
}

QPainterPath SolidBorderDrawer::path(const QPainterPath & path)
{
    QPainterPath temp = path;
    if (m_spacing != 0)
    {
        QPainterPathStroker spacing;
        spacing.setWidth(qAbs(m_spacing));
        spacing.setJoinStyle(Qt::MiterJoin);
        if (m_spacing > 0)
            temp += spacing.createStroke(temp);
        else
            temp -= spacing.createStroke(path);
    }
    else
        temp = path;
    QPainterPathStroker stroker;
    stroker.setJoinStyle(this->m_corners_style);
    stroker.setWidth(m_width);
    m_path = stroker.createStroke( temp );
    return m_path;
}

void SolidBorderDrawer::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/)
{
    if (m_path.isEmpty())
        return;
    painter->save();
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(m_path, m_color);
    painter->restore();
}

QString SolidBorderDrawer::propertyName(const QMetaProperty & property) const
{
    return m_properties.value(property.name());
}

QVariant SolidBorderDrawer::propertyValue(const QString & propertyName) const
{
    const QMetaObject * meta = this->metaObject();
    int index = meta->indexOfProperty( m_properties.key(propertyName) );
    if (index >= meta->propertyCount())
        return QVariant();
    return meta->property( index ).read(this);
}

void SolidBorderDrawer::setPropertyValue(const QString & propertyName, const QVariant & value)
{
    const QMetaObject * meta = this->metaObject();
    int index = meta->indexOfProperty( m_properties.key(propertyName) );
    if (index >= meta->propertyCount())
        return;
    meta->property( index ).write(this, value);
}

QDomElement SolidBorderDrawer::toSvg(QDomDocument & document) const
{
    QDomElement result = document.createElement(QLatin1String("path"));
    int count = m_path.elementCount();
    QString str_path_d;
    for (int i = 0; i < count; ++i)
    {
        QPainterPath::Element e = m_path.elementAt(i);
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
    result.setAttribute(QLatin1String("d"), str_path_d);
    result.setAttribute(QLatin1String("fill"), m_color.name());
    return result;
}

QString SolidBorderDrawer::name() const
{
    return i18n("Solid border");
}

QString SolidBorderDrawer::toString() const
{
    return name().append(QLatin1String(" [")) + QString::number(m_width).append(QLatin1String(" ")) + m_color.name().append(QLatin1String("]"));
}

SolidBorderDrawer::operator QString() const
{
    return this->toString();
}

QVariant SolidBorderDrawer::stringNames(const QMetaProperty & property)
{
    const char * name = property.name();
    if (!QString::fromLatin1("corners_style").compare(QLatin1String(name)))
        return QVariant(m_corners_style_names.values());
    return QVariant();
}

QVariant SolidBorderDrawer::minimumValue(const QMetaProperty & property)
{
    const char * name = property.name();
    if (!QString::fromLatin1("width").compare(QLatin1String(name)))
        return 0;
    if (!QString::fromLatin1("spacing").compare(QLatin1String(name)))
        return -100;
    return QVariant();
}

QVariant SolidBorderDrawer::maximumValue(const QMetaProperty & property)
{
    const char * name = property.name();
    if (!QString::fromLatin1("width").compare(QLatin1String(name)))
        return 100;
    if (!QString::fromLatin1("spacing").compare(QLatin1String(name)))
        return 100;
    return QVariant();
}

QVariant SolidBorderDrawer::stepValue(const QMetaProperty & property)
{
    const char * name = property.name();
    if (!QString::fromLatin1("width").compare(QLatin1String(name)))
        return 1;
    if (!QString::fromLatin1("spacing").compare(QLatin1String(name)))
        return 1;
    return QVariant();
}
