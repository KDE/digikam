/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify search albums in views
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

#include "searchmodificationhelper.moc"

// KDE includes

#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandardguiitem.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "haariface.h"
#include "imageinfo.h"
#include "searchxml.h"
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
    : QObject(parent), d(new Private)
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
    int result = KMessageBox::warningYesNo(d->dialogParent,
                                           i18n("Are you sure you want to "
                                                "delete the selected search "
                                                "\"%1\"?", searchAlbum->title()),
                                           i18n("Delete Search?"),
                                           KGuiItem(i18n("Delete")),
                                           KStandardGuiItem::cancel());

    if (result != KMessageBox::Yes)
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
        SAlbum* album = (SAlbum*)(*it);

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
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label,
                                                 name, &ok, d->dialogParent);

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
    QString name = KInputDialog::getText(i18n("Rename Album (%1)", oldName),
                                         i18n("Enter new album name:"),
                                         oldName, &ok, d->dialogParent);

    if (!ok || name == oldName || name.isEmpty())
    {
        return;
    }

    if (!checkName(name))
    {
        return;
    }

    AlbumManager::instance()->updateSAlbum(searchAlbum, searchAlbum->query(),
                                           name);
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
        writer.writeField("creationdate", SearchXml::GreaterThan);
        writer.writeValue(dateRanges.at(i).first);
        writer.finishField();
        writer.writeField("creationdate", SearchXml::LessThan);
        writer.writeValue(dateRanges.at(i).second);
        writer.finishField();
        writer.finishGroup();
    }

    writer.finish();

    kDebug() << "Date search XML:\n" << writer.xml();

    SAlbum* album = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::TimeLineSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
    return album;
}

SAlbum* SearchModificationHelper::createFuzzySearchFromSketch(const QString& proposedName,
                                                              SketchWidget* sketchWidget,
                                                              unsigned int numberOfResults,
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
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "signature");         // we pass a signature
    writer.writeAttribute("numberofresults", QString::number(numberOfResults));
    writer.writeAttribute("sketchtype", "handdrawn");
    writer.writeValue(haarIface.signatureAsText(sketchWidget->sketchImage()));
    sketchWidget->sketchImageToXML(writer);
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name,
                                                            DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    return salbum;
}

void SearchModificationHelper::slotCreateFuzzySearchFromSketch(const QString& proposedName,
                                                               SketchWidget* sketchWidget,
                                                               unsigned int numberOfResults,
                                                               bool overwriteIfExisting)
{
    createFuzzySearchFromSketch(proposedName, sketchWidget, numberOfResults, overwriteIfExisting);
}

SAlbum* SearchModificationHelper::createFuzzySearchFromImage(const QString& proposedName,
                                                             const ImageInfo& image,
                                                             float threshold,
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
    writer.writeField("similarity", SearchXml::Like);
    writer.writeAttribute("type", "imageid");
    writer.writeAttribute("threshold", QString::number(threshold));
    writer.writeAttribute("sketchtype", "scanned");
    writer.writeValue(image.id());
    writer.finishField();
    writer.finishGroup();

    SAlbum* salbum = AlbumManager::instance()->createSAlbum(name,
                                                            DatabaseSearch::HaarSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    return salbum;
}

void SearchModificationHelper::slotCreateFuzzySearchFromImage(const QString& proposedName,
                                                              const ImageInfo& image,
                                                              float threshold,
                                                              bool overwriteIfExisting)
{
    createFuzzySearchFromImage(proposedName, image, threshold, overwriteIfExisting);
}

} // namespace Digikam
