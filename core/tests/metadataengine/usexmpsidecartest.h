/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read/write metadata from XMP sidecar with DMetadata.
 *
 * Copyright (C) 2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_USE_XMP_SIDECAR_TEST_H
#define DIGIKAM_USE_XMP_SIDECAR_TEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

class UseXmpSidecarTest : public AbstractUnitTest
{
    Q_OBJECT

private:

    void useXmpSidecar(const QString& file,
                       const MetaEngineSettingsContainer& settings);

private Q_SLOTS:

    void testUseXmpSidecar();
};

#endif // DIGIKAM_USE_XMP_SIDECAR_TEST_H
