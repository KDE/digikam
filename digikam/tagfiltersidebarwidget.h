/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : tag filter view for the right sidebar
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TAGFILTERSIDEBARWIDGET_H
#define TAGFILTERSIDEBARWIDGET_H

// Qt includes

#include <QWidget>

// Local includes

#include "imagefiltersettings.h"
#include "statesavingobject.h"
#include "tagcheckview.h"
#include "globals.h"

namespace Digikam
{

class TagModel;

/**
 * A view to filter the currently displayed album by tags.
 *
 * @author jwienke
 */
class TagFilterView : public TagCheckView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for qt parent child mechanism
     * @param tagFilterModel tag model to work on
     */
    TagFilterView(QWidget* parent, TagModel* tagFilterModel);

    /**
     * Destructor.
     */
    virtual ~TagFilterView();

protected:

    virtual void addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album);
    virtual void handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album);

private:

    class TagFilterViewPriv;
    TagFilterViewPriv* const d;
};

// ----------------------------------------------------------------------------------------------------------

/**
 * Sidebar widget containing the tag filtering.
 *
 * @author jwienke
 */
class TagFilterSideBarWidget : public QWidget, public StateSavingObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent parent for qt parent child mechanism
     * @param tagFilterModel tag model to work on
     */
    TagFilterSideBarWidget(QWidget* parent, TagModel* tagFilterModel);

    /**
     * Destructor.
     */
    virtual ~TagFilterSideBarWidget();

    virtual void setConfigGroup(KConfigGroup group);
    virtual void doLoadState();
    virtual void doSaveState();

Q_SIGNALS:

    /**
     * Emitted if the selected filter has changed.
     *
     * @param includedTags a list of included tag ids
     * @param excludedTags a list of excluded tag ids
     * @param matchingCond condition to join the selected tags
     * @param showUnTagged if this is true, only photos without a tag shall be shown
     * @param clTagIds     a list of color label tag ids
     * @param plTagIds     a list of pick label tag ids
     */
    void signalTagFilterChanged(const QList<int>& includedTags, const QList<int>& excludedTags,
                                ImageFilterSettings::MatchingCondition matchingCond, bool showUnTagged,
                                const QList<int>& clTagIds, const QList<int>& plTagIds);

    void signalRatingFilterChanged(int, ImageFilterSettings::RatingCondition);


public Q_SLOTS:

    /**
     * Resets all selected filters.
     */
    void slotResetFilters();

private Q_SLOTS:

    void slotMatchingConditionChanged(int index);
    void slotCheckedTagsChanged(const QList<TAlbum*>& includedTags, const QList<TAlbum*>& excludedTags);
    void slotColorLabelFilterChanged(const QList<ColorLabel>&);
    void slotPickLabelFilterChanged(const QList<PickLabel>&);
    void slotWithoutTagChanged(int newState);

private:

    void filterChanged();

private:

    class TagFilterSideBarWidgetPriv;
    TagFilterSideBarWidgetPriv* const d;
};

} // nameSpace Digikam

#endif /* TAGFILTERSIDEBARWIDGET_H*/
