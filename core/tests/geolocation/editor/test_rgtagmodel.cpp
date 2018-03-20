/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : Test for RG tag model.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#include "test_rgtagmodel.h"

// Qt includes

#include <QDebug>
#include <QUrl>

// local includes

#include "simpletreemodel.h"
#include "rgtagmodel.h"
#include "modeltest.h"

using namespace Digikam;

/**
 * @brief Dummy test that does nothing
 */
void TestRGTagModel::testNoOp()
{
}

/**
 * @brief Create an RGTagModel, but leave it empty
 */
void TestRGTagModel::testModelEmpty()
{
    SimpleTreeModel* const treeModel = new SimpleTreeModel(1, this);
    new ModelTest(treeModel, this);

    RGTagModel* const tagModel = new RGTagModel(treeModel, this);
    new ModelTest(tagModel, this);
}

void TestRGTagModel::testModel2()
{
    SimpleTreeModel* const treeModel = new SimpleTreeModel(1, this);
    new ModelTest(treeModel, this);

    // add some items before the tagModel is created:
    SimpleTreeModel::Item* const treeItem1 = treeModel->addItem();
    const QPersistentModelIndex treeItem1Index = treeModel->itemToIndex(treeItem1);
    SimpleTreeModel::Item* const treeItem11 = treeModel->addItem(treeItem1);
    const QPersistentModelIndex treeItem11Index = treeModel->itemToIndex(treeItem11);

    SimpleTreeModel::Item* const treeItem2 = treeModel->addItem();
    const QPersistentModelIndex treeItem2Index = treeModel->itemToIndex(treeItem2);

    RGTagModel* const tagModel = new RGTagModel(treeModel, this);
    // modeltest will be created at the end of this function to make sure it does not influence the result!

    // first, verify the row and column counts:
    QCOMPARE(tagModel->rowCount(QModelIndex()), 2);
    QCOMPARE(tagModel->columnCount(QModelIndex()), 1);

    // now test toSourceIndex:
    const QModelIndex tagItem1Index = tagModel->index(0, 0);
    Q_ASSERT(tagItem1Index.isValid());
    QCOMPARE(tagModel->rowCount(tagItem1Index), 1);
    QCOMPARE(tagModel->columnCount(tagItem1Index), 1);

    const QModelIndex tagItem1IndexSource = tagModel->toSourceIndex(tagItem1Index);
    Q_ASSERT(tagItem1IndexSource.isValid());
    Q_ASSERT(tagItem1IndexSource==treeItem1Index);

    const QModelIndex tagItem2Index = tagModel->index(1, 0);
    Q_ASSERT(tagItem2Index.isValid());
    QCOMPARE(tagModel->rowCount(tagItem2Index), 0);
    QCOMPARE(tagModel->columnCount(tagItem2Index), 1);

    const QModelIndex tagItem2IndexSource = tagModel->toSourceIndex(tagItem2Index);
    Q_ASSERT(tagItem2IndexSource.isValid());
    Q_ASSERT(tagItem2IndexSource==treeItem2Index);

    const QModelIndex tagItem11Index = tagModel->index(0, 0, tagItem1Index);
    Q_ASSERT(tagItem11Index.isValid());
    QCOMPARE(tagModel->rowCount(tagItem11Index), 0);
    QCOMPARE(tagModel->columnCount(tagItem11Index), 1);

    const QModelIndex tagItem11IndexSource = tagModel->toSourceIndex(tagItem11Index);
    Q_ASSERT(tagItem11IndexSource.isValid());
    Q_ASSERT(tagItem11IndexSource==treeItem11Index);

    // add modeltest as the last test:
    new ModelTest(tagModel, this);
}

