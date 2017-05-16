/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creating tags
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QLineEdit>

// Local includes

#include "taggingaction.h"

namespace Digikam
{

class AlbumFilterModel;
class Album;
class TAlbum;
class TagModel;
class TagPropertiesFilterModel;
class TagTreeView;

class AddTagsLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    explicit AddTagsLineEdit(QWidget* const parent = 0);
    ~AddTagsLineEdit();

    /**
     * Optional: set a model for additional information, like tag icons
     */
    void setSupportingTagModel(TagModel* const model);
    /**
     * Set a tag filter model. Completion suggestions will be limited to tags contained in the filter model.
     */
    void setFilterModel(AlbumFilterModel* const model);
    /**
     * Convenience: Will call setSupportingTagModel() and setFilterModel()
     */
    void setModel(TagModel* const model, TagPropertiesFilterModel* const filteredModel, AlbumFilterModel* const filterModel);

    /** Reads a tag treeview and takes the currently selected tag into account
     *  when suggesting a parent tag for a new tag, and a default action.
     */
    void setTagTreeView(TagTreeView* const treeView);

    /**
     * Adjusts the current default tagging action to assign the given tag
     */
    void setCurrentTag(TAlbum* const tag);

    void setAllowExceedBound(bool value);

    void setCurrentTaggingAction(const TaggingAction& action);

    TaggingAction currentTaggingAction() const;

public Q_SLOTS:

    /** Set a parent tag for suggesting a parent tag for a new tag, and a default action.
     *  If you set a tag tree view, this is taken care for automatically.
     */
    void setParentTag(Album* const album);

Q_SIGNALS:

    /// Emitted when the user activates an action (typically, by pressing return)
    void taggingActionActivated(const TaggingAction& action);
    /// Emitted when an action is selected. This already happens if anything is typed.
    void taggingActionSelected(const TaggingAction& action);
    void taggingActionFinished();

protected Q_SLOTS:

    void completerHighlighted(const TaggingAction& action);
    void completerActivated(const TaggingAction& action);
    void slotReturnPressed();
    void slotEditingFinished();
    void slotTextEdited(const QString& text);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADDTAGSLINEEDIT_H
