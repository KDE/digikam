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

// Local includes
#include "album.h"

namespace Digikam
{

class AlbumModificationHelper : public QObject
{
    Q_OBJECT
public:
    AlbumModificationHelper(QObject *parent);
    virtual ~AlbumModificationHelper();

public Q_SLOTS:
    void slotNewAlbum(PAlbum *parentAlbum);

};

}

#endif /* ALBUMMODIFICATIONHELPER_H */
