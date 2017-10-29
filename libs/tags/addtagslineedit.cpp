/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creating tags
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 1997      by Sven Radej (sven.radej@iname.com)
 * Copyright (c) 1999      by Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * Copyright (c) 1999      by Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000-2001 by Dawit Alemayehu <adawit@kde.org>
 * Copyright (c) 2000-2001 by Carsten Pfeiffer <pfeiffer@kde.org>
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

#include "addtagslineedit.h"

// Local includes

#include "digikam_debug.h"
#include "tagscompleter.h"
#include "album.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "albumtreeview.h"
#include "taggingactionfactory.h"

namespace Digikam
{

class AddTagsLineEdit::Private
{
public:

    Private()
        : completer(0),
          tagView(0),
          tagFilterModel(0),
          parentTagId(0)
    {
    }

    TagCompleter*       completer;
    TagTreeView*        tagView;
    AlbumFilterModel*   tagFilterModel;
    TaggingAction       currentTaggingAction;
    int                 parentTagId;
};

AddTagsLineEdit::AddTagsLineEdit(QWidget* const parent)
    : QLineEdit(parent),
      d(new Private)
{
    setClearButtonEnabled(true);

    d->completer = new TagCompleter(this);
    d->completer->setMaxVisibleItems(15);

    setCompleter(d->completer);

    connect(this, SIGNAL(returnPressed()),
            this, SLOT(slotReturnPressed()));

    connect(this, SIGNAL(editingFinished()),
            this, SLOT(slotEditingFinished()));

    connect(this, SIGNAL(textEdited(QString)),
            this, SLOT(slotTextEdited(QString)));

    connect(d->completer, static_cast<void(TagCompleter::*)(const TaggingAction&)>(&TagCompleter::activated),
            [this](const TaggingAction& action){ completerActivated(action); });

    connect(d->completer, static_cast<void(TagCompleter::*)(const TaggingAction&)>(&TagCompleter::highlighted),
            [this](const TaggingAction& action){ completerHighlighted(action); });
}

AddTagsLineEdit::~AddTagsLineEdit()
{
    delete d;
}

void AddTagsLineEdit::setSupportingTagModel(TagModel* const model)
{
    d->completer->setSupportingTagModel(model);
}

void AddTagsLineEdit::setFilterModel(AlbumFilterModel* const model)
{
    d->tagFilterModel = model;
    d->completer->setTagFilterModel(d->tagFilterModel);
}

void AddTagsLineEdit::setModel(TagModel* const model, TagPropertiesFilterModel* const filteredModel, AlbumFilterModel* const filterModel)
{
    if (filterModel)
    {
        setFilterModel(filterModel);
    }
    else if (filteredModel)
    {
        setFilterModel(filteredModel);
    }

    setSupportingTagModel(model);
}

void AddTagsLineEdit::setTagTreeView(TagTreeView* const view)
{
    if (d->tagView)
    {
        disconnect(d->tagView, &TagTreeView::currentAlbumChanged, this,
                   &AddTagsLineEdit::setParentTag);
    }

    d->tagView = view;

    if (d->tagView)
    {
        connect(d->tagView, &TagTreeView::currentAlbumChanged, this,
                &AddTagsLineEdit::setParentTag);

        setParentTag(d->tagView->currentAlbum());
    }
}

void AddTagsLineEdit::setCurrentTag(TAlbum* const album)
{
    setCurrentTaggingAction(album ? TaggingAction(album->id()) : TaggingAction());
    setText(album ? album->title() : QString());
}

void AddTagsLineEdit::setParentTag(Album* const album)
{
    d->parentTagId = album ? album->id() : 0;
    d->completer->setContextParentTag(d->parentTagId);
}

void AddTagsLineEdit::setAllowExceedBound(bool value)
{
    Q_UNUSED(value);
    // -> the pop-up is allowed to be bigger than the line edit widget
    // Currently unimplemented, QCompleter calculates the size automatically.
    // Idea: intercept show event via event filter on completer->popup(); from there, change width.
}

// Tagging action is used by facemanagement and assignwidget

void AddTagsLineEdit::slotReturnPressed()
{
    if (text().isEmpty())
    {
        //focus back to mainview
        emit taggingActionFinished();
    }
    else if (!d->currentTaggingAction.isValid())
    {
        emit taggingActionActivated(currentTaggingAction());
    }
}

void AddTagsLineEdit::slotEditingFinished()
{
    d->currentTaggingAction = TaggingAction();
}

void AddTagsLineEdit::slotTextEdited(const QString& text)
{
    d->currentTaggingAction = TaggingAction();

    if (text.isEmpty())
    {
        emit taggingActionSelected(TaggingAction());
    }
    else
    {
        emit taggingActionSelected(TaggingActionFactory::defaultTaggingAction(text, d->parentTagId));
    }

    d->completer->update(text);
}

void AddTagsLineEdit::completerActivated(const TaggingAction& action)
{
    setCurrentTaggingAction(action);
    emit taggingActionActivated(action);
}

void AddTagsLineEdit::completerHighlighted(const TaggingAction& action)
{
    setCurrentTaggingAction(action);
}

void AddTagsLineEdit::setCurrentTaggingAction(const TaggingAction& action)
{
    d->currentTaggingAction = action;
    emit taggingActionSelected(action);
}

TaggingAction AddTagsLineEdit::currentTaggingAction() const
{
    if (d->currentTaggingAction.isValid())
    {
        return d->currentTaggingAction;
    }
    else if (text().isEmpty())
    {
        return TaggingAction();
    }

    return TaggingActionFactory::defaultTaggingAction(text(), d->parentTagId);
}

} // namespace Digikam
