/* ============================================================
 * File  : albumxmlparser.h
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


#ifndef ALBUMXMLPARSER_H
#define ALBUMXMLPARSER_H


class QString;

/*! 
 * AlbumXMLParser
 * 
 * A fast album parser to get the album properties withour reading the
 * entire xml file
 */

namespace Digikam
{

class AlbumInfo;
class AlbumXMLParserPriv;

class AlbumXMLParser
{
public:

    AlbumXMLParser();
    ~AlbumXMLParser();

    void setAlbum(AlbumInfo *album);

private:

    void restore(const QString& path);

    AlbumXMLParserPriv* d;
};

}

#endif /* ALBUMXMLPARSER_H */
