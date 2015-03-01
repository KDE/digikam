/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creatingtags
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 1997      Sven Radej (sven.radej@iname.com)
 * Copyright (c) 1999      Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 * Copyright (c) 1999      Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000-2001 Dawit Alemayehu <adawit@kde.org>
 * Copyright (c) 2000-2001 Carsten Pfeiffer <pfeiffer@kde.org>
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

// KDE includes
#include <QKeyEvent>

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "addtagscompletionbox.h"
#include "tageditdlg.h"
#include "album.h"
#include "albummodel.h"
#include "albumtreeview.h"
#include "tagscache.h"
#include "databaseaccess.h"
#include "albumdb.h"

namespace Digikam
{

class AddTagsLineEdit::Private
{
public:

    Private()
    {
        completion    = 0;
        completionBox = 0;
        tagView       = 0;
        resetFromCompleter = false;
    }

    TagModelCompletion*   completion;
    AddTagsCompletionBox* completionBox;
    TagTreeView*          tagView;
    TaggingAction         currentTaggingAction;
    bool                  resetFromCompleter;
    TAlbum*               parentTag;
    TAlbum*               currentTag;
    TagModel*             tagModel;

//public:

    //TaggingAction makeTaggingAction(const QString& userText);
};

// ---------------------------------------------------------------------------------------

AddTagsLineEdit::AddTagsLineEdit(QWidget* const parent)
    : QLineEdit(parent), d(new Private)
{
//    setEnableSignals(true);
//    setHandleSignals(false);

    d->completion = new TagModelCompletion;
    //d->completionBox = new AddTagsCompletionBox(this);
    //QLineEdit::setCompleter(d->completion);
    setCompleter(d->completion);

    //setCompletionObject(d->completion);
    //setAutoDeleteCompletionObject(true);



    //setCompletionBox(d->completionBox);

    //setCompletionMode(KCompletion::CompletionPopup);
    //d->completion->setCompletionMode(QCompleter::PopupCompletion);
    //d->completion->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
//     //disconnect(d->completion, 0, this, 0);
//    if(disconnect(d->completion, SIGNAL(activated(QString)), this, SLOT(QLineEdit::setText(QString))))
//        qDebug() << "Successfully disconnected";
//    else
//        qDebug() << "Not disconnected";
    //disconnect(d->completion, SIGNAL(activated(QString)),0, 0);
   // connect(d->completion, SIGNAL(activated(QModelIndex)), this, SLOT(completerActivated(QModelIndex)));

    //connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
    //setTrapReturnKey(true);

//    connect(d->completionBox, SIGNAL(currentCompletionTextChanged(QString)),
//            this, SLOT(slotCompletionBoxTextChanged(QString)));

//    connect(d->completionBox, SIGNAL(currentTaggingActionChanged(TaggingAction)),
//            this, SLOT(slotCompletionBoxTaggingActionChanged(TaggingAction)));

//    connect(d->completionBox, SIGNAL(userCancelled(QString)),
//            this, SLOT(slotCompletionBoxCancelled()));

//    connect(d->completionBox, SIGNAL(completionActivated(QString)),
//            this, SIGNAL(completionBoxActivated(QString)));

//    connect(this, SIGNAL(completion(QString)),
//            this, SLOT(makeCompletion(QString)));

//    connect(this, SIGNAL(substringCompletion(QString)),
//            this, SLOT(makeSubstringCompletion(QString)));

//    connect(this, SIGNAL(textRotation(KCompletionBase::KeyBindingType)),
//            this, SLOT(rotateText(KCompletionBase::KeyBindingType)));

//    connect(this, SIGNAL(returnPressed(QString)),
//            this, SLOT(slotReturnPressed(QString)));

//    connect(this, SIGNAL(textChanged(QString)),
//            this, SLOT(slotTextChanged(QString)));
}

AddTagsLineEdit::~AddTagsLineEdit()
{
    delete d;
}

void AddTagsLineEdit::setModel(TagModel* model)
{
    d->completion->setModel(model);
    d->tagModel = model;
    //setCompleter(d->completion);
   // d->completionBox->setTagModel(model);
}

void AddTagsLineEdit::setModel(AlbumFilterModel* model)
{
    d->completion->setModel(model);
    //d->completionBox->setTagModel(model);
}

void AddTagsLineEdit::setModel(TagModel* model, TagPropertiesFilterModel* filteredModel, AlbumFilterModel* filterModel)
{
    if (filterModel)
    {
        setModel(filterModel);
    }
    else if (filteredModel)
    {
        setModel(filteredModel);
    }
    else
    {
        setModel(model);
    }
}

void AddTagsLineEdit::setTagTreeView(TagTreeView* view)
{
    if (d->tagView)
    {
        disconnect(d->tagView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                   this, SLOT(setParentTag(QModelIndex)));
    }

    d->tagView = view;
    setParentTag(d->tagView->currentAlbum());

    if (d->tagView)
    {
        //d->completionBox->setParentTag(d->tagView->currentIndex());
        connect(d->tagView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                this, SLOT(setParentTag(QModelIndex)));
    }
}

void AddTagsLineEdit::setCurrentTag(TAlbum *album)
{
    d->parentTag = album;
}

void AddTagsLineEdit::setParentTag(QModelIndex& index)
{
    if(index.isValid())
        d->parentTag = dynamic_cast<TAlbum*>(d->tagModel->albumForIndex(index));
}

void AddTagsLineEdit::setParentTag(TAlbum *album)
{
    if(album != NULL)
        d->parentTag = album;
}

void AddTagsLineEdit::completerActivated(QModelIndex index)
{

    //QLineEdit::clear();

    int id = index.data(Qt::UserRole+5).toInt();

    qDebug() << "Completer activated" << index.data() << id;
    if(id == -5)
    {
        QMap<QString, QString> errMap;
        AlbumList al = TagEditDlg::createTAlbum(d->tagView->currentAlbum(),
                                                index.data(Qt::UserRole+4).toString(),
                                                "tag",QKeySequence(),errMap);
        if(!al.isEmpty())
        {
            d->tagModel->setCheckState((TAlbum*)(al.first()),Qt::Checked);
        }
    }
    else
    {
        TAlbum* tAlbum = AlbumManager::instance()->findTAlbum(id);
        d->tagModel->setCheckState(tAlbum,Qt::Checked);
    }
    QLineEdit::clear();
}

void AddTagsLineEdit::setAllowExceedBound(bool value)
{
    Q_UNUSED(value);
    // set maximum size of pop-up widget
}

void AddTagsLineEdit::setCompleter(TagModelCompletion *c)
{
    if (d->completion)
        QObject::disconnect(d->completion, 0, this, 0);

    d->completion = c;

    if (!d->completion)
    return;

    d->completion->setWidget(this);
    connect(d->completion, SIGNAL(activated(QModelIndex)), this, SLOT(completerActivated(QModelIndex)));
}

void AddTagsLineEdit::focusInEvent(QFocusEvent *f)
{
    QLineEdit::focusInEvent(f);
    /** Need to disconnect completer from QLineEdit, otherwise
     *  we won't be able to clear completion after tag was added
     */
    disconnect(d->completion, SIGNAL(activated(QString)), this, SLOT(setText(QString)));
}

void AddTagsLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (d->completion && d->completion->popup()->isVisible())
    {
    // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
            e->ignore();
        return; // Let the completer do default behavior
        }
    }

    bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
    if (!isShortcut)
        QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

    if (!d->completion)
        return;

    bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
    {
        d->completion->popup()->hide();
        return;
    }
    d->completion->update(text());
    d->completion->popup()->setCurrentIndex(d->completion->completionModel()->index(0, 0));
}

