/* ============================================================
 * File  : dragobjects.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-26
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
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

#include <qdragobject.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <kurldrag.h>

class QWidget;

class AlbumItemsDrag : public KURLDrag
{
public:

    AlbumItemsDrag(const KURL::List& urls, const QValueList<int>& dirIDs,
                   QWidget* dragSource=0, const char* name=0);

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, KURL::List &urls, 
		       QValueList<int>& dirIDs);

protected:

    virtual const char* format(int i) const;
    virtual QByteArray encodedData(const char* mime) const;

private:

    QValueList<int> m_dirIDs;
};

class TagItemsDrag : public AlbumItemsDrag
{
public:

    TagItemsDrag(const KURL::List& urls, const QValueList<int>& dirIDs,
                 QWidget* dragSource=0, const char* name=0);

    static bool canDecode(const QMimeSource* e);

protected:

    virtual const char* format(int i) const;
};

#endif /* DRAGOBJECTS_H */
