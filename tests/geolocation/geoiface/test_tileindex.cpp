/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-12
 * Description : Test the TileIndex class
 *
 * Copyright (C) 2011 by Michael G. Hansen <mike at mghansen dot de>
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

// local includes

#include "test_tileindex.h"
#include "tileindex.h"

using namespace Digikam;

void TestTileIndex::testNoOp()
{
}

void TestTileIndex::testBasics()
{
    {
        // an empty index:
        TileIndex index1;
        QCOMPARE(index1.indexCount(), 0);
        QCOMPARE(index1.level(), 0);
        QVERIFY(index1.toIntList().isEmpty());

        TileIndex index2 = TileIndex::fromIntList(QIntList());
        QCOMPARE(index2.indexCount(), 0);
        QCOMPARE(index2.level(), 0);
        QVERIFY(index2.toIntList().isEmpty());
    }

    {
        // appending indexes:
        TileIndex index1;
        index1.appendLinearIndex(1);
        QCOMPARE(index1.indexCount(), 1);
        QCOMPARE(index1.level(), 0);
        QCOMPARE(index1.at(0), 1);
        QCOMPARE(index1.lastIndex(), 1);
    }
}

void TestTileIndex::testResizing()
{
    TileIndex i1 = TileIndex::fromIntList(QIntList()<<1<<2<<3<<4);
    QCOMPARE(i1.indexCount(), 4);

    TileIndex i2 = i1.mid(1, 2);
    QCOMPARE(i2.indexCount(), 2);
    QCOMPARE(i2.at(0), 2);
    QCOMPARE(i2.at(1), 3);

    TileIndex i3 = i1;
    QCOMPARE(i3.indexCount(), 4);
    i3.oneUp();
    QCOMPARE(i3.indexCount(), 3);
    i3.oneUp();
    QCOMPARE(i3.indexCount(), 2);
}

void TestTileIndex::testIntListInteraction()
{
    {
        for (int l = 0; l <= TileIndex::MaxLevel; ++l)
        {
            QIntList myList;
            TileIndex i1;

            for (int i = 0; i <= l; ++i)
            {
                i1.appendLinearIndex(i);
                myList << i;
            }

            const TileIndex i2 = TileIndex::fromIntList(myList);
            QVERIFY(TileIndex::indicesEqual(i1, i2, l));

            const QIntList il2 = i1.toIntList();
            QCOMPARE(myList, il2);
        }
    }
}

/**
 * TileIndex is declared as Q_MOVABLE_TYPE, and here we verify that it still works with QList.
 */
void TestTileIndex::testMovable()
{
    {
        TileIndex i1;

        for (int i = 0; i <= TileIndex::MaxLevel; ++i)
        {
            i1.appendLinearIndex(i);
        }

        TileIndex::List l1;

        for (int i = 0; i < 10; ++i)
        {
            l1 << i1;
        }

        TileIndex::List l2 = l1;
        l2[0]              = l1.at(0);

        for (int i = 0; i < l1.count(); ++i)
        {
            QVERIFY(TileIndex::indicesEqual(i1, l2.at(i), TileIndex::MaxLevel));
        }
    }

//     QBENCHMARK
//     {
//         TileIndex i1;
//
//         for (int i = 0; i <= TileIndex::MaxLevel; ++i)
//         {
//             i1.appendLinearIndex(i);
//         }
//
//         TileIndex::List l1;
//
//         for (int i = 0; i < 100000; ++i)
//         {
//             l1 << i1;
//         }
//
// //         QBENCHMARK
//         {
//             for (int i = 0; i < 100; ++i)
//             {
//                 TileIndex::List l2 = l1;
//                 l2[0]              = i1;
//             }
//         }
//     }
}

QTEST_GUILESS_MAIN(TestTileIndex)
