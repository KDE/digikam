/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class for manipulating modifications changeset for non-destruct. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "dimagehistory.h"

// Qt includes

#include <QFile>
#include <QSharedData>
#include <QBuffer>
#include <QHashIterator>
#include <QFileInfo>

// KDE includes

#include <kglobal.h>
#include <KDebug>
#include <KUrl>

namespace Digikam
{

class DImageHistory::ImageHistoryPriv : public QSharedData
{
public:

    ImageHistoryPriv()
    {
        imageid = -1;
    }

    QString                     originalFile;
    QString                     originalPath;
    qlonglong                   imageid;
    QList<DImageHistory::Entry> entries;
};

// -----------------------------------------------------------------------------------------------

class ImageHistoryPrivSharedNull : public QSharedDataPointer<DImageHistory::ImageHistoryPriv>
{
public:

    ImageHistoryPrivSharedNull() : QSharedDataPointer<DImageHistory::ImageHistoryPriv>(new DImageHistory::ImageHistoryPriv) {}
};

K_GLOBAL_STATIC(ImageHistoryPrivSharedNull, imageHistoryPrivSharedNull)

// -----------------------------------------------------------------------------------------------

DImageHistory::DImageHistory()
             : d(*imageHistoryPrivSharedNull)
{
}

DImageHistory::DImageHistory(const DImageHistory& other)
{
    d = other.d;
}

DImageHistory::~DImageHistory()
{
    d->entries.clear();
}

DImageHistory& DImageHistory::operator=(const DImageHistory& other)
{
    d = other.d;
    return *this;
}

bool DImageHistory::isNull() const
{
    return d == *imageHistoryPrivSharedNull;
}

bool DImageHistory::isEmpty() const
{
    if(d->entries.isEmpty())
    {
        return true;
    }
    else if (d->entries.count() == 1
             && d->entries.first().referredImages.count() == 1
             && d->entries.first().referredImages.first().isCurrentFile())
    {
        return true;
    }
    else return false;
}

bool DImageHistory::hasFilters() const
{
    for(int i = 0; i < d->entries.size(); i++)
    {
        if(!d->entries.at(i).action.isNull())
            return true;
    }
    return false;
}

int DImageHistory::size() const
{
    return d->entries.size();
}

static bool operator==(const DImageHistory::Entry &e1, const DImageHistory::Entry &e2)
{
    return e1.action == e2.action
     && e1.referredImages == e2.referredImages;
}

bool DImageHistory::operator==(const DImageHistory& other) const
{
    return d->entries == other.d->entries;
}

DImageHistory& DImageHistory::operator<<(const FilterAction& action)
{
    Entry entry;
    entry.action = action;
    d->entries << entry;
    //kDebug() << "Entry added, total count " << d->entries.count();
    return *this;
}

DImageHistory& DImageHistory::operator<<(const HistoryImageId& id)
{
    if (!id.isValid())
    {
        kWarning() << "Attempt to add an invalid HistoryImageId";
        return *this;
    }

    if (id.isCurrentFile())
    {
        // enforce to have exactly one Current id
        adjustReferredImages();
    }

    if (d->entries.isEmpty())
        d->entries << Entry();
    d->entries.last().referredImages << id;
    return *this;
}

void DImageHistory::adjustReferredImages()
{
    for (int i=0; i<d->entries.size(); ++i)
    {
        Entry &entry = d->entries[i];
        for (int e=0; e<entry.referredImages.size(); ++e)
        {
            HistoryImageId &id = entry.referredImages[e];
            if (id.isCurrentFile())
            {
                id.m_type = i==0 ? HistoryImageId::Original : HistoryImageId::Intermediate;
            }
        }
    }
}

void DImageHistory::adjustCurrentUuid(const QString& uuid)
{
    for (int i=0; i<d->entries.size(); ++i)
    {
        Entry &entry = d->entries[i];
        for (int e=0; e<entry.referredImages.size(); ++e)
        {
            HistoryImageId &id = entry.referredImages[e];
            if (id.isCurrentFile())
            {
                if (id.m_uuid.isNull())
                    id.m_uuid = uuid;
            }
        }
    }
}

bool DImageHistory::hasCurrentReferredImage() const
{
    foreach (const Entry& entry, d->entries)
    {
        foreach (const HistoryImageId& id, entry.referredImages)
        {
            if (id.isCurrentFile())
                return true;
        }
    }
    return false;
}

HistoryImageId DImageHistory::currentReferredImage() const
{
    foreach (const Entry& entry, d->entries)
    {
        foreach (const HistoryImageId& id, entry.referredImages)
        {
            if (id.isCurrentFile())
                return id;
        }
    }
    return HistoryImageId();
}

HistoryImageId DImageHistory::originalReferredImage() const
{
    foreach (const Entry& entry, d->entries)
    {
        foreach (const HistoryImageId& id, entry.referredImages)
        {
            if (id.isOriginalFile())
                return id;
        }
    }
    return HistoryImageId();
}

bool DImageHistory::operator<(const Digikam::DImageHistory& other)
{
    if(d->entries.size() < other.size())
    {
        return true;
    }
    return false;
}

bool DImageHistory::operator>(const Digikam::DImageHistory& other)
{
    if(d->entries.size() > other.size())
    {
        return true;
    }
    return false;
}


QList<DImageHistory::Entry> &DImageHistory::entries()
{
    return d->entries;
}

const QList<DImageHistory::Entry> &DImageHistory::entries() const
{
    return d->entries;
}

const FilterAction &DImageHistory::action(int i) const
{
    return d->entries[i].action;
}

QList<HistoryImageId> &DImageHistory::referredImages(int i)
{
    return d->entries[i].referredImages;
}

const QList<HistoryImageId> &DImageHistory::referredImages(int i) const
{
    return d->entries[i].referredImages;
}

QList<HistoryImageId> DImageHistory::allReferredImages() const
{
    QList<HistoryImageId> ids;
    foreach (const Entry& entry, d->entries)
        ids << entry.referredImages;
    return ids;
}

QString DImageHistory::toXml() const
{
    QString xmlHistory;

    QXmlStreamWriter stream(&xmlHistory);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    stream.writeStartElement("history");
    stream.writeAttribute("version", QString::number(1));

    for(int i = 0; i < entries().count(); i++)
    {
        const Entry& step = entries().at(i);
        if (!step.action.isNull())
        {
            stream.writeStartElement("filter");
            stream.writeAttribute("filterName", step.action.identifier());
            stream.writeAttribute("filterDisplayName", step.action.displayableName());
            stream.writeAttribute("filterVersion", QString::number(step.action.version()));
            stream.writeAttribute("filterCategory", QString::number(step.action.category()));

            stream.writeStartElement("params");

            if(!step.action.parameters().isEmpty())
            {
                QHashIterator<QString, QVariant> iter(step.action.parameters());
                while (iter.hasNext()) 
                {
                  iter.next();

                  stream.writeStartElement("param");
                  stream.writeAttribute("name", iter.key());
                  stream.writeAttribute("value", iter.value().toString());
                  stream.writeEndElement(); //param
                }
            }

            stream.writeEndElement(); //params
            stream.writeEndElement(); //filter
        }

        if (!step.referredImages.isEmpty())
        {
            foreach (const HistoryImageId& imageId, step.referredImages)
            {
                if (!imageId.isValid())
                    continue;

                if (imageId.isCurrentFile())
                    continue;

                stream.writeStartElement("file");

                if (!imageId.m_uuid.isNull())
                    stream.writeAttribute("uuid", imageId.m_uuid);

                if (imageId.isOriginalFile())
                    stream.writeAttribute("type", "original");
                else if (imageId.isSourceFile())
                    stream.writeAttribute("type", "source");

                stream.writeStartElement("fileParams");

                if (!imageId.m_fileName.isNull())
                    stream.writeAttribute("fileName", imageId.m_fileName);
                if (!imageId.m_filePath.isNull())
                    stream.writeAttribute("filePath", imageId.m_filePath);
                if (!imageId.m_uniqueHash.isNull())
                    stream.writeAttribute("fileHash", imageId.m_uniqueHash);
                if (imageId.m_fileSize)
                    stream.writeAttribute("fileSize", QString::number(imageId.m_fileSize));

                if (imageId.isOriginalFile() && !imageId.m_creationDate.isNull())
                    stream.writeAttribute("creationDate", imageId.m_creationDate.toString(Qt::ISODate));

                stream.writeEndElement(); //fileParams

                stream.writeEndElement(); //file
            }

        }
    }

    stream.writeEndElement(); //history

    stream.writeEndDocument();

    //kDebug() << xmlHistory;

    return xmlHistory;
}

DImageHistory DImageHistory::fromXml(const QString& xml) //DImageHistory
{
    //kDebug() << "Parsing image history XML";
    DImageHistory h;
    if (xml.isEmpty())
    {
        return h;
    }

    QXmlStreamReader stream(xml);
    if (!stream.readNextStartElement())
        return h;

    if (stream.name() != "history")
        return h;

    QString originalUUID;
    QDateTime originalCreationDate;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "file")
        {
            //kDebug() << "Parsing file tag";
            HistoryImageId imageId(stream.attributes().value("uuid").toString());

            if (stream.attributes().value("type") == "original")
                imageId.m_type = HistoryImageId::Original;
            else if (stream.attributes().value("type") == "source")
                imageId.m_type = HistoryImageId::Source;
            else
                imageId.m_type = HistoryImageId::Intermediate;


            while (stream.readNextStartElement())
            {
                if (stream.name() == "fileParams")
                {
                    imageId.m_fileName = stream.attributes().value("fileName").toString();
                    imageId.m_filePath = stream.attributes().value("filePath").toString();
                    QString date = stream.attributes().value("creationDate").toString();
                    if (!date.isNull())
                        imageId.m_creationDate = QDateTime::fromString(date, Qt::ISODate);
                    imageId.m_uniqueHash = stream.attributes().value("fileHash").toString();
                    QString size = stream.attributes().value("fileSize").toString();
                    if (!size.isNull())
                        imageId.m_fileSize = size.toInt();
                    stream.skipCurrentElement();
                }
                else
                {
                    stream.skipCurrentElement();
                }
            }

            if (imageId.isOriginalFile())
            {
                h.setOriginalFileName(imageId.m_fileName);
                h.setOriginalFilePath(imageId.m_filePath);
                originalUUID = imageId.m_uuid;
                originalCreationDate = imageId.m_creationDate;
            }
            else
            {
                imageId.m_originalUUID = originalUUID;
                if (imageId.m_creationDate.isNull())
                    imageId.m_creationDate = originalCreationDate;
            }

            if (imageId.isValid())
                h << imageId;

        }
        else if (stream.name() == "filter")
        {
            //kDebug() << "Parsing filter tag";
            FilterAction::Category c
                    = static_cast<FilterAction::Category>(stream.attributes().value("filterCategory").toString().toInt());
            if (c < FilterAction::CategoryFirst || c > FilterAction::CategoryLast)
                c = FilterAction::ComplexFilter;

            FilterAction action(stream.attributes().value("filterName").toString(),
                                stream.attributes().value("filterVersion").toString().toInt(), c);
            action.setDisplayableName(stream.attributes().value("filterDisplayName").toString());

            while (stream.readNextStartElement())
            {
                if (stream.name() == "params")
                {

                    while (stream.readNextStartElement())
                    {
                        if (stream.name() == "param" && stream.attributes().hasAttribute("name"))
                        {
                            action.addParameter(stream.attributes().value("name").toString(),
                                                stream.attributes().value("value").toString());
                            stream.skipCurrentElement();
                        }
                        else
                        {
                            stream.skipCurrentElement();
                        }
                    }

                }
                else
                {
                    stream.skipCurrentElement();
                }
           }

            h << action;
        }
        else
        {
            stream.skipCurrentElement();
        }
    }

    if (stream.hasError())
    {
        //TODO: error handling
        kDebug() << "An error ocurred during parsing: " << stream.errorString();
    }

    //kDebug() << "Parsing done";
    return h;
}

QString DImageHistory::originalFileName()
{
    return d->originalFile;
}

void DImageHistory::setOriginalFileName(const QString& fileName)
{
    d->originalFile = fileName;
}

QString DImageHistory::originalFilePath()
{
    return d->originalPath;
}

void DImageHistory::setOriginalFilePath(const QString& filePath)
{
    d->originalPath = filePath;
}

void DImageHistory::removeLastFilter()
{
    if(!d->entries.last().action.isNull())
        d->entries.removeLast();
}

QString DImageHistory::originalUUID() const
{
    for(int i = 0; i < entries().count(); i++)
    {
        if(!entries().at(i).referredImages.isEmpty())
        {
            for(int j = 0; j < entries().at(i).referredImages.size(); j++)
            {
                if(entries().at(i).referredImages.at(j).isOriginalFile())
                {
                    return entries().at(i).referredImages.at(j).m_uuid;
                 }
            }
        }
    }
    return QString();
}

} // namespace digikam