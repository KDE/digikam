/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit-test to print item info from file using DMetadata.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_PRINT_ITEM_INFO_TEST_H
#define DIGIKAM_PRINT_ITEM_INFO_TEST_H

// Local includes

#include "abstractunittest.h"
#include "dmetadata.h"

using namespace Digikam;

class PrintItemInfoTest : public AbstractUnitTest
{
    Q_OBJECT

private:

    void printMetadataMap(const DMetadata::MetaDataMap& map);
    void printItemInfo(const QString& filePath,
                       bool com, bool ttl,            // Comments and titles
                       bool cnt, bool loc, bool isb,  // Iptc
                       bool pho, bool vid,            // Media
                       bool key, bool xsb, bool cat   // Xmp
                      );

    /// NOTE: 'expected' paramareters want mean that we expect a info container non empty
    void printComments(const DMetadata& meta, bool expected);
    void printTitles(const DMetadata& meta, bool expected);

    void printIptcContact(const DMetadata& meta, bool expected);
    void printIptcLocation(const DMetadata& meta, bool expected);
    void printIptcSubjects(const DMetadata& meta, bool expected);

    void printPhotoInfo(const DMetadata& meta, bool expected);
    void printVideoInfo(const DMetadata& meta, bool expected);

    void printXmpKeywords(const DMetadata& meta, bool expected);
    void printXmpSubjects(const DMetadata& meta, bool expected);
    void printXmpSubCategories(const DMetadata& meta, bool expected);

private Q_SLOTS:

    void testPrintItemInfo();
};

#endif // DIGIKAM_PRINT_ITEM_INFO_TEST_H
