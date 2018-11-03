/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : an unit-test tool to erase metadata tag from photo with DMetadata
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

#ifndef DIGIKAM_ERASE_METADATA_TAG_TEST_H
#define DIGIKAM_ERASE_METADATA_TAG_TEST_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

using namespace Digikam;

class EraseMetadataTagTest : public QObject
{
    Q_OBJECT

private:

    void eraseMetadataTag(const QString& file,
                          const QString& metadataTag,
                          const MetaEngineSettingsContainer& settings);

private Q_SLOTS:

    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void testEraseMetadataTag();
};

#endif // DIGIKAM_ERASE_METADATA_TAG_TEST_H
