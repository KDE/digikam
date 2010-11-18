/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creatingtags
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
 * Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000, 2001 Dawit Alemayehu <adawit@kde.org>
 * Copyright (c) 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>
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

#include "addtagslineedit.moc"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumthumbnailloader.h"
#include "albumtreeview.h"
#include "modelcompletion.h"
#include "tagscache.h"

namespace Digikam
{

static TaggingAction makeDefaultTaggingAction(const QString& text, int parentTagId)
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
                tagId = TagsCache::instance()->tagForName(text); // toplevel tag
            }
            if (!tagId)
            {
                // sort lexically
                QMap<QString, int> map;
                foreach (int id, tagIds)
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

TagModelCompletion::TagModelCompletion()
{
}

void TagModelCompletion::setModel(TagModel* model)
{
    ModelCompletion::setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
}

TagModel* TagModelCompletion::model() const
{
    return static_cast<TagModel*>(ModelCompletion::model());
}

class AddTagsCompletionBoxItem : public QListWidgetItem
{
public:

    AddTagsCompletionBoxItem()
    {
    }

    void setAction(const TaggingAction& action) { m_action = action; }
    TaggingAction &action()                     { return m_action; }
    const TaggingAction action() const          { return m_action; }

protected:

    TaggingAction m_action;
};

// ---------------------------------------------------------------------------------------

class AddTagsCompletionBoxPriv
{
public:

    enum SpecialRoles
    {
        CompletionTextRole = Qt::UserRole,
        TaggingActionRole  = Qt::UserRole + 1
    };

    AddTagsCompletionBoxPriv() :
        upwardBox(false),
        model(0)
    {
    }

    bool upwardBox;

    TagModel*             model;
    AlbumPointer<TAlbum> parentTag;

    AddTagsCompletionBoxItem* createItemForExistingTag(TAlbum* talbum, bool uniqueName);
    AddTagsCompletionBoxItem* createItemForNewTag(const QString& newName, TAlbum* parent);
    int maximumAvailableScreenHeight(const QPoint& globalPos);
};

AddTagsCompletionBox::AddTagsCompletionBox(QWidget* parent)
                    : KCompletionBox(parent), d(new AddTagsCompletionBoxPriv)
{
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(slotCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));

    connect(this, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotItemActivated(QListWidgetItem*)));
}

AddTagsCompletionBox::~AddTagsCompletionBox()
{
    delete d;
}

void AddTagsCompletionBox::setTagModel(TagModel* model)
{
    d->model = model;
}

AddTagsCompletionBoxItem* AddTagsCompletionBoxPriv::createItemForExistingTag(TAlbum* talbum, bool uniqueName)
{
    if (!talbum || talbum->isRoot())
    {
        return 0;
    }

    AddTagsCompletionBoxItem *item = new AddTagsCompletionBoxItem;

    TAlbum *parent = static_cast<TAlbum*>(talbum->parent());
    if (parent->isRoot() || uniqueName)
    {
        item->setText(talbum->title());
    }
    else
    {
        item->setText(i18nc("<tag name> in <tag path>", "%1\n  in %2",
                            talbum->title(), parent->tagPath(false)));
    }
    if (model)
    {
        QModelIndex index = model->indexForAlbum(talbum);
        item->setData(Qt::DecorationRole, index.data(Qt::DecorationRole));
    }

    item->setData(CompletionTextRole, talbum->title());
    item->setAction(TaggingAction(talbum->id()));

    return item;
}

AddTagsCompletionBoxItem* AddTagsCompletionBoxPriv::createItemForNewTag(const QString& newName, TAlbum* parent)
{
    int parentTagId = parent ? parent->id() : 0;

    // If tag exists, do not add an entry to create it
    if (TagsCache::instance()->tagForName(newName, parentTagId))
    {
        return 0;
    }

    AddTagsCompletionBoxItem *item = new AddTagsCompletionBoxItem;

    if (parent)
    {
        item->setText(i18nc("Create New Tag <tag name> in <parent tag path>", "Create New Tag \"%1\"\n  in %2",
                            newName, parent->tagPath(false)));
    }
    else
    {
        item->setText(i18n("Create New Tag \"%1\"", newName));
    }
    item->setData(Qt::DecorationRole, AlbumThumbnailLoader::instance()->getNewTagIcon());

    item->setData(CompletionTextRole, newName);
    item->setAction(TaggingAction(newName, parentTagId));

    return item;
}

