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

// Local includes

namespace Digikam
{
  
class ImageHistoryPriv : public QSharedData
{
public:

    ImageHistoryPriv()
    {
        imageid = -1;
    }

    qlonglong imageid;
    QList<DImageHistory::Entry> entries;
};

K_GLOBAL_STATIC(ImageHistoryPriv, imageHistoryPrivSharedNull)

DImageHistory::DImageHistory()
            : d(imageHistoryPrivSharedNull)
{
}

DImageHistory::DImageHistory(const DImageHistory& other)
{
    d = other.d;
}

DImageHistory::~DImageHistory()
{
}

DImageHistory &DImageHistory::operator=(const DImageHistory& other)
{
    d = other.d;
    return *this;
}

bool DImageHistory::isNull() const
{
    return d == imageHistoryPrivSharedNull;
}

bool DImageHistory::isEmpty() const
{
    return d->entries.isEmpty();
}

int DImageHistory::size() const
{
    return d->entries.size();
}

DImageHistory &DImageHistory::operator<<(const FilterAction& action)
{
    Entry entry;
    entry.action = action;
    d->entries << entry;
    kDebug() << "Entry added, total count " << d->entries.count();
    return *this;
}

DImageHistory &DImageHistory::operator<<(const HistoryImageId& imageId)
{
    Entry entry;
    entry.referredImages = imageId;
    d->entries << entry;
    return *this;
}
DImageHistory& DImageHistory::operator<<(const Digikam::DImageHistory::Entry& entry)
{
    d->entries << entry;
    return *this;
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

const HistoryImageId &DImageHistory::referredImages(int i) const
{
    return d->entries[i].referredImages;
}


QString DImageHistory::toXml() const
{
  QByteArray xmlHistory;
  QBuffer buffer(&xmlHistory);
  buffer.open(QBuffer::ReadWrite);
  
  QXmlStreamWriter stream(&buffer);
  stream.setAutoFormatting(true);
  stream.writeStartDocument();
  
  stream.writeStartElement("history");
  
  for(int i = 0; i < entries().count(); i++)
  {
      if(!entries().at(i).referredImages.isEmpty()) {
          //this entry is a imageHistoryId
          stream.writeStartElement("file");
          stream.writeStartElement("fileParams");
          
          if(entries().at(i).referredImages.isOriginalFile()) {
              stream.writeAttribute("type", "original");
              stream.writeAttribute("fileUUID", entries().at(i).referredImages.m_originalUUID);
          } else {
              stream.writeAttribute("type", "intermediate");
              stream.writeAttribute("fileUUID", entries().at(i).referredImages.m_fileUUID);          
          }
          
          stream.writeAttribute("fileName", entries().at(i).referredImages.m_fileName);
          stream.writeAttribute("filePath", entries().at(i).referredImages.m_fileName);
              
          stream.writeEndElement(); //fileParams
          stream.writeEndElement(); //file     
          
      }
      if(!entries().at(i).action.isNull()) {
          //this entry is a filter
          stream.writeStartElement("filter");
          stream.writeAttribute("filterName", entries().at(i).action.identifier());
          stream.writeAttribute("filterVersion", QString::number(entries().at(i).action.version()));
          stream.writeAttribute("filterCategory", QString::number(entries().at(i).action.category()));
        
          stream.writeStartElement("params");

          if(!entries().at(i).action.parameters().isEmpty()) {
              QHashIterator<QString, QVariant> iter(entries().at(i).action.parameters());
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
  }
  
  stream.writeEndElement(); //history
  
  stream.writeEndDocument();
  
  buffer.close();
  
  kDebug() << xmlHistory;
  
  return QString(xmlHistory);
  
}

DImageHistory DImageHistory::fromXml(const QString& xml) //DImageHistory
{
   DImageHistory h;
   QXmlStreamReader stream(xml);
   QString originalUUID;
   
   while (!stream.atEnd()) {
         Entry entry;
         stream.readNextStartElement();
         if(stream.name() == "file" && stream.tokenType() == QXmlStreamReader::StartElement) {
                stream.readNext();
                stream.readNext();
                if(stream.attributes().value("type") == "original") {
                    originalUUID = stream.attributes().value("fileUUID").toString();
                    entry.referredImages = HistoryImageId(originalUUID, "", stream.attributes().value("fileName").toString(), QDateTime(QFileInfo(stream.attributes().value("filePath").toString()).created()));
                    h << entry;
                    continue;
                }
                else {
                    entry.referredImages = HistoryImageId(originalUUID, stream.attributes().value("fileUUID").toString(), stream.attributes().value("fileName").toString(), QDateTime(QFileInfo(stream.attributes().value("filePath").toString()).created()));
                    //stream.readNextStartElement();
                }
         }
         else if(stream.name() == "filter") {
                FilterAction::Category c;
                switch(stream.attributes().value("filterCategory").toString().toInt()) {
                  case 0: 
                    c = FilterAction::ReproducibleFilter;
                    break;
                  case 1:
                    c = FilterAction::ComplexFilter;
                    break;
                  case 2:
                    c = FilterAction::ReproducibleFilter;
                    break;
                  default:
                    c = FilterAction::ComplexFilter;
                    break;
                }
                entry.action = FilterAction(stream.attributes().value("filterName").toString(), stream.attributes().value("filterVersion").toString().toInt(), c);
                stream.readNextStartElement(); //params tag
                if(stream.name() != "params") continue;
                stream.readNext(); //param .. tag
                while(stream.name() != "params") {
                  stream.readNext();
                    if(stream.name() == "param" && !stream.attributes().value("name").isEmpty()) {
                        entry.action.addParameter(stream.attributes().value("name").toString(), stream.attributes().value("value").toString());
                    }
                }            
         }
         else continue;
         
         h << entry;
   }
   if (stream.hasError()) {
         // do error handling
   }
  
  return h;
}


} //namespace digikam

