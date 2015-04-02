/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015
 * Description : a test for timestamp trigger for re-reading metadata from image
 *
 * Copyright (C) 2015 by Kristian Karl <kristian dot hermann dot karl at gmail dot com>
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

#include "timestampupdatetest.moc"

// Qt includes

#include <QtTest>
#include <QFileInfo>
#include <QDBusConnection>

// KDE includes

#include <kdemacros.h>

// Libkexiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "imageinfo.h"
#include "metadatasettings.h"

const QString originalImageFolder(KDESRCDIR"timestampupdatetestimages");
const QString originalImageFile(KDESRCDIR"timestampupdatetestimages/1.jpg");

QTEST_MAIN(TimeStampUpdateTest);

using namespace Digikam;

QString TimeStampUpdateTest::tempFileName(const QString& purpose) const
{
    return QString("digikamtests-") + metaObject()->className() + '-' + purpose + '-' + QTime::currentTime().toString();
}

QString TimeStampUpdateTest::tempFilePath(const QString& purpose) const
{
    return QDir::tempPath() + '/' + tempFileName(purpose);
}

/*
 * Create a nd simulate a new startup of Digikam.
 * A new temporary database is created.
 * A collection is added, and scanned.
 */
void TimeStampUpdateTest::initTestCase()
{
    // Setup the collection folder
    QDir collectionDir = QDir(originalImageFolder);
    QVERIFY(collectionDir.exists());

    // Create new temporary database
    dbFile = tempFilePath("database");
    DatabaseParameters params("QSQLITE", dbFile, "QSQLITE", dbFile);
    DatabaseAccess::setParameters(params, DatabaseAccess::MainApplication);
    QVERIFY(DatabaseAccess::checkReadyForUse(0));
    QVERIFY(QFile(dbFile).exists());

    // Add collection and scan
    CollectionManager::instance()->addLocation(collectionDir.path());
    CollectionScanner().completeScan();

    // Verify that the scanned collection is correct
    QList<AlbumShortInfo> albums = DatabaseAccess().db()->getAlbumShortInfos();
    QVERIFY(albums.size() == 1);
    QStringList readOnlyImages;

    foreach(const AlbumShortInfo& album, albums)
    {
        readOnlyImages << DatabaseAccess().db()->getItemURLsInAlbum(album.id);
    }

    foreach(const QString& file, readOnlyImages)
    {
        ids << ImageInfo::fromLocalFile(file).id();
    }

    QVERIFY(!ids.contains(-1));
    QVERIFY(ids.size() == 1);
}

/*
 * Remove the database file
 */
void TimeStampUpdateTest::cleanupTestCase()
{
    QFile(dbFile).remove();
}

/*
 * Re-set the database and image file to it's original metadata state
 */
void TimeStampUpdateTest::cleanup()
{
    KExiv2Iface::KExiv2 meta;
    meta.setMetadataWritingMode(KExiv2Iface::KExiv2::WRITETOIMAGEONLY);
    meta.setUpdateFileTimeStamp(true);
    meta.load(originalImageFile);
    meta.removeExifTag("Exif.Image.Model", false);
    QVERIFY2(meta.applyChanges(), "Exif.Image.Model is removed");
    QVERIFY(meta.getExifTagString("Exif.Image.Model").isEmpty());

    CollectionScanner().scanFile(originalImageFile, CollectionScanner::Rescan);

    // Check that Exif.Image.Model in database is empty
    QVariantList dbModel = DatabaseAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY2(dbModel.at(0).toString().isEmpty(), "Exif.Image.Model should be empty");
}

/*
 * This test manipulates the Exif.Image.Model without updating
 * the database.
 * A CollectionScanner().completeScan() is then launched, simulating a
 * startup of Digikam.
 * The test verifies that the change in the file is detected and
 * that new value of Exif.Image.Model is read into the database.
 */
void TimeStampUpdateTest::testRescanImageIfModifiedSet2True()
{
    // Setup metadata settings
    MetadataSettingsContainer set;
    set.updateFileTimeStamp = true; // Deafult value
    set.rescanImageIfModified = true;
    MetadataSettings::instance()->setSettings(set);

    // Load the test image and verify that it's there
    QFileInfo originalFileInfo(originalImageFile);
    QVERIFY(originalFileInfo.isReadable());

    // Check that Exif.Image.Model in database is empty
    QVariantList dbModel = DatabaseAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY2(dbModel.at(0).toString().isEmpty(), "Exif.Image.Model should be empty");

    // Verify that Exif.Image.Model in image file is empty
    KExiv2Iface::KExiv2 meta;
    meta.setMetadataWritingMode(KExiv2Iface::KExiv2::WRITETOIMAGEONLY);
    meta.setUpdateFileTimeStamp(true);
    meta.load(originalImageFile);
    QString model = meta.getExifTagString("Exif.Image.Model");
    QVERIFY(model.isEmpty());

    // Change the metadata in image file
    meta.setExifTagString("Exif.Image.Model", "TimeStampUpdateTestCamera", false);
    QVERIFY2(meta.applyChanges(), "Exif.Image.Model is added");
    QVERIFY(meta.getExifTagString("Exif.Image.Model") == "TimeStampUpdateTestCamera");

    // Simulate restart of Digikam
    // The scan should detect that image file has changed
    CollectionScanner().completeScan();

    // Verify that the change is detected, and no exists in the database
    dbModel = DatabaseAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY(dbModel.at(0).toString() == "TimeStampUpdateTestCamera");
}

/*
 * This test manipulates the Exif.Image.Model without updating
 * the database.
 * A CollectionScanner().completeScan() is then launched, simulating a
 * startup of Digikam.
 * The test verifies that the change in the file is disregarden and
 * that the value of Exif.Image.Model is unchanged the database.
 */
void TimeStampUpdateTest::testRescanImageIfModifiedSet2False()
{
    // Setup metadata settings
    MetadataSettingsContainer set;
    set.updateFileTimeStamp = true; // Deafult value
    set.rescanImageIfModified = false; // Deafult value
    MetadataSettings::instance()->setSettings(set);

    // Load the test image and verify that it's there
    QFileInfo originalFileInfo(originalImageFile);
    QVERIFY(originalFileInfo.isReadable());

    // Check that Exif.Image.Model in database is empty
    QVariantList dbModel = DatabaseAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY2(dbModel.at(0).toString().isEmpty(), "Exif.Image.Model should be empty");

    // Verify that Exif.Image.Model in image file is empty
    KExiv2Iface::KExiv2 meta;
    meta.setMetadataWritingMode(KExiv2Iface::KExiv2::WRITETOIMAGEONLY);
    meta.setUpdateFileTimeStamp(true);
    meta.load(originalImageFile);
    QString model = meta.getExifTagString("Exif.Image.Model");
    QVERIFY(model.isEmpty());

    // Change the metadata in image file
    meta.setExifTagString("Exif.Image.Model", "TimeStampUpdateTestCamera", false);
    QVERIFY2(meta.applyChanges(), "Exif.Image.Model is added");
    QVERIFY(meta.getExifTagString("Exif.Image.Model") == "TimeStampUpdateTestCamera");

    // Simulate restart of Digikam
    // The scan should detect that image file has changed
    CollectionScanner().completeScan();

    // Verify that the changed image did not change the database
    dbModel = DatabaseAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY(dbModel.at(0).toString().isEmpty());
}
