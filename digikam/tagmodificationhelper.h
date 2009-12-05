/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : helper class used to modify tag albums in views
 *
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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
#include <qobject.h>
#include <qstring.h>

// Local includes
#include "album.h"

namespace Digikam
{

class TagModificationHelperPriv;

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
    TagModificationHelper(QObject *parent, QWidget *dialogParent);

    /**
     * Destructor.
     */
    virtual ~TagModificationHelper();

public Q_SLOTS:

    /**
     * Creates one ore more new tags under the given parent. If only the parent
     * is given, then a dialog is shown to create new tags. Else, if also a
     * title and optionally an icon are given, then these values will be
     * suggested in the dialog.
     *
     * @param parent parent tag album under which to create the new tags. May be
     *               0 to use the root album
     * @param title if this isn't an empty string, then this tag name is
     *              suggested
     * @param iconName an optional name for the icon to suggest for the new tag
     */
    void slotTagNew(TAlbum *parent, const QString &title = QString(),
                    const QString &iconName = QString());

    /**
     * Edits the given tag via a user dialog.
     *
     * @param tag the tag to change
     */
    void slotTagEdit(TAlbum *tag);

    /**
     * Deletes the given tag and after prompting the user for this.
     *
     * @param tag the tag to delete, must not be the root tag album
     */
    void slotTagDelete(TAlbum *tag);

    /**
     * Assigns the given tag to the list of images in the background using the
     * progress signals to indicate the progress.
     *
     * @param tagID id of the tag to assign
     * @param imageIDs list of images that the tag will be assigned to
     */
    // TODO why don't we use real domain objects here instead of ids?
    void slotAssignTags(int tagId, const QList<int>& imageIDs);

Q_SIGNALS:

    // TODO create an interface class for this two methods with a proper
    // encapsulation

    /**
     * Signal indicating that a new background tag assigning process has started
     * that should be displayed in a progress bar.
     *
     * @param progressBarMode mode of the progress bar
     * @param title for the progress bar
     */
    // TODO this encapsulation is wrong. The helper object should not know
    // anything about a progress bar
    void signalProgressBarMode(int progressBarMode, const QString &progressTitle);

    /**
     * Progress indication for the background process.
     *
     * @param progressValue percent of background process
     *                      0 <= progressValue <= 100
     */
    void signalProgressValue(int progressValue);

private:
    TagModificationHelperPriv *d;

};

}

#endif /* TAGMODIFICATIONHELPER_H */
