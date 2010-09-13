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

#include <kcombobox.h>
#include <kcompletionbox.h>
#include <klineedit.h>

// Local includes

#include "albumselectcombobox.h"
#include "modelcompletion.h"

namespace Digikam
{

class AlbumFilterModel;
class TAlbum;
class TagModel;
class TagTreeView;

class TagModelCompletion : public ModelCompletion
{
public:

    /** A KCompletion object operating on a TagModel */

    TagModelCompletion();

    void setModel(TagModel *model);
    void setModel(AlbumFilterModel *model);
    TagModel *model() const;
};

class TaggingAction
{
public:

    /** Describes two possible actions:
     *  Assigning an existing tag, known by tag id,
     *  or creation of a new tag, with a given tag name and a parent tag.
     */

    enum Type
    {
        NoAction,
        AssignTag,
        CreateNewTag
    };

    /// Create a NoAction
    TaggingAction() : m_type(NoAction), m_tagId(-1) {}
    /// Assign the existing tag with given id
    TaggingAction(int tagId) : m_type(AssignTag), m_tagId(tagId) {}
    /** Create a new tag with the given name.
     *  The parent shall be the tag with the given id,
     *  or 0 for a toplevel tag.*/
    TaggingAction(const QString& name, int parentTagId)
        : m_type(CreateNewTag), m_tagId(parentTagId), m_tagName(name) {}

    bool operator==(const TaggingAction& other)
    {
        return m_type == other.m_type &&
               m_tagId == other.m_tagId &&
               m_tagName == other.m_tagName;
    }

    Type type() const              { return m_type; }
    bool isValid() const           { return m_type != NoAction; }
    bool shallAssignTag() const    { return m_type == AssignTag; }
    bool shallCreateNewTag() const { return m_type == CreateNewTag; }

    /// If shallAssignTag(), returns the tag id
    int tagId() const          { return m_tagId; }

    /// If shallCreateNewTag(), returns the tag name and the parent tag id, 0 for toplevel tag
    QString newTagName() const { return m_tagName; }
    int parentTagId() const    { return m_tagId; }

protected:

    Type    m_type;
    int     m_tagId;
    QString m_tagName;
};

// ---------------------------------------------------------------------------------------

class AddTagsCompletionBoxPriv;

class AddTagsCompletionBox : public KCompletionBox
{
    Q_OBJECT

public:

    /** A KCompletionBox drop down box optimized for use with an AddTagsLineEdit
     *  and a tag model.
     *  Reimplements a couple of methods in KCompletionBox for suitable behavior.
     *  Keeps a current TaggingAction, which is set when the user selects
     *  a tag in the drop down box.
     */

    AddTagsCompletionBox(QWidget* parent = 0);
    ~AddTagsCompletionBox();

    /** Updates the completion box. Gives the current text in the line edit
     *  and the completion matches. */
    void setItems(const QString& currentText, const QStringList& completionEntries);

    /** Optional: Reads a tag model and takes the currently selected tag into account
     *  when suggesting a parent tag for a new tag, and a default action.
     */
    void setTagModel(TagModel* model);

    /** Returns the current TaggingAction. When setItems was called, this is
     *  the default action. Changes when the user selected a different selection.
     */
    TaggingAction currentTaggingAction();

    /** Returns the current completion text, the text for the current item for display in the line edit,
     *  while the text() of the current item can be user presentable and formatted.
     */
    QString currentCompletionText() const;

    // Reimplemented
    virtual void setVisible( bool visible );
    virtual void popup();
    virtual QSize sizeHint() const;

public Q_SLOTS:

    /** Set a "parent tag" taken into account when suggesting a
     *  parent tag for a new tag, and a default action.
     *  If you set a tag model (setTagModel()), this is taken care for automatically.
     */
    void setCurrentParentTag(const QModelIndex& index);
    void setCurrentParentTag(TAlbum* album);

Q_SIGNALS:

    /// Emitted when the completion text changes, see above.
    void currentCompletionTextChanged(const QString& completionText);
    /// Emitted when the current tagging action changes
    void currentTaggingActionChanged(const TaggingAction& action);

    void completionActivated(const QString& completionText);

protected:

    // Reimplemented
    virtual QRect calculateGeometry() const;
    void sizeAndPosition();
    void sizeAndPosition(bool wasVisible);

protected Q_SLOTS:

    void slotItemActivated(QListWidgetItem* item);
    void slotCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:

    AddTagsCompletionBoxPriv* const d;
};

// ---------------------------------------------------------------------------------------

class AddTagsLineEditPriv;

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

Q_SIGNALS:

    /// Emitted when the user activates an action (typically, by pressing return)
    void taggingActionActivated(const TaggingAction& action);

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

    AddTagsLineEditPriv* const d;
};

class AddTagsComboBoxPriv;

class AddTagsComboBox : public AlbumSelectComboBox
{
    Q_OBJECT

public:

    AddTagsComboBox(QWidget* parent = 0);
    ~AddTagsComboBox();

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

private:

    virtual void installLineEdit();
    // make private
    void setEditable(bool editable);

    AddTagsComboBoxPriv* const d;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::TaggingAction)

#endif // ADDTAGSLINEEDIT_H
