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

// Local includes

#include "albumselectcombobox.h"
#include "taggingaction.h"

namespace Digikam
{

class AddTagsCompletionBox;
class AddTagsLineEdit;
class Album;
class CheckableAlbumFilterModel;
class TAlbum;
class TagModel;
class TagPropertiesFilterModel;
class TagTreeView;

class AddTagsComboBox : public TagTreeViewSelectComboBox
{
    Q_OBJECT

public:

    explicit AddTagsComboBox(QWidget* const parent = 0);
    ~AddTagsComboBox();

    /** You must call this after construction.
     *  If filtered/filterModel is 0, a default one is constructed
     */
    void setModel(TagModel* model, TagPropertiesFilterModel* filteredModel = 0, CheckableAlbumFilterModel* filterModel = 0);

    /** Returns the currently set tagging action.
     *  This is the last action emitted by either taggingActionActivated()
     *  or taggingActionSelected()
     */
    TaggingAction currentTaggingAction();

    /**
     * Sets the currently selected tag
     */
    void setCurrentTag(TAlbum* album);

    /** Reads a tag treeview and takes the currently selected tag into account
     *  when suggesting a parent tag for a new tag, and a default action.
     */
    void setTagTreeView(TagTreeView* treeView);

    void setClickMessage(const QString& message);

    QString text() const;
    void setText(const QString& text);

    AddTagsLineEdit* lineEdit()           const;
    AddTagsCompletionBox* completionBox() const;

public Q_SLOTS:

    /** Set a parent tag for suggesting a parent tag for a new tag, and a default action. */
    void setParentTag(TAlbum* album);

Q_SIGNALS:

    /** Emitted when the user activates an action (typically, by pressing return)
     */
    void taggingActionActivated(const TaggingAction& action);

    /** Emitted when an action is selected, but not explicitly activated.
     *  (typically by selecting an item in the tree view
     */
    void taggingActionSelected(const TaggingAction& action);

protected Q_SLOTS:

    void slotViewIndexActivated(const QModelIndex&);
    void slotLineEditActionActivated(const TaggingAction& action);
    void slotLineEditActionSelected(const TaggingAction& action);

private:

    virtual void installLineEdit();
    // make private
    void setEditable(bool editable);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADDTAGSCOMBOBOX_H
