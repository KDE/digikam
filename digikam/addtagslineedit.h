/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creatingtags
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ADDTAGSLINEEDIT_H
#define ADDTAGSLINEEDIT_H

// Qt includes

// KDE includes

#include <klineedit.h>

// Local includes

#include "taggingaction.h"

namespace Digikam
{

class AddTagsCompletionBox;
class AlbumFilterModel;
class TAlbum;
class TagModel;
class TagTreeView;

class AddTagsLineEdit : public KLineEdit
{
    Q_OBJECT

public:

    AddTagsLineEdit(QWidget* parent = 0);
    ~AddTagsLineEdit();

    /** Set the tag model to use for completion. */
    void setTagModel(TagModel* model);
    /** Reads a tag treeview and takes the currently selected tag into account
     *  when suggesting a parent tag for a new tag, and a default action. */
    void setTagTreeView(TagTreeView* treeView);

    /// The custom completion box in use
    AddTagsCompletionBox* completionBox() const;

public Q_SLOTS:

    /** Set a parent tag for suggesting a parent tag for a new tag, and a default action. */
    void setParentTag(TAlbum* album);

Q_SIGNALS:

    /// Emitted when the user activates an action (typically, by pressing return)
    void taggingActionActivated(const TaggingAction& action);
    /// Emitted when an action is selected. This already happens if anything is typed.
    void taggingActionSelected(const TaggingAction& action);

protected Q_SLOTS:

    virtual void makeCompletion(const QString&);
    void setCompletedItems(const QStringList& items, bool autoSuggest=true);
    void makeSubstringCompletion(const QString&);
    void slotCompletionBoxTextChanged(const QString& text);
    void slotCompletionBoxTaggingActionChanged(const TaggingAction& action);
    void slotCompletionBoxCancelled();
    void slotReturnPressed(const QString& text);

protected:

    void setCompletionObject(KCompletion* comp, bool dontuse=false);

private:

class AddTagsLineEditPriv;
    AddTagsLineEditPriv* const d;
};

} // namespace Digikam


#endif // ADDTAGSLINEEDIT_H
