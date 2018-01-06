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

#include "addtagscombobox.h"

// Qt includes

#include <QKeyEvent>
#include <QApplication>
#include <QGraphicsProxyWidget>

// Local includes

#include "digikam_debug.h"
#include "addtagslineedit.h"
#include "albumtreeview.h"
#include "albummodel.h"

namespace Digikam
{

class AddTagsComboBox::Private
{
public:

    Private()
    {
        lineEdit   = 0;
    }

    AddTagsLineEdit* lineEdit;
    TaggingAction    viewTaggingAction;
};

// ---------------------------------------------------------------------------------------

AddTagsComboBox::AddTagsComboBox(QWidget* const parent)
    : TagTreeViewSelectComboBox(parent),
      d(new Private)
{
    setInsertPolicy(QComboBox::NoInsert); // do not let Qt interfere when Enter is pressed
    setCloseOnActivate(true);
    setCheckable(false);
    setParent(0);

    d->lineEdit = new AddTagsLineEdit(this);
    setLineEdit(d->lineEdit);

    // See QTBUG-20531
    d->lineEdit->completer()->popup()->setParent(d->lineEdit, Qt::Popup);

    connect(d->lineEdit, SIGNAL(taggingActionActivated(TaggingAction)),
            this, SLOT(slotLineEditActionActivated(TaggingAction)));

    connect(d->lineEdit, SIGNAL(taggingActionSelected(TaggingAction)),
            this, SLOT(slotLineEditActionSelected(TaggingAction)));

    TagTreeView::Flags flags;
    m_treeView = new TagTreeView(this, flags);

    connect(m_treeView, SIGNAL(activated(QModelIndex)),
            this, SLOT(slotViewIndexActivated(QModelIndex)));

    d->lineEdit->completer()->popup()->installEventFilter(this);
    installEventFilter(this);
}

AddTagsComboBox::~AddTagsComboBox()
{
    delete d;
}

void AddTagsComboBox::setModel(TagModel* const model, TagPropertiesFilterModel* const filteredModel, CheckableAlbumFilterModel* const filterModel)
{
    TagTreeViewSelectComboBox::setModel(model, filteredModel, filterModel);

    // the line edit will pick one
    d->lineEdit->setModel(model, filteredModel, filterModel);
}

AddTagsLineEdit* AddTagsComboBox::lineEdit() const
{
    return d->lineEdit;
}

void AddTagsComboBox::setParentTag(TAlbum* const album)
{
    d->lineEdit->setParentTag(album);
}

void AddTagsComboBox::setCurrentTag(TAlbum* const album)
{
    view()->setCurrentAlbums(QList<Album*>() << album);
    slotViewIndexActivated(view()->currentIndex());
}

void AddTagsComboBox::setPlaceholderText(const QString& message)
{
    d->lineEdit->setPlaceholderText(message);
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

bool AddTagsComboBox::eventFilter(QObject* object, QEvent* event)
{
    if (object == d->lineEdit->completer()->popup())
    {
        if (event->type() == QEvent::Show)
        {
            QGraphicsProxyWidget* const proxyWidget = d->lineEdit->completer()->popup()->graphicsProxyWidget();

            if (proxyWidget)
            {
                proxyWidget->setMinimumHeight(0);
            }

            setFocus();
        }
        else if (event->type() == QEvent::Hide)
        {
            d->lineEdit->completer()->popup()->setFocus();
        }
        else if (event->type() == QEvent::Move)
        {
            if (parentWidget())
            {
                QPoint pos    = geometry().bottomLeft();
                QPoint newPos = parentWidget()->mapToGlobal(pos);
                newPos.ry()--;

                if (d->lineEdit->completer()->popup()->pos() != newPos)
                {
                    d->lineEdit->completer()->popup()->move(newPos);
                }
            }
        }

        return false;
    }
    else if (object == this)
    {
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        {
            if (d->lineEdit->completer()->popup()->isVisible())
            {
                QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(event);

                if (keyEvent && (keyEvent->key() == Qt::Key_Up ||
                                 keyEvent->key() == Qt::Key_Down ||
                                 keyEvent->key() == Qt::Key_Enter ||
                                 keyEvent->key() == Qt::Key_Return))
                {
                    QApplication::sendEvent(d->lineEdit->completer()->popup(), event);
                }
            }
        }
        else if (event->type() == QEvent::ShortcutOverride)
        {
            if (d->lineEdit->completer()->popup()->isVisible())
            {
                QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(event);

                if (keyEvent && (keyEvent->key() == Qt::Key_Up ||
                                 keyEvent->key() == Qt::Key_Down))
                {
                    event->accept();
                }
            }
        }
        else if (event->type() == QEvent::HoverMove)
        {
            if (d->lineEdit->completer()->popup()->isVisible())
            {
                d->lineEdit->completer()->popup()->hide();
            }
        }

        return false;
    }

    return TagTreeViewSelectComboBox::eventFilter(object, event);
}

} // namespace Digikam
