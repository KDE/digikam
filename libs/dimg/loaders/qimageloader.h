/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2005-06-14
 * Description : A QImage loader for DImg framework.
 * 
 * Copyright 2005 by Renchi Raju
 * Copyright 2006 by Caulier Gilles
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

#ifndef QIMAGELOADER_H
#define QIMAGELOADER_H

// Local includes.

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT QImageLoader : public DImgLoader
{
public:

    QImageLoader(DImg* image);
    
    virtual bool load(const QString& filePath, DImgLoaderObserver *observer);
    virtual bool save(const QString& filePath, DImgLoaderObserver *observer);
    
    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const { return false; };
    virtual bool isReadOnly() const { return false; };

private:

    bool m_hasAlpha;
};

}  // NameSpace Digikam

#endif /* QIMAGELOADER_H */
