/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-15
 * Description : Special combo box for adding or creating tags
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

#ifndef ADDTAGSCOMBOBOX_H
#define ADDTAGSCOMBOBOX_H

// Qt includes

// KDE includes

// Local includes

#include "albumselectcombobox.h"
#include "taggingaction.h"

namespace Digikam
{

class AlbumFilterModel;
class TagModel;
class TagTreeView;

class AddTagsComboBox : public AlbumSelectComboBox
{
    Q_OBJECT

public:

    AddTagsComboBox(QWidget* parent = 0);
    ~AddTagsComboBox();

    /** Returns the currently set tagging action. */
    TaggingAction currentTaggingAction();

    /** Set the tag model to use for completion. */
    void setModel(TagModel* model, AlbumFilterModel *filterModel = 0);
    /** Reads a tag treeview and takes the currently selected tag into account
     *  when suggesting a parent tag for a new tag, and a default action. */
    void setTagTreeView(TagTreeView* treeView);

    void setClickMessage(const QString& message);

    QString text() const;
    void setText(const QString& text);

Q_SIGNALS:

    /// Emitted when the user activates an action (typically, by pressing return)
    void taggingActionActivated(const TaggingAction& action);
    /** Emitted when an action is selected, but not explicitly activated.
     *  (typically by selecting an item in the tree view */
    void taggingActionSelected(const TaggingAction& action);

private:

    virtual void installLineEdit();
    // make private
    void setEditable(bool editable);

    class AddTagsComboBoxPriv;
    AddTagsComboBoxPriv* const d;
};

} // namespace Digikam

#endif // ADDTAGSCOMBOBOX_H
