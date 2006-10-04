/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-20
 * Description : A widget to display IPTC metadata
 * 
 * Copyright 2006 by Gilles Caulier
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

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>

// Qt includes.

#include <qmap.h>
#include <qfile.h>

// KDE includes.

#include <klocale.h>
#include <kdebug.h>

// LibExiv2 includes.

#include <exiv2/iptc.hpp>

// Local includes.

#include "dmetadata.h"
#include "iptcwidget.h"

namespace Digikam
{

static const char* IptcHumanList[] =
{
     "Headline",
     "Source",
     "DateCreated",
     "Caption",
     "Copyright",
     "Credit",
     "Contact",
     "Source",
     "Writer",
     "Copyright",
     "Program",
     "ProgramVersion",
     "Keywords",
     "Urgency",
     "-1"
};

IptcWidget::IptcWidget(QWidget* parent, const char* name)
          : MetadataWidget(parent, name)
{
    for (int i=0 ; QString(IptcHumanList[i]) != QString("-1") ; i++)
        m_tagsfilter << IptcHumanList[i];
}

IptcWidget::~IptcWidget()
{
}

QString IptcWidget::getMetadataTitle(void)
{
    return i18n("IPTC Records");
}

bool IptcWidget::loadFromURL(const KURL& url)
{
    setFileName(url.filename());

    if (url.isEmpty())
    {
        setMetadata();
        return false;
    }
    else
    {    
        DMetadata metadata(url.path());
        QByteArray iptcData = metadata.getIptc();

        if (iptcData.isEmpty())
        {
            setMetadata();
            return false;
        }
        else
            setMetadata(iptcData);
    }

    return true;
}

bool IptcWidget::decodeMetadata()
{
    try
    {
        Exiv2::IptcData iptcData;
        if (iptcData.load((Exiv2::byte*)getMetadata().data(), getMetadata().size()) != 0)
        {
            kdDebug() << "Cannot parse IPTC metadata using Exiv2" << endl;
            return false;
        }

        iptcData.sortByKey();
        
        QString ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::IptcData::iterator md = iptcData.begin(); md != iptcData.end(); ++md)
        {
            QString key = QString::fromAscii(md->key().c_str());
            
            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;
            QString value = QString::fromAscii(os.str().c_str());
            // To make a string just on one line.
            value.replace("\n", " ");

            // Some IPTC key are redondancy. check if already one exist...
            MetaDataMap::iterator it = metaDataMap.find(key);
            
            if (it == metaDataMap.end())
                metaDataMap.insert(key, value);
            else
            {
                QString v = *it;
                v.append(", ");
                v.append(value);
                metaDataMap.replace(key, v);
            }                
        }

        // Update all metadata contents.        
        setMetadataMap(metaDataMap);

        return true;
    }
    catch (Exiv2::Error& e)
    {
        kdDebug() << "Cannot parse IPTC metadata using Exiv2 ("
                  << QString::fromAscii(e.what().c_str())
                  << ")" << endl;
        return false;
    }
}

void IptcWidget::buildView(void)
{
    if (getMode() == SIMPLE)
    {
        setIfdList(getMetadataMap(), m_tagsfilter);
    }
    else
    {
        setIfdList(getMetadataMap());
    }
}

QString IptcWidget::getTagTitle(const QString& key)
{
    try 
    {
        std::string iptckey(key.ascii());
        Exiv2::IptcKey ik(iptckey); 
        return QString::fromAscii( Exiv2::IptcDataSets::dataSetTitle(ik.tag(), ik.record()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag title using Exiv2 ("
                  << QString::fromAscii(e.what().c_str())
                  << ")" << endl;
        return i18n("Unknow");
    }
}

QString IptcWidget::getTagDescription(const QString& key)
{
    try 
    {
        std::string iptckey(key.ascii());
        Exiv2::IptcKey ik(iptckey); 
        return QString::fromAscii( Exiv2::IptcDataSets::dataSetDesc(ik.tag(), ik.record()) );
    }
    catch (Exiv2::Error& e) 
    {
        kdDebug() << "Cannot get metadata tag description using Exiv2 ("
                  << QString::fromAscii(e.what().c_str())
                  << ")" << endl;
        return i18n("No description available");
    }
}

void IptcWidget::slotSaveMetadataToFile(void)
{
    KURL url = saveMetadataToFile(i18n("IPTC File to Save"),
                                  QString("*.dat|"+i18n("IPTC binary Files (*.dat)")));
    storeMetadataToFile(url);
}

}  // namespace Digikam

#include "iptcwidget.moc"
