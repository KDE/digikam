/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-11-03
 * Description : An abstract digiKam unit-test template.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ABSTRACT_UNIT_TEST_H
#define DIGIKAM_ABSTRACT_UNIT_TEST_H

// Qt includes

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QTest>
#include <QString>

// Local includes

// Local includes

#include "dmetadata.h"
#include "wstoolutils.h"

using namespace Digikam;

class AbstractUnitTest : public QObject
{
    Q_OBJECT

public:

    AbstractUnitTest()
        : m_originalImageFolder(QFINDTESTDATA("data/"))
    {
    }

protected Q_SLOTS:

    // Re-implemented from QTest framework
    virtual void initTestCase()
    {
        MetaEngine::initializeExiv2();
        qDebug() << "Using Exiv2 Version:" << MetaEngine::Exiv2Version();
        m_tempPath = QString::fromLatin1(QTest::currentAppName());
        m_tempPath.replace(QLatin1String("./"), QString());
    }

    virtual void init()
    {
        m_tempDir = WSToolUtils::makeTemporaryDir(m_tempPath.toLatin1().data());
    }

    virtual void cleanup()
    {
        WSToolUtils::removeTemporaryDir(m_tempPath.toLatin1().data());
    }

    virtual void cleanupTestCase()
    {
        MetaEngine::cleanupExiv2();
    }

protected:

    QString       m_tempPath;
    QDir          m_tempDir;
    const QString m_originalImageFolder;
};

#endif // DIGIKAM_ABSTRACT_UNIT_TEST_H
