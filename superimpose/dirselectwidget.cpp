/* ============================================================
 * File   : dirselectwidet.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-11
 * Description : a directory selection widget.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * ============================================================ */

// Qt includes. 
  
#include <qlayout.h>
#include <qheader.h>
#include <qlistview.h>
#include <qdir.h>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "dirselectwidget.h"

namespace DigikamSuperImposeImagesPlugin
{

struct DirSelectWidget::Private
{
    KFileTreeBranch* m_item;
    QStringList      m_pendingPath;
    QString          m_handled;
    KURL             m_rootUrl;
};

DirSelectWidget::DirSelectWidget( KURL rootUrl, KURL currentUrl, 
                                  QWidget* parent, const char* name, QString headerLabel)
               : KFileTreeView( parent, name)
{
    d = new Private;
    
    addColumn( headerLabel );
    
    if ( headerLabel.isNull() )
        header()->hide();
        
    setAlternateBackground(QColor::QColor());
    setRootPath(rootUrl, currentUrl);
}

DirSelectWidget::~DirSelectWidget()
{
    delete d;
}

KURL DirSelectWidget::path() const
{
    return currentURL();
}

void DirSelectWidget::load()
{
    if ( d->m_pendingPath.isEmpty() ) 
        {
        disconnect( d->m_item, SIGNAL( populateFinished(KFileTreeViewItem *) ), 
                    this, SLOT( load() ) );
        
        emit folderItemSelected(currentURL());
        return;
        }

    QString item = d->m_pendingPath.front();
    d->m_pendingPath.pop_front();
    d->m_handled += item;    
    KFileTreeViewItem* branch = findItem( d->m_item, d->m_handled );
    
    if ( !branch ) 
        {
        kdDebug() << "Unable to open " << d->m_handled << endl;
        }
    else
        {
        branch->setOpen( true );
        setSelected( branch, true );
        ensureItemVisible ( branch );
        d->m_handled += "/";
        
        if ( branch->alreadyListed() )
            load();
        }
}

void DirSelectWidget::setCurrentPath(KURL currentUrl)
{
    if ( !currentUrl.isValid() )
       return;
    
    QString currentPath = QDir::cleanDirPath(currentUrl.path());
    currentPath = currentPath.mid( d->m_rootUrl.path().length() );
    d->m_pendingPath.clear();    
    d->m_handled = QString("");
    d->m_pendingPath = QStringList::split( "/", currentPath, true );
    
    if ( !d->m_pendingPath[0].isEmpty() )
        d->m_pendingPath.prepend( "" ); // ensure we open the root first.        
        
    connect( d->m_item, SIGNAL( populateFinished(KFileTreeViewItem *) ),
             this, SLOT( load() ) );
    load();
}

void DirSelectWidget::setRootPath(KURL rootUrl, KURL currentUrl)
{
    d->m_rootUrl = rootUrl;
    clear();
    QString root = QDir::cleanDirPath(rootUrl.path());
    
    if ( !root.endsWith("/"))
       root.append("/");
    
    QString currentPath = QDir::cleanDirPath(currentUrl.isValid() ? currentUrl.path() : root);
    
    d->m_item = addBranch( rootUrl, rootUrl.fileName() );    
    setDirOnlyMode( d->m_item, true );
    currentPath = currentPath.mid( root.length() );
    d->m_pendingPath = QStringList::split( "/", currentPath, true );

    if ( !d->m_pendingPath[0].isEmpty() )
        d->m_pendingPath.prepend( "" ); // ensure we open the root first.
                    
    connect( d->m_item, SIGNAL( populateFinished(KFileTreeViewItem *) ),
             this, SLOT( load() ) );
    
    load();
    
    connect( this, SIGNAL( executed(QListViewItem *) ),
             this, SLOT( slotFolderSelected(QListViewItem *) ) );
}

KURL DirSelectWidget::rootPath(void)
{
    return d->m_rootUrl;
}

void DirSelectWidget::slotFolderSelected(QListViewItem *)
{
    emit folderItemSelected(currentURL());
}

}   // NameSpace DigikamSuperImposeImagesPlugin

#include "dirselectwidget.moc"
