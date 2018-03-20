/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Xmp manipulation methods.
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

bool MetaEngine::canWriteXmp(const QString& filePath)
{
#ifdef _XMP_SUPPORT_
    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(filePath).constData()));

        Exiv2::AccessMode mode = image->checkMode(Exiv2::mdXmp);
        return (mode == Exiv2::amWrite || mode == Exiv2::amReadWrite);
    }
    catch( Exiv2::Error& e )
    {
        std::string s(e.what());
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Xmp access mode using Exiv2 (Error #"
                    << e.code() << ": " << s.c_str() << ")";
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(filePath);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::hasXmp() const
{
#ifdef _XMP_SUPPORT_

    return !d->xmpMetadata().empty();

#else

    return false;

#endif // _XMP_SUPPORT_
}

bool MetaEngine::clearXmp() const
{
#ifdef _XMP_SUPPORT_

    try
    {
        d->xmpMetadata().clear();
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot clear Xmp data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#endif // _XMP_SUPPORT_

    return false;
}

QByteArray MetaEngine::getXmp() const
{
#ifdef _XMP_SUPPORT_

    try
    {
        if (!d->xmpMetadata().empty())
        {

            std::string xmpPacket;
            Exiv2::XmpParser::encode(xmpPacket, d->xmpMetadata());
            QByteArray data(xmpPacket.data(), xmpPacket.size());
            return data;
        }
    }
    catch( Exiv2::Error& e )
    {
        if (!d->filePath.isEmpty())
            d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Xmp data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#endif // _XMP_SUPPORT_

    return QByteArray();
}

bool MetaEngine::setXmp(const QByteArray& data) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        if (!data.isEmpty())
        {
            std::string xmpPacket;
            xmpPacket.assign(data.data(), data.size());

            if (Exiv2::XmpParser::decode(d->xmpMetadata(), xmpPacket) != 0)
                return false;
            else
                return true;
        }
    }
    catch( Exiv2::Error& e )
    {
        if (!d->filePath.isEmpty())
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();

        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(data);

#endif // _XMP_SUPPORT_

    return false;
}

MetaEngine::MetaDataMap MetaEngine::getXmpTagsDataList(const QStringList& xmpKeysFilter, bool invertSelection) const
{
#ifdef _XMP_SUPPORT_

    if (d->xmpMetadata().empty())
       return MetaDataMap();

    try
    {
        Exiv2::XmpData xmpData = d->xmpMetadata();
        xmpData.sortByKey();

        QString     ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::XmpData::const_iterator md = xmpData.begin(); md != xmpData.end(); ++md)
        {
            QString key = QString::fromLatin1(md->key().c_str());

            // Decode the tag value with a user friendly output.
            std::ostringstream os;
            os << *md;
            QString value = QString::fromUtf8(os.str().c_str());

            // If the tag is a language alternative type, parse content to detect language.
            if (md->typeId() == Exiv2::langAlt)
            {
                QString lang;
                value = detectLanguageAlt(value, lang);
            }
            else
            {
                value = QString::fromUtf8(os.str().c_str());
            }

            // To make a string just on one line.
            value.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

            // Some XMP key are redondancy. check if already one exist...
            MetaDataMap::const_iterator it = metaDataMap.constFind(key);

            // We apply a filter to get only the XMP tags that we need.

            if (!xmpKeysFilter.isEmpty())
            {
                if (!invertSelection)
                {
                    if (xmpKeysFilter.contains(key.section(QString::fromLatin1("."), 1, 1)))
                    {
                        if (it == metaDataMap.constEnd())
                        {
                            metaDataMap.insert(key, value);
                        }
                        else
                        {
                            QString v = *it;
                            v.append(QString::fromLatin1(", "));
                            v.append(value);
                            metaDataMap.insert(key, v);
                        }
                    }
                }
                else
                {
                    if (!xmpKeysFilter.contains(key.section(QString::fromLatin1("."), 1, 1)))
                    {
                        if (it == metaDataMap.constEnd())
                        {
                            metaDataMap.insert(key, value);
                        }
                        else
                        {
                            QString v = *it;
                            v.append(QString::fromLatin1(", "));
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
                    v.append(QString::fromLatin1(", "));
                    v.append(value);
                    metaDataMap.insert(key, v);
                }
            }
        }

        return metaDataMap;
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot parse Xmp metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpKeysFilter);
    Q_UNUSED(invertSelection);

#endif // _XMP_SUPPORT_

    return MetaDataMap();
}

QString MetaEngine::getXmpTagTitle(const char* xmpTagName)
{
#ifdef _XMP_SUPPORT_

    try
    {
        std::string xmpkey(xmpTagName);
        Exiv2::XmpKey xk(xmpkey);
        return QString::fromLocal8Bit( Exiv2::XmpProperties::propertyTitle(xk) );
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Xmp metadata tag title using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);

#endif // _XMP_SUPPORT_

    return QString();
}

QString MetaEngine::getXmpTagDescription(const char* xmpTagName)
{
#ifdef _XMP_SUPPORT_
    try
    {
        std::string xmpkey(xmpTagName);
        Exiv2::XmpKey xk(xmpkey);
        return QString::fromLocal8Bit( Exiv2::XmpProperties::propertyDesc(xk) );
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Xmp metadata tag description using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);

#endif // _XMP_SUPPORT_

    return QString();
}

QString MetaEngine::getXmpTagString(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            std::ostringstream os;
            os << *it;
            QString tagValue = QString::fromUtf8(os.str().c_str());

            if (escapeCR)
                tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

            return tagValue;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image using Exiv2 ").arg(QString::fromLatin1(xmpTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QString();
}

bool MetaEngine::setXmpTagString(const char* xmpTagName, const QString& value) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        const std::string &txt(value.toUtf8().constData());
        Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::xmpText);
        xmpTxtVal->read(txt);
        d->xmpMetadata()[xmpTagName].setValue(xmpTxtVal.get());
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp tag string into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(value);

#endif // _XMP_SUPPORT_

    return false;
}
bool MetaEngine::setXmpTagString(const char* xmpTagName, const QString& value,
                                 MetaEngine::XmpTagType type) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        const std::string &txt(value.toUtf8().constData());
        Exiv2::XmpTextValue xmpTxtVal("");

        if (type == MetaEngine::NormalTag) // normal type
        {
            xmpTxtVal.read(txt);
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName),&xmpTxtVal);
            return true;
        }

        if (type == MetaEngine::ArrayBagTag) // xmp type = bag
        {
            xmpTxtVal.setXmpArrayType(Exiv2::XmpValue::xaBag);
            xmpTxtVal.read("");
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName),&xmpTxtVal);
        }

        if (type == MetaEngine::StructureTag) // xmp type = struct
        {
            xmpTxtVal.setXmpStruct();
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName),&xmpTxtVal);
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp tag string into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(value);

