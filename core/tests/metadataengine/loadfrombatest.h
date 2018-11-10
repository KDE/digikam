/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit test to load metadata from byte array
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

#ifndef DIGIKAM_LOAD_FROM_BA_TEST_H
#define DIGIKAM_LOAD_FROM_BA_TEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

using namespace Digikam;

class LoadFromBATest : public AbstractUnitTest
{
    Q_OBJECT

private:

    void loadFromByteArray(const QString& file);

private Q_SLOTS:

    void testLoadFromByteArray();
};

#endif // DIGIKAM_LOAD_FROM_BA_TEST_H
