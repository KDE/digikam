/* ============================================================
 * File  : albumxmlparser.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-09-25
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
#include "albumxmlparser.h"


namespace Digikam
{

class AlbumXMLHandler : public QXmlDefaultHandler
{
public:

    AlbumXMLHandler() : QXmlDefaultHandler() {
        album = 0;
    }
        
    bool startElement( const QString&, const QString& localName,
                       const QString&, const QXmlAttributes& att) {

        if (!album)
            return false;
    
        int index;
    
        if (localName == "album") {
            if ( att.length() > 0 ) {
            
                index = att.index("comments");
                if (index != -1)
                    album->comments_ = att.value(index);
            
                index = att.index("collection");
                if (index != -1)
                    album->collection_ = att.value(index);

                index = att.index("date");
                if (index != -1)            
                    album->date_ = QDate::fromString(att.value(index),
                                                     Qt::ISODate);
            }

        }
    
        // stop parsing once we got the album attributes
        return false;
    }

    AlbumInfo *album;
};


// -------------------------------------------------------------------

class AlbumXMLParserPriv
{
public:

    AlbumXMLParserPriv() {
        handler = new AlbumXMLHandler();
        reader.setContentHandler(handler);
    }

    ~AlbumXMLParserPriv() {
        delete handler;
    }
    
    AlbumXMLHandler  *handler;
    QXmlSimpleReader  reader;
};

AlbumXMLParser::AlbumXMLParser()
{
    d = new AlbumXMLParserPriv;    
}

AlbumXMLParser::~AlbumXMLParser()
{
    delete d;
}

void AlbumXMLParser::setAlbum(AlbumInfo* album)
{
    if (!album) return;

    // First check and restore if we have a backup file
    restore(album->getPath());

    QFile xmlFile(album->getPath() + "/digikam.xml");
    if (!xmlFile.exists())
        return;
                               
    QXmlInputSource source( &xmlFile );
    d->handler->album = album;
    d->reader.parse( source );
}

void AlbumXMLParser::restore(const QString& path)
{
    QString xmlFile      = path + "/digikam.xml";
    QString xmlFileBak   = path + "/.digikam.xml.bak";

    struct stat info;
    int rc = stat(xmlFileBak.latin1(), &info);
    if (rc != 0) return;
    
    // We have a file which was backed up and not restored. Most
    // likely application crashed and we will restore the backed
    // up file.

    FILE *inFile;
    FILE *outFile;
    
    inFile  = fopen(xmlFileBak.latin1(), "r");
    if (!inFile) {
        std::cerr << "AlbumXMLParser:restore: could not open backup file for restoring"
                  << std::endl;
        return;
    }
    
    outFile = fopen(xmlFile.latin1(), "w");
    if (!outFile) {
        fclose(inFile);
        std::cerr << "AlbumXMLParser:restore: could not open main file for restoring"
                  << std::endl;
        return;
    }

    int inChar;
    
    while ((inChar = getc(inFile)) != EOF) {
        putc(inChar, outFile);
    }

    fclose(inFile);
    fclose(outFile);

    // Now remove the backup file
    if (unlink(xmlFileBak.latin1()) != 0) 
        std::cerr << "AlbumXMLParser:restore: could not remove backup file after saving main file" 
                  << std::endl;
}

}
