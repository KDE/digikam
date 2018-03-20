/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Internal private container.
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

#ifndef META_ENGINE_PRIVATE_H
#define META_ENGINE_PRIVATE_H

#include "metaengine.h"

 // C++ includes

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <iomanip>
#include <string>

// Qt includes

#include <QFile>
#include <QSize>
#include <QLatin1String>
#include <QFileInfo>
#include <QSharedData>

// Exiv2 includes -------------------------------------------------------

// NOTE: All Exiv2 header must be stay there to not expose external source code to Exiv2 API
//       and reduce Exiv2 dependency to client code.

// The pragmas are required to be able to catch exceptions thrown by libexiv2:
// See http://gcc.gnu.org/wiki/Visibility, the section about c++ exceptions.
// They are needed for all libexiv2 versions that do not care about visibility.
#ifdef __GNUC__
#   pragma GCC visibility push(default)
#endif

#include <exiv2/exv_conf.h>
#include <exiv2/error.hpp>
#include <exiv2/image.hpp>
#include <exiv2/jpgimage.hpp>
#include <exiv2/datasets.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/preview.hpp>
#include <exiv2/properties.hpp>
#include <exiv2/types.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/xmpsidecar.hpp>

// Check if Exiv2 support XMP

#ifdef EXV_HAVE_XMP_TOOLKIT
#   define _XMP_SUPPORT_ 1
#endif

// Make sure an EXIV2_TEST_VERSION macro exists:

#ifdef EXIV2_VERSION
#    ifndef EXIV2_TEST_VERSION
#        define EXIV2_TEST_VERSION(major,minor,patch) \
         ( EXIV2_VERSION >= EXIV2_MAKE_VERSION(major,minor,patch) )
#    endif
#else
#    define EXIV2_TEST_VERSION(major,minor,patch) (false)
#endif

// With exiv2 > 0.20.0, all makernote header files have been removed to increase binary compatibility.
// See Exiv2 bugzilla entry http://dev.exiv2.org/issues/719
// and wiki topic           http://dev.exiv2.org/boards/3/topics/583

#ifdef __GNUC__
#   pragma GCC visibility pop
#endif

// End of Exiv2 headers ------------------------------------------------------

namespace Digikam
{

class Q_DECL_HIDDEN MetaEngineData::Private : public QSharedData
{
public:

    void clear();

public:

    std::string     imageComments;

    Exiv2::ExifData exifMetadata;

    Exiv2::IptcData iptcMetadata;

#ifdef _XMP_SUPPORT_
    Exiv2::XmpData  xmpMetadata;
#endif
};

// --------------------------------------------------------------------------

class Q_DECL_HIDDEN MetaEngine::Private
{
public:

    explicit Private();
    virtual ~Private();

    void copyPrivateData(const Private* const other);

    bool saveToXMPSidecar(const QFileInfo& finfo)                                 const;
    bool saveToFile(const QFileInfo& finfo)                                       const;
    bool saveOperations(const QFileInfo& finfo, Exiv2::Image::AutoPtr image)      const;

    /** Wrapper method to convert a Comments content to a QString.
     */
    QString convertCommentValue(const Exiv2::Exifdatum& exifDatum)                const;

    /** Charset autodetection to convert a string to a QString.
     */
    QString detectEncodingAndDecode(const std::string& value)                     const;

    /** UTF8 autodetection from a string.
     */
    bool isUtf8(const char* const buffer)                                         const;

    int getXMPTagsListFromPrefix(const QString& pf, MetaEngine::TagsMap& tagsMap) const;

    const Exiv2::ExifData& exifMetadata()  const { return data.constData()->exifMetadata;  }
    const Exiv2::IptcData& iptcMetadata()  const { return data.constData()->iptcMetadata;  }
    const std::string&     imageComments() const { return data.constData()->imageComments; }

#ifdef _XMP_SUPPORT_
    const Exiv2::XmpData&  xmpMetadata()   const { return data.constData()->xmpMetadata;   }
#endif

