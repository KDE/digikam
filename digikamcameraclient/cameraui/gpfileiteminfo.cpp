/* ============================================================
 * File  : gpfileiteminfo.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-22
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

#include "gpfileiteminfo.h"


GPFileItemInfo::GPFileItemInfo()
{
    name = "";
    folder = "";

    // ----------------------------------------------------------
    
    fileInfoAvailable = false;
    mime = "";
    time = "";
    size = -1;
    width = -1;
    height = -1;
    readPermissions = -1;
    writePermissions = -1;
    downloaded = -1;    

    // ----------------------------------------------------------

    previewInfoAvailable = false;
    previewMime = "";
    previewSize = -1;
    previewWidth = -1;
    previewHeight = -1;
    previewDownloaded = -1;

    // ----------------------------------------------------------

    audioInfoAvailable = false;
    audioMime = "";
    audioSize = -1;
    audioDownloaded = -1;

    // ----------------------------------------------------------

    viewItem = 0;
}

GPFileItemInfo::~GPFileItemInfo()
{
}

GPFileItemInfo::GPFileItemInfo(const GPFileItemInfo& info)
{
    //name = QString(info.name.latin1());
    //folder = QString(info.folder.latin1());
    name.setLatin1(info.name.latin1());
    folder.setLatin1(info.folder.latin1());

    // ----------------------------------------------------------
    
    fileInfoAvailable = info.fileInfoAvailable;
    //mime = QString(info.mime.latin1());
    //time = QString(info.time.latin1());
    mime.setLatin1(info.mime.latin1());
    time.setLatin1(info.time.latin1());
    size = info.size;
    width = info.width;
    height = info.height;
    readPermissions = info.readPermissions;
    writePermissions = info.writePermissions;
    downloaded = info.downloaded;    

    // ----------------------------------------------------------

    previewInfoAvailable = info.previewInfoAvailable;
    //previewMime = QString(info.previewMime.latin1());
    previewMime.setLatin1(info.previewMime.latin1());
    previewSize = info.previewSize;
    previewWidth = info.previewWidth;
    previewHeight = info.previewHeight;
    previewDownloaded = info.previewDownloaded;

    // ----------------------------------------------------------

    audioInfoAvailable = info.audioInfoAvailable;
    audioMime = QString(info.audioMime.latin1());
    audioSize = info.audioSize;
    audioDownloaded = info.audioDownloaded;

    // ----------------------------------------------------------
    viewItem = 0;
}

GPFileItemInfo& GPFileItemInfo::operator=(const GPFileItemInfo& info)
{
    if (this != &info) {

        //name = QString(info.name.latin1());
        //folder = QString(info.folder.latin1());
        name.setLatin1(info.name.latin1());
        folder.setLatin1(info.folder.latin1());

        // ----------------------------------------------------------
    
        fileInfoAvailable = info.fileInfoAvailable;
        //mime = QString(info.mime.latin1());
        //time = QString(info.time.latin1());
        mime.setLatin1(info.mime.latin1());
        time.setLatin1(info.time.latin1());
        size = info.size;
        width = info.width;
        height = info.height;
        readPermissions = info.readPermissions;
        writePermissions = info.writePermissions;
        downloaded = info.downloaded;    

        // ----------------------------------------------------------

        previewInfoAvailable = info.previewInfoAvailable;
        //previewMime = QString(info.previewMime.latin1());
        previewMime.setLatin1(info.previewMime.latin1());
        previewSize = info.previewSize;
        previewWidth = info.previewWidth;
        previewHeight = info.previewHeight;
        previewDownloaded = info.previewDownloaded;

        // ----------------------------------------------------------

        audioInfoAvailable = info.audioInfoAvailable;
        audioMime = QString(info.audioMime.latin1());
        audioSize = info.audioSize;
        audioDownloaded = info.audioDownloaded;
        
        // ----------------------------------------------------------
        viewItem = 0;
    }

    return *this;
}
