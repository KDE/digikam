/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify search albums in views
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "searchmodificationhelper.h"

// Qt includes

#include <QInputDialog>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "haariface.h"
#include "imageinfo.h"
#include "coredbsearchxml.h"
#include "sketchwidget.h"

namespace Digikam
{

class SearchModificationHelper::Private
{
public:

    Private() :
        dialogParent(0)
    {
    }

    QWidget* dialogParent;
};

SearchModificationHelper::SearchModificationHelper(QObject* const parent, QWidget* const dialogParent)
    : QObject(parent),
      d(new Private)
{
    d->dialogParent = dialogParent;
}

SearchModificationHelper::~SearchModificationHelper()
{
    delete d;
}

void SearchModificationHelper::slotSearchDelete(SAlbum* searchAlbum)
{
    if (!searchAlbum)
    {
        return;
    }

    // Make sure that a complicated search is not deleted accidentally
    int result = QMessageBox::warning(d->dialogParent, i18n("Delete Search?"),
                                      i18n("Are you sure you want to "
                                           "delete the selected search "
                                           "\"%1\"?", searchAlbum->title()),
                                      QMessageBox::Yes | QMessageBox::Cancel);


    if (result != QMessageBox::Yes)
    {
        return;
    }

    AlbumManager::instance()->deleteSAlbum(searchAlbum);
}

bool SearchModificationHelper::checkAlbum(const QString& name) const
{
    const AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        SAlbum* const album = (SAlbum*)(*it);

        if (album->title() == name)
        {
            return false;
        }
    }

    return true;
}

