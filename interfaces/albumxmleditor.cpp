/* ============================================================
 * File  : albumxml.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-09-23
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <qmap.h>
#include <qfile.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qxml.h>
#include <iostream>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
}

#include "albuminfo.h"
#include "albumxmleditor.h"

// -------------------------------------------------------------------

namespace Digikam
{

class AlbumXMLEditorPriv
{
public:

    QMap<QString,QString> cmap;

    AlbumInfo *album;
    QString    xmlFile;
    QString    xmlFileBak;
    bool       changed;
    bool       open;
};

AlbumXMLEditor::AlbumXMLEditor(AlbumInfo *album)
{
    d             = new AlbumXMLEditorPriv;
    d->album      = album;
    d->changed    = false;
    d->open       = false;
    d->xmlFile    = album->getPath() + QString("/digikam.xml");
    d->xmlFileBak = album->getPath() + QString("/.digikam.xml.bak");
    d->cmap.clear();
}

AlbumXMLEditor::~AlbumXMLEditor()
{
    delete d;    
}

void AlbumXMLEditor::open()
{
    if (d->open) return;
    
    d->cmap.clear();
    d->changed = false;

    restore();
    backup();
    
    QFile file(d->xmlFile);
    if (!file.exists() || !file.open(IO_ReadOnly)) {
        return;
    }

    QDomDocument doc("XMLAlbumProperties");
    if (!doc.setContent(&file)) {
        std::cerr << "AlbumXMLEditor:open: Failed to set content from xml file"
                  << std::endl;
        return;
    }

    QDomElement elem = doc.documentElement();
    if (elem.tagName() != "album")
        return;

    QDomNode node = elem.firstChild();
    if (node.isNull() || node.toElement().isNull())
        return;

    elem = node.toElement();

    for (QDomNode n = elem.firstChild();
         !n.isNull(); n = n.nextSibling()) {

        QDomElement e = n.toElement();
        if (e.isNull()) continue;
        if (e.tagName() != "item") continue;

        QString key   = e.attribute("name");
        QString value = e.attribute("comments");
        if (!key.isNull() && !value.isNull()) 
            d->cmap[key] = value;
    }

    d->open = true;
    
    return;
}


void AlbumXMLEditor::close()
{
    bool saved = true;
    if (d->changed || d->album->modified_) {
        saved = save();
        if (!saved)
            std::cerr <<
                "AlbumXMLEditor:close:"
                "Failed to save xmlFile" << std::endl;
    }

    // Ok successfully saved. remove backup file
    if (saved) {
        removeBackup();
    }

    d->cmap.clear();
    d->changed = false;
    d->open = false;
}

bool AlbumXMLEditor::save()
{
    QDomDocument doc("XMLAlbumProperties");
    
    QDomElement rootElem = doc.createElement("album");
    rootElem.setAttribute("client","digikam");
    rootElem.setAttribute("version",1.0);
    rootElem.setAttribute("comments",d->album->comments_);
    rootElem.setAttribute("collection",d->album->collection_);
    rootElem.setAttribute("date",d->album->date_.toString(Qt::ISODate));
    doc.appendChild( rootElem );
    
    QDomElement itemElem = doc.createElement("itemlist"); 
    rootElem.appendChild(itemElem);

    QMap<QString,QString>::Iterator it;
    for (it = d->cmap.begin(); it != d->cmap.end(); ++it)
    {
        QDomElement e = doc.createElement("item");
        e.setAttribute("name", it.key());
        e.setAttribute("comments", it.data());
        itemElem.appendChild(e);
    }
    
    QFile cfile(d->xmlFile);
    if (!cfile.open(IO_WriteOnly)) {
        std::cerr <<
            "AlbumXMLEditor:save:"
            "Failed to open xmlFile "
                  << d->xmlFile.latin1() << std::endl;
        return false;
    }
    
    QTextStream stream(&cfile);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    stream << doc.toString();
    cfile.close();

    return true;
}

QString AlbumXMLEditor::find(const QString& item)
{
    QString result = d->cmap[item];
    return result;
}

void AlbumXMLEditor::insert(const QString& item,
                                    const QString& comments)
{
    d->cmap[item] = comments;
    d->changed   = true;
}

void AlbumXMLEditor::remove(const QString& item)
{
    d->cmap.remove(item);
    d->changed = true;
}

void AlbumXMLEditor::backup()
{
    if (d->xmlFile.isEmpty() || d->xmlFileBak.isEmpty())
        return;

    struct stat info;
    int rc = stat(d->xmlFile.latin1(), &info);
    if (rc != 0)
        return; // we don't have a comments file
    
    FILE *inFile;
    FILE *outFile;
    
    inFile  = fopen(d->xmlFile.latin1(), "r");
    if (!inFile) {
        std::cerr << "AlbumXMLEditor:backup could not open main file for backing"
                  << std::endl;
        return;
    }
    
    outFile = fopen(d->xmlFileBak.latin1(), "w");
    if (!outFile) {
        fclose(inFile);
        std::cerr << "CommentsList: could not open backup file for backing"
                  << std::endl;
        return;
    }

    int inChar;
    
    while ((inChar = getc(inFile)) != EOF) {
        putc(inChar, outFile);
    }

    fclose(inFile);
    fclose(outFile);
}

void AlbumXMLEditor::restore()
{
    if (d->xmlFile.isEmpty() || d->xmlFileBak.isEmpty())
        return;

    struct stat info;
    
    int rc = stat(d->xmlFileBak.latin1(), &info);
    if (rc != 0) return;
    
    // We have a file which was backed up and not restored. Most
    // likely application crashed and we will restore the backed
    // up file.

    FILE *inFile;
    FILE *outFile;
    
    inFile  = fopen(d->xmlFileBak.latin1(), "r");
    if (!inFile) {
        std::cerr << "AlbumXMLEditor:restore: could not open backup file for restoring"
                  << std::endl;
        return;
    }
    
    outFile = fopen(d->xmlFile.latin1(), "w");
    if (!outFile) {
        fclose(inFile);
        std::cerr << "AlbumXMLEditor:restore: could not open main file for restoring"
                  << std::endl;
        return;
    }

    int inChar;
    
    while ((inChar = getc(inFile)) != EOF) {
        putc(inChar, outFile);
    }

    fclose(inFile);
    fclose(outFile);

    // Now delete the backup file
    removeBackup();
}

void AlbumXMLEditor::removeBackup()
{
    if (d->xmlFileBak.isEmpty()) return;
    
    struct stat info;
    int rc = stat(d->xmlFileBak.latin1(), &info);
    if (rc != 0) return;
    
    if (unlink(d->xmlFileBak.latin1()) != 0) 
        std::cerr << "AlbumXMLEditor:removeBackup: could not remove backup file after saving main file" 
                  << std::endl;
    
}

bool AlbumXMLEditor::isOpen()
{
    return d->open;    
}

}
