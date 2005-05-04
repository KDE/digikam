/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include "album.h"
#include "albummanager.h"

#include "dragobjects.h"

AlbumItemsDrag::AlbumItemsDrag(const KURL::List &urls, const QValueList<int>& dirIDs,
                               QWidget* dragSource, const char* name )
    : KURLDrag(urls, dragSource, name),
      m_dirIDs(dirIDs)
{
}

bool AlbumItemsDrag::canDecode(const QMimeSource* e)
{
    return e->provides("digikam/album-ids");
}

bool AlbumItemsDrag::decode(const QMimeSource* e, KURL::List &urls, 
                            QValueList<int>& dirIDs)
{
    urls.clear();
    dirIDs.clear();
    
    if (KURLDrag::decode(e, urls))
    {
        QByteArray ba = e->encodedData("digikam/album-ids");
        if (ba.size())
        {

            QDataStream ds(ba, IO_ReadOnly);
            int id;
            while (!ds.atEnd())
            {
                ds >> id;
                dirIDs.append(id);
            }

            return true;
        }
    }

    return false;
}

QByteArray AlbumItemsDrag::encodedData(const char* mime) const
{
    QCString mimetype( mime );
    if (mimetype == "digikam/album-ids")
    {
        QByteArray byteArray;
        QDataStream ds(byteArray, IO_WriteOnly);

        QValueList<int>::const_iterator it;
        for (it = m_dirIDs.begin(); it != m_dirIDs.end(); ++it)
        {
            ds << (*it);
        }
        
        return byteArray;
    }
    else
    {
        return KURLDrag::encodedData(mime);
    }
}

const char* AlbumItemsDrag::format(int i) const
{
    if (i == 0)
        return "text/uri-list";
    else if (i == 1)
        return "digikam/album-ids";
    else
        return 0;
}

TagItemsDrag::TagItemsDrag(const KURL::List& urls, const QValueList<int>& dirIDs,
                           QWidget* dragSource, const char* name )
    : AlbumItemsDrag(urls, dirIDs, dragSource, name)
{
    
}

bool TagItemsDrag::canDecode(const QMimeSource* e)
{
    return e->provides("digikam/tag-ids");
}

const char* TagItemsDrag::format(int i) const
{
    if (i == 0)
        return "text/uri-list";
    else if (i == 1)
        return "digikam/album-ids";
    else if (i == 2)
        // not really true. but hey... whatever floats the boat
        return "digikam/tag-ids";
    else
        return 0;
}

TagDrag::TagDrag( int albumid, QWidget *dragSource, 
                  const char *name ) :
    QDragObject( dragSource, name )
{
    mAlbumID = albumid;
}

bool TagDrag::canDecode( const QMimeSource* e )
{
    return e->provides("digikam/tag-id");
}

const char* TagDrag::format( int i ) const
{
    if ( i == 0 )
        return "digikam/tag-id";

    return 0;
}

QByteArray TagDrag::encodedData( const char* ) const
{
    QByteArray ba;
    QDataStream ds(ba, IO_WriteOnly);
    ds << mAlbumID;
    return ba;
}

AlbumDrag::AlbumDrag(const KURL &url, int albumid, 
                     QWidget *dragSource, 
                     const char *name) :
    KURLDrag(url, dragSource, name)
{
    mAlbumID = albumid;
}

bool AlbumDrag::canDecode( const QMimeSource* e )
{
    return e->provides("digikam/album-id");
}

const char* AlbumDrag::format( int i ) const
{
    if (i == 0)
        return "text/uri-list";
    else if ( i == 1 )
        return "digikam/album-id";

    return 0;
}

QByteArray AlbumDrag::encodedData(const char *mime) const
{
    QCString mimetype( mime );
    if(mimetype == "digikam/album-id")
    {
        QByteArray ba;
        QDataStream ds(ba, IO_WriteOnly);
        ds << mAlbumID;
        return ba;
    }
    else
    {
        return KURLDrag::encodedData(mime);
    }
}

bool AlbumDrag::decode(const QMimeSource* e, KURL::List &urls, 
                       int &albumID)
{
    urls.clear();
    albumID = -1;
    
    if(KURLDrag::decode(e, urls))
    {
        QByteArray ba = e->encodedData("digikam/album-id");
        if (ba.size())
        {
            QDataStream ds(ba, IO_ReadOnly);
            if(!ds.atEnd())
            {
                ds >> albumID;
            }
            return true;
        }
    }

    return false;
}
