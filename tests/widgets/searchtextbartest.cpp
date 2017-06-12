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

#include "searchtextbartest.h"

// Qt includes

#include <QStandardItemModel>
#include <QTest>
#include <qtestkeyboard.h>

// Local includes

#include "searchtextbar.h"

using namespace Digikam;
using namespace QTest;

QTEST_MAIN(SearchTextBarTest)

void SearchTextBarTest::testHighlighting()
{
    SearchTextBar textBar(0, QLatin1String("test"));
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    // highlighting shall only occur if text is entered
    textBar.slotSearchResult(true);
    QCOMPARE(textBar.getCurrentHighlightState(), SearchTextBar::NEUTRAL);

    textBar.setText(QLatin1String("test"));

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
    SearchTextBar textBar(0, QLatin1String("test"));

    SearchTextSettings defaultSettings;
    QCOMPARE(textBar.searchTextSettings(), defaultSettings);

    callCount = 0;

    connect(&textBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(newSearchTextSettings(SearchTextSettings)));

    const QString textEntered = QLatin1String("hello world");
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

