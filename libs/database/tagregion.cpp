/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-26
 * Description : Tag region formatting
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

QString TagRegion::toXml() const
{
    if (m_type == Invalid)
        return QString();

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
        return m_value.toRect();
    return QRect();
}

QRect TagRegion::toCandyRect() const
{
    int gap = 70;
    QRect r = toRect();
    r.setTop(r.top() - gap);
    r.setBottom(r.bottom() + gap);
    r.setLeft(r.left() - gap);
    r.setRight(r.right() + gap);

    return r;
}

QRect TagRegion::mapToOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& reducedSizeDetail)
{
    if (fullImageSize == reducedImageSize)
        return reducedSizeDetail;
    double ratio =  double(fullImageSize.width()) / double(reducedImageSize.width());
    return QRectF(reducedSizeDetail.x() * ratio,
                  reducedSizeDetail.y() * ratio,
                  reducedSizeDetail.width() * ratio,
                  reducedSizeDetail.height() * ratio).toRect();
}

QRect TagRegion::mapFromOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& fullSizeDetail)
{
    if (fullImageSize == reducedImageSize)
        return fullSizeDetail;

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


}

