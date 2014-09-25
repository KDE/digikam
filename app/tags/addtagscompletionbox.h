/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : KCompletionBox for tags
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

#ifndef ADDTAGSCOMPLETIONBOX_H
#define ADDTAGSCOMPLETIONBOX_H

// KDE includes

#include <kcompletionbox.h>

// Local includes

#include "modelcompletion.h"
#include "taggingaction.h"

namespace Digikam
{

class AlbumFilterModel;
class TAlbum;
class TagModel;

class TagModelCompletion : public ModelCompletion
{
public:

    /** A KCompletion object operating on a TagModel
    */
    TagModelCompletion();

    void setModel(TagModel* model);
    void setModel(AlbumFilterModel* model);
    TagModel* model() const;
};

// -------------------------------------------------------------------------------------------

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
    explicit AddTagsCompletionBox(QWidget* const parent = 0);
    ~AddTagsCompletionBox();

    /** Updates the completion box. Gives the current text in the line edit
     *  and the completion matches.
     */
    void setItems(const QString& currentText, const QStringList& completionEntries);

    /**
     * Optional: Reads a tag model for information and data.
     * You can set either, the last set model takes precedence.
     */
    void setTagModel(TagModel* model);
    void setTagModel(AlbumFilterModel* model);

    /**
     * A "parent tag" taken into account when suggesting a
     * parent tag for a new tag, and a default action.
     */
    TAlbum* parentTag() const;

    /**
     * Allow the box to expand horizontally over the bounds of the parent widget.
     * Set this flag if the parent widget is relatively small horizontally,
     * but there is space available.
     * Default is false.
     */
    void setAllowExceedBounds(bool allow);

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

    /** Returns presumedly best action for typed text, without user input from any completion list.
     */
    static TaggingAction makeDefaultTaggingAction(const QString& text, int parentTagId);

public Q_SLOTS:

    /** Set a "parent tag" taken into account when suggesting a
     *  parent tag for a new tag, and a default action.
     */
    void setParentTag(const QModelIndex& index);
    void setParentTag(TAlbum* album);

Q_SIGNALS:

    /** Emitted when the completion text changes, see above.
     */
    void currentCompletionTextChanged(const QString& completionText);

    /** Emitted when the current tagging action changes.
     */
    void currentTaggingActionChanged(const TaggingAction& action);

    void completionActivated(const QString& completionText);

protected:

    // Reimplemented
    void sizeAndPosition();
    void sizeAndPosition(bool wasVisible);

    virtual QRect calculateGeometry() const;

protected Q_SLOTS:

    void slotItemActivated(QListWidgetItem* item);
    void slotCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADDTAGSCOMPLETIONBOX_H
