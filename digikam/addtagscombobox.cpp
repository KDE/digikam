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

// Qt includes

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// Local includes

#include "addtagslineedit.h"
#include "albummodel.h"

namespace Digikam
{

class AddTagsComboBox::AddTagsComboBoxPriv
{
public:

    AddTagsComboBoxPriv()
    {
        lineEdit   = 0;
    }

    AddTagsLineEdit *lineEdit;
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

void AddTagsComboBox::installLineEdit()
{
    d->lineEdit = new AddTagsLineEdit(this);
    connect(d->lineEdit, SIGNAL(taggingActionActivated(const TaggingAction&)),
            this, SIGNAL(taggingActionActivated(const TaggingAction&)));
    d->lineEdit->setClearButtonShown(true);
    setLineEdit(d->lineEdit);
}

void AddTagsComboBox::setEditable(bool)
{
    // just made private, ignored
}

void AddTagsComboBox::setModel(TagModel* model, AlbumFilterModel *filterModel)
{
    AlbumSelectComboBox::setModel(model, filterModel);
    d->lineEdit->setTagModel(model);
}

void AddTagsComboBox::setTagTreeView(TagTreeView *view)
{
    d->lineEdit->setTagTreeView(view);
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


} // namespace Digikam
