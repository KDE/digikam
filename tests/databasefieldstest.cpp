/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-06-29
 * Description : Test the functions for dealing with DatabaseFields
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
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

#include "databasefieldstest.moc"

// KDE includes

#include <qtest_kde.h>

// Local includes

#include "databasefields.h"

using namespace Digikam;

QTEST_KDEMAIN(DatabaseFieldsTest, GUI)

using namespace DatabaseFields;

void DatabaseFieldsTest::cleanupTestCase()
{

}

void DatabaseFieldsTest::initTestCase()
{

}

#define DECLARE_ITERATOR_TEST(Field)                    \
{                                                       \
    Field i;                                            \
    for (Field##Iterator it; !it.atEnd(); ++it)         \
    {                                                   \
        i |= *it;                                       \
    }                                                   \
    QCOMPARE(i, Field##All);                            \
}

void DatabaseFieldsTest::testIterators()
{
    // test that the iterator iterates over all fields
    DECLARE_ITERATOR_TEST(Images)
    DECLARE_ITERATOR_TEST(ImageInformation)
    DECLARE_ITERATOR_TEST(ImageMetadata)
    DECLARE_ITERATOR_TEST(VideoMetadata)
    DECLARE_ITERATOR_TEST(ImagePositions)
    DECLARE_ITERATOR_TEST(ImageComments)
    DECLARE_ITERATOR_TEST(ImageHistoryInfo)
}

void DatabaseFieldsTest::testMetaInfo()
{
    QCOMPARE(FieldMetaInfo<Images>::Last, ImagesLast);
    QCOMPARE(FieldMetaInfo<ImageInformation>::Last, ImageInformationLast);
    QCOMPARE(FieldMetaInfo<ImageMetadata>::Last, ImageMetadataLast);
    QCOMPARE(FieldMetaInfo<ImageComments>::Last, ImageCommentsLast);
    QCOMPARE(FieldMetaInfo<ImagePositions>::Last, ImagePositionsLast);
    QCOMPARE(FieldMetaInfo<ImageHistoryInfo>::Last, ImageHistoryInfoLast);
    QCOMPARE(FieldMetaInfo<VideoMetadata>::Last, VideoMetadataLast);
}

#define DECLARE_ITERATORSETONLY_TEST(Field)                                                 \
    for (Field i = Field##None; i<=Field##All; i=Field(int(i)+1))                           \
    {                                                                                       \
        Field i2 = Field##None;                                                             \
        for (DatabaseFieldsEnumIteratorSetOnly<Field> iOnly(i); !iOnly.atEnd(); ++iOnly)    \
        {                                                                                   \
            i2 |= *iOnly;                                                                   \
        }                                                                                   \
        QCOMPARE(i, i2);                                                                    \
    }

void DatabaseFieldsTest::testIteratorsSetOnly()
{
    DECLARE_ITERATORSETONLY_TEST(Images)
    DECLARE_ITERATORSETONLY_TEST(ImageInformation)
    DECLARE_ITERATORSETONLY_TEST(ImageMetadata)
    DECLARE_ITERATORSETONLY_TEST(VideoMetadata)
    DECLARE_ITERATORSETONLY_TEST(ImagePositions)
    DECLARE_ITERATORSETONLY_TEST(ImageComments)
    DECLARE_ITERATORSETONLY_TEST(ImageHistoryInfo)
}
