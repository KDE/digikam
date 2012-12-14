/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class for manipulating modifications changeset for non-destruct. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <kdebug.h>
#include <kurl.h>

namespace Digikam
{

class DImageHistory::Private : public QSharedData
{
public:

    Private()
    {
    }

    QList<DImageHistory::Entry> entries;
};

// -----------------------------------------------------------------------------------------------

class PrivateSharedNull : public QSharedDataPointer<DImageHistory::Private>
{
public:

    PrivateSharedNull()
        : QSharedDataPointer<DImageHistory::Private>(new DImageHistory::Private)
    {
    }
};

K_GLOBAL_STATIC(PrivateSharedNull, imageHistoryPrivSharedNull)

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
    return d->entries.isEmpty();
}

bool DImageHistory::isValid() const
{
    if (d->entries.isEmpty())
    {
        return false;
    }
    else if (d->entries.count() == 1                        &&
             d->entries.first().referredImages.count() == 1 &&
             d->entries.first().referredImages.first().isCurrentFile())
    {
        return false;
    }
    else
    {
        foreach(const Entry& e, d->entries)
        {
            if (!e.action.isNull())
            {
                return true;
            }

            foreach(const HistoryImageId& id, e.referredImages)
            {
                if (id.isValid() && !id.isCurrentFile())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

int DImageHistory::size() const
{
    return d->entries.size();
}

static bool operator==(const DImageHistory::Entry& e1, const DImageHistory::Entry& e2)
{
    return e1.action         == e2.action &&
           e1.referredImages == e2.referredImages;
}

bool DImageHistory::operator==(const DImageHistory& other) const
{
    return d->entries == other.d->entries;
}

bool DImageHistory::operator<(const Digikam::DImageHistory& other) const
{
    if (d->entries.size() < other.size())
    {
        return true;
    }

    return false;
}

bool DImageHistory::operator>(const Digikam::DImageHistory& other) const
{
    if (d->entries.size() > other.size())
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

DImageHistory::Entry& DImageHistory::operator[](int i)
{
    return d->entries[i];
}

const DImageHistory::Entry& DImageHistory::operator[](int i) const
{
    return d->entries.at(i);
}

DImageHistory& DImageHistory::operator<<(const FilterAction& action)
{
    if (action.isNull())
    {
        return *this;
    }

    Entry entry;
    entry.action = action;
    d->entries << entry;
    //kDebug() << "Entry added, total count " << d->entries.count();
    return *this;
}

DImageHistory& DImageHistory::operator<<(const HistoryImageId& id)
{
    appendReferredImage(id);
    return *this;
}

void DImageHistory::appendReferredImage(const HistoryImageId& id)
{
    insertReferredImage(d->entries.size() - 1, id);
}

void DImageHistory::insertReferredImage(int index, const HistoryImageId& id)
{
    if (!id.isValid())
    {
        kWarning() << "Attempt to add an invalid HistoryImageId";
        return;
    }

    index = qBound(0, index, d->entries.size() - 1);

    if (id.isCurrentFile())
    {
        // enforce to have exactly one Current id
        adjustReferredImages();
    }

    if (d->entries.isEmpty())
    {
        d->entries << Entry();
    }

    d->entries[index].referredImages << id;
}

void DImageHistory::removeLast()
{
    if (!d->entries.isEmpty())
    {
        d->entries.removeLast();
    }
}

const FilterAction& DImageHistory::action(int i) const
{
    return d->entries.at(i).action;
}

QList<FilterAction> DImageHistory::allActions() const
{
    QList<FilterAction> actions;

    foreach(const Entry& entry, d->entries)
    {
        if (!entry.action.isNull())
        {
            actions << entry.action;
        }
    }

    return actions;
}

int DImageHistory::actionCount() const
{
    int count = 0;

    foreach(const Entry& entry, d->entries)
    {
        if (!entry.action.isNull())
        {
            ++count;
        }
    }

    return count;
}

bool DImageHistory::hasActions() const
{
    foreach(const Entry& entry, d->entries)
    {
        if (!entry.action.isNull())
        {
            return true;
        }
    }

    return false;
}

QList<HistoryImageId> &DImageHistory::referredImages(int i)
{
    return d->entries[i].referredImages;
}

const QList<HistoryImageId> &DImageHistory::referredImages(int i) const
{
    return d->entries.at(i).referredImages;
}

QList<HistoryImageId> DImageHistory::allReferredImages() const
{
    QList<HistoryImageId> ids;

    foreach(const Entry& entry, d->entries)
    {
        ids << entry.referredImages;
    }

    return ids;
}

bool DImageHistory::hasReferredImages() const
{
    foreach(const Entry& entry, d->entries)
    {
        if (!entry.referredImages.isEmpty())
        {
            return true;
        }
    }

    return false;
}

bool DImageHistory::hasReferredImageOfType(HistoryImageId::Type type) const
{
    foreach(const Entry& entry, d->entries)
    {
        foreach(const HistoryImageId& id, entry.referredImages)
        {
            if (id.m_type == type)
            {
                return true;
            }
        }
    }

    return false;
}

bool DImageHistory::hasCurrentReferredImage() const
{
    return hasReferredImageOfType(HistoryImageId::Current);
}

bool DImageHistory::hasOriginalReferredImage() const
{
    return hasReferredImageOfType(HistoryImageId::Original);
}

QList<HistoryImageId> DImageHistory::referredImagesOfType(HistoryImageId::Type type) const
{
    QList<HistoryImageId> ids;

    foreach(const Entry& entry, d->entries)
    {
        foreach(const HistoryImageId& id, entry.referredImages)
        {
            if (id.m_type == type)
            {
                ids << id;
            }
        }
    }

    return ids;
}

HistoryImageId DImageHistory::currentReferredImage() const
{
    foreach(const Entry& entry, d->entries)
    {
        foreach(const HistoryImageId& id, entry.referredImages)
        {
            if (id.isCurrentFile())
            {
                return id;
            }
        }
    }

    return HistoryImageId();
}

HistoryImageId DImageHistory::originalReferredImage() const
{
    foreach(const Entry& entry, d->entries)
    {
        foreach(const HistoryImageId& id, entry.referredImages)
        {
            if (id.isOriginalFile())
            {
                return id;
            }
        }
    }

    return HistoryImageId();
}

void DImageHistory::clearReferredImages()
{
    for (int i=0; i<d->entries.size(); ++i)
    {
        d->entries[i].referredImages.clear();
    }
}

void DImageHistory::adjustReferredImages()
{
    for (int i=0; i<d->entries.size(); ++i)
    {
        Entry& entry = d->entries[i];

        for (int e=0; e<entry.referredImages.size(); ++e)
        {
            HistoryImageId& id = entry.referredImages[e];

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
        Entry& entry = d->entries[i];

        for (int e=0; e<entry.referredImages.size(); ++e)
        {
            HistoryImageId& id = entry.referredImages[e];

            if (id.isCurrentFile())
            {
                if (id.m_uuid.isNull())
                {
                    id.m_uuid = uuid;
                }
            }
        }
    }
}

void DImageHistory::purgePathFromReferredImages(const QString& path, const QString& fileName)
{
    for (int i=0; i<d->entries.size(); ++i)
    {
        Entry& entry = d->entries[i];

        for (int e=0; e<entry.referredImages.size(); ++e)
        {
            HistoryImageId& id = entry.referredImages[e];
            {
                if (id.m_filePath == path && id.m_fileName == fileName)
                {
                    id.m_filePath.clear();
                    id.m_fileName.clear();
                }
            }
        }
    }
}

void DImageHistory::moveCurrentReferredImage(const QString& newPath, const QString& newFileName)
{
    for (int i=0; i<d->entries.size(); ++i)
    {
        Entry& entry = d->entries[i];

        for (int e=0; e<entry.referredImages.size(); ++e)
        {
            HistoryImageId& id = entry.referredImages[e];

            if (id.isCurrentFile())
            {
                id.setPath(newPath);
                id.setFileName(newFileName);
            }
        }
    }
}

QString DImageHistory::toXml() const
{
    QString xmlHistory;

    QXmlStreamWriter stream(&xmlHistory);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("history");
    stream.writeAttribute("version", QString::number(1));

    for (int i = 0; i < entries().count(); ++i)
    {
        const Entry& step = entries().at(i);

        if (!step.action.isNull())
        {
            stream.writeStartElement("filter");
            stream.writeAttribute("filterName", step.action.identifier());
            stream.writeAttribute("filterDisplayName", step.action.displayableName());
            stream.writeAttribute("filterVersion", QString::number(step.action.version()));

            switch (step.action.category())
            {
                case FilterAction::ReproducibleFilter:
                    stream.writeAttribute("filterCategory", "reproducible");
                    break;
                case FilterAction::ComplexFilter:
                    stream.writeAttribute("filterCategory", "complex");
                    break;
                case FilterAction::DocumentedHistory:
                    stream.writeAttribute("filterCategory", "documentedHistory");
                    break;
            }

            if (step.action.flags() & FilterAction::ExplicitBranch)
            {
                stream.writeAttribute("branch", "true");
            }

            stream.writeStartElement("params");

            const QHash<QString,QVariant>& params = step.action.parameters();

            if (!params.isEmpty())
            {
                QList<QString> keys = params.keys();
                qSort(keys);

                foreach(const QString& key, keys)
                {
                    QHash<QString, QVariant>::const_iterator it;

                    for (it = params.find(key); it != params.end() && it.key() == key; ++it)
                    {
                        stream.writeStartElement("param");
                        stream.writeAttribute("name", key);
                        stream.writeAttribute("value", it.value().toString());
                        stream.writeEndElement(); //param
                    }
                }
            }

            stream.writeEndElement(); //params
            stream.writeEndElement(); //filter
        }

        if (!step.referredImages.isEmpty())
        {
            foreach(const HistoryImageId& imageId, step.referredImages)
            {
                if (!imageId.isValid())
                {
                    continue;
                }

                if (imageId.isCurrentFile())
                {
                    continue;
                }

                stream.writeStartElement("file");

                if (!imageId.m_uuid.isNull())
                {
                    stream.writeAttribute("uuid", imageId.m_uuid);
                }

                if (imageId.isOriginalFile())
                {
                    stream.writeAttribute("type", "original");
                }
                else if (imageId.isSourceFile())
                {
                    stream.writeAttribute("type", "source");
                }

                stream.writeStartElement("fileParams");

                if (!imageId.m_fileName.isNull())
                {
                    stream.writeAttribute("fileName", imageId.m_fileName);
                }

                if (!imageId.m_filePath.isNull())
                {
                    stream.writeAttribute("filePath", imageId.m_filePath);
                }

                if (!imageId.m_uniqueHash.isNull())
                {
                    stream.writeAttribute("fileHash", imageId.m_uniqueHash);
                }

                if (imageId.m_fileSize)
                {
                    stream.writeAttribute("fileSize", QString::number(imageId.m_fileSize));
                }

                if (imageId.isOriginalFile() && !imageId.m_creationDate.isNull())
                {
                    stream.writeAttribute("creationDate", imageId.m_creationDate.toString(Qt::ISODate));
                }

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
    {
        return h;
    }

    if (stream.name() != "history")
    {
        return h;
    }

    QString originalUUID;
    QDateTime originalCreationDate;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "file")
        {
            //kDebug() << "Parsing file tag";
            HistoryImageId imageId(stream.attributes().value("uuid").toString());

            if (stream.attributes().value("type") == "original")
            {
                imageId.m_type = HistoryImageId::Original;
            }
            else if (stream.attributes().value("type") == "source")
            {
                imageId.m_type = HistoryImageId::Source;
            }
            else
            {
                imageId.m_type = HistoryImageId::Intermediate;
            }


            while (stream.readNextStartElement())
            {
                if (stream.name() == "fileParams")
                {
                    imageId.setFileName(stream.attributes().value("fileName").toString());
                    imageId.setPath(stream.attributes().value("filePath").toString());
                    QString date = stream.attributes().value("creationDate").toString();

                    if (!date.isNull())
                    {
                        imageId.setCreationDate(QDateTime::fromString(date, Qt::ISODate));
                    }

                    QString size = stream.attributes().value("fileSize").toString();

                    if (stream.attributes().hasAttribute("fileHash") && !size.isNull())
                    {
                        imageId.setUniqueHash(stream.attributes().value("fileHash").toString(), size.toInt());
                    }

                    stream.skipCurrentElement();
                }
                else
                {
                    stream.skipCurrentElement();
                }
            }

            if (imageId.isOriginalFile())
            {
                originalUUID = imageId.m_uuid;
                originalCreationDate = imageId.m_creationDate;
            }
            else
            {
                imageId.m_originalUUID = originalUUID;

                if (imageId.m_creationDate.isNull())
                {
                    imageId.m_creationDate = originalCreationDate;
                }
            }

            if (imageId.isValid())
            {
                h << imageId;
            }

        }
        else if (stream.name() == "filter")
        {
            //kDebug() << "Parsing filter tag";
            FilterAction::Category c = FilterAction::ComplexFilter;
            QStringRef categoryString = stream.attributes().value("filterCategory");

            if (categoryString == "reproducible")
            {
                c = FilterAction::ReproducibleFilter;
            }
            else if (categoryString == "complex")
            {
                c = FilterAction::ComplexFilter;
            }
            else if (categoryString == "documentedHistory")
            {
                c = FilterAction::DocumentedHistory;
            }

            FilterAction action(stream.attributes().value("filterName").toString(),
                                stream.attributes().value("filterVersion").toString().toInt(), c);
            action.setDisplayableName(stream.attributes().value("filterDisplayName").toString());

            if (stream.attributes().value("branch") == "true")
            {
                action.addFlag(FilterAction::ExplicitBranch);
            }

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
        kDebug() << "An error occurred during parsing: " << stream.errorString();
    }

    //kDebug() << "Parsing done";
    return h;
}

} // namespace digikam