void AddTagsCompletionBox::setItems(const QString& currentText, const QStringList& completionEntries)
{
    clear();

    int parentTagId                                 = d->parentTag ? d->parentTag->id() : 0;
    // We use this action to find the right entry to select
    TaggingAction defaultAction                     = makeDefaultTaggingAction(currentText, parentTagId);

    AddTagsCompletionBoxItem* createItemUnderParent = d->createItemForNewTag(currentText, d->parentTag);
    AddTagsCompletionBoxItem* createItemTopLevel    = d->createItemForNewTag(currentText, 0);

    QList<AddTagsCompletionBoxItem*> assignItems;
    foreach (const QString &tagName, completionEntries)
    {
        QList<int> tagIds = TagsCache::instance()->tagsForName(tagName);
        bool uniqueName = tagIds.count() == 1;
        foreach (int tagId, tagIds)
        {
            AddTagsCompletionBoxItem* item = d->createItemForExistingTag(AlbumManager::instance()->findTAlbum(tagId), uniqueName);
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
            addItem(createItemUnderParent);
            setCurrentItem(createItemUnderParent);
            foreach (AddTagsCompletionBoxItem* item, assignItems)
            {
                addItem(item);
            }
            addItem(createItemTopLevel);
        }
        else if (createItemTopLevel && createItemTopLevel->action() == defaultAction)
        {
            addItem(createItemTopLevel);
            setCurrentItem(createItemTopLevel);
            foreach (AddTagsCompletionBoxItem* item, assignItems)
            {
                addItem(item);
            }
            addItem(createItemUnderParent);
        }
    }
    else
    {
        foreach (AddTagsCompletionBoxItem* item, assignItems)
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
                                             AddTagsCompletionBoxPriv::CompletionTextRole,
                                             completionText, 1, Qt::MatchExactly);
    if (!indexes.isEmpty())
        setCurrentIndex(indexes.first());
}
*/

QString AddTagsCompletionBox::currentCompletionText() const
{
    QListWidgetItem* current = currentItem();
    if (current)
    {
        return current->data(AddTagsCompletionBoxPriv::CompletionTextRole).toString();
    }
    return QString();
}

TaggingAction AddTagsCompletionBox::currentTaggingAction()
{
    AddTagsCompletionBoxItem* current = static_cast<AddTagsCompletionBoxItem*>(currentItem());
    if (current)
    {
        return current->action();
    }
    return TaggingAction();
}

void AddTagsCompletionBox::setCurrentParentTag(const QModelIndex& index)
{
    d->parentTag = static_cast<TAlbum*>(AbstractAlbumModel::retrieveAlbum(index));
}

