/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGE_GENERATION_FUNCTOR_H
#define IMAGE_GENERATION_FUNCTOR_H

// Local includes

#include "uniquenamehelper.h"

namespace Digikam
{

class GalleryInfo;
class GalleryGenerator;
class ImageElement;

/**
 * This functor generates images (full and thumbnail) for an url and returns an
 * ImageElement initialized to fill the xml writer.
 * It is used as an argument to QtConcurrent::mapped().
 */
class ImageGenerationFunctor
{
public:

    typedef ImageElement result_type;

public:

    explicit ImageGenerationFunctor(GalleryGenerator* const generator,
                                    GalleryInfo* const info,
                                    const QString& destDir);
    ~ImageGenerationFunctor();

    void operator()(ImageElement& element);

private:

    bool writeDataToFile(const QByteArray& data, const QString& destPath);
    void emitWarning(const QString& msg);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGE_GENERATION_FUNCTOR_H
