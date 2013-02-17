/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-12
 * Description : a test for the searchtextbar
 *
 * Copyright (C) 2010 by Johannes Wienke <languitar at semipol dot de>
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

#include "searchtextbartest.moc"

// Qt includes

#include <qstandarditemmodel.h>
#include <qtest.h>
#include <qtestkeyboard.h>

// KDE includes

#include <qtest_kde.h>
#include <kdebug.h>

// Local includes

#include "searchtextbar.h"

using namespace Digikam;
using namespace QTest;

QTEST_KDEMAIN(SearchTextBarTest, GUI)

void SearchTextBarTest::testHighlighting()
{
    SearchTextBar textBar(0, "test");
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    // highlighting shall only occur if text is entered
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    textBar.setText("test");

    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::HAS_RESULT);
    textBar.slotSearchResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NO_RESULT);

    textBar.setHighlightOnResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);
    textBar.slotSearchResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    textBar.setHighlightOnResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::HAS_RESULT);
    textBar.slotSearchResult(false);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NO_RESULT);

    // re-eetting highlighting to true must not change highlight state
    textBar.setHighlightOnResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NO_RESULT);
}

void SearchTextBarTest::testSearchTextSettings()
{
    SearchTextBar textBar(0, "test");

    SearchTextSettings defaultSettings;
    QCOMPARE(textBar.searchTextSettings(), defaultSettings);

    callCount = 0;

    connect(&textBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(newSearchTextSettings(SearchTextSettings)));

    const QString textEntered = "hello world";
    keyClicks(&textBar, textEntered);

    QCOMPARE(textBar.searchTextSettings().caseSensitive, defaultSettings.caseSensitive);
    QCOMPARE(textBar.searchTextSettings().text, textEntered);

    QCOMPARE(callCount, textEntered.size());
    QCOMPARE(lastSearchTextSettings.caseSensitive, defaultSettings.caseSensitive);
    QCOMPARE(lastSearchTextSettings.text, textEntered);

    /** @todo test case modifications, but how to click context menu? */
}

void SearchTextBarTest::newSearchTextSettings(const SearchTextSettings& settings)
{
    lastSearchTextSettings = settings;
    callCount++;
}

void SearchTextBarTest::testModelParsing()
{
    SearchTextBar textBar(0, "test");

    // create a simple test model
    QStandardItemModel model(4, 1);

    const int idRole = 120;

    const QString parent0("test row 0");
    const QString parent1("test row 1");
    const QString parent2("test row 2");
    const QString parent3("test row 3");
    const QString firstChild("first child");

    QModelIndex index = model.index(0, 0, QModelIndex());
    model.setData(index, QVariant(parent0), Qt::DisplayRole);
    model.setData(index, QVariant(0), idRole);
    index = model.index(1, 0, QModelIndex());
    model.setData(index, QVariant(parent1), Qt::DisplayRole);
    model.setData(index, QVariant(1), idRole);
    index = model.index(2, 0, QModelIndex());
    model.setData(index, QVariant(parent2), Qt::DisplayRole);
    model.setData(index, QVariant(2), idRole);
    index = model.index(3, 0, QModelIndex());
    model.setData(index, QVariant(parent3), Qt::DisplayRole);
    model.setData(index, QVariant(3), idRole);

    // add a child to the last index to ensure tree parsing
    model.insertRow(0, index);
    model.insertColumn(0, index);
    QModelIndex childIndex = model.index(0, 0, index);
    model.setData(childIndex, QVariant(firstChild), Qt::DisplayRole);
    model.setData(childIndex, QVariant(10), idRole);

    textBar.setModel(&model, idRole, Qt::DisplayRole);

    // check that all entries are in the completion object now
    QCOMPARE(textBar.completionObject()->items().size(), 5);
    QVERIFY(textBar.completionObject()->items().contains(parent0));
    QVERIFY(textBar.completionObject()->items().contains(parent1));
    QVERIFY(textBar.completionObject()->items().contains(parent2));
    QVERIFY(textBar.completionObject()->items().contains(parent3));
    QVERIFY(textBar.completionObject()->items().contains(firstChild));

    /**
     * @todo I can't test inserting new items, because this stub model only
     * supports adding columns after having added a row with 0 columns. The
     * searchtextbar is only registered at rowInserted, so that there won't be
     * a valid index in this row yet...
     */

    // check that deleting an item is mirrored
    model.removeRow(1, QModelIndex());
    QCOMPARE(textBar.completionObject()->items().size(), 4);
    QVERIFY(!textBar.completionObject()->items().contains(parent1));

    // ensure that resetting a completely new model works
    QStandardItemModel newModel(2, 1);
    // use the same ids by purpose
    index = newModel.index(0, 0, QModelIndex());
    newModel.setData(index, QVariant(parent0 + 'x'), Qt::DisplayRole);
    newModel.setData(index, QVariant(0), idRole);
    index = newModel.index(1, 0, QModelIndex());
    newModel.setData(index, QVariant(parent1 + 'x'), Qt::DisplayRole);
    newModel.setData(index, QVariant(1), idRole);

    textBar.setModel(&newModel, idRole, Qt::DisplayRole);

    QCOMPARE(textBar.completionObject()->items().size(), 2);
    QVERIFY(textBar.completionObject()->items().contains(parent0 + 'x'));
    QVERIFY(textBar.completionObject()->items().contains(parent1 + 'x'));
}