void AddTagsCompletionBox::setCurrentParentTag(TAlbum* album)
{
    d->parentTag = album;
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
        AddTagsCompletionBoxItem *boxItem = static_cast<AddTagsCompletionBoxItem*>(current);
        emit currentCompletionTextChanged(current->data(AddTagsCompletionBoxPriv::CompletionTextRole).toString());
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
    int currentGeom = height();
    QPoint currentPos = pos();
    QRect geom = calculateGeometry();
    resize( geom.size() );

    int x = currentPos.x();
    int y = currentPos.y();
    if ( parentWidget() )
    {
        if (!wasVisible)
        {
            QPoint orig = globalPositionHint();
            QRect screenSize = QApplication::desktop()->availableGeometry(orig);

            x = orig.x() + geom.x();
            y = orig.y() + geom.y();

            //kDebug() << orig << screenSize << y << height() << screenSize.bottom() << (y + height() > screenSize.bottom());

            if (x + width() > screenSize.right())
            {
                x = screenSize.right() - width();
            }

            if (y + height() > screenSize.bottom())
            {
                y = y - height() - parentWidget()->height();
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
        move(x, y);
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
    for (int i = 0; i < count(); i++)
    {
        if (item(i)->flags() & Qt::ItemIsSelectable)
        {
            itemSizeHint = sizeHintForIndex(indexFromItem(item(i)));
            break;
        }
    }
    if (!itemSizeHint.isValid() || itemSizeHint.isNull())
    {
        return QRect();
    }

    const int frameHeight = 2*frameWidth();
    int maxHeight         = d->maximumAvailableScreenHeight(globalPositionHint());
    int suggestedHeight   = qMin(15, count()) * itemSizeHint.height();
    //kDebug() << itemSizeHint << maxHeight << suggestedHeight;
    int h                 = qMin(maxHeight, suggestedHeight + frameHeight);

    int w = KListWidget::minimumSizeHint().width();
    if (parentWidget())
    {
        w = qMax(parentWidget()->width(), KListWidget::minimumSizeHint().width());
    }

    return QRect(0, 0, w, h);
}

void AddTagsCompletionBox::setVisible( bool visible )
{
    // KCompletionBox is doing some important stuff, we need to call it
    bool wasVisible = isVisible();
    KCompletionBox::setVisible(visible);
    if (visible && parentWidget())
        sizeAndPosition(wasVisible);
}

void AddTagsCompletionBox::popup()
{
    if (count() == 0)
    {
        hide();
    }
    else
    {
        /*bool block = signalsBlocked();
        blockSignals( true );
        setCurrentRow( -1 );
        blockSignals( block );
        clearSelection();*/
        if (!isVisible())
        {
            show();
        }
        else if (size().height() != sizeHint().height())
        {
            sizeAndPosition(isVisible());
        }
    }
}

QSize AddTagsCompletionBox::sizeHint() const
{
    return calculateGeometry().size();
}

int AddTagsCompletionBoxPriv::maximumAvailableScreenHeight(const QPoint &orig)
{
    QRect screenSize = QApplication::desktop()->availableGeometry(orig);
    //kDebug() << screenSize << orig << qMax(orig.y() - screenSize.top(), screenSize.bottom() - orig.y());
    if (!screenSize.contains(orig))
    {
        return screenSize.height(); // bail out
    }
    return qMax(orig.y() - screenSize.top(), screenSize.bottom() - orig.y());
}

class AddTagsLineEditPriv
{
public:

    AddTagsLineEditPriv() :
        completion(0),
        completionBox(0),
        tagModel(0),
        tagView(0)
    {
    }

    TagModelCompletion*   completion;
    AddTagsCompletionBox* completionBox;
    TagModel*             tagModel;
    TagTreeView*          tagView;
    TaggingAction         completionBoxAction;

public:

    TaggingAction makeTaggingAction(const QString& userText);
};

// ---------------------------------------------------------------------------------------

AddTagsLineEdit::AddTagsLineEdit(QWidget* parent)
               : KLineEdit(parent), d(new AddTagsLineEditPriv)
{
    setEnableSignals(true);
    setHandleSignals(false);

    d->completion = new TagModelCompletion;
    setCompletionObject(d->completion);
    setAutoDeleteCompletionObject(true);

    d->completionBox = new AddTagsCompletionBox(this);
    setCompletionBox(d->completionBox);

    setCompletionMode(KGlobalSettings::CompletionPopup);

    connect(d->completionBox, SIGNAL(currentCompletionTextChanged( const QString& )),
            this, SLOT(slotCompletionBoxTextChanged( const QString& )) );

    connect(d->completionBox, SIGNAL(currentTaggingActionChanged(const TaggingAction&)),
            this, SLOT(slotCompletionBoxTaggingActionChanged(const TaggingAction&)));

    connect(d->completionBox, SIGNAL(userCancelled( const QString& )),
            this, SLOT(slotCompletionBoxCancelled()));

    connect(d->completionBox, SIGNAL(completionActivated(QString)),
            this, SIGNAL(completionBoxActivated(QString)) );

    connect(this, SIGNAL(completion(const QString&)),
            this, SLOT(makeCompletion(const QString&)));

    connect(this, SIGNAL(substringCompletion(const QString&)),
            this, SLOT(makeSubstringCompletion(const QString&)));

    connect(this, SIGNAL(textRotation(KCompletionBase::KeyBindingType)),
            this, SLOT(rotateText(KCompletionBase::KeyBindingType)));

    connect(this, SIGNAL(returnPressed(const QString&)),
            this, SLOT(slotReturnPressed(const QString&)));
}

AddTagsLineEdit::~AddTagsLineEdit()
{
    delete d;
}

void AddTagsLineEdit::setTagModel(TagModel* model)
{
    d->tagModel = model;
    d->completion->setModel(model);
    d->completionBox->setTagModel(model);
}

void AddTagsLineEdit::setTagTreeView(TagTreeView *view)
{
    if (d->tagView)
    {
        disconnect(d->tagView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                   d->completionBox, SLOT(setCurrentParentTag(const QModelIndex&)));
    }


    d->tagView = view;

    if (d->tagView)
    {
        d->completionBox->setCurrentParentTag(d->tagView->currentIndex());
        connect(d->tagView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                d->completionBox, SLOT(setCurrentParentTag(const QModelIndex&)));
    }
}

void AddTagsLineEdit::setCompletionObject(KCompletion* comp, bool)
{
    if (compObj())
    {
        disconnect(compObj(), SIGNAL( matches( const QStringList& )),
                   this, SLOT( setCompletedItems( const QStringList& )));
    }

    if (comp)
    {
        connect(comp, SIGNAL( matches( const QStringList& )),
                this, SLOT( setCompletedItems( const QStringList& )));
    }

    KCompletionBase::setCompletionObject(comp, false);
}

AddTagsCompletionBox* AddTagsLineEdit::completionBox() const
{
    return d->completionBox;
}

void AddTagsLineEdit::makeSubstringCompletion(const QString&)
{
    setCompletedItems(compObj()->substringCompletion(text()));
}

void AddTagsLineEdit::makeCompletion(const QString& text)
{
    // Need to reimplement already because setCompletedItems is not virtual
    KCompletion* comp = compObj();
    KGlobalSettings::Completion mode = completionMode();

    d->completionBoxAction = TaggingAction();

    if (!comp || mode == KGlobalSettings::CompletionNone)
    {
        return; // No completion object...
    }

    const QString match = comp->makeCompletion( text );

    if (mode == KGlobalSettings::CompletionPopup ||
        mode == KGlobalSettings::CompletionPopupAuto)
    {
        if (text.isEmpty())
        {
            if (d->completionBox)
            {
                d->completionBox->hide();
                d->completionBox->clear();
            }
        }
        else
        {
            setCompletedItems(comp->allMatches());
        }
    }
    else // Auto,  ShortAuto (Man) and Shell
    {
        // all other completion modes
        // If no match or the same match, simply return without completing.
        if (match.isEmpty() || match == text)
        {
            return;
        }

        if (mode != KGlobalSettings::CompletionShell)
        {
            setUserSelection(false);
        }

        if (autoSuggest())
        {
            setCompletedText(match);
        }
    }
}

void AddTagsLineEdit::setCompletedItems(const QStringList &items, bool doAutoSuggest)
{
    // Solution in kdelibs is suboptimal for our needs
    QString txt;

    if (d->completionBox->isVisible())
    {
        // The popup is visible already - do the matching on the initial string,
        // not on the currently selected one.
        txt = d->completionBox->cancelledText();
    }
    else
    {
        txt = text();
    }

    if (txt.isEmpty() || (items.count() == 1 && txt == items.first()))
    {
        if (d->completionBox->isVisible())
        {
            d->completionBox->hide();
        }
    }
    else
    {
        if (d->completionBox->isVisible())
        {
            //QString currentSelection = d->completionBox->currentCompletionText();

            d->completionBox->setItems(txt, items);

            /*const bool blocked = d->completionBox->blockSignals( true );
            d->completionBox->setCurrentCompletionText(currentSelection);
            d->completionBox->blockSignals( blocked );*/
        }
        else // completion box not visible yet -> show it
        {
            if (!txt.isEmpty())
            {
                d->completionBox->setCancelledText(txt);
            }
            d->completionBox->setItems(txt, items);
            d->completionBox->popup();
        }

        if (autoSuggest() && doAutoSuggest)
        {
            const int index = items.first().indexOf(txt);
            const QString newText = items.first().mid(index);
            setUserSelection(false); // can be removed? setCompletedText sets it anyway
            setCompletedText(newText, true);
        }
    }
}

void AddTagsLineEdit::slotCompletionBoxTextChanged(const QString& t)
{
    if (!t.isEmpty() && t != text())
    {
        setText(t);
        setModified(true);
        emit textEdited(t);
        end(false); // force cursor at end
    }
}

void AddTagsLineEdit::slotCompletionBoxTaggingActionChanged(const TaggingAction& action)
{
    d->completionBoxAction = action;
}

void AddTagsLineEdit::slotCompletionBoxCancelled()
{
    d->completionBoxAction = TaggingAction();
}

void AddTagsLineEdit::slotReturnPressed(const QString& text)
{
    if (d->completionBoxAction.isValid())
    {
        emit taggingActionActivated(d->completionBoxAction);
    }
    else
    {
        emit taggingActionActivated(d->makeTaggingAction(text));
    }
}

TaggingAction AddTagsLineEditPriv::makeTaggingAction(const QString& text)
{
    TAlbum* parentTag = 0;
    if (tagView)
    {
        parentTag = tagView->currentAlbum();
    }
    int parentTagId = parentTag ? parentTag->id() : 0;
    return makeDefaultTaggingAction(text, parentTagId);
}

} // namespace Digikam
