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

#ifndef COLORIZEPHOTOEFFECT_H
#define COLORIZEPHOTOEFFECT_H

#include "AbstractPhotoEffectInterface.h"

#define COLOR_PROPERTY QLatin1String("Color")

namespace PhotoLayoutsEditor
{
    class StandardEffectsFactory;
    class ColorizePhotoEffect : public AbstractPhotoEffectInterface
    {
            Q_OBJECT

            static QColor m_last_color;
            QColor m_color;

        public:

            explicit ColorizePhotoEffect(StandardEffectsFactory * factory, QObject * parent = 0);
            virtual QImage apply(const QImage & image) const;
            virtual QString name() const;
            virtual QString toString() const;
            virtual operator QString() const;

            virtual QString propertyName(const QMetaProperty & property) const
            {
                if (!QString::fromLatin1("color").compare(QLatin1String(property.name())))
                    return COLOR_PROPERTY;
                return AbstractPhotoEffectInterface::propertyName(property);
            }
            virtual QVariant propertyValue(const QString & propertyName) const
            {
                if (propertyName == COLOR_PROPERTY)
                    return m_color;
                return AbstractPhotoEffectInterface::propertyValue(propertyName);
            }
            virtual void setPropertyValue(const QString & propertyName, const QVariant & value)
            {
                if (COLOR_PROPERTY == propertyName)
                    this->setColor(value.value<QColor>());
                else
                    AbstractPhotoEffectInterface::setPropertyValue(propertyName, value);
            }

            Q_PROPERTY(QColor color READ color WRITE setColor)
            QColor color() const
            {
                return m_color;
            }
            void setColor(QColor color)
            {
                if (!color.isValid())
                    return;
                m_color = color;
                m_last_color = color;
                this->propertiesChanged();
            }

        private:

            static inline QImage colorized(const QImage & image, const QColor & color)
            {
                QImage result = image;
                unsigned int pixels = result.width() * result.height();
                unsigned int * data = reinterpret_cast<unsigned int*>(result.bits());
                for (unsigned int i = 0; i < pixels; ++i)
                {
                    int val = qGray(data[i]);
                    data[i] = qRgb(val,val,val);
                }
                QPainter p(&result);
                p.setCompositionMode(QPainter::CompositionMode_Overlay);
                p.fillRect(result.rect(),color);
                p.end();
                return result;
            }

        friend class StandardEffectsFactory;
    };
}

#endif // COLORIZEPHOTOEFFECT_H
