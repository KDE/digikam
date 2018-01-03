/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify tag albums in views
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TAGMODIFICATIONHELPER_H
#define TAGMODIFICATIONHELPER_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "album.h"

class QAction;

namespace Digikam
{

/**
 * Utility class providing methods to modify tag albums (TAlbum) in a way
 * useful to implement views.
 *
 * This class can do background processing for batch tag operations. So be sure
 * that the signals indicating the progress of these operations are used.
 *
 * @author jwienke
 */
class TagModificationHelper: public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for qt parent child mechanism
     * @param dialogParent paret widget for dialogs displayed by this object
     */
    TagModificationHelper(QObject* const parent, QWidget* const dialogParent);

    /**
     * Destructor.
     */
    virtual ~TagModificationHelper();

public Q_SLOTS:

    /**
     * Creates one ore more new tags under the given parent. If only the parent
     * is given, then a dialog is shown to create new tags. Else, if also a
     * title and optionally an icon are given, then these values will be used
     * directly to create the tag.
     *
     * @param parent parent tag album under which to create the new tags. May be
     *               0 to use the root album
     * @param title if this isn't an empty string, then this tag name is
     *              suggested
     * @param iconName an optional name for the icon to suggest for the new tag
     * @return new tag album or 0 if not created
     */
    TAlbum* slotTagNew(TAlbum* parent, const QString& title = QString(),
                       const QString& iconName = QString());

    /**
     * Same as above, but this slot can be triggered from a QAction
     * if a parent tag is bound to this action, see below.
     * Without this mechanism, will add a toplevel tag.
     *
     * @return new tag created or 0 if no tag was created
     */
    TAlbum* slotTagNew();

    /**
     * Edits the given tag via a user dialog.
     *
     * @param tag the tag to change
     */
    void slotTagEdit(TAlbum* tag);
    void slotTagEdit(); /// must use bindTag and a QAction

    /**
     * Deletes the given tag and after prompting the user for this.
     *
     * @param tag the tag to delete, must not be the root tag album
     */
    void slotTagDelete(TAlbum* tag);

    /**
     * must use bindTag and a QAction
     */
    void slotTagDelete(); /// must use bindTag and a QAction

    /**
     * Delete multiple tags and prompt user only once for all
     *
     * @param tags tags to be deleted, without root tag
     */
    void slotMultipleTagDel(QList<TAlbum* >& tags);

    /**
     * must use bindMultipleTags and a QAction
     */
    void slotMultipleTagDel();

    /**
     * Deletes the given face tag and after prompting the user for this.
     * The tag itself is not deleted. Only its property as face tag.
     *
     * @param tag the face tag to delete
     */
    void slotFaceTagDelete(TAlbum* tag);

    /**
     * must use bindTag and a QAction
     */
    void slotFaceTagDelete(); /// must use bindTag and a QAction

    /**
     * Delete multiple face tags and prompt user only once for all
     * The tags itself are not deleted. Only their properties as face tags.
     *
     * @param tags face tags to be deleted.
     */
    void slotMultipleFaceTagDel(QList<TAlbum* >& tags);

    /**
     * must use bindMultipleTags and a QAction
     */
    void slotMultipleFaceTagDel();
    
    /**
     * Marks the tag as face tag if it is not already.
     *
     * @param tag the tag to mark
     */
    void slotTagToFaceTag(TAlbum* tag);

    /**
     * must use bindTag and a QAction
     */
    void slotTagToFaceTag(); /// must use bindTag and a QAction

    /**
     * Marks the tags as face tags if they are not already.
     *
     * @param tags the tags to mark.
     */
    void slotMultipleTagsToFaceTags(QList<TAlbum* >& tags);

    /**
     * must use bindMultipleTags and a QAction
     */
    void slotMultipleTagsToFaceTags();

    /**
     * Sets the tag that the given action operates on.
     * You must call bindTag and then connect the action's triggered
     * to the desired slot, slotTagNew(), slotTagEdit() or slotTagDelete().
     * Note: Changes the Action's user data.
     */
    void bindTag(QAction* action, TAlbum* parent) const;

    /**
     * Returns the tag bound with bindTag. The given QObject shall be
     * a QAction, but for convenience the given object
     * will be checked with qobject_cast first, so you can pass QObject::sender().
     */
    TAlbum* boundTag(QObject* action) const;

    /**
     * Set QVector's pointer into action's data. Make sure that QVector is not
     * a local object and it's not destroyed before boundMultipleTags are called
     *
     * @param action    - action to store pointer
     * @param tags      - QVector pointer to be stored
     */
    void bindMultipleTags(QAction* action, QList<TAlbum*> tags);

    /**
     * Return QVector pointer bound with bindMultipleTags. Use when context menu
     * should delete more than one item: multiple-selection.
     */
    QList< TAlbum* > boundMultipleTags(QObject* sender);

Q_SIGNALS:

    void tagCreated(TAlbum* tag);
    void tagEdited(TAlbum* tag);
    void aboutToDeleteTag(TAlbum* tag);

private:

    class Private;
    Private* const d;

    /**
     * Returns all sub-tags of the given one which have the person property.
     * This includes the root tag, if it has the property, too.
     */
    QList<TAlbum*> getFaceTags(TAlbum* rootTags = 0);

    /**
     * Returns all sub-tags of the given ones which have the person property.
     * This includes the root tags, if they have the property, too.
     */
    QSet<TAlbum*> getFaceTags(QList<TAlbum*> tags);

};

} // namespace Digikam

#endif /* TAGMODIFICATIONHELPER_H */
