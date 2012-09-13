/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
 *
 * Copyright (C) 2010 by Marcel Wiesweg <user dot wiesweg at gmx dot de>
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

#include "abstractdimagehistorytest.moc"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "config-digikam.h"
#include "bcgfilter.h"
#include "curvesfilter.h"
#include "autolevelsfilter.h"
#include "infraredfilter.h"
#include "blurfilter.h"
#include "dimagehistory.h"
#include "editorcore.h"
#include "editortooliface.h"
#include "editorwindow.h"
#include "iccsettings.h"
#include "imageiface.h"
#include "iofilesettings.h"

using namespace Digikam;

QString AbstractDImageHistoryTest::imagePath()
{
    return QString(KDESRCDIR)+QString("albummodeltestimages");
}

QDebug operator<<(QDebug dbg, const HistoryImageId& id)
{
    dbg.nospace() << " { ";
    dbg.nospace() << id.m_uuid;
    dbg.space() << id.m_type;
    dbg.space() << id.m_fileName;
    dbg.space() << id.m_filePath;
    dbg.space() << id.m_creationDate;
    dbg.space() << id.m_uniqueHash;
    dbg.space() << id.m_fileSize;
    dbg.space() << id.m_originalUUID;
    dbg.nospace() << " } ";
    return dbg;
}

HistoryImageId AbstractDImageHistoryTest::id1() const
{
    HistoryImageId id("abc123");
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName("file1.jpg");
    id.setPathOnDisk("/home/user/file1.jpg");
    id.setUniqueHash("cde567", 987654);
    return id;
}

HistoryImageId AbstractDImageHistoryTest::id2() const
{
    HistoryImageId id("12345");
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName("file2.jpg");
    id.setPathOnDisk("/home/user/file2.jpg");
    id.setUniqueHash("abc654", 876549);
    return id;
}

HistoryImageId AbstractDImageHistoryTest::id3() const
{
    HistoryImageId id("abcdef");
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName("file3.jpg");
    id.setPathOnDisk("/home/user/file3.jpg");
    id.setUniqueHash("c76543", 765489);
    return id;
}

HistoryImageId AbstractDImageHistoryTest::id4() const
{
    HistoryImageId id("aaabbb");
    id.setCreationDate(QDateTime(QDate(1984, 7, 14), QTime(13, 0, 0)));
    id.setFileName("file4.jpg");
    id.setPathOnDisk("/home/user/file4.jpg");
    id.setUniqueHash("c96542", 654987);
    return id;
}

FilterAction AbstractDImageHistoryTest::action1() const
{
    FilterAction fa("digikam:BCGFilter", 1);
    fa.addParameter("contrast", 1);
    fa.addParameter("channel", 1);
    fa.addParameter("brightness", 1);
    fa.addParameter("gamma", 1.2);
    return fa;
}

FilterAction AbstractDImageHistoryTest::action2() const
{
    FilterAction fa("digikam:SomeComplexFilter", 1, FilterAction::ComplexFilter);
    return fa;
}

DImageHistory AbstractDImageHistoryTest::history1() const
{
    DImageHistory history;

    history << id1();

    history << action1();

    history << id2();

    history << action2();

    history << id3() << id4();

    return history;
}

void AbstractDImageHistoryTest::applyFilters1()
{
    ImageIface iface;

    BCGFilter filter1(iface.original(), this);
    filter1.startFilterDirectly();
    iface.setOriginal(i18n("Brightness / Contrast / Gamma"), filter1.filterAction(), filter1.getTargetImage());

    CurvesFilter filter2(iface.original(), this);
    filter2.startFilterDirectly();
    iface.setOriginal("Curves", filter2.filterAction(), filter2.getTargetImage());
}

void AbstractDImageHistoryTest::applyFilters2()
{
    ImageIface iface;

    AutoLevelsFilter filter(iface.original(), iface.original(), this);
    filter.startFilterDirectly();
    iface.setOriginal("", filter.filterAction(), filter.getTargetImage());
}

void AbstractDImageHistoryTest::applyFilters3()
{
    ImageIface iface;

    InfraredFilter filter(iface.original(), this);
    filter.startFilterDirectly();
    iface.setOriginal("", filter.filterAction(), filter.getTargetImage());
}

void AbstractDImageHistoryTest::applyFilters4()
{
    ImageIface iface;

    BlurFilter filter(iface.original(), this);
    filter.startFilterDirectly();
    iface.setOriginal("", filter.filterAction(), filter.getTargetImage());
}

QString AbstractDImageHistoryTest::tempFileName(const QString& purpose) const
{
    return QString("digikamtests-") + metaObject()->className() + '-' + purpose + '-' + QTime::currentTime().toString();
}

QString AbstractDImageHistoryTest::tempFilePath(const QString& purpose) const
{
    return QDir::tempPath() + '/' + tempFileName(purpose);
}

void AbstractDImageHistoryTest::initBaseTestCase()
{
    // initialize kexiv2 before doing any multitasking
    KExiv2Iface::KExiv2::initializeExiv2();

    ICCSettingsContainer c = IccSettings::instance()->settings();
    c.enableCM = false;
    IccSettings::instance()->setSettings(c);

    m_im = new EditorCore();
    EditorCore::setDefaultInstance(m_im);

    connect(m_im, SIGNAL(signalImageLoaded(QString,bool)),
            this, SLOT(slotImageLoaded(QString,bool)));

    connect(m_im, SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    m_tempFile = tempFilePath("tempfile");
}

void AbstractDImageHistoryTest::cleanupBaseTestCase()
{
    delete m_im;
    QFile file(m_tempFile);
    file.remove();

    // clean up the kexiv2 memory:
    KExiv2Iface::KExiv2::cleanupExiv2();
}

void AbstractDImageHistoryTest::slotImageLoaded(const QString&, bool)
{
}
void AbstractDImageHistoryTest::slotImageSaved(const QString&, bool)
{
}