#endif // _XMP_SUPPORT_

    return false;
}
MetaEngine::AltLangMap MetaEngine::getXmpTagStringListLangAlt(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData = d->xmpMetadata();

        for (Exiv2::XmpData::const_iterator it = xmpData.begin(); it != xmpData.end(); ++it)
        {
            if (it->key() == xmpTagName && it->typeId() == Exiv2::langAlt)
            {
                AltLangMap map;
                const Exiv2::LangAltValue &value = static_cast<const Exiv2::LangAltValue &>(it->value());

                for (Exiv2::LangAltValue::ValueType::const_iterator it2 = value.value_.begin();
                     it2 != value.value_.end(); ++it2)
                {
                    QString lang = QString::fromUtf8(it2->first.c_str());
                    QString text = QString::fromUtf8(it2->second.c_str());

                    if (escapeCR)
                        text.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                    map.insert(lang, text);
                }

                return map;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image using Exiv2 ").arg(QString::fromLatin1(xmpTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return AltLangMap();
}

bool MetaEngine::setXmpTagStringListLangAlt(const char* xmpTagName, const MetaEngine::AltLangMap& values) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        // Remove old XMP alternative Language tag.
        removeXmpTag(xmpTagName);

        if (!values.isEmpty())
        {
            Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::langAlt);

            for (AltLangMap::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
            {
                QString lang = it.key();
                QString text = it.value();
                QString txtLangAlt = QString::fromLatin1("lang=%1 %2").arg(lang).arg(text);
                const std::string &txt(txtLangAlt.toUtf8().constData());
                xmpTxtVal->read(txt);
            }

            // ...and add the new one instead.
            d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), xmpTxtVal.get());
        }
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp tag string lang-alt into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(values);

