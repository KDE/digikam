/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : KCompletionBox for tags
 *
 * Copyright (C) 2010       by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 1997       by Sven Radej (sven.radej@iname.com)
 * Copyright (c) 1999       by Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * Copyright (c) 1999       by Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000, 2001 by Dawit Alemayehu <adawit@kde.org>
 * Copyright (c) 2000, 2001 by Carsten Pfeiffer <pfeiffer@kde.org>
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

#include "addtagscompletionbox.moc"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>
#include <QScrollBar>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "album.h"
#include "albumfiltermodel.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumthumbnailloader.h"
#include "albumtreeview.h"
#include "tagscache.h"

namespace Digikam
{

TagModelCompletion::TagModelCompletion()
{
}

void TagModelCompletion::setModel(TagModel* model)
{
    ModelCompletion::setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
}

void TagModelCompletion::setModel(AlbumFilterModel* model)
{
    ModelCompletion::setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
}

TagModel* TagModelCompletion::model() const
{
    QAbstractItemModel* const model = ModelCompletion::model();

    if (dynamic_cast<TagModel*>(model))
    {
        return static_cast<TagModel*>(model);
    }
    else if (dynamic_cast<AlbumFilterModel*>(model))
    {
        return static_cast<TagModel*>(static_cast<AlbumFilterModel*>(model)->sourceAlbumModel());
    }

    return 0;
}

// ---------------------------------------------------------------------------------------

class AddTagsCompletionBoxItem : public QListWidgetItem
{
public:

    AddTagsCompletionBoxItem()
    {
    }

    void setAction(const TaggingAction& action)
    {
        m_action = action;
    }

    TaggingAction& action()
    {
        return m_action;
    }

    const TaggingAction action() const
    {
        return m_action;
    }

protected:

    TaggingAction m_action;
};

// ---------------------------------------------------------------------------------------

class AddTagsCompletionBox::Private
{
public:

    enum SpecialRoles
    {
        CompletionTextRole = Qt::UserRole,
        TaggingActionRole  = Qt::UserRole + 1
    };

    Private()
    {
        upwardBox          = false;
        model              = 0;
        filterModel        = 0;
        allowExceedBounds  = false;
    }

    AddTagsCompletionBoxItem* createItemForExistingTag(TAlbum* talbum, bool uniqueName);
    AddTagsCompletionBoxItem* createItemForNewTag(const QString& newName, TAlbum* parent);
    QSize maximumAvailableScreenSize(const QPoint& globalPos);

public:

    bool                 upwardBox;

    TagModel*            model;
    AlbumFilterModel*    filterModel;

