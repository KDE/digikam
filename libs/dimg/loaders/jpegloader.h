/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at free.fr> 
 * Date  : 2005-06-14
 * Description : A JPEG IO file for DImg framework
 * 
 * Copyright 2005 by Renchi Raju, Gilles Caulier
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
 * ============================================================ */

#ifndef JPEGLOADER_H
#define JPEGLOADER_H

// Qt includes.

#include <qstring.h>

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT JPEGLoader : public DImgLoader
{

public:

    JPEGLoader(DImg* image);
    
    bool load(const QString& filePath);
    bool save(const QString& filePath);

    virtual bool hasAlpha()   const { return false; }
    virtual bool sixteenBit() const { return false; }
    virtual bool isReadOnly() const { return false; };
    
};

}  // NameSpace Digikam

#endif /* JPEGLOADER_H */
