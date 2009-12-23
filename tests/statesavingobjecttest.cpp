/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : a test for the StateSavingObject class
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "statesavingobjecttest.moc"

// KDE includes

#include <kconfiggroup.h>
#include <kglobal.h>
#include <ksharedconfig.h>

// Local includes

#include "statesavingobject.h"

using namespace Digikam;

QTEST_MAIN(StateSavingObjectTest)

class StubStateSaverPriv
{
public:

    StubStateSaverPriv() :
        loadCalled(false),
        saveCalled(false)
    {
    }

    bool loadCalled;
    bool saveCalled;
};

StubStateSaver::StubStateSaver()
    : QObject(0), StateSavingObject(this), d(new StubStateSaverPriv)
{
}

StubStateSaver::~StubStateSaver()
{
    delete d;
}

KConfigGroup StubStateSaver::getGroup()
{
    return getConfigGroup();
}

QString StubStateSaver::getEntryKey(QString base)
{
    return entryName(base);
}

void StubStateSaver::doLoadState()
{
    d->loadCalled = true;
}

void StubStateSaver::doSaveState()
{
    d->saveCalled = true;
}

bool StubStateSaver::loadCalled()
{
    return d->loadCalled;
}

bool StubStateSaver::saveCalled()
{
    return d->saveCalled;
}

// -----------------------------------------------------------------------------

void StateSavingObjectTest::testGroupName()
{

    StubStateSaver saver;
    QCOMPARE(saver.getGroup().name(), QString("<default>"));

    const QString name = "testName 2";
    saver.setObjectName(name);
    QCOMPARE(saver.getGroup().name(), name);

    KConfigGroup group = KGlobal::config()->group("SimpleTest Group");
    saver.setConfigGroup(group);
    QCOMPARE(saver.getGroup().name(), group.name());

    // setting object name must not change returned group
    saver.setObjectName("new Name");
    QCOMPARE(saver.getGroup().name(), group.name());

    // resetting group must work
    KConfigGroup group2 = KGlobal::config()->group("Another SimpleTest Group");
    saver.setConfigGroup(group2);
    QCOMPARE(saver.getGroup().name(), group2.name());

}

void StateSavingObjectTest::testPrefixUsage()
{

    // default, empty prefix
    StubStateSaver saver;
    QCOMPARE(saver.getEntryKey(""), QString(""));
    QCOMPARE(saver.getEntryKey("test"), QString("test"));

    const QString prefix = " _Pr efix_ ";
    saver.setEntryPrefix(prefix);
    QCOMPARE(saver.getEntryKey(""), prefix);
    QCOMPARE(saver.getEntryKey("test"), prefix + QString("test"));

}

void StateSavingObjectTest::testDirectCalling()
{

    StubStateSaver loader;
    QVERIFY(!loader.loadCalled());
    loader.loadState();
    QVERIFY(loader.loadCalled());
    QVERIFY(!loader.saveCalled());

    StubStateSaver saver;
    QVERIFY(!saver.saveCalled());
    saver.saveState();
    QVERIFY(saver.saveCalled());
    QVERIFY(!saver.loadCalled());

}