    AlbumPointer<TAlbum> parentTag;
    bool                 allowExceedBounds;

};

AddTagsCompletionBox::AddTagsCompletionBox(QWidget* const parent)
    : KCompletionBox(parent), d(new Private)
{
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(slotCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    connect(this, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotItemActivated(QListWidgetItem*)));
}

AddTagsCompletionBox::~AddTagsCompletionBox()
{
    delete d;
}

void AddTagsCompletionBox::setTagModel(TagModel* model)
{
    d->model       = model;
    d->filterModel = 0;
}

void AddTagsCompletionBox::setTagModel(AlbumFilterModel* model)
{
    d->filterModel = model;
    d->model       = 0;
}

AddTagsCompletionBoxItem* AddTagsCompletionBox::Private::createItemForExistingTag(TAlbum* talbum, bool uniqueName)
{
    if (!talbum || talbum->isRoot())
    {
        return 0;
    }

    AddTagsCompletionBoxItem* const item = new AddTagsCompletionBoxItem;

    TAlbum* const parent = static_cast<TAlbum*>(talbum->parent());

    if (parent->isRoot() || uniqueName)
    {
        item->setText(talbum->title());
    }
    else
    {
        item->setText(i18nc("<tag name> in <tag path>", "%1 in %2",
                            talbum->title(), parent->tagPath(false)));
    }

    if (model || filterModel)
    {
        QModelIndex index;

        if (filterModel)
        {
            index = filterModel->indexForAlbum(talbum);
        }
        else if (model)
        {
            index = model->indexForAlbum(talbum);
        }

        item->setData(Qt::DecorationRole, index.data(Qt::DecorationRole));
    }

    item->setData(CompletionTextRole, talbum->title());
    item->setAction(TaggingAction(talbum->id()));

    return item;
}

AddTagsCompletionBoxItem* AddTagsCompletionBox::Private::createItemForNewTag(const QString& newName, TAlbum* parent)
{
    int parentTagId = parent ? parent->id() : 0;

    // If tag exists, do not add an entry to create it
    if (TagsCache::instance()->tagForName(newName, parentTagId))
    {
        return 0;
    }

    AddTagsCompletionBoxItem* const item = new AddTagsCompletionBoxItem;

    if (parent)
    {
        item->setText(i18nc("Create New Tag <tag name> in <parent tag path>", "Create \"%1\" in %2",
                            newName, parent->tagPath(false)));
    }
    else
    {
        item->setText(i18n("Create \"%1\"", newName));
    }

    item->setData(Qt::DecorationRole, AlbumThumbnailLoader::instance()->getNewTagIcon());
    item->setData(CompletionTextRole, newName);
    item->setAction(TaggingAction(newName, parentTagId));

    return item;
}

void AddTagsCompletionBox::setItems(const QString& currentText, const QStringList& completionEntries)
{
    clear();

    int parentTagId                                    = d->parentTag ? d->parentTag->id() : 0;
    // We use this action to find the right entry to select
    TaggingAction defaultAction                        = makeDefaultTaggingAction(currentText, parentTagId);

    AddTagsCompletionBoxItem* createItemUnderParent    = 0;

    if (d->parentTag)
    {
        createItemUnderParent = d->createItemForNewTag(currentText, d->parentTag);
    }

    AddTagsCompletionBoxItem* const createItemTopLevel = d->createItemForNewTag(currentText, 0);

    QList<AddTagsCompletionBoxItem*> assignItems;

    foreach(const QString& tagName, completionEntries)
    {
        QList<int> tagIds = TagsCache::instance()->tagsForName(tagName);
        bool uniqueName   = (tagIds.count() == 1);

        foreach(int tagId, tagIds)
        {
            AddTagsCompletionBoxItem* const item = d->createItemForExistingTag(AlbumManager::instance()->findTAlbum(tagId), uniqueName);

            if (!item)
            {
                continue;
            }

            assignItems << item;
        }
    }

    if (defaultAction.shallCreateNewTag())
    {
        // If it is the default action, we place the "Create Tag" entry at the top of the list.
        if (createItemUnderParent && createItemUnderParent->action() == defaultAction)
        {
            //Case A
            //a tag is currently selected in the listbox, we have the choice of toplevel and underparent for a new tag
            //the entire text currently written by the user doesn't exist as a tag. However, it might be a part of a tag

            foreach(AddTagsCompletionBoxItem* const item, assignItems)
            {
                addItem(item);
            }

            addItem(createItemUnderParent);
            addItem(createItemTopLevel);
            setCurrentRow(0);
        }
        else // if (createItemTopLevel && createItemTopLevel->action() == defaultAction)
        {
            //Case B
            //no tag is currently selected in the listbox, only toplevel choice for a new tag
            //the entire text currently written by the user doesn't exist as a tag. However, it might be a part of a tag

            foreach(AddTagsCompletionBoxItem* const item, assignItems)
            {
                addItem(item);
            }

            addItem(createItemTopLevel);
            setCurrentRow(0);
        }
    }
    else
    {
        //Case C
        //the entire text currently written by the user exists as a tag

        foreach(AddTagsCompletionBoxItem* const item, assignItems)
        {
            addItem(item);

            if (defaultAction == item->action())
            {
                setCurrentItem(item);
            }
        }

        addItem(createItemUnderParent);
        addItem(createItemTopLevel);
    }

    if (isVisible() && size().height() != sizeHint().height())
    {
        sizeAndPosition();
    }
}

/*
/// If an item matches the given completion text exactly, it is made the current item
void setCurrentCompletionText(const QString& completionText);
QString currentCompletionText() const;

void AddTagsCompletionBox::setCurrentCompletionText(const QString &completionText)
{
    QModelIndexList indexes = model()->match(model()->index(0, 0, QModelIndex()),
                                             AddTagsCompletionBox::Private::CompletionTextRole,
                                             completionText, 1, Qt::MatchExactly);
    if (!indexes.isEmpty())
        setCurrentIndex(indexes.first());
}
*/

QString AddTagsCompletionBox::currentCompletionText() const
{
    QListWidgetItem* const current = currentItem();

    if (current)
    {
        return current->data(AddTagsCompletionBox::Private::CompletionTextRole).toString();
    }

    return QString();
}

TaggingAction AddTagsCompletionBox::currentTaggingAction()
{
    AddTagsCompletionBoxItem* const current = static_cast<AddTagsCompletionBoxItem*>(currentItem());

    if (current)
    {
        return current->action();
    }

    return TaggingAction();
}

void AddTagsCompletionBox::setParentTag(const QModelIndex& index)
{
    d->parentTag = static_cast<TAlbum*>(AbstractAlbumModel::retrieveAlbum(index));
}

void AddTagsCompletionBox::setParentTag(TAlbum* album)
{
    d->parentTag = album;
}

TAlbum* AddTagsCompletionBox::parentTag() const
{
    return d->parentTag;
}

void AddTagsCompletionBox::setAllowExceedBounds(bool allow)
{
    d->allowExceedBounds = allow;
}

void AddTagsCompletionBox::slotItemActivated(QListWidgetItem* item)
{
    if (item)
    {
        emit completionActivated(item->data(Qt::UserRole).toString());
    }
}

void AddTagsCompletionBox::slotCurrentItemChanged(QListWidgetItem* current, QListWidgetItem*)
{
    if (current)
    {
        //was fired on each pressed char by user
        AddTagsCompletionBoxItem* const boxItem = static_cast<AddTagsCompletionBoxItem*>(current);        
        emit currentTaggingActionChanged(boxItem->action());
    }
    else
    {
        emit currentCompletionTextChanged(QString());
        emit currentTaggingActionChanged(TaggingAction());
    }
}

void AddTagsCompletionBox::sizeAndPosition()
{
    sizeAndPosition(isVisible());
}

void AddTagsCompletionBox::sizeAndPosition(bool wasVisible)
{
    // Solution in kdelibs is suboptimal for our needs
    int currentGeom   = height();
    QPoint currentPos = pos();
    QRect geom        = calculateGeometry();
    resize( geom.size() );

    int x             = currentPos.x();
    int y             = currentPos.y();

    if ( parentWidget() )
    {
        if (!wasVisible)
        {
            QPoint orig      = globalPositionHint();
            QRect screenSize = QApplication::desktop()->availableGeometry(orig);

            x                = orig.x() + geom.x();
            y                = orig.y() + geom.y();

            //kDebug() << orig << screenSize << y << height() << screenSize.bottom() << (y + height() > screenSize.bottom());

            if ( x + width() > screenSize.right() )
            {
                x = screenSize.right() - width();
            }

            if (y + height() > screenSize.bottom() )
            {
                y            = y - height() - parentWidget()->height();
                d->upwardBox = true;
            }
            else
            {
                d->upwardBox = false;
            }
        }
        else
        {
            // Are we above our parent? If so we must keep bottom edge anchored.
            if (d->upwardBox)
            {
                y += (currentGeom - height());
            }
        }

        move( x, y);
    }
}

QRect AddTagsCompletionBox::calculateGeometry() const
{
    // Solution in kdelibs is suboptimal for our needs
    if (!count())
    {
        return QRect();
    }

    QSize itemSizeHint;
    const int suggestedShownItems = 15;

    if (d->allowExceedBounds)
    {
        for (int i=0; i<count() && i<suggestedShownItems; ++i)
        {
            if (item(i)->flags() & Qt::ItemIsSelectable)
            {
                itemSizeHint = itemSizeHint.expandedTo(sizeHintForIndex(indexFromItem(item(i))));
            }
        }
    }
    else
    {
        for (int i=0; i<count(); ++i)
        {
            if (item(i)->flags() & Qt::ItemIsSelectable)
            {
                itemSizeHint = sizeHintForIndex(indexFromItem(item(i)));
                break;
            }
        }
    }

    if (!itemSizeHint.isValid() || itemSizeHint.isNull())
    {
        return QRect();
    }

    const int frameHeight = 2*frameWidth();
    const QSize maxSize   = d->maximumAvailableScreenSize(globalPositionHint());
    int suggestedHeight   = qMin(suggestedShownItems, count()) * itemSizeHint.height();

    //kDebug() << itemSizeHint << maxHeight << suggestedHeight;

    int h                 = qMin(maxSize.height(), suggestedHeight + frameHeight);
    int w                 = KListWidget::minimumSizeHint().width();

    if (d->allowExceedBounds)
    {
        const int scrollBarMargin = (count() > suggestedShownItems && verticalScrollBar()) ?
                                    verticalScrollBar()->sizeHint().width() : 0;
        w                         = qMin(itemSizeHint.width() + scrollBarMargin, maxSize.width());
    }
    else
    {
        if (parentWidget())
        {
            w = qMax(w, parentWidget()->width());
        }
    }

    return QRect(0, 0, w, h);
}

void AddTagsCompletionBox::setVisible( bool visible )
{
    // KCompletionBox is doing some important stuff, we need to call it
    bool wasVisible = isVisible();
    KCompletionBox::setVisible(visible);

    if (visible && parentWidget())
    {
        sizeAndPosition(wasVisible);
    }
}

void AddTagsCompletionBox::popup()
{
    if ( count() == 0 )
    {
        hide();
    }
    else
    {
/*
        bool block = signalsBlocked();
        blockSignals( true );
        setCurrentRow( -1 );
        blockSignals( block );
        clearSelection();
*/
        if ( !isVisible() )
        {
            show();
        }
        else if ( size().height() != sizeHint().height() )
        {
            sizeAndPosition(isVisible());
        }
    }
}

QSize AddTagsCompletionBox::sizeHint() const
{
    return calculateGeometry().size();
}

QSize AddTagsCompletionBox::Private::maximumAvailableScreenSize(const QPoint& orig)
{
    QRect screenGeom = QApplication::desktop()->availableGeometry(orig);

    //kDebug() << screenSize << orig << qMax(orig.y() - screenSize.top(), screenSize.bottom() - orig.y());

    if (!screenGeom.contains(orig))
    {
        return screenGeom.size();    // bail out
    }

    QSize size;
    size.setWidth(screenGeom.width());
    size.setHeight(qMax(orig.y() - screenGeom.top(), screenGeom.bottom() - orig.y()));
    return size;
}

TaggingAction AddTagsCompletionBox::makeDefaultTaggingAction(const QString& text, int parentTagId)
{
    // We now take the presumedly best action, without autocompletion popup.
    // 1. Tag exists?
    //    a) Single tag? Assign.
    //    b) Multiple tags? 1. Existing tag under parent. 2. Toplevel tag 3. Alphabetically lowest tag
    // 2. Create tag under parent. No parent selected? Toplevel

    QList<int> tagIds = TagsCache::instance()->tagsForName(text);

    if (!tagIds.isEmpty())
    {
        if (tagIds.count() == 1)
        {
            return TaggingAction(tagIds.first());
        }
        else
        {
            int tagId = 0;

            if (parentTagId)
            {
                tagId = TagsCache::instance()->tagForName(text, parentTagId);
            }

            if (!tagId)
            {
                tagId = TagsCache::instance()->tagForName(text);    // toplevel tag
            }

            if (!tagId)
            {
                // sort lexically
                QMap<QString, int> map;

                foreach(int id, tagIds)
                {
                    map[TagsCache::instance()->tagPath(id, TagsCache::NoLeadingSlash)] = id;
                }

                tagId = map.begin().value();
            }

            return TaggingAction(tagId);
        }
    }
    else
    {
        return TaggingAction(text, parentTagId);
    }
}

} // namespace Digikam
