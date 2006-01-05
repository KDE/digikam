/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004, 2005 by Renchi Raju, Joern Ahrens

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

#ifndef DRAGOBJECTS_H
#define DRAGOBJECTS_H

// Qt includes.

#include <qdragobject.h>
#include <qstringlist.h>
#include <qvaluelist.h>

// KDE includes.

#include <kurldrag.h>

class QWidget;

/** @file */

namespace Digikam
{

/**
 * Provides a drag object for a list of KURLs with its related database IDs.
 *
 * Images can be moved through ItemDrag. It is possible to move them on
 * another application which understands KURLDrag, like konqueror, to
 * copy the images. Digikam can use the IDs, if ItemDrag is dropped
 * on digikam itself.
 * The kioURLs are internally used with the digikamalbums kioslave.
 * The "normal" KURLDrag urls are used for external drops (k3b, gimp, ...)
 */
class ItemDrag : public KURLDrag
{
public:

    ItemDrag(const KURL::List& urls, const KURL::List& kioURLs,
             const QValueList<int>& albumIDs,
             const QValueList<int>& albumIDs,
             QWidget* dragSource=0, const char* name=0);

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e,
                       KURL::List &urls,
                       KURL::List &kioURLs,
                       QValueList<int>& albumIDs,
                       QValueList<int>& imageIDs);

protected:

    virtual const char* format(int i) const;
    virtual QByteArray encodedData(const char* mime) const;

private:
    
    KURL::List      m_kioURLs;
    QValueList<int> m_albumIDs;    
    QValueList<int> m_imageIDs;
};


/**
 * Provides a drag object for a tag
 *
 * When a tag is moved through drag'n'drop an object of this class 
 * is created.
 */
class TagDrag : public QDragObject
{
public:

    TagDrag(int albumid, QWidget *dragSource = 0, const char *name = 0);
    static bool     canDecode(const QMimeSource* e);
    
protected:

    QByteArray      encodedData( const char* ) const;
    const char*     format(int i) const;
    
private:
    int     mAlbumID;
};

/**
 * Provides a drag object for an album
 *
 * When an album is moved through drag'n'drop an object of this class 
 * is created.
 */
class AlbumDrag : public KURLDrag
{
public:
    AlbumDrag(const KURL &url, int albumid, 
              QWidget *dragSource = 0, const char *name = 0);
    static bool     canDecode(const QMimeSource* e);
    static bool     decode(const QMimeSource* e, KURL::List &urls, 
                           int &albumID);
protected:

    QByteArray      encodedData(const char*) const;
    const char*     format(int i) const;
    
private:
    int     mAlbumID;
};

/**
 * Provides a drag object for a list of tags
 *
 * When a tag is moved through drag'n'drop an object of this class 
 * is created.
 */
class TagListDrag : public QDragObject
{
public:

    TagListDrag(const QValueList<int>& tagIDs, QWidget *dragSource = 0,
                const char *name = 0);
    static bool canDecode(const QMimeSource* e);
    
protected:

    QByteArray  encodedData( const char* ) const;
    const char* format(int i) const;
    
private:

    QValueList<int> m_tagIDs;
};

}  // namespace Digikam

#endif /* DRAGOBJECTS_H */
