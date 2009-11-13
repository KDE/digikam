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

// Local includes
#include "albumsettings.h"
#include "collectionmanager.h"
#include "albumpropsedit.h"
#include "albummanager.h"

namespace Digikam
{

AlbumModificationHelper::AlbumModificationHelper(QObject *parent) : QObject(parent)
{
}

AlbumModificationHelper::~AlbumModificationHelper()
{
}

void AlbumModificationHelper::slotNewAlbum(PAlbum *parent)
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

}
