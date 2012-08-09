/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A QImage loader for DImg framework.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

// Local includes

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT QImageLoader : public DImgLoader
{
public:

    QImageLoader(DImg* image);

    virtual bool load(const QString& filePath, DImgLoaderObserver* observer);
    virtual bool save(const QString& filePath, DImgLoaderObserver* observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const
    {
        return false;
    };
    virtual bool isReadOnly() const
    {
        return false;
    };

private:

    bool m_hasAlpha;
};

}  // namespace Digikam

#endif /* QIMAGELOADER_H */