bool SearchModificationHelper::checkName(QString& name)
{
    bool checked = checkAlbum(name);

    while (!checked)
    {
        QString label = i18n( "Search name already exists.\n"
                              "Please enter a new name:" );
        bool ok;
        QString newTitle = QInputDialog::getText(d->dialogParent,
                                                 i18n("Name exists"),
                                                 label,
                                                 QLineEdit::Normal,
                                                 name,
                                                 &ok);

        if (!ok)
        {
            return false;
        }

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

void SearchModificationHelper::slotSearchRename(SAlbum* searchAlbum)
{
    if (!searchAlbum)
    {
        return;
    }

    QString oldName(searchAlbum->title());
    bool    ok;
    QString name = QInputDialog::getText(d->dialogParent,
                                         i18n("Rename Album (%1)", oldName),
                                         i18n("Enter new album name:"),
                                         QLineEdit::Normal,
                                         oldName,
                                         &ok);

    if (!ok || name == oldName || name.isEmpty())
    {
        return;
    }

    if (!checkName(name))
    {
        return;
    }

    AlbumManager::instance()->updateSAlbum(searchAlbum, searchAlbum->query(), name);
}

SAlbum* SearchModificationHelper::slotCreateTimeLineSearch(const QString& desiredName,
                                                           const DateRangeList& dateRanges,
                                                           bool overwriteIfExisting)
{
    QString name = desiredName;

    if (!overwriteIfExisting)
    {
        if (!checkName(name))
        {
            return 0;
        }
    }

    if (dateRanges.isEmpty())
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
        return 0;
    }

    // Create an XML search query for the list of date ranges
    SearchXmlWriter writer;

    // for each range, write a group with two fields
    for (int i = 0; i < dateRanges.size(); ++i)
    {
        writer.writeGroup();
        writer.writeField(QLatin1String("creationdate"), SearchXml::GreaterThanOrEqual);
        writer.writeValue(dateRanges.at(i).first);
        writer.finishField();
        writer.writeField(QLatin1String("creationdate"), SearchXml::LessThan);
        writer.writeValue(dateRanges.at(i).second);
        writer.finishField();
        writer.finishGroup();
    }

    writer.finish();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Date search XML:\n" << writer.xml();

    SAlbum* const album = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::TimeLineSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
    return album;
}

SAlbum* SearchModificationHelper::createFuzzySearchFromSketch(const QString& proposedName,
                                                              SketchWidget* sketchWidget,
                                                              unsigned int numberOfResults,
                                                              QList<int>& targetAlbums,
                                                              bool overwriteIfExisting)
{
    if (sketchWidget->isClear())
    {
        return 0;
    }

    QString name = proposedName;

    if (!overwriteIfExisting)
    {
        if (!checkName(name))
        {
            return 0;
        }
    }

    // We query database here

    HaarIface haarIface;
    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField(QLatin1String("similarity"), SearchXml::Like);
    writer.writeAttribute(QLatin1String("type"), QLatin1String("signature"));         // we pass a signature
    writer.writeAttribute(QLatin1String("numberofresults"), QString::number(numberOfResults));
    writer.writeAttribute(QLatin1String("sketchtype"), QLatin1String("handdrawn"));
    writer.writeValue(haarIface.signatureAsText(sketchWidget->sketchImage()));
    sketchWidget->sketchImageToXML(writer);
    writer.finishField();

    // Add the target albums, i.e. define that the found similar images
    // must be located in one of the target albums.
    writer.writeField(QLatin1String("noeffect_targetAlbums"), SearchXml::OneOf);
    writer.writeValue(targetAlbums);
    writer.finishField();

    writer.finishGroup();
    writer.finish();

    SAlbum* const salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    return salbum;
}

void SearchModificationHelper::slotCreateFuzzySearchFromSketch(const QString& proposedName,
                                                               SketchWidget* sketchWidget,
                                                               unsigned int numberOfResults,
                                                               QList<int>& targetAlbums,
                                                               bool overwriteIfExisting)
{
    createFuzzySearchFromSketch(proposedName, sketchWidget, numberOfResults, targetAlbums, overwriteIfExisting);
}

SAlbum* SearchModificationHelper::createFuzzySearchFromDropped(const QString& proposedName,
                                                               const QString& filePath,
                                                               float threshold,
                                                               float maxThreshold,
                                                               QList<int>& targetAlbums,
                                                               bool overwriteIfExisting)
{
    QString name = proposedName;

    if (!overwriteIfExisting)
    {
        if (!checkName(name))
        {
            return 0;
        }
    }

    // We query database here

    HaarIface haarIface;
    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField(QLatin1String("similarity"), SearchXml::Like);
    writer.writeAttribute(QLatin1String("type"), QLatin1String("image")); // we pass an image path
    writer.writeAttribute(QLatin1String("threshold"), QString::number(threshold));
    writer.writeAttribute(QLatin1String("maxthreshold"), QString::number(maxThreshold));
    writer.writeAttribute(QLatin1String("sketchtype"), QLatin1String("scanned"));
    writer.writeValue(filePath);
    writer.finishField();

    // Add the target albums, i.e. define that the found similar images
    // must be located in one of the target albums.
    writer.writeField(QLatin1String("noeffect_targetAlbums"), SearchXml::OneOf);
    writer.writeValue(targetAlbums);
    writer.finishField();

    writer.finishGroup();
    writer.finish();

    SAlbum* const salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    return salbum;
}

void SearchModificationHelper::slotCreateFuzzySearchFromDropped(const QString& proposedName,
                                                                const QString& filePath,
                                                                float threshold,
                                                                float maxThreshold,
                                                                QList<int>& targetAlbums,
                                                                bool overwriteIfExisting)
{
    createFuzzySearchFromDropped(proposedName, filePath, threshold, maxThreshold, targetAlbums, overwriteIfExisting);
}

SAlbum* SearchModificationHelper::createFuzzySearchFromImage(const QString& proposedName,
                                                             const ImageInfo& image,
                                                             float threshold,
                                                             float maxThreshold,
                                                             QList<int>& targetAlbums,
                                                             bool overwriteIfExisting)
{
    if (image.isNull())
    {
        return 0;
    }

    QString name = proposedName;

    if (!overwriteIfExisting)
    {
        if (!checkName(name))
        {
            return 0;
        }
    }

    // We query database here

    SearchXmlWriter writer;

    writer.writeGroup();
    writer.writeField(QLatin1String("similarity"), SearchXml::Like);
    writer.writeAttribute(QLatin1String("type"), QLatin1String("imageid"));
    writer.writeAttribute(QLatin1String("threshold"), QString::number(threshold));
    writer.writeAttribute(QLatin1String("maxthreshold"), QString::number(maxThreshold));
    writer.writeAttribute(QLatin1String("sketchtype"), QLatin1String("scanned"));
    writer.writeValue(image.id());
    writer.finishField();

    // Add the target albums, i.e. define that the found similar images
    // must be located in one of the target albums.
    writer.writeField(QLatin1String("noeffect_targetAlbums"), SearchXml::OneOf);
    writer.writeValue(targetAlbums);
    writer.finishField();

    writer.finishGroup();
    writer.finish();

    SAlbum* const salbum = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    return salbum;
}

void SearchModificationHelper::slotCreateFuzzySearchFromImage(const QString& proposedName,
                                                              const ImageInfo& image,
                                                              float threshold,
                                                              float maxThreshold,
                                                              QList<int>& targetAlbums,
                                                              bool overwriteIfExisting)
{
    createFuzzySearchFromImage(proposedName, image, threshold, maxThreshold, targetAlbums, overwriteIfExisting);
}

} // namespace Digikam