#endif // _XMP_SUPPORT_

    return false;
}

QString MetaEngine::getXmpTagStringLangAlt(const char* xmpTagName, const QString& langAlt, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);

        for (Exiv2::XmpData::const_iterator it = xmpData.begin(); it != xmpData.end(); ++it)
        {
            if (it->key() == xmpTagName && it->typeId() == Exiv2::langAlt)
            {
                for (int i = 0; i < it->count(); i++)
                {
                    std::ostringstream os;
                    os << it->toString(i);
                    QString lang;
                    QString tagValue = QString::fromUtf8(os.str().c_str());
                    tagValue = detectLanguageAlt(tagValue, lang);

                    if (langAlt == lang)
                    {
                        if (escapeCR)
                            tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                        return tagValue;
                    }
                }
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image using Exiv2 ").arg(QString::fromLatin1(xmpTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(langAlt);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QString();
}

bool MetaEngine::setXmpTagStringLangAlt(const char* xmpTagName, const QString& value,
                                        const QString& langAlt) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        QString language(QString::fromLatin1("x-default")); // default alternative language.

        if (!langAlt.isEmpty())
            language = langAlt;

        QString txtLangAlt = QString(QString::fromLatin1("lang=%1 %2")).arg(language).arg(value);

        const std::string &txt(txtLangAlt.toUtf8().constData());
        Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::langAlt);

        // Search if an Xmp tag already exist.

        AltLangMap map = getXmpTagStringListLangAlt(xmpTagName, false);

        if (!map.isEmpty())
        {
            for (AltLangMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
            {
                if (it.key() != langAlt)
                {
                    const std::string &val((*it).toUtf8().constData());
                    xmpTxtVal->read(val);
                    qCDebug(DIGIKAM_METAENGINE_LOG) << *it;
                }
            }
        }

        xmpTxtVal->read(txt);
        removeXmpTag(xmpTagName);
        d->xmpMetadata().add(Exiv2::XmpKey(xmpTagName), xmpTxtVal.get());
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp tag string lang-alt into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(value);
    Q_UNUSED(langAlt);

#endif // _XMP_SUPPORT_

    return false;
}

QStringList MetaEngine::getXmpTagStringSeq(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            if (it->typeId() == Exiv2::xmpSeq)
            {
                QStringList seq;

                for (int i = 0; i < it->count(); i++)
                {
                    std::ostringstream os;
                    os << it->toString(i);
                    QString seqValue = QString::fromUtf8(os.str().c_str());

                    if (escapeCR)
                        seqValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                    seq.append(seqValue);
                }
                qCDebug(DIGIKAM_METAENGINE_LOG) << "XMP String Seq (" << xmpTagName << "): " << seq;

                return seq;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image using Exiv2 ").arg(QString::fromLatin1(xmpTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QStringList();
}

bool MetaEngine::setXmpTagStringSeq(const char* xmpTagName, const QStringList& seq) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        if (seq.isEmpty())
        {
            removeXmpTag(xmpTagName);
        }
        else
        {
            const QStringList list = seq;
            Exiv2::Value::AutoPtr xmpTxtSeq = Exiv2::Value::create(Exiv2::xmpSeq);

            for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
            {
                const std::string &txt((*it).toUtf8().constData());
                xmpTxtSeq->read(txt);
            }

            d->xmpMetadata()[xmpTagName].setValue(xmpTxtSeq.get());
        }
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp tag string Seq into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(seq);

#endif // _XMP_SUPPORT_

    return false;
}

