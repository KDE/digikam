/*
 * AlbumModificationHelper.h
 *
 *  Created on: 13.11.2009
 *      Author: languitar
 */

#ifndef ALBUMMODIFICATIONHELPER_H
#define ALBUMMODIFICATIONHELPER_H

// QT includes
#include <qobject.h>
#include <qwidget.h>

// KDE includes
#include <kjob.h>

// Local includes
#include "album.h"

namespace Digikam
{

class AlbumModificationHelperPriv;
class AlbumModificationHelper : public QObject
{
    Q_OBJECT
public:
    AlbumModificationHelper(QObject *parent, QWidget *dialogParent);
    virtual ~AlbumModificationHelper();

public Q_SLOTS:
    void slotAlbumNew(PAlbum *parentAlbum);
    void slotAlbumDelete(PAlbum *album);
    void slotAlbumRename(PAlbum *album);
    void slotAlbumEdit(PAlbum *album);

private Q_SLOTS:
    void slotDIOResult(KJob* kjob);

private:
    void addAlbumChildrenToList(KUrl::List& list, Album *album);

    AlbumModificationHelperPriv *d;

};

}

#endif /* ALBUMMODIFICATIONHELPER_H */
