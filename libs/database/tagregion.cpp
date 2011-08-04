/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-26
 * Description : Tag region formatting
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include <QDebug>
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
    : m_value(descriptor), m_type(Invalid)
{
    QString xmlStartDocument = "<?xml version=\"1.0\"?>";
    QXmlStreamReader reader(xmlStartDocument + descriptor);

    if (reader.readNextStartElement())
    {
        if (reader.name() == "rect")
        {
            QRect r(reader.attributes().value("x").toString().toInt(),
                    reader.attributes().value("y").toString().toInt(),
                    reader.attributes().value("width").toString().toInt(),
                    reader.attributes().value("height").toString().toInt());

            if (r.isValid())
            {
                m_value = r;
                m_type  = Rect;
            }
        }
    }
}

TagRegion::TagRegion(const QRect& rect)
    : m_value(rect), m_type(Rect)
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
    return m_type  == other.m_type &&
           m_value == other.m_value;
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
        writer.writeStartElement("rect");
        writer.writeAttribute("x", QString::number(rect.x()));
        writer.writeAttribute("y", QString::number(rect.y()));
        writer.writeAttribute("width", QString::number(rect.width()));
        writer.writeAttribute("height", QString::number(rect.height()));
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
                return (double(i.width() * i.height()) / double(r.width() * r.height())) > fraction;
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

    double ratio = double(fullImageSize.width()) / double(reducedImageSize.width());
    return QRectF(reducedSizeDetail.x() * ratio,
                  reducedSizeDetail.y() * ratio,
                  reducedSizeDetail.width() * ratio,
                  reducedSizeDetail.height() * ratio).toRect();
}

QRect TagRegion::mapFromOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& fullSizeDetail)
{
    if (fullImageSize == reducedImageSize)
    {
        return fullSizeDetail;
    }

    double ratio = double(reducedImageSize.width()) / double(fullImageSize.width());
    return QRectF(fullSizeDetail.x() * ratio,
                  fullSizeDetail.y() * ratio,
                  fullSizeDetail.width() * ratio,
                  fullSizeDetail.height() * ratio).toRect();
}

QRect TagRegion::mapToOriginalSize(const DImg& reducedSizeImage, const QRect& reducedSizeDetail)
{
    return mapToOriginalSize(reducedSizeImage.originalSize(), reducedSizeImage.size(), reducedSizeDetail);
}

QRect TagRegion::mapFromOriginalSize(const DImg& reducedSizeImage, const QRect& fullSizeDetail)
{
    return mapFromOriginalSize(reducedSizeImage.originalSize(), reducedSizeImage.size(), fullSizeDetail);
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
