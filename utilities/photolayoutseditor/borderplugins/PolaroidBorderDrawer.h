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

#ifndef POLAROIDBORDERDRAWER_H
#define POLAROIDBORDERDRAWER_H

#include "BorderDrawerInterface.h"

#include <QColor>

namespace PhotoLayoutsEditor
{
    class StandardBordersFactory;

    class PolaroidBorderDrawer : public BorderDrawerInterface
    {
            Q_OBJECT

            int m_width;
            QString m_text;
            QColor m_color;
            QFont m_font;
            QPainterPath m_path;
            QRectF m_text_rect;

            static QMap<const char *,QString> m_properties;
            static int m_default_width;
            static QString m_default_text;
            static QColor m_default_color;
            static QFont m_default_font;

        public:

            explicit PolaroidBorderDrawer(StandardBordersFactory * factory, QObject * parent = 0);

            virtual QPainterPath path(const QPainterPath & path);

            virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option);

            virtual QString propertyName(const QMetaProperty & property) const;

            virtual QVariant propertyValue(const QString & propertyName) const;

            virtual void setPropertyValue(const QString & propertyName, const QVariant & value);

            virtual QDomElement toSvg(QDomDocument & document) const;

            virtual QString name() const;

            virtual QString toString() const;

            virtual operator QString() const;

            Q_PROPERTY(int width READ width WRITE setWidth)
            int width() const
            {
                return m_width;
            }
            void setWidth(int width)
            {
                if (width > 0)
                {
                    m_default_width = m_width = width;
                    this->propertiesChanged();
                }
            }

            Q_PROPERTY(QString text READ text WRITE setText)
            QString text() const
            {
                return m_text;
            }
            void setText(const QString & text)
            {
                m_text = text;
                this->propertiesChanged();
            }

            Q_PROPERTY(QColor color READ color WRITE setColor)
            QColor color() const
            {
                return m_color;
            }
            void setColor(const QColor & color)
            {
                if (color.isValid())
                {
                    m_default_color = m_color = color;
                    this->propertiesChanged();
                }
            }

            Q_PROPERTY(QFont font READ font WRITE setFont)
            QFont font() const
            {
                return m_font;
            }
            void setFont(const QFont & font)
            {
                m_default_font = m_font = font;
                this->propertiesChanged();
            }

            virtual QVariant minimumValue(const QMetaProperty & property);
            virtual QVariant maximumValue(const QMetaProperty & property);
            virtual QVariant stepValue(const QMetaProperty & property);

        private:

            QString pathToSvg(const QPainterPath & path) const;
    };
}

#endif // POLAROIDBORDERDRAWER_H