QStringList MetaEngine::getXmpTagStringBag(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            if (it->typeId() == Exiv2::xmpBag)
            {
                QStringList bag;

                for (int i = 0; i < it->count(); i++)
                {
                    std::ostringstream os;
                    os << it->toString(i);
                    QString bagValue = QString::fromUtf8(os.str().c_str());

                    if (escapeCR)
                        bagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                    bag.append(bagValue);
                }

                return bag;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image using Exiv2 ").arg(QString::fromLatin1(xmpTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(escapeCR);

#endif // _XMP_SUPPORT_

    return QStringList();
}

bool MetaEngine::setXmpTagStringBag(const char* xmpTagName, const QStringList& bag) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        if (bag.isEmpty())
        {
            removeXmpTag(xmpTagName);
        }
        else
        {
            QStringList list = bag;
            Exiv2::Value::AutoPtr xmpTxtBag = Exiv2::Value::create(Exiv2::xmpBag);

            for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
            {
                const std::string &txt((*it).toUtf8().constData());
                xmpTxtBag->read(txt);
            }

            d->xmpMetadata()[xmpTagName].setValue(xmpTxtBag.get());
        }
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Xmp tag string Bag into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(bag);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::addToXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToAdd) const
{
    QStringList oldEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries = entriesToAdd;

    // Create a list of keywords including old one which already exists.
    for (QStringList::const_iterator it = oldEntries.constBegin(); it != oldEntries.constEnd(); ++it )
    {
        if (!newEntries.contains(*it))
            newEntries.append(*it);
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
        return true;

    return false;
}

bool MetaEngine::removeFromXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToRemove) const
{
    QStringList currentEntries = getXmpTagStringBag(xmpTagName, false);
    QStringList newEntries;

    // Create a list of current keywords except those that shall be removed
    for (QStringList::const_iterator it = currentEntries.constBegin(); it != currentEntries.constEnd(); ++it )
    {
        if (!entriesToRemove.contains(*it))
            newEntries.append(*it);
    }

    if (setXmpTagStringBag(xmpTagName, newEntries))
        return true;

    return false;
}

