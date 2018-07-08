/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Iptc manipulation methods.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Local includes

#include "metaengine.h"
#include "metaengine_p.h"
#include "digikam_debug.h"

namespace Digikam
{

bool MetaEngine::canWriteIptc(const QString& filePath)
{
    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(filePath).constData()));

        Exiv2::AccessMode mode = image->checkMode(Exiv2::mdIptc);
        return (mode == Exiv2::amWrite || mode == Exiv2::amReadWrite);
    }
    catch(Exiv2::Error& e)
    {
        std::string s(e.what());
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Iptc access mode using Exiv2 (Error #"
                                  << e.code() << ": " << s.c_str() << ")";
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::hasIptc() const
{
    return !d->iptcMetadata().empty();
}

bool MetaEngine::clearIptc() const
{
    try
    {
        d->iptcMetadata().clear();
        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot clear Iptc data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getIptc(bool addIrbHeader) const
{
    try
    {
        if (!d->iptcMetadata().empty())
        {
            Exiv2::IptcData& iptc = d->iptcMetadata();
            Exiv2::DataBuf c2;

            if (addIrbHeader)
            {
                c2 = Exiv2::Photoshop::setIptcIrb(0, 0, iptc);
            }
            else
            {
                c2 = Exiv2::IptcParser::encode(d->iptcMetadata());
            }

            QByteArray data((const char*)c2.pData_, c2.size_);
            return data;

        }
    }
    catch(Exiv2::Error& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

bool MetaEngine::setIptc(const QByteArray& data) const
{
    try
    {
        if (!data.isEmpty())
        {
            Exiv2::IptcParser::decode(d->iptcMetadata(), (const Exiv2::byte*)data.data(), data.size());
            return (!d->iptcMetadata().empty());
        }
    }
    catch(Exiv2::Error& e)
    {
        if (!d->filePath.isEmpty())
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();
        }

        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::MetaDataMap MetaEngine::getIptcTagsDataList(const QStringList& iptcKeysFilter, bool invertSelection) const
{
    if (d->iptcMetadata().empty())
       return MetaDataMap();

    try
    {
        Exiv2::IptcData iptcData = d->iptcMetadata();
        iptcData.sortByKey();

        QString     ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::IptcData::const_iterator md = iptcData.begin(); md != iptcData.end(); ++md)
        {
            QString key = QString::fromLocal8Bit(md->key().c_str());

            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;

            QString value;

            if (key == QLatin1String("Iptc.Envelope.CharacterSet"))
            {
                value = QLatin1String(iptcData.detectCharset());
            }
            else
            {
                value = QString::fromUtf8(os.str().c_str());
            }

            // To make a string just on one line.
            value.replace(QLatin1Char('\n'), QLatin1String(" "));

            // Some Iptc key are redondancy. check if already one exist...
            MetaDataMap::const_iterator it = metaDataMap.constFind(key);

            // We apply a filter to get only the Iptc tags that we need.

            if (!iptcKeysFilter.isEmpty())
            {
                if (!invertSelection)
                {
                    if (iptcKeysFilter.contains(key.section(QLatin1String("."), 1, 1)))
                    {
                        if (it == metaDataMap.constEnd())
                        {
                            metaDataMap.insert(key, value);
                        }
                        else
                        {
                            QString v = *it;
                            v.append(QLatin1String(", "));
                            v.append(value);
                            metaDataMap.insert(key, v);
                        }
                    }
                }
                else
                {
                    if (!iptcKeysFilter.contains(key.section(QLatin1String("."), 1, 1)))
                    {
                        if (it == metaDataMap.constEnd())
                        {
                            metaDataMap.insert(key, value);
                        }
                        else
                        {
                            QString v = *it;
                            v.append(QLatin1String(", "));
                            v.append(value);
                            metaDataMap.insert(key, v);
                        }
                    }
                }
            }
            else // else no filter at all.
            {
                if (it == metaDataMap.constEnd())
                {
                    metaDataMap.insert(key, value);
                }
                else
                {
                    QString v = *it;
                    v.append(QLatin1String(", "));
                    v.append(value);
                    metaDataMap.insert(key, v);
                    }
            }

        }

        return metaDataMap;
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot parse Iptc metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return MetaDataMap();
}

QString MetaEngine::getIptcTagTitle(const char* iptcTagName)
{
    try
    {
        std::string iptckey(iptcTagName);
        Exiv2::IptcKey ik(iptckey);
        return QString::fromLocal8Bit( Exiv2::IptcDataSets::dataSetTitle(ik.tag(), ik.record()) );
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get metadata tag title using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

QString MetaEngine::getIptcTagDescription(const char* iptcTagName)
{
    try
    {
        std::string iptckey(iptcTagName);
        Exiv2::IptcKey ik(iptckey);
        return QString::fromLocal8Bit( Exiv2::IptcDataSets::dataSetDesc(ik.tag(), ik.record()) );
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get metadata tag description using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::removeIptcTag(const char* iptcTagName) const
{
    try
    {
        Exiv2::IptcData::iterator it = d->iptcMetadata().begin();
        int i                        = 0;

        while(it != d->iptcMetadata().end())
        {
            QString key = QString::fromLocal8Bit(it->key().c_str());

            if (key == QLatin1String(iptcTagName))
            {
                it = d->iptcMetadata().erase(it);
                ++i;
            }
            else
            {
                ++it;
            }
        };

        if (i > 0)
            return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot remove Iptc tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setIptcTagData(const char* iptcTagName, const QByteArray& data) const
{
    if (data.isEmpty())
        return false;

    try
    {
        Exiv2::DataValue val((Exiv2::byte *)data.data(), data.size());
        d->iptcMetadata()[iptcTagName] = val;
        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc tag data into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getIptcTagData(const char* iptcTagName) const
{
    try
    {
        Exiv2::IptcKey  iptcKey(iptcTagName);
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::const_iterator it = iptcData.findKey(iptcKey);

        if (it != iptcData.end())
        {
            char* const s = new char[(*it).size()];
            (*it).copy((Exiv2::byte*)s, Exiv2::bigEndian);
            QByteArray data(s, (*it).size());
            delete [] s;
            return data;
        }
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Iptc key '%1' into image using Exiv2 ")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

QString MetaEngine::getIptcTagString(const char* iptcTagName, bool escapeCR) const
{
    try
    {
        Exiv2::IptcKey  iptcKey(iptcTagName);
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::const_iterator it = iptcData.findKey(iptcKey);

        if (it != iptcData.end())
        {
            std::ostringstream os;
            os << *it;
            QString tagValue(QLatin1String(os.str().c_str()));

            if (escapeCR)
                tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));

            return tagValue;
        }
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Iptc key '%1' into image using Exiv2 ")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::setIptcTagString(const char* iptcTagName, const QString& value) const
{
    try
    {
        d->iptcMetadata()[iptcTagName] = std::string(value.toUtf8().constData());

        // Make sure we have set the charset to UTF-8
        d->iptcMetadata()["Iptc.Envelope.CharacterSet"] = "\33%G";
        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc tag string into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcTagsStringList(const char* iptcTagName, bool escapeCR) const
{
    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList values;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
                QString key = QString::fromLocal8Bit(it->key().c_str());

                if (key == QLatin1String(iptcTagName))
                {
                    QString tagValue = QString::fromUtf8(it->toString().c_str());

                    if (escapeCR)
                        tagValue.replace(QLatin1Char('\n'), QLatin1String(" "));

                    values.append(tagValue);
                }
            }

            return values;
        }
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Iptc key '%1' into image using Exiv2 ")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcTagsStringList(const char* iptcTagName, int maxSize,
                                       const QStringList& oldValues, const QStringList& newValues) const
{
    try
    {
        QStringList oldvals = oldValues;
        QStringList newvals = newValues;

        qCDebug(DIGIKAM_METAENGINE_LOG) << d->filePath.toLatin1().constData() << " : " << iptcTagName
                 << " => " << newvals.join(QString::fromLatin1(",")).toLatin1().constData();

        // Remove all old values.
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it = iptcData.begin();

        while(it != iptcData.end())
        {
            QString key = QString::fromLocal8Bit(it->key().c_str());
            QString val = QString::fromUtf8(it->toString().c_str());

            // Also remove new values to avoid duplicates. They will be added again below.
            if ( key == QLatin1String(iptcTagName) &&
                 (oldvals.contains(val) || newvals.contains(val))
               )
                it = iptcData.erase(it);
            else
                ++it;
        };

        // Add new values.

        Exiv2::IptcKey iptcTag(iptcTagName);

        for (QStringList::const_iterator it = newvals.constBegin(); it != newvals.constEnd(); ++it)
        {
            QString key = *it;
            key.truncate(maxSize);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8
        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Iptc key '%1' into image using Exiv2 ")
                                    .arg(QLatin1String(iptcTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcKeywords() const
{
    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList keywords;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
                QString key = QString::fromLocal8Bit(it->key().c_str());

                if (key == QLatin1String("Iptc.Application2.Keywords"))
                {
                    QString val = QString::fromUtf8(it->toString().c_str());
                    keywords.append(val);
                }
            }

            qCDebug(DIGIKAM_METAENGINE_LOG) << d->filePath << " ==> Read Iptc Keywords: " << keywords;

            return keywords;
        }
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Keywords from image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcKeywords(const QStringList& oldKeywords, const QStringList& newKeywords) const
{
    try
    {
        QStringList oldkeys = oldKeywords;
        QStringList newkeys = newKeywords;

        qCDebug(DIGIKAM_METAENGINE_LOG) << d->filePath << " ==> New Iptc Keywords: " << newkeys;

        // Remove all old keywords.
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it = iptcData.begin();

        while(it != iptcData.end())
        {
            QString key = QString::fromLocal8Bit(it->key().c_str());
            QString val = QString::fromUtf8(it->toString().c_str());

            // Also remove new keywords to avoid duplicates. They will be added again below.
            if ( key == QLatin1String("Iptc.Application2.Keywords") &&
                 (oldKeywords.contains(val) || newKeywords.contains(val))
               )
                it = iptcData.erase(it);
            else
                ++it;
        };

        // Add new keywords. Note that Keywords Iptc tag is limited to 64 char but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.Keywords");

        for (QStringList::const_iterator it = newkeys.constBegin(); it != newkeys.constEnd(); ++it)
        {
            QString key = *it;
            key.truncate(64);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8
        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc Keywords into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcSubjects() const
{
    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList subjects;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
                QString key = QString::fromLocal8Bit(it->key().c_str());

                if (key == QLatin1String("Iptc.Application2.Subject"))
                {
                    QString val(QLatin1String(it->toString().c_str()));
                    subjects.append(val);
                }
            }

            return subjects;
        }
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Subjects from image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcSubjects(const QStringList& oldSubjects, const QStringList& newSubjects) const
{
    try
    {
        QStringList oldDef = oldSubjects;
        QStringList newDef = newSubjects;

        // Remove all old subjects.
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it = iptcData.begin();

        while(it != iptcData.end())
        {
            QString key = QString::fromLocal8Bit(it->key().c_str());
            QString val = QString::fromUtf8(it->toString().c_str());

            if (key == QLatin1String("Iptc.Application2.Subject") && oldDef.contains(val))
                it = iptcData.erase(it);
            else
                ++it;
        };

        // Add new subjects. Note that Keywords Iptc tag is limited to 236 char but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.Subject");

        for (QStringList::const_iterator it = newDef.constBegin(); it != newDef.constEnd(); ++it)
        {
            QString key = *it;
            key.truncate(236);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8
        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc Subjects into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QStringList MetaEngine::getIptcSubCategories() const
{
    try
    {
        if (!d->iptcMetadata().empty())
        {
            QStringList subCategories;
            Exiv2::IptcData iptcData(d->iptcMetadata());

            for (Exiv2::IptcData::const_iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
                QString key = QString::fromLocal8Bit(it->key().c_str());

                if (key == QLatin1String("Iptc.Application2.SuppCategory"))
                {
                    QString val(QLatin1String(it->toString().c_str()));
                    subCategories.append(val);
                }
            }

            return subCategories;
        }
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Sub Categories from image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QStringList();
}

bool MetaEngine::setIptcSubCategories(const QStringList& oldSubCategories, const QStringList& newSubCategories) const
{
    try
    {
        QStringList oldkeys = oldSubCategories;
        QStringList newkeys = newSubCategories;

        // Remove all old Sub Categories.
        Exiv2::IptcData iptcData(d->iptcMetadata());
        Exiv2::IptcData::iterator it = iptcData.begin();

        while(it != iptcData.end())
        {
            QString key = QString::fromLocal8Bit(it->key().c_str());
            QString val = QString::fromUtf8(it->toString().c_str());

            if (key == QLatin1String("Iptc.Application2.SuppCategory") && oldSubCategories.contains(val))
                it = iptcData.erase(it);
            else
                ++it;
        };

        // Add new Sub Categories. Note that SubCategories Iptc tag is limited to 32
        // characters but can be redondant.

        Exiv2::IptcKey iptcTag("Iptc.Application2.SuppCategory");

        for (QStringList::const_iterator it = newkeys.constBegin(); it != newkeys.constEnd(); ++it)
        {
            QString key = *it;
            key.truncate(32);

            Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
            val->read(key.toUtf8().constData());
            iptcData.add(iptcTag, val.get());
        }

        d->iptcMetadata() = iptcData;

        // Make sure character set is UTF-8
        setIptcTagString("Iptc.Envelope.CharacterSet", QLatin1String("\33%G"));

        return true;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot set Iptc Sub Categories into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::TagsMap MetaEngine::getIptcTagsList() const
{
    try
    {
        QList<const Exiv2::DataSet*> tags;
        tags << Exiv2::IptcDataSets::envelopeRecordList()
             << Exiv2::IptcDataSets::application2RecordList();

        TagsMap tagsMap;

        for (QList<const Exiv2::DataSet*>::iterator it = tags.begin(); it != tags.end(); ++it)
        {
            do
            {
                QString     key = QLatin1String( Exiv2::IptcKey( (*it)->number_, (*it)->recordId_ ).key().c_str() );
                QStringList values;
                values << QLatin1String((*it)->name_) << QLatin1String((*it)->title_) << QLatin1String((*it)->desc_);
                tagsMap.insert(key, values);
                ++(*it);
            }
            while((*it)->number_ != 0xffff);
        }

        return tagsMap;
    }
    catch(Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QLatin1String("Cannot get Iptc Tags list using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return TagsMap();
}

} // namespace Digikam