//void AddTagsLineEdit::setCurrentTag(TAlbum* album)
//{
//    setCurrentTaggingAction(album ? TaggingAction(album->id()) : TaggingAction());
//    setText(album ? album->title() : QString());
//}

//void AddTagsLineEdit::setCurrentTaggingAction(const TaggingAction& action)
//{
//    if (d->currentTaggingAction == action)
//    {
//        return;
//    }

//    d->currentTaggingAction = action;
//    emit taggingActionSelected(action);
//}

//TaggingAction AddTagsLineEdit::currentTaggingAction() const
//{
//    if (d->currentTaggingAction.isValid())
//    {
//        return d->currentTaggingAction;
//    }
//    else if (!text().isEmpty())
//    {
//        return d->makeTaggingAction(text());
//    }
//    else
//    {
//        return TaggingAction();
//    }
//}



//void AddTagsLineEdit::setCompletionObject(KCompletion* comp, bool)
//{
//    if (compObj())
//    {
//        disconnect(compObj(), SIGNAL(matches(QStringList)),
//                   this, SLOT(setCompletedItems(QStringList)));
//    }

//    if (comp)
//    {
//        connect(comp, SIGNAL(matches(QStringList)),
//                this, SLOT(setCompletedItems(QStringList)));
//    }

//    KCompletionBase::setCompletionObject(comp, false);
//}

//AddTagsCompletionBox* AddTagsLineEdit::tagCompletionBox() const
//{
//    return d->completionBox;
//}




//void AddTagsLineEdit::makeSubstringCompletion(const QString&)
//{
//    setCompletedItems(compObj()->substringCompletion(text()));
//}

//void AddTagsLineEdit::makeCompletion(const QString& text)
//{
//    // Need to reimplement already because setCompletedItems is not virtual
//    KCompletion* const comp          = compObj();
//    KCompletion::CompletionMode mode = completionMode();
//    d->currentTaggingAction          = TaggingAction();

//    if (text.isEmpty())
//    {
//        emit taggingActionSelected(TaggingAction());
//    }

//    if ( !comp || mode == KCompletion::CompletionNone )
//    {
//        return;    // No completion object...
//    }

//    const QString match = comp->makeCompletion( text );

