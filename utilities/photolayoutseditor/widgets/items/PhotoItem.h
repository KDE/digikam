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

#ifndef PHOTOITEM_H
#define PHOTOITEM_H

#include <QUrl>

#include "AbstractPhoto.h"

namespace PhotoLayoutsEditor
{
    class PhotoItemPixmapChangeCommand;
    class PhotoItemUrlChangeCommand;
    class PhotoItemImagePathChangeCommand;
    class PhotoItemImageMovedCommand;
    class PhotoItemLoader;

    class PhotoItem : public AbstractPhoto
    {
            Q_OBJECT

        public:

            explicit PhotoItem(const QImage & photo, const QString & name = QString(), Scene * scene = 0);
            explicit PhotoItem(const QPainterPath & shape, const QString & name = QString(), Scene * scene = 0);
            virtual ~PhotoItem();

            /// Convert photo item to SVG format
            virtual QDomDocument toSvg() const;

            /// Convert photo item to SVG template format
            virtual QDomDocument toTemplateSvg() const;

            /// Create Photo item from SVG format code
            static PhotoItem * fromSvg(QDomElement & element);

            /// Pixmap data
            Q_PROPERTY(QImage m_image READ image WRITE setImage)
            QImage & image();
            const QImage & image() const;
            void setImage(const QImage & image);

            /// Pixmap and pixmap's url
            void setImageUrl(const QUrl & url);

            /// Scales image to fit scenes rect
            void fitToRect(const QRect & rect);

            /// Reimplemented from QGraphicsItem
            virtual bool contains(const QPointF & point) const
            {
                return m_image_path.contains(point);
            }

            /// Reimplemented from AbstractPhoto
            virtual QPainterPath itemShape() const
            {
                if (this->cropShape().isEmpty())
                    return m_image_path;
                else
                    return m_image_path & this->cropShape();
            }

            /// Reimplemented from AbstractPhoto
            virtual QPainterPath itemOpaqueArea() const
            {
                if (this->cropShape().isEmpty())
                    return m_image_path;
                else
                    return m_image_path & this->cropShape();
            }

            /// Reimplemented from AbstractPhoto
            virtual QPainterPath itemDrawArea() const
            {
                return m_image_path;
            }

            /// Returns item's property browser
            virtual QtAbstractPropertyBrowser * propertyBrowser();

            /// Returns if item is empty (not contains image)
            bool isEmpty() const;

        protected:

            explicit PhotoItem(const QString & name = QString(), Scene * scene = 0);

            /// Converts item data to SVG format
            virtual QDomDocument svgVisibleArea() const;

            /// Converts item data to SVG format
            virtual QDomDocument svgTemplateArea() const;

            virtual void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
            virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
            virtual void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
            virtual void dropEvent(QGraphicsSceneDragDropEvent * event);
            virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

            /// Updates item icon
            virtual void updateIcon();

            /// Reimplemented from AbstractPhoto
            void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

        private Q_SLOTS:

            void imageLoaded(const QUrl & url, const QImage & image);

        private:

            // Refreshes items data
            virtual void refreshItem();

            // Setups items
            void setupItem(const QImage & image);

            // Recalculates item shape
            void recalcShape();

            // Highlight item
            Q_PROPERTY(bool m_highlight READ highlightItem WRITE setHighlightItem)
            bool highlightItem();
            void setHighlightItem(bool isHighlighted);
            bool m_highlight;


            class PhotoItemPrivate
            {
                PhotoItemPrivate(PhotoItem * item) :
                    m_item(item),
                    m_image_moving(false)
                {}

                static QString locateFile(const QString & filePath);

                PhotoItem * m_item;

                // Pixmap
                void setImage(const QImage & image);
                inline QImage & image();
                QImage m_image;

                // Pixmap's url
                void setFileUrl(const QUrl & url);
                inline QUrl & fileUrl();
                QUrl m_file_path;

                QTransform m_brush_transform;
                QTransform m_complete_path_transform;
                bool m_image_moving;

                friend class PhotoItem;
                friend class PhotoItemLoader;
                friend class PhotoItemPixmapChangeCommand;
                friend class PhotoItemUrlChangeCommand;
                friend class PhotoItemImageMovedCommand;
            };
            PhotoItemPrivate * d;
            friend class PhotoItemPrivate;

            QImage m_temp_image;

            // Widget path
            QPainterPath m_complete_path;
            QPainterPath m_image_path;

        friend class Scene;
        friend class PhotoItemPixmapChangeCommand;
        friend class PhotoItemUrlChangeCommand;
        friend class PhotoItemImagePathChangeCommand;
        friend class PhotoItemImageMovedCommand;
        friend class PhotoItemLoader;
    };
}

#endif // PHOTOITEM_H
