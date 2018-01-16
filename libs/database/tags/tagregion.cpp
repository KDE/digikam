/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-26
 * Description : Tag region formatting
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "tagregion.h"

// Qt includes

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// Local includes

#include "dimg.h"

namespace Digikam
{

TagRegion::TagRegion()
    : m_type(Invalid)
{
}

TagRegion::TagRegion(const QString& descriptor)
    : m_value(descriptor),
      m_type(Invalid)
{
    QString xmlStartDocument = QLatin1String("<?xml version=\"1.0\"?>");
    QXmlStreamReader reader(xmlStartDocument + descriptor);

    if (reader.readNextStartElement())
    {
        if (reader.name() == QLatin1String("rect"))
        {
            QRect r(reader.attributes().value(QLatin1String("x")).toString().toInt(),
                    reader.attributes().value(QLatin1String("y")).toString().toInt(),
                    reader.attributes().value(QLatin1String("width")).toString().toInt(),
                    reader.attributes().value(QLatin1String("height")).toString().toInt());

            if (r.isValid())
            {
                m_value = r;
                m_type  = Rect;
            }
        }
    }
}

TagRegion::TagRegion(const QRect& rect)
    : m_value(rect),
      m_type(Rect)
{
}

TagRegion::Type TagRegion::type() const
{
    return m_type;
}

bool TagRegion::isValid() const
{
    return m_type != Invalid;
}

bool TagRegion::operator==(const TagRegion& other) const
{
    return ((m_type  == other.m_type) &&
            (m_value == other.m_value));
}

QString TagRegion::toXml() const
{
    if (m_type == Invalid)
    {
        return QString();
    }

    QString output;
    QXmlStreamWriter writer(&output);
    writer.writeStartDocument();
    int lengthOfHeader = output.length();

    if (m_type == Rect)
    {
        QRect rect = m_value.toRect();
        writer.writeStartElement(QLatin1String("rect"));
        writer.writeAttribute(QLatin1String("x"),      QString::number(rect.x()));
        writer.writeAttribute(QLatin1String("y"),      QString::number(rect.y()));
        writer.writeAttribute(QLatin1String("width"),  QString::number(rect.width()));
        writer.writeAttribute(QLatin1String("height"), QString::number(rect.height()));
        writer.writeEndElement();
    }

    // cut off the <?xml> tag at start of document
    return output.mid(lengthOfHeader);
}

QRect TagRegion::toRect() const
{
    if (m_type == Rect)
    {
        return m_value.toRect();
    }

    return QRect();
}

QVariant TagRegion::toVariant() const
{
    return m_value;
}

TagRegion TagRegion::fromVariant(const QVariant& var)
{
    switch (var.type())
    {
        case QVariant::Rect:
            return TagRegion(var.toRect());
        case QVariant::String:
            return TagRegion(var.toString());
        default:
            return TagRegion();
    }
}

bool TagRegion::intersects(const TagRegion& other, double fraction)
{
    if (m_type == Invalid || other.m_type == Invalid)
    {
        return false;
    }

    if (m_type == Rect)
    {
        QRect r = toRect();

        if (other.m_type == Rect)
        {
            QRect r2 = other.toRect();

            if (fraction == 0)
            {
                return r.intersects(r2);
            }
            else if (fraction == 1)
            {
                return r.contains(r2);
            }
            else
            {
                QRect i = r.intersected(r2);

                return ( (double(i.width() * i.height()) / double(r.width() * r.height())) > fraction );
            }
        }
    }

    return false;
}

QRect TagRegion::mapToOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& reducedSizeDetail)
{
    if (fullImageSize == reducedImageSize)
    {
        return reducedSizeDetail;
    }

    double ratioWidth  = double(fullImageSize.width())  / double(reducedImageSize.width());
    double ratioHeight = double(fullImageSize.height()) / double(reducedImageSize.height());

    return QRectF(reducedSizeDetail.x()      * ratioWidth,
                  reducedSizeDetail.y()      * ratioHeight,
                  reducedSizeDetail.width()  * ratioWidth,
                  reducedSizeDetail.height() * ratioHeight).toRect();
}

QRect TagRegion::mapFromOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& fullSizeDetail)
{
    if (fullImageSize == reducedImageSize)
    {
        return fullSizeDetail;
    }

    double ratioWidth  = double(reducedImageSize.width())  / double(fullImageSize.width());
    double ratioHeight = double(reducedImageSize.height()) / double(fullImageSize.height());

    return QRectF(fullSizeDetail.x()      * ratioWidth,
                  fullSizeDetail.y()      * ratioHeight,
                  fullSizeDetail.width()  * ratioWidth,
                  fullSizeDetail.height() * ratioHeight).toRect();
}

QRect TagRegion::mapToOriginalSize(const DImg& reducedSizeImage, const QRect& reducedSizeDetail)
{
    return mapToOriginalSize(reducedSizeImage.originalSize(), reducedSizeImage.size(), reducedSizeDetail);
}

QRect TagRegion::mapFromOriginalSize(const DImg& reducedSizeImage, const QRect& fullSizeDetail)
{
    return mapFromOriginalSize(reducedSizeImage.originalSize(), reducedSizeImage.size(), fullSizeDetail);
}

QRect TagRegion::relativeToAbsolute(const QRectF& region, const QSize& fullSize)
{
    return QRectF(region.x()      * fullSize.width(),
                  region.y()      * fullSize.height(),
                  region.width()  * fullSize.width(),
                  region.height() * fullSize.height()).toRect();
}

QRect TagRegion::relativeToAbsolute(const QRectF& region, const DImg& reducedSizeImage)
{
    return relativeToAbsolute(region, reducedSizeImage.originalSize());
}

QRectF TagRegion::absoluteToRelative(const QRect& region, const QSize& fullSize)
{
    return QRectF((qreal)region.x()      / (qreal)fullSize.width(),
                  (qreal)region.y()      / (qreal)fullSize.height(),
                  (qreal)region.width()  / (qreal)fullSize.width(),
                  (qreal)region.height() / (qreal)fullSize.height());
}

QRect TagRegion::ajustToRotatedImg(const QRect& region, const QSize &fullSize, int rotation)
{
    int x, y, w, h;
    region.getRect(&x, &y, &w, &h);
    int newx, newy, neww, newh;

    if (rotation == 0) // Rotate right 90 degress
    {
       newx = fullSize.height() - y -h;
       newy = x;
       neww = h;
       newh = w;

    }
    else             // Rotate left 90 degress
    {
        newx = y;
        newy = fullSize.width() - x -w;
        neww = h;
        newh = w;
    }

    return QRect(newx, newy, neww, newh);
}

QRect TagRegion::ajustToFlippedImg(const QRect& region, const QSize& fullSize, int flip)
{
    int x, y, w, h;
    region.getRect(&x, &y, &w, &h);
    int newx, newy, neww, newh;

    if (flip == 0) // Flip horizontally
    {
       newx = fullSize.width() - x -w;
       newy = y;
       neww = w;
       newh = h;

    }
    else             // Flip vertically
    {
        newx = x;
        newy = fullSize.height() - y -h;
        neww = w;
        newh = h;
    }

    return QRect(newx, newy, neww, newh);
}

QDebug operator<<(QDebug dbg, const TagRegion& r)
{
    QVariant var = r.toVariant();

    switch (var.type())
    {
        case QVariant::Rect:
            dbg.nospace() << var.toRect();
            break;
        case QVariant::String:
            dbg.nospace() << var.toString();
            break;
        default:
            dbg.nospace() << var;
            break;
    }

    return dbg;
}

} // namespace Digikam