QVariant MetaEngine::getXmpTagVariant(const char* xmpTagName, bool rationalAsListOfInts, bool stringEscapeCR) const
{
#ifdef _XMP_SUPPORT_
    try
    {
        Exiv2::XmpData xmpData(d->xmpMetadata());
        Exiv2::XmpKey key(xmpTagName);
        Exiv2::XmpData::const_iterator it = xmpData.findKey(key);

        if (it != xmpData.end())
        {
            switch (it->typeId())
            {
                case Exiv2::unsignedByte:
                case Exiv2::unsignedShort:
                case Exiv2::unsignedLong:
                case Exiv2::signedShort:
                case Exiv2::signedLong:
                    return QVariant((int)it->toLong());
                case Exiv2::unsignedRational:
                case Exiv2::signedRational:
                    if (rationalAsListOfInts)
                    {
                        QList<QVariant> list;
                        list << (*it).toRational().first;
                        list << (*it).toRational().second;
                        return QVariant(list);
                    }
                    else
                    {
                        // prefer double precision
                        double num = (*it).toRational().first;
                        double den = (*it).toRational().second;

                        if (den == 0.0)
                            return QVariant(QVariant::Double);

                        return QVariant(num / den);
                    }
                case Exiv2::date:
                case Exiv2::time:
                {
                    QDateTime dateTime = QDateTime::fromString(QString::fromLatin1(it->toString().c_str()), Qt::ISODate);
                    return QVariant(dateTime);
                }
                case Exiv2::asciiString:
                case Exiv2::comment:
                case Exiv2::string:
                {
                    std::ostringstream os;
                    os << *it;
                    QString tagValue = QString::fromLocal8Bit(os.str().c_str());

                    if (stringEscapeCR)
                        tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                    return QVariant(tagValue);
                }
                case Exiv2::xmpText:
                {
                    std::ostringstream os;
                    os << *it;
                    QString tagValue = QString::fromUtf8(os.str().c_str());

                    if (stringEscapeCR)
                        tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                    return tagValue;
                }
                case Exiv2::xmpBag:
                case Exiv2::xmpSeq:
                case Exiv2::xmpAlt:
                {
                    QStringList list;

                    for (int i=0; i < it->count(); i++)
                    {
                        list << QString::fromUtf8(it->toString(i).c_str());
                    }

                    return list;
                }
                case Exiv2::langAlt:
                {
                    // access the value directly
                    const Exiv2::LangAltValue &value = static_cast<const Exiv2::LangAltValue &>(it->value());
                    QMap<QString, QVariant> map;
                    // access the ValueType std::map< std::string, std::string>
                    Exiv2::LangAltValue::ValueType::const_iterator i;

                    for (i = value.value_.begin(); i != value.value_.end(); ++i)
                    {
                        map[QString::fromUtf8(i->first.c_str())] = QString::fromUtf8(i->second.c_str());
                    }

                    return map;
                }
                default:
                    break;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Xmp key '%1' into image using Exiv2 ").arg(QString::fromLatin1(xmpTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);
    Q_UNUSED(rationalAsListOfInts);
    Q_UNUSED(stringEscapeCR);

#endif // _XMP_SUPPORT_

    return QVariant();
}

bool MetaEngine::registerXmpNameSpace(const QString& uri, const QString& prefix)
{
#ifdef _XMP_SUPPORT_

    try
    {
        QString ns = uri;

        if (!uri.endsWith(QString::fromLatin1("/")))
            ns.append(QString::fromLatin1("/"));

        Exiv2::XmpProperties::registerNs(ns.toLatin1().constData(), prefix.toLatin1().constData());
        return true;
    }
    catch( Exiv2::Error& e )
    {
        Private::printExiv2ExceptionError(QString::fromLatin1("Cannot register a new Xmp namespace using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(uri);
    Q_UNUSED(prefix);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::unregisterXmpNameSpace(const QString& uri)
{
#ifdef _XMP_SUPPORT_

    try
    {
        QString ns = uri;

        if (!uri.endsWith(QString::fromLatin1("/")))
            ns.append(QString::fromLatin1("/"));

        Exiv2::XmpProperties::unregisterNs(ns.toLatin1().constData());
        return true;
    }
    catch( Exiv2::Error& e )
    {
        Private::printExiv2ExceptionError(QString::fromLatin1("Cannot unregister a new Xmp namespace using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(uri);

#endif // _XMP_SUPPORT_

    return false;
}

bool MetaEngine::removeXmpTag(const char* xmpTagName) const
{
#ifdef _XMP_SUPPORT_

    try
    {
        Exiv2::XmpKey xmpKey(xmpTagName);
        Exiv2::XmpData::iterator it = d->xmpMetadata().findKey(xmpKey);

        if (it != d->xmpMetadata().end())
        {
            d->xmpMetadata().erase(it);
            return true;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot remove Xmp tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(xmpTagName);

#endif // _XMP_SUPPORT_

    return false;
}

QStringList MetaEngine::getXmpKeywords() const
{
    return (getXmpTagStringBag("Xmp.dc.subject", false));
}

bool MetaEngine::setXmpKeywords(const QStringList& newKeywords) const
{
    return addToXmpTagStringBag("Xmp.dc.subject", newKeywords);
}

bool MetaEngine::removeXmpKeywords(const QStringList& keywordsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove);
}

QStringList MetaEngine::getXmpSubCategories() const
{
    return (getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false));
}

bool MetaEngine::setXmpSubCategories(const QStringList& newSubCategories) const
{
    return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories);
}

bool MetaEngine::removeXmpSubCategories(const QStringList& subCategoriesToRemove)
{
    return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove);
}

QStringList MetaEngine::getXmpSubjects() const
{
    return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}

bool MetaEngine::setXmpSubjects(const QStringList& newSubjects) const
{
    return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects);
}

bool MetaEngine::removeXmpSubjects(const QStringList& subjectsToRemove)
{
    return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove);
}

MetaEngine::TagsMap MetaEngine::getXmpTagsList() const
{
    TagsMap tagsMap;
    d->getXMPTagsListFromPrefix(QString::fromLatin1("dc"),             tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("digiKam"),        tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("xmp"),            tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("xmpRights"),      tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("xmpMM"),          tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("xmpBJ"),          tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("xmpTPg"),         tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("xmpDM"),          tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("MicrosoftPhoto"), tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("pdf"),            tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("photoshop"),      tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("crs"),            tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("tiff"),           tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("exif"),           tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("aux"),            tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("iptc"),           tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("iptcExt"),        tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("plus"),           tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("mwg-rs"),         tagsMap);
    d->getXMPTagsListFromPrefix(QString::fromLatin1("dwc"),            tagsMap);
    return tagsMap;
}

}  // namespace Digikam
