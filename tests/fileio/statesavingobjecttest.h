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

#ifndef STATESAVINGOBJECTTEST_H
#define STATESAVINGOBJECTTEST_H

// Qt includes

#include <QtTest/QtTest>

// Local includes

#include "statesavingobject.h"

class KConfigGroup;

class StateSavingObjectTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGroupName();
    void testPrefixUsage();
    void testDirectCalling();
    void testDirectChildrenLoading();
    void testDirectChildrenSaving();
    void testRecursiveChildrenLoading();
    void testRecursiveChildrenSaving();

};

class StubStateSaverPriv;
class StubStateSaver: public QObject, public Digikam::StateSavingObject
{
    Q_OBJECT
public:
    explicit StubStateSaver(QObject* parent = 0);
    virtual ~StubStateSaver();

    KConfigGroup getGroup();
    QString getEntryKey(const QString& base);

    void doLoadState();
    void doSaveState();

    bool loadCalled();
    bool saveCalled();

    unsigned int numLoadCalls();
    unsigned int numSaveCalls();

private:
    StubStateSaverPriv* d;

};

#endif /* STATESAVINGOBJECTTEST_H */
