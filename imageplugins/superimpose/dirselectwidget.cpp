/* ============================================================
 * File   : dirselectwidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
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

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// Local includes.

#include "dirselectwidget.h"

namespace DigikamSuperImposeImagesPlugin
{

struct DirSelectWidget::Private
{
    KFileTreeView*   m_treeView;
    KFileTreeBranch* m_item;
    QStringList      m_pendingPath;
    QString          m_handled;
    KURL             m_rootUrl;
};

DirSelectWidget::DirSelectWidget( KURL rootUrl, KURL currentUrl, QWidget* parent, const char* name)
               : QWidget( parent, name)
{
    d = new Private;
    
    QVBoxLayout* layout = new QVBoxLayout( this, 0 );
    d->m_treeView = new KFileTreeView( this );
    layout->addWidget( d->m_treeView );

    d->m_treeView->addColumn( i18n("Folders" ) );
    d->m_treeView->header()->setStretchEnabled( true, 0 );
    
    setRootPath(rootUrl, currentUrl);
}

DirSelectWidget::~DirSelectWidget()
{
    delete d;
}

KURL DirSelectWidget::path() const
{
    return d->m_treeView->currentURL();
}

void DirSelectWidget::load()
{
    if ( d->m_pendingPath.isEmpty() ) 
        {
        disconnect( d->m_item, SIGNAL( populateFinished(KFileTreeViewItem *) ), 
                    this, SLOT( load() ) );
        return;
        }

    QString item = d->m_pendingPath.front();

    d->m_pendingPath.pop_front();

    d->m_handled += item;
    
    KFileTreeViewItem* branch = d->m_treeView->findItem( d->m_item, d->m_handled );
    
    if ( !branch ) 
        {
        kdDebug() << "Unable to open " << d->m_handled << endl;
        }
    else
        {
        branch->setOpen( true );
        d->m_treeView->setSelected( branch, true );
        d->m_treeView->ensureItemVisible ( branch );
        
        if ( branch->alreadyListed() )
            load();
        }
}

void DirSelectWidget::slotFolderSelected(QListViewItem *)
{
    emit folderItemSelected(d->m_treeView->currentURL());
}

void DirSelectWidget::setRootPath(KURL rootUrl, KURL currentUrl)
{
    d->m_rootUrl = rootUrl;
    d->m_treeView->clear();
    QString root = rootUrl.path();
    QString uploadPath = currentUrl.isValid() ? currentUrl.path() : root;

    d->m_item = d->m_treeView->addBranch( rootUrl, rootUrl.fileName() );    
    
    d->m_treeView->setDirOnlyMode( d->m_item, true );
        
    uploadPath = uploadPath.mid( root.length() );
        
    d->m_pendingPath = QStringList::split( "/", uploadPath, true );
        
    if ( !d->m_pendingPath[0].isEmpty() )
        d->m_pendingPath.prepend( "" ); // ensure we open the root first.
        
    load();
        
    connect( d->m_item, SIGNAL( populateFinished(KFileTreeViewItem *) ),
             this, SLOT( load() ) );
    
    connect( d->m_treeView, SIGNAL( executed(QListViewItem *) ),
             this, SLOT( slotFolderSelected(QListViewItem *) ) );
}

KURL DirSelectWidget::rootPath(void)
{
    return d->m_rootUrl;
}

}   // NameSpace DigikamSuperImposeImagesPlugin

#include "dirselectwidget.moc"
