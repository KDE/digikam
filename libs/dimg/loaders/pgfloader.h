/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PGFLOADER_H
#define PGFLOADER_H

// Local includes

#include "dimgloader.h"
#include "digikam_export.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT PGFLoader : public DImgLoader
{

public:

    explicit PGFLoader(DImg* const image);

    bool load(const QString& filePath, DImgLoaderObserver* const observer);
    bool save(const QString& filePath, DImgLoaderObserver* const observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const;

    bool progressCallback(double percent, bool escapeAllowed);

private:

    bool                m_sixteenBit;
    bool                m_hasAlpha;

    DImgLoaderObserver* m_observer;
};

}  // namespace Digikam

#endif /* PGFLOADER_H */
