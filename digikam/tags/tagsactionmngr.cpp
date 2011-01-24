/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "tagsactionmngr.moc"

// Qt includes

#include <QSortFilterProxyModel>

// KDE includes

#include <KAction>
#include <KActionCollection>
#include <KLocale>

// Local includes

#include "tagproperties.h"
#include "albumdb.h"
#include "albuminfo.h"
#include "databaseaccess.h"

namespace Digikam
{

TagsActionMngr* TagsActionMngr::m_defaultManager = 0;

TagsActionMngr* TagsActionMngr::defaultManager()
{
    return m_defaultManager;
}

class TagsActionMngr::TagsActionMngrPrivate
{
public:

    TagsActionMngrPrivate()
    {
        actionCollection = 0;
        view             = 0;
    }

    QMap<int, KAction*> tagsActionMap;
    QWidget*            view;
    KActionCollection*  actionCollection;
};

// -------------------------------------------------------------------------------------------------

TagsActionMngr::TagsActionMngr(QWidget* parent, KActionCollection* actionCollection)
    : QObject(parent), d(new TagsActionMngrPrivate)
{
    d->actionCollection = actionCollection;
    d->view             = parent;
}

TagsActionMngr::~TagsActionMngr()
{
    delete d;
}

void TagsActionMngr::createActions()
{
    TagInfo::List tList = DatabaseAccess().db()->scanTags();

    for (TagInfo::List::const_iterator it = tList.constBegin(); it != tList.constEnd(); ++it)
    {
        TagProperties tprop((*it).id);

        if (tprop.hasProperty("KEYBOARD_SHORTCUT"))
        {
            // Create action relevant of tag which have a keyboard shortcut.

            KAction* action = new KAction(i18n("Assign Tag \"%1\"", (*it).name), this);
            action->setShortcut(KShortcut(tprop.value("KEYBOARD_SHORTCUT")));
            d->actionCollection->addAction(QString("tagshortcut-%1").arg((*it).id), action);

            connect(action, SIGNAL(triggered()), 
                    d->view, SLOT(slotAssignTagsFromShortcut(int)));
        }
    }

/*
    const QAbstractItemModel* model = KernelIf->collectionModel();

    connect(model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
            this, SLOT( slotRowsInserted( const QModelIndex&, int, int ) ), Qt::UniqueConnection );

    connect(KernelIf->folderCollectionMonitor(), SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
            this, SLOT( slotCollectionRemoved( const Akonadi::Collection& ) ), Qt::UniqueConnection );

    if ( model->rowCount() > 0 )
        updateShortcutsForIndex( QModelIndex(), 0, model->rowCount() - 1 );
*/
}

void TagsActionMngr::updateShortcutsForTag(int tagId, const QString& ks)
{
/*    QAbstractItemModel* model = KernelIf->collectionModel();
    for ( int i = start; i <= end; i++ )
    {
        const QModelIndex child = model->index( i, 0, parent );
        Akonadi::Collection collection =
            model->data( child, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
        if ( collection.isValid() )
        {
            shortcutChanged( collection );
        }
        if ( model->rowCount( child ) > 0 )
        {
            updateShortcutsForIndex( child, 0, model->rowCount( child ) - 1 );
        }
    }*/
}

void TagsActionMngr::slotTagRemoved(int tagId)
{
    delete d->tagsActionMap.take(tagId);
}

void TagsActionMngr::shortcutChanged(int tagId)
{
    // remove the old one, no autodelete in Qt4
    slotTagRemoved(tagId);
/*
    QSharedPointer<FolderCollection> folderCollection( FolderCollection::forCollection( col ) );
    if ( folderCollection->shortcut().isEmpty() )
        return;

    FolderShortcutCommand* command = new FolderShortcutCommand( mParent, col );
    mFolderShortcutCommands.insert( col.id(), command );

    KIcon icon( "folder" );
    if ( col.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
        !col.attribute<Akonadi::EntityDisplayAttribute>()->iconName().isEmpty() )
    {
        icon = KIcon( col.attribute<Akonadi::EntityDisplayAttribute>()->iconName() );
    }

    const QString actionLabel = i18n( "Folder Shortcut %1", col.name() );
    QString actionName = i18n( "Folder Shortcut %1", folderCollection->idString() );
    actionName.replace( ' ', '_' );
    KAction *action = mActionCollection->addAction( actionName );
    // The folder shortcut is set in the folder shortcut dialog.
    // The shortcut set in the shortcut dialog would not be saved back to
    // the folder settings correctly.
    action->setShortcutConfigurable( false );
    action->setText( actionLabel );
    action->setShortcuts( folderCollection->shortcut() );
    action->setIcon( icon );

    connect(action, SIGNAL( triggered( bool ) ), 
            command, SLOT( start() ) );

    command->setAction( action ); // will be deleted along with the command
*/
}

} // namespace Digikam
