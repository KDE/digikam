/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a
 *               template to an image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
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

#include "dirselectwidget.moc"

// Qt includes

#include <Q3Header>
#include <Q3ListView>
#include <QLayout>
#include <QDir>

// KDE includes

#include <klocale.h>
#include <kfiletreebranch.h>
#include <kdebug.h>

namespace DigikamDecorateImagePlugin
{

class DirSelectWidget::DirSelectWidgetPrivate
{
public:

    DirSelectWidgetPrivate() {};

    KFileTreeBranch* m_item;
    QStringList      m_pendingPath;
    QString          m_handled;
    KUrl             m_rootUrl;
};

DirSelectWidget::DirSelectWidget(QWidget* parent, const char* name, const QString& headerLabel)
    : K3FileTreeView( parent), d(new DirSelectWidgetPrivate)
{
    setObjectName(name);
    addColumn(headerLabel);

    if ( headerLabel.isNull() )
    {
        header()->hide();
    }

    setAlternateBackground(QColor());
}

DirSelectWidget::DirSelectWidget(const KUrl& rootUrl, const KUrl& currentUrl,
                                 QWidget* parent, const char* name, const QString& headerLabel)
    : K3FileTreeView( parent), d(new DirSelectWidgetPrivate)
{
    setObjectName(name);
    addColumn(headerLabel);

    if ( headerLabel.isNull() )
    {
        header()->hide();
    }

    setAlternateBackground(QColor());
    setRootPath(rootUrl, currentUrl);
}

DirSelectWidget::~DirSelectWidget()
{
    delete d;
}

KUrl DirSelectWidget::path() const
{
    return currentUrl();
}

void DirSelectWidget::load()
{
    if ( d->m_pendingPath.isEmpty() )
    {
        disconnect( d->m_item, SIGNAL(populateFinished(K3FileTreeViewItem*)),
                    this, SLOT(load()) );

        emit folderItemSelected(currentUrl());
        return;
    }

    QString item = d->m_pendingPath.front();
    d->m_pendingPath.pop_front();
    d->m_handled += item;
    K3FileTreeViewItem* branch = findItem( d->m_item, d->m_handled );

    if ( !branch )
    {
        kDebug() << "Unable to open " << d->m_handled;
    }
    else
    {
        branch->setOpen( true );
        setSelected( branch, true );
        ensureItemVisible ( branch );
        d->m_handled += '/';

        if ( branch->alreadyListed() )
        {
            load();
        }
    }
}

void DirSelectWidget::setCurrentPath(const KUrl& currentUrl)
{
    if ( !currentUrl.isValid() )
    {
        return;
    }

    QString currentPath = QDir::cleanPath(currentUrl.toLocalFile());
    currentPath = currentPath.mid(d->m_rootUrl.toLocalFile().length());
    d->m_pendingPath.clear();
    d->m_handled = QString("");
    d->m_pendingPath = currentPath.split('/', QString::KeepEmptyParts);

    if (!d->m_pendingPath.at(0).isEmpty())
    {
        d->m_pendingPath.prepend("");    // ensure we open the root first.
    }

    connect( d->m_item, SIGNAL(populateFinished(K3FileTreeViewItem*)),
             this, SLOT(load()) );
    load();
}

void DirSelectWidget::setRootPath(const KUrl& rootUrl, const KUrl& currentUrl)
{
    d->m_rootUrl = rootUrl;
    clear();
    QString root = QDir::cleanPath(rootUrl.toLocalFile());

    if (!root.endsWith('/'))
    {
        root.append("/");
    }

    QString currentPath = QDir::cleanPath(currentUrl.isValid() ? currentUrl.toLocalFile() : root);

    d->m_item = addBranch( rootUrl, rootUrl.fileName() );
    setDirOnlyMode( d->m_item, true );
    currentPath      = currentPath.mid( root.length() );
    d->m_pendingPath = currentPath.split('/', QString::KeepEmptyParts);

    if ( !d->m_pendingPath.at(0).isEmpty() )
    {
        d->m_pendingPath.prepend( "" );    // ensure we open the root first.
    }

    connect( d->m_item, SIGNAL(populateFinished(K3FileTreeViewItem*)),
             this, SLOT(load()) );

    load();

    connect( this, SIGNAL(executed(Q3ListViewItem*)),
             this, SLOT(slotFolderSelected(Q3ListViewItem*)) );
}

KUrl DirSelectWidget::rootPath() const
{
    return d->m_rootUrl;
}

void DirSelectWidget::slotFolderSelected(Q3ListViewItem*)
{
    emit folderItemSelected(currentUrl());
}

}   // namespace DigikamDecorateImagePlugin
