/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Date   : 2006-06-14
 * Description : A JPEG2000 IO file for DImg framework
 * 
 * Copyright 2006-2007 by Gilles Caulier
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

#ifndef JP2KLOADER_H
#define JP2KLOADER_H

// C ansi includes.

extern "C" 
{
#include <jasper/jasper.h>
}

// Qt includes.

#include <qstring.h>

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT JP2KLoader : public DImgLoader
{

public:

    JP2KLoader(DImg* image);
    
    bool load(const QString& filePath, DImgLoaderObserver *observer);
    bool save(const QString& filePath, DImgLoaderObserver *observer);

    virtual bool hasAlpha() const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const { return false; };

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
};

}  // NameSpace Digikam

#endif /* JP2KLOADER_H */
