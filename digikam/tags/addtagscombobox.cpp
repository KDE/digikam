/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : Special combo box for adding or creating tags
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

#include "addtagscombobox.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "addtagslineedit.h"
#include "albummodel.h"
#include "albumtreeview.h"

namespace Digikam
{

class AddTagsComboBox::Private
{
public:

    Private()
    {
        treeView   = 0;
        lineEdit   = 0;
    }

    TagTreeView*     treeView;
    AddTagsLineEdit* lineEdit;
    TaggingAction    viewTaggingAction;
};

// ---------------------------------------------------------------------------------------

AddTagsComboBox::AddTagsComboBox(QWidget* const parent)
    : TagTreeViewSelectComboBox(parent), d(new Private)
{
    QComboBox::setAutoCompletion(false);
    setInsertPolicy(QComboBox::NoInsert); // do not let Qt interfere when Enter is pressed
    setCloseOnActivate(true);
    setCheckable(false);

    d->lineEdit = new AddTagsLineEdit(this);
    setLineEdit(d->lineEdit);

    connect(d->lineEdit, SIGNAL(taggingActionActivated(TaggingAction)),
            this, SLOT(slotLineEditActionActivated(TaggingAction)));

    connect(d->lineEdit, SIGNAL(taggingActionSelected(TaggingAction)),
            this, SLOT(slotLineEditActionSelected(TaggingAction)));

    d->lineEdit->setClearButtonShown(true);

    TagTreeView::Flags flags;
    m_treeView = new TagTreeView(this, flags);

    connect(m_treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotViewIndexActivated(QModelIndex)));
}

AddTagsComboBox::~AddTagsComboBox()
{
    delete d;
}

void AddTagsComboBox::setModel(TagModel* model, TagPropertiesFilterModel* filteredModel, CheckableAlbumFilterModel* filterModel)
{
    TagTreeViewSelectComboBox::setModel(model, filteredModel, filterModel);

    // the line edit will pick one
    d->lineEdit->setModel(model, filteredModel, filterModel);
}

void AddTagsComboBox::installLineEdit()
{
}

AddTagsLineEdit* AddTagsComboBox::lineEdit() const
{
    return d->lineEdit;
}

AddTagsCompletionBox* AddTagsComboBox::completionBox() const
{
    return d->lineEdit->completionBox();
}

void AddTagsComboBox::setTagTreeView(TagTreeView* view)
{
    // this is a completely different view! Remove this functionality?
    d->lineEdit->setTagTreeView(view);
}

void AddTagsComboBox::setParentTag(TAlbum* album)
{
    d->lineEdit->setParentTag(album);
}

void AddTagsComboBox::setCurrentTag(TAlbum* album)
{
    view()->setCurrentAlbums(QList<Album*>() << album);
    slotViewIndexActivated(view()->currentIndex());
}

void AddTagsComboBox::setClickMessage(const QString& message)
{
    d->lineEdit->setClickMessage(message);
}

QString AddTagsComboBox::text() const
{
    return d->lineEdit->text();
}

void AddTagsComboBox::setText(const QString& text)
{
    d->lineEdit->setText(text);
}

TaggingAction AddTagsComboBox::currentTaggingAction()
{
    if (d->viewTaggingAction.isValid())
    {
        return d->viewTaggingAction;
    }

    return d->lineEdit->currentTaggingAction();
}

void AddTagsComboBox::slotViewIndexActivated(const QModelIndex& index)
{
    TAlbum* const album = view()->albumForIndex(index);

    //d->lineEdit->selectAll();
    if (album)
    {
        d->lineEdit->setText(album->title());
        d->viewTaggingAction = TaggingAction(album->id());
    }
    else
    {
        d->lineEdit->setText(QString());
        d->viewTaggingAction = TaggingAction();
    }

    emit taggingActionSelected(currentTaggingAction());
}

void AddTagsComboBox::slotLineEditActionActivated(const TaggingAction& action)
{
    d->viewTaggingAction = TaggingAction();
    emit taggingActionActivated(action);
}

void AddTagsComboBox::slotLineEditActionSelected(const TaggingAction& action)
{
    d->viewTaggingAction = TaggingAction();
    emit taggingActionSelected(action);
}

} // namespace Digikam
