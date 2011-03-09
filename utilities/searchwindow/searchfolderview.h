/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SEARCHFOLDERVIEW_H
#define SEARCHFOLDERVIEW_H

// Local includes

#include "editablesearchtreeview.h"

namespace Digikam
{

class NormalSearchTreeViewPriv;

/**
 * Tree view for all saved "normal" searches. Allows editing and creating
 * searches in the context menu.
 *
 * @author jwienke
 */
class NormalSearchTreeView: public EditableSearchTreeView
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
    NormalSearchTreeView(QWidget* parent, SearchModel* searchModel,
                         SearchModificationHelper* searchModificationHelper);

    /**
     * Destructor.
     */
    virtual ~NormalSearchTreeView();

Q_SIGNALS:

    /**
     * Emitted of a new search shall be created.
     */
    void newSearch();

    /**
     * Emitted if the given search shall be edited.
     *
     * @param album search to edit
     */
    void editSearch(SAlbum* album);

protected:

    virtual void addCustomContextMenuActions(ContextMenuHelper& cmh,
                                             Album* album);
    virtual void handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album);

private:
    NormalSearchTreeViewPriv* d;

};

}

#endif /* SEARCHFOLDERVIEW_H */