    Exiv2::ExifData&       exifMetadata()        { return data.data()->exifMetadata;       }
    Exiv2::IptcData&       iptcMetadata()        { return data.data()->iptcMetadata;       }
    std::string&           imageComments()       { return data.data()->imageComments;      }

#ifdef _XMP_SUPPORT_
    Exiv2::XmpData&        xmpMetadata()         { return data.data()->xmpMetadata;        }

    void loadSidecarData(Exiv2::Image::AutoPtr xmpsidecar);
#endif

public:

    /** Generic method to print the Exiv2 C++ Exception error message from 'e'.
     *  'msg' string is printed using qDebug rules.
     */
    static void printExiv2ExceptionError(const QString& msg, Exiv2::Error& e);

    /** Generic method to print debug message from Exiv2.
     *  'msg' string is printed using qDebug rules. 'lvl' is the debug level of Exiv2 message.
     */
    static void printExiv2MessageHandler(int lvl, const char* msg);

public:

    bool                                        writeRawFiles;
    bool                                        updateFileTimeStamp;

    bool                                        useXMPSidecar4Reading;

    /// A mode from #MetadataWritingMode enum.
    int                                         metadataWritingMode;

    /// XMP, and parts of EXIF/IPTC, were loaded from an XMP sidecar file
    bool                                        loadedFromSidecar;

    QString                                     filePath;
    QSize                                       pixelSize;
    QString                                     mimeType;

    QSharedDataPointer<MetaEngineData::Private> data;
};

// --------------------------------------------------------------------------------------------

template <class Data, class Key, class KeyString, class KeyStringList = QList<KeyString> >

class MergeHelper
{
public:

    KeyStringList keys;

    MergeHelper& operator<<(const KeyString& key)
    {
        keys << key;
        return *this;
    }

    /**
      * Merge two (Exif,IPTC,Xmp) Data packages, where the result is stored in dest
      * and fields from src take precedence over existing data from dest.
      */
    void mergeAll(const Data& src, Data& dest)
    {
        for (typename Data::const_iterator it = src.begin() ; it != src.end() ; ++it)
        {
            typename Data::iterator destIt = dest.findKey(Key(it->key()));

            if (destIt == dest.end())
            {
                dest.add(*it);
            }
            else
            {
                *destIt = *it;
            }
        }
    }

    /**
     * Merge two (Exif,IPTC,Xmp) Data packages, the result is stored in dest.
     * Only keys in keys are considered for merging.
     * Fields from src take precedence over existing data from dest.
     */
    void mergeFields(const Data& src, Data& dest)
    {
        foreach (const KeyString& keyString, keys)
        {
            Key key(keyString.latin1());
            typename Data::const_iterator it = src.findKey(key);

            if (it == src.end())
            {
                continue;
            }

            typename Data::iterator destIt = dest.findKey(key);

            if (destIt == dest.end())
            {
                dest.add(*it);
            }
            else
            {
                *destIt = *it;
            }
        }
    }

    /**
     * Merge two (Exif,IPTC,Xmp) Data packages, the result is stored in dest.
     * The following steps apply only to keys in "keys":
     * The result is determined by src.
     * Keys must exist in src to kept in dest.
     * Fields from src take precedence over existing data from dest.
     */
    void exclusiveMerge(const Data& src, Data& dest)
    {
        foreach (const KeyString& keyString, keys)
        {
            Key key(keyString.latin1());
            typename Data::const_iterator it = src.findKey(key);
            typename Data::iterator destIt   = dest.findKey(key);

            if (destIt == dest.end())
            {
                if (it != src.end())
                {
                    dest.add(*it);
                }
            }
            else
            {
                if (it == src.end())
                {
                    dest.erase(destIt);
                }
                else
                {
                    *destIt = *it;
                }
            }
        }
    }
};

class ExifMergeHelper : public MergeHelper<Exiv2::ExifData, Exiv2::ExifKey, QLatin1String>
{
};

class IptcMergeHelper : public MergeHelper<Exiv2::IptcData, Exiv2::IptcKey, QLatin1String>
{
};

#ifdef _XMP_SUPPORT_
class XmpMergeHelper : public MergeHelper<Exiv2::XmpData, Exiv2::XmpKey, QLatin1String>
{
};
#endif

} // namespace Digikam

#endif // META_ENGINE_PRIVATE_H
