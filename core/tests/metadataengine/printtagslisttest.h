/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-12
 * Description : An unit-test to print all available metadata tags provided by Exiv2.
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

#ifndef DIGIKAM_PRINT_TAGS_LIST_TEST_H
#define DIGIKAM_PRINT_TAGS_LIST_TEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

using namespace Digikam;

class PrintTagsListTest : public AbstractUnitTest
{
    Q_OBJECT

private Q_SLOTS:

    void testPrintAllAvailableStdExifTags();
    void testPrintAllAvailableMakernotesTags();
    void testPrintAllAvailableIptcTags();
    void testPrintAllAvailableXmpTags();
};

#endif // DIGIKAM_PRINT_TAGS_LIST_TEST_H
