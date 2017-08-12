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

#ifndef ABSTRACTPROTOEFFECTINTERFACE_H
#define ABSTRACTPROTOEFFECTINTERFACE_H

#include <QtPlugin>
#include <QVariant>
#include <QList>
#include <QImage>
#include <QPainter>
#include <QUndoCommand>
#include <QMetaProperty>
#include <QDebug>

#include <klocalizedstring.h>

#define STRENGTH_PROPERTY QLatin1String("Strength")

namespace PhotoLayoutsEditor
{
    class PhotoEffectsGroup;
    class AbstractPhotoEffectFactory;

    class AbstractPhotoEffectInterface : public QObject
    {
            Q_OBJECT

        public:

            explicit AbstractPhotoEffectInterface(AbstractPhotoEffectFactory* factory, QObject* parent = 0) :
                QObject(parent),
                m_factory(factory),
                m_group(0),
                m_strength(100)
            {
#ifdef QT_DEBUG
                if (!m_factory)
                    qDebug() << "No factory object for effect" << this << "from:" << __FILE__ << __LINE__;
#endif
            }

            virtual ~AbstractPhotoEffectInterface()
            {
            }

            virtual QImage apply(const QImage& image) const
            {
                int _opacity = strength();

                if (_opacity != 100)
                {
                    QImage result(image.size(),QImage::Format_ARGB32_Premultiplied);
                    QPainter p(&result);
                    p.drawImage(0,0,image);
                    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                    p.fillRect(image.rect(), QColor(0, 0, 0, _opacity*255/100));
                    return result;
                }

                return image;
            }

            virtual QString name()     const = 0;
            virtual QString toString() const = 0;
            virtual operator QString() const = 0;

            void setGroup(PhotoEffectsGroup* group)
            {
                if (group)
                {
                    m_group = group;
                    disconnect(this, SIGNAL(changed()), 0, 0);
                    connect(this, SIGNAL(changed()), (QObject*)group, SLOT(emitEffectsChanged()));
                }
            }

            PhotoEffectsGroup* group() const
            {
                return m_group;
            }

            AbstractPhotoEffectFactory* factory() const
            {
                return m_factory;
            }

            virtual QString propertyName(const QMetaProperty& property) const
            {
                if (!QString::fromLatin1("strength").compare(QLatin1String(property.name())))
                    return STRENGTH_PROPERTY;

                return QString();
            }

            virtual QVariant propertyValue(const QString& propertyName) const
            {
                if (propertyName == STRENGTH_PROPERTY)
                    return m_strength;

                return QVariant();
            }

            virtual void setPropertyValue(const QString& propertyName, const QVariant& value)
            {
                if (STRENGTH_PROPERTY == propertyName)
                    this->setStrength(value.toInt());
            }

            virtual QVariant stringNames(const QMetaProperty& /*property*/)
            {
                return QVariant();
            }

            virtual QVariant minimumValue(const QMetaProperty& property)
            {
                if (!QString::fromLatin1("strength").compare(QLatin1String(property.name())))
                    return QVariant(0);

                return QVariant();
            }

            virtual QVariant maximumValue(const QMetaProperty& property)
            {
                if (!QString::fromLatin1("strength").compare(QLatin1String(property.name())))
                    return QVariant(100);

                return QVariant();
            }

            virtual QVariant stepValue(const QMetaProperty& property)
            {
                if (!QString::fromLatin1("strength").compare(QLatin1String(property.name())))
                    return 1;

                return QVariant();
            }

            Q_PROPERTY(int strength READ strength WRITE setStrength)

            int strength() const
            {
                return m_strength;
            }

            void setStrength(int strength)
            {
                qDebug() << strength;

                if (strength < 0 || strength > 100)
                    return;

                m_strength = strength;
                propertiesChanged();
            }

        Q_SIGNALS:

            void changed();

        public:

            AbstractPhotoEffectFactory* m_factory;
            PhotoEffectsGroup*          m_group;
            int                         m_strength;

        protected:

            void propertiesChanged()
            {
                emit changed();
            }

        friend class AbstractPhotoEffectFactory;
    };
}

#endif // ABSTRACTPROTOEFFECTINTERFACE_H
