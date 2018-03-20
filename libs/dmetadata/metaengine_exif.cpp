/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Exif manipulation methods
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

#include "metaengine.h"
#include "metaengine_p.h"

// C++ includes

#include <cctype>

// Qt includes

#include <QTextCodec>
#include <QBuffer>

// Local includes

#include "metaengine_rotation.h"
#include "digikam_debug.h"

namespace Digikam
{

bool MetaEngine::canWriteExif(const QString& filePath)
{
    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)
                                      (QFile::encodeName(filePath).constData()));

        Exiv2::AccessMode mode = image->checkMode(Exiv2::mdExif);

        return (mode == Exiv2::amWrite || mode == Exiv2::amReadWrite);
    }
    catch( Exiv2::Error& e )
    {
        std::string s(e.what());
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Cannot check Exif access mode using Exiv2 (Error #"
                    << e.code() << ": " << s.c_str() << ")";
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::hasExif() const
{
    return !d->exifMetadata().empty();
}

bool MetaEngine::clearExif() const
{
    try
    {
        d->exifMetadata().clear();
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot clear Exif data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getExifEncoded(bool addExifHeader) const
{
    try
    {
        if (!d->exifMetadata().empty())
        {
            QByteArray data;
            Exiv2::ExifData& exif = d->exifMetadata();
            Exiv2::Blob blob;
            Exiv2::ExifParser::encode(blob, Exiv2::bigEndian, exif);
            QByteArray ba((const char*)&blob[0], blob.size());

            if (addExifHeader)
            {
                const uchar ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
                data.resize(ba.size() + sizeof(ExifHeader));
                memcpy(data.data(), ExifHeader, sizeof(ExifHeader));
                memcpy(data.data() + sizeof(ExifHeader), ba.data(), ba.size());
            }
            else
            {
                data = ba;
            }
            return data;
        }
    }
    catch( Exiv2::Error& e )
    {
        if (!d->filePath.isEmpty())
            qCDebug(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();

        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Exif data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

bool MetaEngine::setExif(const QByteArray& data) const
{
    try
    {
        if (!data.isEmpty())
        {
            Exiv2::ExifParser::decode(d->exifMetadata(), (const Exiv2::byte*)data.data(), data.size());
            return (!d->exifMetadata().empty());
        }
    }
    catch( Exiv2::Error& e )
    {
        if (!d->filePath.isEmpty())
            qCCritical(DIGIKAM_METAENGINE_LOG) << "From file " << d->filePath.toLatin1().constData();

        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif data using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::MetaDataMap MetaEngine::getExifTagsDataList(const QStringList& exifKeysFilter, bool invertSelection) const
{
    if (d->exifMetadata().empty())
       return MetaDataMap();

    try
    {
        Exiv2::ExifData exifData = d->exifMetadata();
        exifData.sortByKey();

        QString     ifDItemName;
        MetaDataMap metaDataMap;

        for (Exiv2::ExifData::const_iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
            QString key = QString::fromLatin1(md->key().c_str());

            // Decode the tag value with a user friendly output.
            QString tagValue;

            if (key == QString::fromLatin1("Exif.Photo.UserComment"))
            {
                tagValue = d->convertCommentValue(*md);
            }
            else if (key == QString::fromLatin1("Exif.Image.0x935c"))
            {
                tagValue = QString::number(md->value().size());
            }
            else if (key == QString::fromLatin1("Exif.CanonCs.LensType") && md->toLong() == 65535)
            {
                // FIXME: workaround for a possible crash in Exiv2 pretty-print function for the Exif.CanonCs.LensType.
                tagValue = QString::fromLocal8Bit(md->toString().c_str());
            }
            else
            {
                std::ostringstream os;
                md->write(os, &exifData);

                // Exif tag contents can be an translated strings, no only simple ascii.
                tagValue = QString::fromLocal8Bit(os.str().c_str());
            }

            tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

            // We apply a filter to get only the Exif tags that we need.

            if (!exifKeysFilter.isEmpty())
            {
                if (!invertSelection)
                {
                    if (exifKeysFilter.contains(key.section(QString::fromLatin1("."), 1, 1)))
                        metaDataMap.insert(key, tagValue);
                }
                else
                {
                    if (!exifKeysFilter.contains(key.section(QString::fromLatin1("."), 1, 1)))
                        metaDataMap.insert(key, tagValue);
                }
            }
            else // else no filter at all.
            {
                metaDataMap.insert(key, tagValue);
            }
        }

        return metaDataMap;
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot parse EXIF metadata using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return MetaDataMap();
}

QString MetaEngine::getExifComment() const
{
    try
    {
        if (!d->exifMetadata().empty())
        {
            Exiv2::ExifData exifData(d->exifMetadata());
            Exiv2::ExifKey key("Exif.Photo.UserComment");
            Exiv2::ExifData::const_iterator it = exifData.findKey(key);

            if (it != exifData.end())
            {
                QString exifComment = d->convertCommentValue(*it);

                // some cameras fill the UserComment with whitespace
                if (!exifComment.isEmpty() && !exifComment.trimmed().isEmpty())
                    return exifComment;
            }

            Exiv2::ExifKey key2("Exif.Image.ImageDescription");
            Exiv2::ExifData::const_iterator it2 = exifData.findKey(key2);

            if (it2 != exifData.end())
            {
                QString exifComment = d->convertCommentValue(*it2);

                // Some cameras fill in nonsense default values
                QStringList blackList;
                blackList << QString::fromLatin1("SONY DSC"); // + whitespace
                blackList << QString::fromLatin1("OLYMPUS DIGITAL CAMERA");
                blackList << QString::fromLatin1("MINOLTA DIGITAL CAMERA");

                QString trimmedComment = exifComment.trimmed();

                // some cameras fill the UserComment with whitespace
                if (!exifComment.isEmpty() && !trimmedComment.isEmpty() && !blackList.contains(trimmedComment))
                    return exifComment;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif User Comment using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

static bool is7BitAscii(const QByteArray& s)
{
    const int size = s.size();

    for (int i=0; i<size; i++)
    {
        if (!isascii(s[i]))
        {
            return false;
        }
    }

    return true;
}

bool MetaEngine::setExifComment(const QString& comment) const
{
    try
    {
        removeExifTag("Exif.Image.ImageDescription");
        removeExifTag("Exif.Photo.UserComment");

        if (!comment.isNull())
        {
            setExifTagString("Exif.Image.ImageDescription", comment);

            // Write as Unicode only when necessary.
            QTextCodec* latin1Codec = QTextCodec::codecForName("iso8859-1");
            if (latin1Codec->canEncode(comment))
            {
                // We know it's in the ISO-8859-1 8bit range.
                // Check if it's in the ASCII 7bit range
                if (is7BitAscii(comment.toLatin1()))
                {
                    // write as ASCII
                    std::string exifComment("charset=\"Ascii\" ");
                    exifComment += comment.toLatin1().constData();
                    d->exifMetadata()["Exif.Photo.UserComment"] = exifComment;
                    return true;
                }
            }
            // write as Unicode (UCS-2)
            std::string exifComment("charset=\"Unicode\" ");
            exifComment += comment.toUtf8().constData();
            d->exifMetadata()["Exif.Photo.UserComment"] = exifComment;
        }

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif Comment using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QString MetaEngine::getExifTagTitle(const char* exifTagName)
{
    try
    {
        std::string exifkey(exifTagName);
        Exiv2::ExifKey ek(exifkey);

        return QString::fromLocal8Bit( ek.tagLabel().c_str() );
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get metadata tag title using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

QString MetaEngine::getExifTagDescription(const char* exifTagName)
{
    try
    {
        std::string exifkey(exifTagName);
        Exiv2::ExifKey ek(exifkey);

        return QString::fromLocal8Bit( ek.tagDesc().c_str() );
    }
    catch (Exiv2::Error& e)
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get metadata tag description using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::removeExifTag(const char* exifTagName) const
{
    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData::iterator it = d->exifMetadata().findKey(exifKey);

        if (it != d->exifMetadata().end())
        {
            d->exifMetadata().erase(it);
            return true;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot remove Exif tag using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::getExifTagRational(const char* exifTagName, long int& num, long int& den, int component) const
{
    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            num = (*it).toRational(component).first;
            den = (*it).toRational(component).second;

            return true;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif Rational value from key '%1' into image using Exiv2 ").arg(QString::fromLatin1(exifTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagLong(const char* exifTagName, long val) const
{
    try
    {
        d->exifMetadata()[exifTagName] = static_cast<int32_t>(val);
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif tag long value into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagRational(const char* exifTagName, long int num, long int den) const
{
    try
    {
        d->exifMetadata()[exifTagName] = Exiv2::Rational(num, den);
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif tag rational value into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagData(const char* exifTagName, const QByteArray& data) const
{
    if (data.isEmpty())
        return false;

    try
    {
        Exiv2::DataValue val((Exiv2::byte*)data.data(), data.size());
        d->exifMetadata()[exifTagName] = val;
        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif tag data into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setExifTagVariant(const char* exifTagName, const QVariant& val,
                               bool rationalWantSmallDenominator) const
{
    switch (val.type())
    {
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Bool:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            return setExifTagLong(exifTagName, val.toInt());

        case QVariant::Double:
        {
            long num, den;

            if (rationalWantSmallDenominator)
                convertToRationalSmallDenominator(val.toDouble(), &num, &den);
            else
                convertToRational(val.toDouble(), &num, &den, 4);

            return setExifTagRational(exifTagName, num, den);
        }
        case QVariant::List:
        {
            long num = 0, den = 1;
            QList<QVariant> list = val.toList();

            if (list.size() >= 1)
                num = list[0].toInt();

            if (list.size() >= 2)
                den = list[1].toInt();

            return setExifTagRational(exifTagName, num, den);
        }

        case QVariant::Date:
        case QVariant::DateTime:
        {
            QDateTime dateTime = val.toDateTime();

            if(!dateTime.isValid())
                return false;

            try
            {
                const std::string &exifdatetime(dateTime.toString(QString::fromLatin1("yyyy:MM:dd hh:mm:ss")).toLatin1().constData());
                d->exifMetadata()[exifTagName] = exifdatetime;
            }
            catch( Exiv2::Error &e )
            {
                d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Date & Time in image using Exiv2 "), e);
            }
            catch(...)
            {
                qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
            }

            return false;
        }

        case QVariant::String:
        case QVariant::Char:
            return setExifTagString(exifTagName, val.toString());

        case QVariant::ByteArray:
            return setExifTagData(exifTagName, val.toByteArray());
        default:
            break;
    }
    return false;
}

QString MetaEngine::createExifUserStringFromValue(const char* exifTagName, const QVariant& val, bool escapeCR)
{
    try
    {
        Exiv2::ExifKey key(exifTagName);
        Exiv2::Exifdatum datum(key);

        switch (val.type())
        {
            case QVariant::Int:
            case QVariant::Bool:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                datum = (int32_t)val.toInt();
                break;
            case QVariant::UInt:
                datum = (uint32_t)val.toUInt();
                break;

            case QVariant::Double:
            {
                long num, den;
                convertToRationalSmallDenominator(val.toDouble(), &num, &den);
                Exiv2::Rational rational;
                rational.first  = num;
                rational.second = den;
                datum = rational;
                break;
            }
            case QVariant::List:
            {
                long num = 0, den = 1;
                QList<QVariant> list = val.toList();
                if (list.size() >= 1)
                    num = list[0].toInt();
                if (list.size() >= 2)
                    den = list[1].toInt();
                Exiv2::Rational rational;
                rational.first  = num;
                rational.second = den;
                datum = rational;
                break;
            }

            case QVariant::Date:
            case QVariant::DateTime:
            {
                QDateTime dateTime = val.toDateTime();
                if(!dateTime.isValid())
                    break;

                const std::string &exifdatetime(dateTime.toString(QString::fromLatin1("yyyy:MM:dd hh:mm:ss")).toLatin1().constData());
                datum = exifdatetime;
                break;
            }

            case QVariant::ByteArray:
            case QVariant::String:
            case QVariant::Char:
                datum = (std::string)val.toString().toLatin1().constData();
                break;
            default:
                break;
        }

        std::ostringstream os;
        os << datum;
        QString tagValue = QString::fromLocal8Bit(os.str().c_str());

        if (escapeCR)
            tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

        return tagValue;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Iptc tag string into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::getExifTagLong(const char* exifTagName, long& val) const
{
    return getExifTagLong(exifTagName, val, 0);
}

bool MetaEngine::getExifTagLong(const char* exifTagName, long& val, int component) const
{
    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end() && it->count() > 0)
        {
            val = it->toLong(component);

            return true;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' into image using Exiv2 ").arg(QString::fromLatin1(exifTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QByteArray MetaEngine::getExifTagData(const char* exifTagName) const
{
    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            char* const s = new char[(*it).size()];
            (*it).copy((Exiv2::byte*)s, Exiv2::bigEndian);
            QByteArray data(s, (*it).size());
            delete[] s;

            return data;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' into image using Exiv2 ").arg(QString::fromLatin1(exifTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QByteArray();
}

QVariant MetaEngine::getExifTagVariant(const char* exifTagName, bool rationalAsListOfInts, bool stringEscapeCR, int component) const
{
    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            switch (it->typeId())
            {
                case Exiv2::unsignedByte:
                case Exiv2::unsignedShort:
                case Exiv2::unsignedLong:
                case Exiv2::signedShort:
                case Exiv2::signedLong:
                    if (it->count() > component)
                        return QVariant((int)it->toLong(component));
                    else
                        return QVariant(QVariant::Int);
                case Exiv2::unsignedRational:
                case Exiv2::signedRational:

                    if (rationalAsListOfInts)
                    {
                        if (it->count() <= component)
                            return QVariant(QVariant::List);

                        QList<QVariant> list;
                        list << (*it).toRational(component).first;
                        list << (*it).toRational(component).second;

                        return QVariant(list);
                    }
                    else
                    {
                        if (it->count() <= component)
                            return QVariant(QVariant::Double);

                        // prefer double precision
                        double num = (*it).toRational(component).first;
                        double den = (*it).toRational(component).second;

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
                    it->write(os, &exifData);
                    QString tagValue = QString::fromLocal8Bit(os.str().c_str());

                    if (stringEscapeCR)
                        tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

                    return QVariant(tagValue);
                }
                default:
                    break;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' in the image using Exiv2 ").arg(QString::fromLatin1(exifTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QVariant();
}

QString MetaEngine::getExifTagString(const char* exifTagName, bool escapeCR) const
{
    try
    {
        Exiv2::ExifKey exifKey(exifTagName);
        Exiv2::ExifData exifData(d->exifMetadata());
        Exiv2::ExifData::const_iterator it = exifData.findKey(exifKey);

        if (it != exifData.end())
        {
            QString tagValue;
            QString key = QString::fromLatin1(it->key().c_str());

            if (key == QString::fromLatin1("Exif.CanonCs.LensType") && it->toLong() == 65535)
            {
                // FIXME: workaround for a possible crash in Exiv2 pretty-print function for the Exif.CanonCs.LensType.
                tagValue = QString::fromLocal8Bit(it->toString().c_str());
            }
            else
            {
                // See BUG #184156 comment #13
                std::string val  = it->print(&exifData);
                tagValue = QString::fromLocal8Bit(val.c_str());
            }

            if (escapeCR)
                tagValue.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));

            return tagValue;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot find Exif key '%1' into image using Exiv2 ").arg(QString::fromLatin1(exifTagName)), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

bool MetaEngine::setExifTagString(const char* exifTagName, const QString& value) const
{
    try
    {
        d->exifMetadata()[exifTagName] = std::string(value.toLatin1().constData());

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif tag string into image using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

QImage MetaEngine::getExifThumbnail(bool fixOrientation) const
{
    QImage thumbnail;

    if (d->exifMetadata().empty())
       return thumbnail;

    try
    {
        Exiv2::ExifThumbC thumb(d->exifMetadata());
        Exiv2::DataBuf const c1 = thumb.copy();
        thumbnail.loadFromData(c1.pData_, c1.size_);

        if (!thumbnail.isNull())
        {
            if (fixOrientation)
            {
                Exiv2::ExifKey key1("Exif.Thumbnail.Orientation");
                Exiv2::ExifKey key2("Exif.Image.Orientation");
                Exiv2::ExifData exifData(d->exifMetadata());
                Exiv2::ExifData::const_iterator it = exifData.findKey(key1);

                if (it == exifData.end())
                {
                    it = exifData.findKey(key2);
                }

                if (it != exifData.end() && it->count())
                {
                    long orientation = it->toLong();
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "Exif Thumbnail Orientation: " << (int)orientation;
                    rotateExifQImage(thumbnail, (ImageOrientation)orientation);
                }

                return thumbnail;
            }
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Exif Thumbnail using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return thumbnail;
}

bool MetaEngine::rotateExifQImage(QImage& image, ImageOrientation orientation) const
{
    QMatrix matrix = MetaEngineRotation::toMatrix(orientation);

    if ((orientation != ORIENTATION_NORMAL) && (orientation != ORIENTATION_UNSPECIFIED))
    {
        image = image.transformed(matrix);
        return true;
    }

    return false;
}

bool MetaEngine::setExifThumbnail(const QImage& thumbImage) const
{
    if (thumbImage.isNull())
    {
        return removeExifThumbnail();
    }

    try
    {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        thumbImage.save(&buffer, "JPEG");
        Exiv2::ExifThumb thumb(d->exifMetadata());
        thumb.setJpegThumbnail((Exiv2::byte *)data.data(), data.size());

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set Exif Thumbnail using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::setTiffThumbnail(const QImage& thumbImage) const
{
    removeExifThumbnail();

    try
    {
        // Make sure IFD0 is explicitly marked as a main image
        Exiv2::ExifData::const_iterator pos = d->exifMetadata().findKey(Exiv2::ExifKey("Exif.Image.NewSubfileType"));

        if (pos == d->exifMetadata().end() || pos->count() != 1 || pos->toLong() != 0)
        {
            throw Exiv2::Error(1, "Exif.Image.NewSubfileType missing or not set as main image");
        }

        // Remove sub-IFD tags
        std::string subImage1("SubImage1");

        for (Exiv2::ExifData::iterator md = d->exifMetadata().begin(); md != d->exifMetadata().end();)
        {
            if (md->groupName() == subImage1)
            {
                md = d->exifMetadata().erase(md);
            }
            else
            {
                ++md;
            }
        }

        if (!thumbImage.isNull())
        {
            // Set thumbnail tags
            QByteArray data;
            QBuffer buffer(&data);
            buffer.open(QIODevice::WriteOnly);
            thumbImage.save(&buffer, "JPEG");

            Exiv2::DataBuf buf((Exiv2::byte *)data.data(), data.size());
            Exiv2::ULongValue val;
            val.read("0");
            val.setDataArea(buf.pData_, buf.size_);
            d->exifMetadata()["Exif.SubImage1.JPEGInterchangeFormat"]       = val;
            d->exifMetadata()["Exif.SubImage1.JPEGInterchangeFormatLength"] = uint32_t(buf.size_);
            d->exifMetadata()["Exif.SubImage1.Compression"]                 = uint16_t(6);          // JPEG (old-style)
            d->exifMetadata()["Exif.SubImage1.NewSubfileType"]              = uint32_t(1);          // Thumbnail image

            return true;
        }
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot set TIFF Thumbnail using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::removeExifThumbnail() const
{
    try
    {
        // Remove all IFD0 subimages.
        Exiv2::ExifThumb thumb(d->exifMetadata());
        thumb.erase();

        return true;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot remove Exif Thumbnail using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

MetaEngine::TagsMap MetaEngine::getStdExifTagsList() const
{
    try
    {
        QList<const Exiv2::TagInfo*> tags;
        TagsMap                      tagsMap;

        const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();

        while (gi->tagList_ != 0)
        {
            // NOTE: See BUG #375809 : MPF tags = execption Exiv2 0.26

            if (QString::fromLatin1(gi->ifdName_) != QString::fromLatin1("Makernote"))
            {
                Exiv2::TagListFct tl     = gi->tagList_;
                const Exiv2::TagInfo* ti = tl();

                while (ti->tag_ != 0xFFFF)
                {
                    tags << ti;
                    ++ti;
                }
            }

            ++gi;
        }

        for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin() ; it != tags.end() ; ++it)
        {
            do
            {
                const Exiv2::TagInfo* const ti = *it;

                if (ti)
                {
                    QString key = QLatin1String(Exiv2::ExifKey(*ti).key().c_str());
                    QStringList values;
                    values << QString::fromLatin1(ti->name_) << QString::fromLatin1(ti->title_) << QString::fromLatin1(ti->desc_);
                    tagsMap.insert(key, values);
                }

                ++(*it);
            }
            while((*it)->tag_ != 0xffff);
        }

        return tagsMap;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Exif Tags list using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return TagsMap();
}

MetaEngine::TagsMap MetaEngine::getMakernoteTagsList() const
{
    try
    {
        QList<const Exiv2::TagInfo*> tags;
        TagsMap                      tagsMap;

        const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();

        while (gi->tagList_ != 0)
        {
            if (QString::fromLatin1(gi->ifdName_) == QString::fromLatin1("Makernote"))
            {
                Exiv2::TagListFct tl     = gi->tagList_;
                const Exiv2::TagInfo* ti = tl();

                while (ti->tag_ != 0xFFFF)
                {
                    tags << ti;
                    ++ti;
                }
            }

            ++gi;
        }

        for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin() ; it != tags.end() ; ++it)
        {
            do
            {
                const Exiv2::TagInfo* const ti = *it;

                if (ti)
                {
                    QString key = QLatin1String(Exiv2::ExifKey(*ti).key().c_str());
                    QStringList values;
                    values << QString::fromLatin1(ti->name_) << QString::fromLatin1(ti->title_) << QString::fromLatin1(ti->desc_);
                    tagsMap.insert(key, values);
                }

                ++(*it);
            }
            while((*it)->tag_ != 0xffff);
        }

        return tagsMap;
    }
    catch( Exiv2::Error& e )
    {
        d->printExiv2ExceptionError(QString::fromLatin1("Cannot get Makernote Tags list using Exiv2 "), e);
    }
    catch(...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return TagsMap();
}

}  // namespace Digikam
