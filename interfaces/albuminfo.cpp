/* ============================================================
 * File  : albuminfo.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-08
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

#include <qdir.h>

#include "albummanager.h"
#include "albumxmleditor.h"
#include "albumitemhandler.h"
#include "albuminfo.h"

namespace Digikam
{

AlbumInfo::AlbumInfo(AlbumManager *parent,
                     const QString& title)
{
    modified_   = false;
    hidden_     = 0;
    
    parent_     = parent;
    title_      = title;

    viewItem_ = 0;
    next_     = 0;
    prev_     = 0;
    db_       = new AlbumXMLEditor(this);
    
    if (parent_) 
        parent_->insertAlbum(this);
}


AlbumInfo::~AlbumInfo()
{
    delete db_;
    
    if (parent_)
        parent_->takeAlbum(this);
}

void AlbumInfo::openDB()
{
    db_->open();    
}

void AlbumInfo::closeDB()
{
    // Don't close the db if we are current
    if (this == AlbumManager::instance()->currentAlbum())
        return;
    db_->close();
}

QString AlbumInfo::getTitle() const
{
    return title_;
}

QString AlbumInfo::getPath() const
{
    QString path(AlbumManager::instance()->getLibraryPath());
    path += QString("/") + title_;
    return QDir::cleanDirPath(path);
}

void AlbumInfo::setComments(const QString& comments)
{
    comments_ = comments;
    modified_ = true;
}

QString AlbumInfo::getComments() const
{
    return comments_;
}

void AlbumInfo::setCollection(const QString& collection)
{
    collection_ = collection;    
    modified_   = true;
}

QString AlbumInfo::getCollection() const
{
    return collection_;
}

QDate AlbumInfo::getDate() const
{
    return date_;
}

void AlbumInfo::setDate(const QDate& date )
{
    date_     = date;
    modified_ = true;
}

void AlbumInfo::setViewItem(void *viewItem)
{
    viewItem_ = viewItem;
}

void* AlbumInfo::getViewItem()
{
    return viewItem_;
}

AlbumInfo* AlbumInfo::nextAlbum()
{
    return next_;    
}

AlbumInfo* AlbumInfo::prevAlbum()
{
    return prev_;
}

void AlbumInfo::setItemComments(const QString& name,
                                const QString& comments)
{
    db_->insert(name, comments);    
}

QString AlbumInfo::getItemComments(const QString& name)
{
    return db_->find(name);
}

void AlbumInfo::deleteItemComments(const QString& name)
{
    db_->remove(name);
}

QStringList AlbumInfo::getAllItems()
{
    AlbumItemHandler* handler =
        AlbumManager::instance()->getItemHandler();
    if (handler)
        return handler->allItems();
    else
        return QStringList();
}

QStringList AlbumInfo::getSelectedItems()
{
    AlbumItemHandler* handler =
        AlbumManager::instance()->getItemHandler();
    if (handler)
        return handler->selectedItems();
    else
        return QStringList();
}

QStringList AlbumInfo::getAllItemsPath()
{
    AlbumItemHandler* handler =
        AlbumManager::instance()->getItemHandler();
    if (handler)
        return handler->allItemsPath();
    else
        return QStringList();
}

QStringList AlbumInfo::getSelectedItemsPath()
{
    AlbumItemHandler* handler =
        AlbumManager::instance()->getItemHandler();
    if (handler)
        return handler->selectedItemsPath();
    else
        return QStringList();
}

}

