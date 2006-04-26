/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : image metadata loader interface
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

#ifndef DMETALOADER_H
#define DMETALOADER_H

// C++ includes.

#include <string>

// Qt includes.

#include <qstring.h>

// Exiv2 includes.

#include <exiv2/iptc.hpp>
#include <exiv2/exif.hpp>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class DMetadata;

class DIGIKAM_EXPORT DMetaLoader
{

public:

    virtual ~DMetaLoader() {};

    virtual bool load(const QString& filePath) = 0;
    virtual bool save(const QString& filePath) = 0;
    
    virtual bool hasExif()     const { return m_hasExif;     };
    virtual bool hasIptc()     const { return m_hasIptc;     };
    virtual bool hasComments() const { return m_hasComments; };
    virtual bool isReadOnly()  const = 0;

protected:

    DMetaLoader(DMetadata* metadata);
    
    bool loadWithExiv2(const QString& filePath);
    bool saveWithExiv2(const QString& filePath);
    
    Exiv2::ExifData& exifMetadata();
    Exiv2::IptcData& iptcMetadata();
    
    std::string& imageComments();

protected:

    bool       m_hasExif;
    bool       m_hasIptc;
    bool       m_hasComments;

    DMetadata *m_metadata;

private:
    
    DMetaLoader();    
};

}  // NameSpace Digikam

#endif /* DMETALOADER_H */