void TestRGTagModel::testModel3()
{
    SimpleTreeModel* const treeModel = new SimpleTreeModel(1, this);
    new ModelTest(treeModel, this);

    // add some items before the tagModel is created:
    SimpleTreeModel::Item* const treeItem1 = treeModel->addItem();
    const QPersistentModelIndex treeItem1Index = treeModel->itemToIndex(treeItem1);
    SimpleTreeModel::Item* const treeItem11 = treeModel->addItem(treeItem1);
    const QPersistentModelIndex treeItem11Index = treeModel->itemToIndex(treeItem11);

    SimpleTreeModel::Item* const treeItem2 = treeModel->addItem();
    const QPersistentModelIndex treeItem2Index = treeModel->itemToIndex(treeItem2);

    RGTagModel* const tagModel = new RGTagModel(treeModel, this);
    // modeltest will be created at the end of this function to make sure it does not influence the result!

    // first, verify the row and column counts:
    QCOMPARE(tagModel->rowCount(QModelIndex()), 2);
    QCOMPARE(tagModel->columnCount(QModelIndex()), 1);

    // now test toSourceIndex:
    //const QModelIndex tagItem1Index = tagModel->index(0, 0);

    // now add a new item to the source model, before the existing item:
    SimpleTreeModel::Item* const treeItem11a = treeModel->addItem(treeItem1, 0);
    const QPersistentModelIndex treeItem11aIndex = treeModel->itemToIndex(treeItem11a);
    QCOMPARE(treeItem11Index.row(), 1);
    QCOMPARE(treeItem11aIndex.row(), 0);

    // now add a new item to the source model, this time in the middle:
    SimpleTreeModel::Item* const treeItem11b = treeModel->addItem(treeItem1, 1);
    const QPersistentModelIndex treeItem11bIndex = treeModel->itemToIndex(treeItem11b);
    QCOMPARE(treeItem11Index.row(), 2);
    QCOMPARE(treeItem11aIndex.row(), 0);
    QCOMPARE(treeItem11bIndex.row(), 1);

    // add modeltest as the last test:
    new ModelTest(tagModel, this);
}

void TestRGTagModel::testModel1()
{
    SimpleTreeModel* const treeModel = new SimpleTreeModel(1, this);
    new ModelTest(treeModel, this);

    // add some items before the tagModel is created:
    SimpleTreeModel::Item* const treeItem1 = treeModel->addItem();
    QPersistentModelIndex treeItem1Index = treeModel->itemToIndex(treeItem1);
    SimpleTreeModel::Item* const treeItem11 = treeModel->addItem(treeItem1);
    QPersistentModelIndex treeItem11Index = treeModel->itemToIndex(treeItem11);

    RGTagModel* const tagModel = new RGTagModel(treeModel, this);
    // TODO: make sure the ModelTest does not find any errors, currently it does find errors ;-)
    //new ModelTest(tagModel, this);

    // simple tests
    Q_ASSERT(tagModel->rowCount()==treeModel->rowCount());

    const QPersistentModelIndex tagItem1Index = tagModel->fromSourceIndex(treeItem1Index);
    Q_ASSERT(tagItem1Index.isValid());
    qDebug()<<tagItem1Index;

    Q_ASSERT(tagModel->rowCount(tagItem1Index)==treeModel->rowCount(treeItem1Index));


    // make sure the tagModel handles items inserted after it was created
    // - both top level
    SimpleTreeModel::Item* const treeItem2 = treeModel->addItem();
    QPersistentModelIndex treeItem2Index = treeModel->itemToIndex(treeItem2);
    Q_ASSERT(tagModel->rowCount()==treeModel->rowCount());
    const QPersistentModelIndex tagItem2Index = tagModel->fromSourceIndex(treeItem2Index);

    // - and sub items:
    SimpleTreeModel::Item* const treeItem21 = treeModel->addItem(treeItem2);
    Q_ASSERT(tagItem2Index.isValid());

    Q_ASSERT(tagModel->rowCount(tagItem2Index)==treeModel->rowCount(treeItem2Index));

    const QPersistentModelIndex tagItem11Index = tagModel->fromSourceIndex(treeItem11Index);
    Q_ASSERT(tagItem11Index.isValid());

    QPersistentModelIndex treeItem21Index = treeModel->itemToIndex(treeItem21);
    const QPersistentModelIndex tagItem21Index = tagModel->fromSourceIndex(treeItem21Index);
    Q_ASSERT(tagItem21Index.isValid());

    // now make sure we can descend:
    const QModelIndex ti1 = tagModel->index(0, 0);
    Q_ASSERT(ti1.isValid());
    Q_ASSERT(ti1 == tagItem1Index);

    // descends level 1 row 0
    const QModelIndex ti11 = tagModel->index(0, 0, ti1);
    Q_ASSERT(ti11.isValid());
    Q_ASSERT(ti11 == tagItem11Index);

    qDebug()<<"----------------------_";

    // descends level 0 row 1
    const QModelIndex ti2 = tagModel->index(1, 0);
    Q_ASSERT(ti2.isValid());
    Q_ASSERT(ti2 == tagItem2Index);

    // descends level 1 row 0
    QModelIndex ti21 = tagModel->index(0, 0, ti2);
    Q_ASSERT(ti21.isValid());
    Q_ASSERT(ti21 == tagItem21Index);

    //checks invalid index
    const QModelIndex ti111 = tagModel->index(0,0, ti11);
    Q_ASSERT(!ti111.isValid());

    //checks parent of tagItem1
    const QModelIndex parent_ti1 = tagModel->parent(ti1);
    Q_ASSERT(!parent_ti1.isValid());

    //checks parent of tagItem11
    const QModelIndex parent_ti11 = tagModel->parent(ti11);
    Q_ASSERT(parent_ti11 == tagItem1Index);

    //checks parent of tagItem2
    const QModelIndex parent_ti2 = tagModel->parent(ti2);
    Q_ASSERT(!parent_ti2.isValid());

    const QModelIndex parent_ti21 = tagModel->parent(ti21);
    Q_ASSERT(parent_ti21.isValid());
}

