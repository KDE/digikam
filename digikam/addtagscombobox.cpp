/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : Special combo box for adding or creating tags
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

class AddTagsComboBox::AddTagsComboBoxPriv
{
public:

    AddTagsComboBoxPriv()
    {
        treeView   = 0;
        lineEdit   = 0;
    }

    TagTreeView*     treeView;
    AddTagsLineEdit* lineEdit;
    TaggingAction    currentTaggingAction;
};

// ---------------------------------------------------------------------------------------

AddTagsComboBox::AddTagsComboBox(QWidget* parent)
               : AlbumSelectComboBox(parent), d(new AddTagsComboBoxPriv)
{
    QComboBox::setAutoCompletion(false);
    setEditable(true);
    setCloseOnActivate(true);
    setCheckable(false);

}

AddTagsComboBox::~AddTagsComboBox()
{
    delete d;
}

void AddTagsComboBox::setModel(TagModel* model, TagPropertiesFilterModel *filteredModel, CheckableAlbumFilterModel* filterModel)
{
    // AlbumSelectComboBox calls virtual installView() and  installLineEdit(), below
    AlbumSelectComboBox::setModel(model, filterModel);
    d->treeView->setAlbumModel(model);
    d->treeView->setAlbumFilterModel(filteredModel, filterModel);

    connect(view(), SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotViewCurrentAlbumChanged(Album*)));

    d->lineEdit->setTagModel(model);
}

void AddTagsComboBox::installLineEdit()
{
    delete d->lineEdit;
    d->lineEdit = new AddTagsLineEdit(this);

    connect(d->lineEdit, SIGNAL(taggingActionActivated(const TaggingAction&)),
            this, SLOT(slotLineEditActionActivated(const TaggingAction&)));

    d->lineEdit->setClearButtonShown(true);
    m_comboLineEdit = d->lineEdit;
    setLineEdit(d->lineEdit);
}

void AddTagsComboBox::installView(QAbstractItemView *view)
{
    if (!view && !d->treeView)
    {
        TagTreeView::Flags flags;
        d->treeView = new TagTreeView(this, flags);
    }
    // called from initialize, tree view is constructed
    AlbumSelectComboBox::installView(view ? view : d->treeView);
}

TagTreeView* AddTagsComboBox::view() const
{
    // the view in the combo box popup
    return d->treeView;
}

void AddTagsComboBox::setEditable(bool)
{
    // just made private, ignored
}

void AddTagsComboBox::sendViewportEventToView(QEvent* e)
{
    // needed for StayPoppedUpComboBox
    view()->viewportEvent(e);
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

void AddTagsComboBox::setCurrentTag(int tagId)
{
    view()->setCurrentAlbum(tagId);
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
    return d->lineEdit->setText(text);
}

TaggingAction AddTagsComboBox::currentTaggingAction()
{
    return d->currentTaggingAction;
}

void AddTagsComboBox::slotViewCurrentAlbumChanged(Album* album)
{
    d->lineEdit->selectAll();
    if (album)
    {
        d->lineEdit->setText(album->title());
        d->currentTaggingAction = TaggingAction(album->id());
    }
    else
    {
        d->lineEdit->setText(QString());
        d->currentTaggingAction = TaggingAction();
    }
    emit taggingActionSelected(d->currentTaggingAction);
}

void AddTagsComboBox::slotLineEditActionActivated(const TaggingAction& action)
{
    d->currentTaggingAction = action;
    emit taggingActionActivated(d->currentTaggingAction);
}

} // namespace Digikam
