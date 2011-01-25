/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify tag albums in views
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    TagModificationHelper(QObject* parent, QWidget* dialogParent);

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
     * Same as above, but this slot is using the parent TAlbum set previously
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
    void slotTagEdit();

    /**
     * Deletes the given tag and after prompting the user for this.
     *
     * @param tag the tag to delete, must not be the root tag album
     */
    void slotTagDelete(TAlbum* tag);
    void slotTagDelete();

    /**
     * Sets the parent tag. This will be used by the variants which do not
     * take a TAlbum* argument.
     * You may find this useful if you want to connect a signal to this object.
     */
    void setParentTag(TAlbum* parent);

Q_SIGNALS:

    void tagCreated(TAlbum* tag);
    void tagEdited(TAlbum* tag);
    void aboutToDeleteTag(TAlbum* tag);

private:

    class TagModificationHelperPriv;
    TagModificationHelperPriv* const d;
};

} // namespace Digikam

#endif /* TAGMODIFICATIONHELPER_H */
