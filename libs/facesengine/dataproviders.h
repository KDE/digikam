/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-05-18
 * Description : Wrapper class for face recognition
 *
 * Copyright (C)      2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FACESENGINE_DATAPROVIDERS_H
#define FACESENGINE_DATAPROVIDERS_H

// Qt includes

#include <QImage>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "identity.h"

namespace Digikam
{

/**
 * This class provides access to a list of unspecified entities,
 * where for each entry a QImage can be provided.
 * Only forward iteration is required.
 */
class DIGIKAM_DATABASE_EXPORT ImageListProvider
{
public:

    virtual ~ImageListProvider();

    virtual int    size() const           = 0;
    virtual bool   atEnd() const          = 0;
    virtual void   proceed(int steps = 1) = 0;
    virtual QImage image()                = 0;
};

// ----------------------------------------------------------------------------------------

/**
 * A wrapper implementation for ImageListProvider if you have a QList of QImages
 */
class DIGIKAM_DATABASE_EXPORT QListImageListProvider : public ImageListProvider
{
public:

    QListImageListProvider(const QList<QImage>& lst);
    QListImageListProvider();

    void reset();

public:

    virtual int    size()  const;
    virtual bool   atEnd() const;
    virtual void   proceed(int steps = 1);
    virtual QImage image();

public:

    QList<QImage>                 list;
    QList<QImage>::const_iterator it;
};

// ----------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT EmptyImageListProvider : public ImageListProvider
{
public:

    virtual int    size()  const;
    virtual bool   atEnd() const;
    virtual void   proceed(int steps = 1);
    virtual QImage image();
};

// ----------------------------------------------------------------------------------------

/**
 * A TrainingDataProvider provides a call-back interface
 * for the training process to retrieve the necessary information.
 * It is not specified, but depends on the backend which of the methods
 * in which order and for which identities will be called.
 */
class DIGIKAM_DATABASE_EXPORT TrainingDataProvider
{
public:

    virtual ~TrainingDataProvider();

    /**
     * Provides those images for the given identity that have not yet been
     * supplied for training.
     * Ownership of the returned object stays with the TrainingDataProvider.
     */
    virtual ImageListProvider* newImages(const Identity& identity) = 0;

    /**
     * Provides all images known for the given identity.
     * Ownership of the returned object stays with the TrainingDataProvider.
     */
    virtual ImageListProvider* images(const Identity& identity) = 0;
};

} // namespace Digikam

#endif // FACESENGINE_DATAPROVIDERS_H
