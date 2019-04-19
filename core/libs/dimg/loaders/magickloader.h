/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : ImageMagick loader for DImg framework.
 *
 * Copyright (C) 2019 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGEMAGICK_LOADER_H
#define DIGIKAM_IMAGEMAGICK_LOADER_H

// Local includes

#include "dimgloader.h"
#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{
class DImg;

class DIGIKAM_EXPORT MagickLoader : public DImgLoader
{
public:

    explicit MagickLoader(DImg* const image);

    virtual bool load(const QString& filePath, DImgLoaderObserver* const observer);
    virtual bool save(const QString& filePath, DImgLoaderObserver* const observer);

    virtual bool hasAlpha()   const;
    virtual bool sixteenBit() const;
    virtual bool isReadOnly() const;

private:

    bool m_hasAlpha;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGEMAGICK_LOADER_H
