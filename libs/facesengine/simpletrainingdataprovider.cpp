/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2010-06-16
 * Description : A convenience class to train faces
 *
 * Copyright (C)      2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "simpletrainingdataprovider.h"

namespace Digikam
{

SimpleTrainingDataProvider::SimpleTrainingDataProvider(const Identity& identity,
                                                       const QList<QImage>& newImages)
    : m_identity(identity),
      m_toTrain(newImages)
{
}

SimpleTrainingDataProvider::~SimpleTrainingDataProvider()
{
}

ImageListProvider* SimpleTrainingDataProvider::newImages(const Identity& id)
{
    if (m_identity == id)
    {
        m_toTrain.reset();
        return &m_toTrain;
    }

    return &m_empty;
}

ImageListProvider* SimpleTrainingDataProvider::images(const Identity&)
{
    return &m_empty;
}

} // namespace Digikam