//    if ( mode == KCompletion::CompletionPopup ||
//         mode == KCompletion::CompletionPopupAuto )
//    {
//        if ( text.isEmpty() )
//        {
//            if (d->completionBox)
//            {
//                d->completionBox->hide();
//                d->completionBox->clear();
//            }
//        }
//        else
//        {

//            TagsCache* const tc     = TagsCache::instance();
//            QStringList allMatches  = comp->allMatches();

//            //get previously entered tags
//            QList<int> recentTagIDs = DatabaseAccess().db()->getRecentlyAssignedTags();

//            //reorder matches according to previously entered tags
//            QListIterator<int> id(recentTagIDs);
//            id.toBack();

//            while (id.hasPrevious())
//            {
//                QString tagName = tc->tagName(id.previous());
//                int pos         = allMatches.indexOf(tagName);

//                if (pos>0)
//                {
//                    allMatches.move(pos,0);
//                }
//            }

//            setCompletedItems(allMatches);
//        }
//    }
//    else // Auto,  ShortAuto (Man) and Shell
//    {
//        // all other completion modes
//        // If no match or the same match, simply return without completing.
//        if ( match.isEmpty() || match == text )
//        {
//            return;
//        }

//        if ( mode != KCompletion::CompletionShell )
//        {
//            setUserSelection(false);
//        }

//        if ( autoSuggest() )
//        {
//            setCompletedText( match );
//        }
//    }
//}

//void AddTagsLineEdit::setCompletedItems(const QStringList& items, bool doAutoSuggest)
//{
//    // Solution in kdelibs is suboptimal for our needs
//    QString txt;

//    if (d->completionBox->isVisible())
//    {
//        // The popup is visible already - do the matching on the initial string,
//        // not on the currently selected one.
//        txt = d->completionBox->cancelledText();
//    }
//    else
//    {
//        txt = text();
//    }

//    // The part commented out hides the popup when there is exactly one, full, hit
//    if (txt.isEmpty() /*|| (items.count() == 1 && txt == items.first())*/)
//    {
//        if (d->completionBox->isVisible())
//        {
//            d->completionBox->hide();
//        }
//    }
//    else
//    {
//        if ( d->completionBox->isVisible() )
//        {
//            //QString currentSelection = d->completionBox->currentCompletionText();

//            d->completionBox->setItems(txt, items);

///*
//            const bool blocked = d->completionBox->blockSignals( true );
//            d->completionBox->setCurrentCompletionText(currentSelection);
//            d->completionBox->blockSignals( blocked );
//*/
//        }
//        else // completion box not visible yet -> show it
//        {
//            if ( !txt.isEmpty() )
//            {
//                d->completionBox->setCancelledText( txt );
//            }

//            d->completionBox->setItems(txt, items);
//            d->completionBox->popup();
//        }

//        if ( autoSuggest() && doAutoSuggest && !items.isEmpty())
//        {
//            const int index       = items.first().indexOf( txt );
//            const QString newText = items.first().mid( index );
//            setUserSelection(false); // can be removed? setCompletedText sets it anyway
//            setCompletedText(newText,true);
//        }
//    }
//}

//void AddTagsLineEdit::slotCompletionBoxTextChanged(const QString& t)
//{
//    if (!t.isEmpty() && t != text())
//    {
//        setText(t);
//        setModified(true);
//        emit textEdited(t);
//        end(false); // force cursor at end
//    }
//}

//void AddTagsLineEdit::slotCompletionBoxTaggingActionChanged(const TaggingAction& action)
//{
//    setCurrentTaggingAction(action);
//}

//void AddTagsLineEdit::slotCompletionBoxCancelled()
//{
//    if (text().isEmpty())
//    {
//        setCurrentTaggingAction(TaggingAction());
//    }
//    else
//    {
//        setCurrentTaggingAction(d->makeTaggingAction(text()));
//    }
//}

//void AddTagsLineEdit::slotReturnPressed(const QString& text)
//{
//    if (text.isEmpty())
//    {
//      //focus back to mainview
//      emit taggingActionFinished();
//      return;
//    }

//    //Q_UNUSED(text);
//    emit taggingActionActivated(currentTaggingAction());
//}

//void AddTagsLineEdit::slotTextChanged(const QString& text)
//{
//    qDebug() << "Text changed" << text;
//    if(d->resetFromCompleter)
//    {
//        QLineEdit::clear();
//        d->resetFromCompleter = false;
//    }
//    if (text.isEmpty())
//    {
//        setCurrentTaggingAction(TaggingAction());
//    }

//    // for cases like copy+paste where autocompletion does not activate
//    else if (!d->currentTaggingAction.isValid())
//    {
//        setCurrentTaggingAction(d->makeTaggingAction(text));
    //    }
//}



//TaggingAction AddTagsLineEdit::Private::makeTaggingAction(const QString& text)
//{
//    TAlbum* const parentTag = completionBox->parentTag();
//    int parentTagId         = parentTag ? parentTag->id() : 0;
//    return AddTagsCompletionBox::makeDefaultTaggingAction(text, parentTagId);
//}

} // namespace Digikam
