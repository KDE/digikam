/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-24
 * Description : Tags Action Manager
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TAGSACTIONMNGR_H
#define TAGSACTIONMNGR_H

// Qt includes

#include <QMap>
#include <QObject>
#include <QWidget>

// KDE includes

#include <kactioncollection.h>

// Local includes

#include "tagproperties.h"
#include "albuminfo.h"

class KActionCollection;

namespace Digikam
{

class Album;

class TagsActionMngr : public QObject
{
    Q_OBJECT

public:

    TagsActionMngr(QWidget* parent);
    ~TagsActionMngr();

    /** Register all action collections managed with keyboard shorcuts.
     *  Must be called after all root window instances created.
     */
    void registerActionCollections();

    /** Return the list of whole action collections managed.
      */
    QList<KActionCollection*> actionCollections() const;

    /**
      * Updates the shortcut action for a tag. Call this when a shortcut was
      * added, removed or changed.
      */
    void updateTagShortcut(int tagId, const QKeySequence& ks);

    QString ratingShortcutPrefix() const;
    QString tagShortcutPrefix()    const;
    QString pickShortcutPrefix()   const;
    QString colorShortcutPrefix()  const;

    static TagsActionMngr* defaultManager();

private Q_SLOTS:

    /**
      * Removes the shortcut actions associated with a tag.
      */
    void slotAlbumDeleted(Album*);

    /**
      * Wrapper around windows to run relevant code about keyboard shorcuts in GUI.
      */
    void slotAssignFromShortcut();

    /**
      * Called by KDE config shortcuts dialog, when user change action properties.
      */
    void slotTagActionChanged();

private:

    void tagRemoved(int tagId);
    bool createTagActionShortcut(int tagId);
    void createTagActionShortcut(const TagInfo& tinfo, const TagProperties& tprop);

    /** Create all actions relevant of whole tags from DB which have a keyboard shortcut.
      * This method is called by registerActionCollection() method.
      */
    void createActions();

    bool createRatingActionShortcut(KActionCollection* ac, int rating);
    bool createPickLabelActionShortcut(KActionCollection* ac, int pickId);
    bool createColorLabelActionShortcut(KActionCollection* ac, int colorId);

private:

    static TagsActionMngr* m_defaultManager;

    class TagsActionMngrPrivate;
    TagsActionMngrPrivate* const d;
};

} // namespace Digikam

#endif // TAGSACTIONMNGR_H
