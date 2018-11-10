/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-27
 * Description : an unit-test to test XMP sidecar creation with DMetadata
 *
 * Copyright (C) 2010      by Jakob Malm <jakob dot malm at gmail dot com>
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

#ifndef DIGIKAM_CREATE_XMP_SIDECAR_TEST_H
#define DIGIKAM_CREATE_XMP_SIDECAR_TEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

class CreateXmpSidecarTest : public AbstractUnitTest
{
    Q_OBJECT

private:

    void createXmpSidecar(const QString& file,
                          const MetaEngineSettingsContainer& settings);

private Q_SLOTS:

    void testCreateXmpSidecar();
};

#endif // DIGIKAM_CREATE_XMP_SIDECAR_TEST_H
