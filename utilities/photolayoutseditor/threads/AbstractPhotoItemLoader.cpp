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

#include "AbstractPhotoItemLoader.h"
#include "AbstractPhoto.h"
#include "AbstractPhoto_p.h"
#include "ProgressObserver.h"
#include "global.h"

#include <klocalizedstring.h>
#include <QDebug>

using namespace PhotoLayoutsEditor;

AbstractPhotoItemLoader::AbstractPhotoItemLoader(AbstractPhoto * item, QDomElement & element, QObject * parent) :
    QThread(parent),
    m_item(item),
    m_element(element),
    m_observer(0)
{
    connect(this, SIGNAL(finished()), item, SLOT(refresh()));
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

AbstractPhoto * AbstractPhotoItemLoader::item() const
{
    return m_item;
}

QDomElement AbstractPhotoItemLoader::element() const
{
    return m_element;
}

void AbstractPhotoItemLoader::setObserver(ProgressObserver * observer)
{
    m_observer = observer;
}

ProgressObserver * AbstractPhotoItemLoader::observer() const
{
    return m_observer;
}

void AbstractPhotoItemLoader::run()
{
    if (m_element.tagName() != QLatin1String("g"))
        this->exit(1);

    ProgressObserver * observer = this->observer();
    if (observer)
    {
        observer->progresChanged(0.1);
        observer->progresName(i18n("Reading properties..."));
    }
    // Items visibility
    m_item->d->m_visible = (m_element.attribute(QLatin1String("visibility")) != QLatin1String("hide"));

    // ID & name
    m_item->d->m_id = m_element.attribute(QLatin1String("id"));
    m_item->d->m_name = m_element.attribute(QLatin1String("name"));

    // Position & transformation
    m_item->d->m_pos = QPointF(0,0);
    QString transform = m_element.attribute(QLatin1String("transform"));
    if (!transform.isEmpty())
    {
        QRegExp tr(QLatin1String("translate\\([-0-9.]+,[-0-9.]+\\)"));
        if (tr.indexIn(transform) >= 0)
        {
            QStringList list = tr.capturedTexts();
            QString translate = list.at(0);
            list = translate.split(QLatin1Char(','));
            QString x = list.at(0);
            QString y = list.at(1);
            m_item->d->m_pos = QPointF( x.right(x.length()-10).toDouble(),
                                        y.left(y.length()-1).toDouble());
        }

        QRegExp rot(QLatin1String("matrix\\([-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+\\)"));
        if (rot.indexIn(transform) >= 0)
        {
            QStringList list = rot.capturedTexts();
            QString matrix = list.at(0);
            matrix.remove(matrix.length()-1,1).remove(0,7);
            list = matrix.split(QLatin1Char(','));
            QString m11 = list.at(0);
            QString m12 = list.at(1);
            QString m21 = list.at(2);
            QString m22 = list.at(3);
            QString m31 = list.at(4);
            QString m32 = list.at(5);
            m_item->d->m_transform = QTransform(m11.toDouble(), m12.toDouble(), 0,
                                                m21.toDouble(), m22.toDouble(), 0,
                                                m31.toDouble(), m32.toDouble(), 1);
        }
    }

    if (m_element.firstChildElement().tagName() == QLatin1String("g"))
    {
        m_element = m_element.firstChildElement();
        QString transform = m_element.attribute(QLatin1String("transform"));
        QRegExp rot(QLatin1String("matrix\\([-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+,[-0-9.]+\\)"));
        if (rot.indexIn(transform) >= 0)
        {
            QStringList list = rot.capturedTexts();
            QString matrix = list.at(0);
            matrix.remove(matrix.length()-1,1).remove(0,7);
            list = matrix.split(QLatin1Char(','));
            QString m11 = list.at(0);
            QString m12 = list.at(1);
            QString m21 = list.at(2);
            QString m22 = list.at(3);
            QString m31 = list.at(4);
            QString m32 = list.at(5);
            m_item->d->m_transform = QTransform(m11.toDouble(), m12.toDouble(), 0,
                                                m21.toDouble(), m22.toDouble(), 0,
                                                m31.toDouble(), m32.toDouble(), 1);
        }
    }

    // Validation purpose
    QDomElement defs = m_element.firstChildElement(QLatin1String("defs"));
    while (!defs.isNull() && defs.attribute(QLatin1String("id")) != QLatin1String("data_") + m_item->id())
        defs = defs.nextSiblingElement(QLatin1String("defs"));
    if (defs.isNull())
        this->exit(1);

    QDomElement itemDataElement = defs.firstChildElement(QLatin1String("g"));
    while (!itemDataElement.isNull() && itemDataElement.attribute(QLatin1String("id")) != QLatin1String("vis_data_") + m_item->id())
        itemDataElement = itemDataElement.nextSiblingElement(QLatin1String("g"));
    if (itemDataElement.isNull())
        this->exit(1);

    if (observer)
    {
        observer->progresChanged(0.2);
        observer->progresName(i18n("Reading borders..."));
    }
    // Borders
    if (m_item->d->m_borders_group)
    {
        m_item->d->m_borders_group->deleteLater();
        m_item->d->m_borders_group = 0;
    }
    m_item->d->m_borders_group = BordersGroup::fromSvg(itemDataElement, m_item);
    if (!m_item->d->m_borders_group)
        this->exit(1);
    else
        connect(m_item->d->m_borders_group, SIGNAL(drawersChanged()), m_item, SLOT(refresh()));

    QDomElement clipPath = defs.firstChildElement(QLatin1String("clipPath"));
    if (clipPath.isNull() || clipPath.attribute(QLatin1String("id")) != QLatin1String("clipPath_") + m_item->id())
        this->exit(1);

    // Other application specific data
    QDomElement appNS = defs.firstChildElement(QLatin1String("data"));
    if (appNS.isNull() || appNS.prefix() != PhotoLayoutsEditor::name())
        this->exit(1);

    if (observer)
    {
        observer->progresChanged(0.3);
        observer->progresName(i18n("Reading effects..."));
    }
    // Effects
    if (m_item->d->m_effects_group)
    {
        m_item->d->m_effects_group->deleteLater();
        m_item->d->m_effects_group = 0;
    }
    m_item->d->m_effects_group = PhotoEffectsGroup::fromSvg(appNS, m_item);
    if (!m_item->d->m_effects_group)
        this->exit(1);
    else
        connect(m_item->d->m_effects_group, SIGNAL(effectsChanged()), m_item, SLOT(refresh()));

    if (observer)
    {
        observer->progresChanged(0.4);
        observer->progresName(i18n("Reading cropping shape..."));
    }
    // Crop path
    QDomElement cropPath = appNS.firstChildElement(QLatin1String("crop_path"));
    if (!cropPath.isNull())
        m_item->d->m_crop_shape = PhotoLayoutsEditor::pathFromSvg( cropPath.firstChildElement(QLatin1String("path")) );
    else
        this->exit(1);
}
