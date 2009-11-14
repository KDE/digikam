/*
 * AlbumModificationHelper.cpp
 *
 *  Created on: 13.11.2009
 *      Author: languitar
 */

#include "albummodificationhelper.h"
#include "albummodificationhelper.moc"

// KDE includes
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kio/jobuidelegate.h>
#include <kinputdialog.h>
#include <klocale.h>

// Local includes
#include "albumsettings.h"
#include "collectionmanager.h"
#include "albumpropsedit.h"
#include "albummanager.h"
#include "dio.h"
#include "deletedialog.h"

namespace Digikam
{

class AlbumModificationHelperPriv
{
public:
    QWidget *dialogParent;
};

AlbumModificationHelper::AlbumModificationHelper(QObject *parent,
                QWidget *dialogParent) :
    QObject(parent), d(new AlbumModificationHelperPriv)
{
    d->dialogParent = dialogParent;
}

AlbumModificationHelper::~AlbumModificationHelper()
{
    delete d;
}

void AlbumModificationHelper::slotAlbumNew(PAlbum *parent)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if(!settings)
    {
        kWarning() << "could not get Album Settings";
        return;
    }

    /*
    QDir libraryDir(settings->getAlbumLibraryPath());
    if(!libraryDir.exists())
    {
        KMessageBox::error(0,
                           i18n("The album library has not been set correctly.\n"
                                "Select \"Configure Digikam\" from the Settings "
                                "menu and choose a folder to use for the album "
                                "library."));
        return;
    }
    */

    // if we create an album under root, need to supply the album root path.
    QString albumRootPath;
    if (parent->isRoot())
    {
        //TODO: Let user choose an album root
        albumRootPath = CollectionManager::instance()->oneAlbumRootPath();
    }

    QString     title;
    QString     comments;
    QString     category;
    QDate       date;
    QStringList albumCategories;

    if(!AlbumPropsEdit::createNew(parent, title, comments, date, category,
                                  albumCategories))
        return;

    QStringList oldAlbumCategories(AlbumSettings::instance()->getAlbumCategoryNames());
    if(albumCategories != oldAlbumCategories)
    {
        AlbumSettings::instance()->setAlbumCategoryNames(albumCategories);
    }

    QString errMsg;
    PAlbum* album;
    if (parent->isRoot())
        album = AlbumManager::instance()->createPAlbum(albumRootPath, title, comments,
                                          date, category, errMsg);
    else
        album = AlbumManager::instance()->createPAlbum(parent, title, comments,
                                          date, category, errMsg);

    if (!album)
    {
        KMessageBox::error(0, errMsg);
        return;
    }

}

void AlbumModificationHelper::slotAlbumDelete(PAlbum *album)
{

    if(!album || album->isRoot() || album->isAlbumRoot())
        return;

    // find subalbums
    KUrl::List childrenList;
    addAlbumChildrenToList(childrenList, album);

    DeleteDialog dialog(d->dialogParent);

    // All subalbums will be presented in the list as well
    if (!dialog.confirmDeleteList(childrenList,
                                  childrenList.size() == 1 ?
                                  DeleteDialogMode::Albums : DeleteDialogMode::Subalbums,
                                  DeleteDialogMode::UserPreference))
        return;

    bool useTrash = !dialog.shouldDelete();

    // Currently trash kioslave can handle only full paths.
    // pass full folder path to the trashing job
    //TODO: Use digikamalbums:// url?
    KUrl u;
    u.setProtocol("file");
    u.setPath(album->folderPath());
    KIO::Job* job = DIO::del(u, useTrash);
    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(slotDIOResult(KJob *)));

}

void AlbumModificationHelper::slotAlbumRename(PAlbum *album)
{

    if (!album)
    {
        return;
    }

    QString oldTitle(album->title());
    bool    ok;

    QString title = KInputDialog::getText(i18n("Rename Album (%1)", oldTitle),
                                          i18n("Enter new album name:"),
                                          oldTitle, &ok, d->dialogParent);
    if (!ok)
    {
        return;
    }

    if(title != oldTitle)
    {
        QString errMsg;
        if (!AlbumManager::instance()->renamePAlbum(album, title, errMsg))
            KMessageBox::error(0, errMsg);
    }

}

void AlbumModificationHelper::addAlbumChildrenToList(KUrl::List& list, Album *album)
{
    // simple recursive helper function
    if (album)
    {
        list.append(album->databaseUrl());
        AlbumIterator it(album);
        while(it.current())
        {
            addAlbumChildrenToList(list, *it);
            ++it;
        }
    }

}

void AlbumModificationHelper::slotDIOResult(KJob* kjob)
{
    KIO::Job *job = static_cast<KIO::Job*>(kjob);
    if (job->error())
    {
        job->ui()->setWindow(d->dialogParent);
        job->ui()->showErrorMessage();
    }
}

}
