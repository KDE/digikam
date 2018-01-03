/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GALLERY_ELEMENT_FUNCTOR_H
#define GALLERY_ELEMENT_FUNCTOR_H

// Local includes

#include "gallerynamehelper.h"

namespace Digikam
{

class GalleryInfo;
class GalleryGenerator;
class GalleryElement;

/**
 * This functor generates images (full and thumbnail) for an url and returns an
 * GalleryElement initialized to fill the xml writer.
 * It is used as an argument to QtConcurrent::mapped().
 */
class GalleryElementFunctor
{
public:

    typedef GalleryElement result_type;

public:

    explicit GalleryElementFunctor(GalleryGenerator* const generator,
                                    GalleryInfo* const info,
                                    const QString& destDir);
    ~GalleryElementFunctor();

    void operator()(GalleryElement& element);

private:

    bool writeDataToFile(const QByteArray& data, const QString& destPath);
    void emitWarning(const QString& msg);

private:

    // NOTE: Do not use a d private internal container here.

    GalleryGenerator* m_generator;
    GalleryInfo*      m_info;
    QString           m_destDir;
    GalleryNameHelper m_uniqueNameHelper;
};

} // namespace Digikam

#endif // GALLERY_ELEMENT_FUNCTOR_H
