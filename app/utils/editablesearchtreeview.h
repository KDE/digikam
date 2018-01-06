/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-14
 * Description : Basic search tree view with editing functionality
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef EDITABLESEARCHTREEVIEW_H
#define EDITABLESEARCHTREEVIEW_H

// Local includes

#include "albumtreeview.h"
#include "searchmodificationhelper.h"

namespace Digikam
{

/**
 * This tree view for searches adds basic editing functionality via the context
 * menu. This is in detail deleting and renaming existing searches.
 *
 * @author jwienke
 */
class EditableSearchTreeView: public SearchTreeView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent qt parent
     * @param searchModel the model this view should act on
     * @param searchModificationHelper the modification helper object used to
     *                                 perform operations on the displayed
     *                                 searches
     */
    EditableSearchTreeView(QWidget* const parent, SearchModel* const searchModel,
                           SearchModificationHelper* const searchModificationHelper);

    /**
     * Destructor.
     */
    virtual ~EditableSearchTreeView();

protected:

    // implemented hook methods for context menus.
    virtual QString contextMenuTitle() const;

    /**
     * Adds actions to delete or rename existing searches.
     */
    virtual void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album);

    /**
     * Handles deletion and renaming actions.
     */
    virtual void handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* EDITABLESEARCHTREEVIEW_H */