void TestRGTagModel::testModelSpacerTags()
{
    SimpleTreeModel* const treeModel = new SimpleTreeModel(1, this);
    new ModelTest(treeModel, this);

    // add some items before the tagModel is created:
    SimpleTreeModel::Item* const treeItem1 = treeModel->addItem();
    QPersistentModelIndex treeItem1Index = treeModel->itemToIndex(treeItem1);
    treeItem1->data = QLatin1String("oldChildren");

    SimpleTreeModel::Item* const treeItem11 = treeModel->addItem(treeItem1);
    QPersistentModelIndex treeItem11Index = treeModel->itemToIndex(treeItem11);

    RGTagModel* const tagModel = new RGTagModel(treeModel, this);
    // TODO: make sure the ModelTest does not find any errors, currently it does find errors ;-)
    new ModelTest(tagModel, this);

    const QPersistentModelIndex tagItem11Index = tagModel->fromSourceIndex(treeItem11Index);
    Q_ASSERT(tagItem11Index.isValid());

    qDebug()<<"Worked before adding spacers";

    //insert spacer below ti21
    tagModel->addSpacerTag(QModelIndex(), QLatin1String("{Country}"));
    tagModel->addNewTag(QModelIndex(), QLatin1String("New Tag"));

    qDebug()<<"Added the spacers.";

    const QModelIndex index11 = tagModel->index(0,0);
    const QModelIndex index12 = tagModel->index(1,0);
    const QModelIndex index13 = tagModel->index(2,0);

    qDebug()<<tagModel->data(index11, Qt::DisplayRole);
    qDebug()<<tagModel->data(index12, Qt::DisplayRole);
    qDebug()<<tagModel->data(index13, Qt::DisplayRole);
//  qDebug()<<tagModel->data(2,0,QModelIndex());

/*
    qDebug()<<"VERIFY IF NEW TAG EXISTS:";
    QModelIndex ti211Spacer = tagModel->index(0,0,ti21);
    Q_ASSERT(ti211Spacer.isValid());
*/

}

QTEST_GUILESS_MAIN(TestRGTagModel)
