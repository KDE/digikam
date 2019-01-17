/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#include "databasefieldstest.h"

// Qt includes

#include <QTest>

// Local includes

#include "coredbfields.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(DatabaseFieldsTest)

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
    DECLARE_ITERATOR_TEST(ItemInformation)
    DECLARE_ITERATOR_TEST(ImageMetadata)
    DECLARE_ITERATOR_TEST(VideoMetadata)
    DECLARE_ITERATOR_TEST(ItemPositions)
    DECLARE_ITERATOR_TEST(ItemComments)
    DECLARE_ITERATOR_TEST(ImageHistoryInfo)
}

void DatabaseFieldsTest::testMetaInfo()
{
// Do not compile due to undefined reference to `Digikam::DatabaseFields::FieldMetaInfo<QFlags<Digikam::DatabaseFields::ImagesField> >::Last'
// Happens even though file defining those libs/database/framework/databasefields.h is included and linking against digikamgui.


/*
    QCOMPARE(FieldMetaInfo<Images>::Last, ImagesLast);
    QCOMPARE(FieldMetaInfo<ItemInformation>::Last, ItemInformationLast);
    QCOMPARE(FieldMetaInfo<ImageMetadata>::Last, ImageMetadataLast);
    QCOMPARE(FieldMetaInfo<ItemComments>::Last, ItemCommentsLast);
    QCOMPARE(FieldMetaInfo<ItemPositions>::Last, ItemPositionsLast);
    QCOMPARE(FieldMetaInfo<ImageHistoryInfo>::Last, ImageHistoryInfoLast);
    QCOMPARE(FieldMetaInfo<VideoMetadata>::Last, VideoMetadataLast);
*/
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
    DECLARE_ITERATORSETONLY_TEST(ItemInformation)
    DECLARE_ITERATORSETONLY_TEST(ImageMetadata)
    DECLARE_ITERATORSETONLY_TEST(VideoMetadata)
    DECLARE_ITERATORSETONLY_TEST(ItemPositions)
    DECLARE_ITERATORSETONLY_TEST(ItemComments)
    DECLARE_ITERATORSETONLY_TEST(ImageHistoryInfo)
}

void DatabaseFieldsTest::testSet()
{
    QCOMPARE(Set(ImagesFirst).getImages(), ImagesFirst);
    QCOMPARE(Set(ItemInformationFirst).getItemInformation(), ItemInformationFirst);
    QCOMPARE(Set(ImageMetadataFirst).getImageMetadata(), ImageMetadataFirst);
    QCOMPARE(Set(VideoMetadataFirst).getVideoMetadata(), VideoMetadataFirst);
    QCOMPARE(Set(ItemPositionsFirst).getItemPositions(), ItemPositionsFirst);
    QCOMPARE(Set(ItemCommentsFirst).getItemComments(), ItemCommentsFirst);
    QCOMPARE(Set(ImageHistoryInfoFirst).getImageHistoryInfo(), ImageHistoryInfoFirst);
}

typedef Hash<Set> SetHash;

