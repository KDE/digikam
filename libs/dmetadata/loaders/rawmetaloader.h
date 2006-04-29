/* ============================================================
 * Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : RAW file metadata loader
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
 
#ifndef RAWMETALOADER_H
#define RAWMETALOADER_H

// Qt includes.

#include <qstring.h>

// Local includes.

#include "dmetaloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RAWMetaLoader : public DMetaLoader
{

public:

    RAWMetaLoader(DMetadata* metadata);
    ~RAWMetaLoader();

    bool load(const QString& filePath);
    bool save(const QString& filePath);

    bool isReadOnly() const { return true; };
};

}  // NameSpace Digikam

#endif /* RAWMETALOADER_H */
