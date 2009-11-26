/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-05
 * Description : tags filter view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef TAGFILTERVIEW_H
#define TAGFILTERVIEW_H

// Local includes
#include "imagefiltersettings.h"
#include "tagfolderview.h"

namespace Digikam
{

class TagFilterViewNewPriv;
class TagFilterViewNew : public TagFolderViewNew
{
    Q_OBJECT
public:

    enum ToggleAutoTags
    {
        NoToggleAuto = 0,
        Children,
        Parents,
        ChildrenAndParents
    };

    enum RestoreTagFilters
    {
        OffRestoreTagFilters = 0,
        OnRestoreTagFilters
    };

    TagFilterViewNew(QWidget *parent, TagModel *tagModel,
                     TagModificationHelper *tagModificationHelper);
    virtual ~TagFilterViewNew();

    virtual void loadViewState(KConfigGroup &group, QString prefix = QString());
    virtual void saveViewState(KConfigGroup &group, QString prefix = QString());

Q_SIGNALS:

    /**
     * Emitted if the selected filter has changed.
     *
     * @param tags a list of selected tag ids
     * @param matchingcondition condition to join the seleted tags
     * @param showUnTagged if this is true, only photos without a tag shall be
     *                     shown
     */
    void tagFilterChanged(const QList<int>& tags,
                          ImageFilterSettings::MatchingCondition matchingCond,
                          bool showUnTagged);

public Q_SLOTS:

    /**
     * Resets the whole tag filter.
     */
    void slotResetTagFilters();

private Q_SLOTS:

    /**
     * Called if the check state of a single item changes. Wraps this to an
     * event that is more useful for filtering tags.
     */
    void slotCheckStateChange(Album *album, Qt::CheckState state);

private:
    virtual void addCustomContextMenuActions(ContextMenuHelper &cmh, TAlbum *tag);
    virtual void handleCustomContextMenuAction(QAction *action, TAlbum *tag);

    /**
     * Notifies that the filter changed.
     */
    void filterChanged();

private:
    TagFilterViewNewPriv *d;

};

}

#endif /* TAGFILTERVIEW_H */