SetHash SetHashAddSets(const SetHash& targetIn, const Set& bits)
{
    SetHash target(targetIn);

    for (DatabaseFieldsEnumIteratorSetOnly<Images> it(bits.getImages()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    for (DatabaseFieldsEnumIteratorSetOnly<ItemInformation> it(bits.getItemInformation()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    for (DatabaseFieldsEnumIteratorSetOnly<ImageMetadata> it(bits.getImageMetadata()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    for (DatabaseFieldsEnumIteratorSetOnly<VideoMetadata> it(bits.getVideoMetadata()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    for (DatabaseFieldsEnumIteratorSetOnly<ItemComments> it(bits.getItemComments()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    for (DatabaseFieldsEnumIteratorSetOnly<ItemPositions> it(bits.getItemPositions()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    for (DatabaseFieldsEnumIteratorSetOnly<ImageHistoryInfo> it(bits.getImageHistoryInfo()); !it.atEnd(); ++it)
    {
        target.insertField(*it, Set(*it));
    }

    return target;
}

void DatabaseFieldsTest::testSetHashAddSets()
{
    SetHash t;
    int itemCount = 0;

    QVERIFY(t.isEmpty());
    t = SetHashAddSets(t, DatabaseFields::Set(ImagesFirst));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ImagesFirst).getImages(), ImagesFirst);

    t = SetHashAddSets(t, DatabaseFields::Set(ImagesLast));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ImagesFirst).getImages(), ImagesFirst);

    // test insertion of or`ed values
    t = SetHashAddSets(t, DatabaseFields::Set(ItemInformationFirst | ItemInformationLast));
    itemCount+=2;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ItemInformationFirst).getItemInformation(), ItemInformationFirst);
    QCOMPARE(t.value(ItemInformationLast).getItemInformation(), ItemInformationLast);

    t = SetHashAddSets(t, DatabaseFields::Set(ImageMetadataFirst));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ImageMetadataFirst).getImageMetadata(), ImageMetadataFirst);

    t = SetHashAddSets(t, DatabaseFields::Set(VideoMetadataFirst));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(VideoMetadataFirst).getVideoMetadata(), VideoMetadataFirst);

    t = SetHashAddSets(t, DatabaseFields::Set(ItemCommentsFirst));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ItemCommentsFirst).getItemComments(), ItemCommentsFirst);

    t = SetHashAddSets(t, DatabaseFields::Set(ItemPositionsFirst));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ItemPositionsFirst).getItemPositions(), ItemPositionsFirst);

    t = SetHashAddSets(t, DatabaseFields::Set(ImageHistoryInfoFirst));
    ++itemCount;
    QCOMPARE(t.size(), itemCount);
    QCOMPARE(t.value(ImageHistoryInfoFirst).getImageHistoryInfo(), ImageHistoryInfoFirst);
}


void DatabaseFieldsTest::testHashRemoveAll()
{
    Set setToAdd =
                Set(DatabaseFields::Set(ImagesFirst | ImagesLast))
                .setFields(DatabaseFields::Set(ItemInformationFirst))
                .setFields(DatabaseFields::Set(ImageMetadataFirst))
                .setFields(DatabaseFields::Set(VideoMetadataFirst))
                .setFields(DatabaseFields::Set(ItemCommentsFirst))
                .setFields(DatabaseFields::Set(ItemPositionsFirst | ItemPositionsLast))
                .setFields(DatabaseFields::Set(ImageHistoryInfoFirst));

    SetHash t;
    t = SetHashAddSets(t, setToAdd);

    const int c1 = t.size();

    // test regular remove
    SetHash t2(t);
    QCOMPARE(t2.remove(ImagesFirst), 1);
    QCOMPARE(t2.size(), c1-1);

    // test removeAllFields: First and Last are in the hash, None is not --> 2 entries should be removed
    SetHash t3(t);
    QCOMPARE(t3.removeAllFields(ImagesFirst | ImagesLast | ImagesNone), 2);
    QCOMPARE(t3.size(), c1-2);
}

#define DECLARE_MINSIZE_TEST(FieldName)                                     \
    {                                                                       \
        const FieldMetaInfo<FieldName>::MinSizeType m = FieldName##All;     \
        const FieldName v = FieldMetaInfo<FieldName>::fromMinSizeType(m);  \
        QCOMPARE(v, FieldName##All);                                        \
    }

/**
 * Verify that the minimum size types still hold all info of the enum.
 */
void DatabaseFieldsTest::testMinSizeType()
{
    DECLARE_MINSIZE_TEST(Images)
    DECLARE_MINSIZE_TEST(ItemInformation)
    DECLARE_MINSIZE_TEST(ImageMetadata)
    DECLARE_MINSIZE_TEST(ItemComments)
    DECLARE_MINSIZE_TEST(ItemPositions)
    DECLARE_MINSIZE_TEST(ImageHistoryInfo)
    DECLARE_MINSIZE_TEST(VideoMetadata)
}
