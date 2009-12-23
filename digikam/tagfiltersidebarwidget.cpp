/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : tag filter view for the right sidebar
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

#include "tagfiltersidebarwidget.moc"

// Qt includes
#include <qcheckbox.h>
#include <qlayout.h>

// KDE includes
#include <kselectaction.h>

// Local includes
#include "albummodel.h"
#include "contextmenuhelper.h"
#include "tagcheckview.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

class TagFilterViewPriv
{
public:

    TagFilterViewPriv() :
        matchingCond(ImageFilterSettings::OrCondition),
        restoreTagFilters(TagFilterView::OffRestoreTagFilters)
    {
    }

    ImageFilterSettings::MatchingCondition matchingCond;
    // TODO update, implement this
    TagFilterView::RestoreTagFilters    restoreTagFilters;

    TagModel *tagFilterModel;
    TagModificationHelper *tagModificationHelper;

    KSelectAction *matchingCondAction;
    QAction *orBetweenAction;
    QAction *andBetweenAction;
    KSelectAction *restoreTagFiltersAction;
    QAction *onRestoreTagFiltersAction;
    QAction *offRestoreTagFiltersAction;

};

TagFilterView::TagFilterView(QWidget *parent, TagModel *tagFilterModel,
                TagModificationHelper *tagModificationHelper) :
    TagCheckView(parent, tagFilterModel, tagModificationHelper),
    d(new TagFilterViewPriv)
{

    d->tagFilterModel = tagFilterModel;
    d->tagModificationHelper = tagModificationHelper;

    d->matchingCondAction = new KSelectAction(i18n("Matching Condition"), this);
    d->orBetweenAction = d->matchingCondAction->addAction(i18n("Or Between Tags"));
    d->andBetweenAction = d->matchingCondAction->addAction(i18n("And Between Tags"));

    d->restoreTagFiltersAction = new KSelectAction(i18n("Restore Tag Filters"), this);
    d->onRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("On"));
    d->offRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("Off"));

}

TagFilterView::~TagFilterView()
{
    delete d;
}

void TagFilterView::doLoadState()
{
    TagCheckView::doLoadState();

    // TODO update
//    KSharedConfig::Ptr config = KGlobal::config();
//    KConfigGroup group = config->group(objectName());
//    d->matchingCond = (ImageFilterSettings::MatchingCondition)
//                      (group.readEntry("Matching Condition", (int)ImageFilterSettings::OrCondition));
//
//    d->restoreTagFilters = (RestoreTagFilters)
//                           (group.readEntry("Restore Tag Filters", (int)OffRestoreTagFilters));
}

void TagFilterView::doSaveState()
{
    TagCheckView::doSaveState();

    // TODO update
//    KSharedConfig::Ptr config = KGlobal::config();
//    KConfigGroup group        = config->group(objectName());
//    group.writeEntry("Matching Condition",  (int)(d->matchingCond));
//    group.writeEntry("Restore Tag Filters", (int)(d->restoreTagFilters));
//    saveTagFilters();
//    config->sync();

}

ImageFilterSettings::MatchingCondition TagFilterView::getMatchingCondition() const
{
    return d->matchingCond;
}

void TagFilterView::addCustomContextMenuActions(ContextMenuHelper &cmh, Album *album)
{
    TagCheckView::addCustomContextMenuActions(cmh, album);

    // matching condition

    cmh.addAction(d->matchingCondAction);

    d->orBetweenAction->setChecked(d->matchingCond == ImageFilterSettings::OrCondition);
    d->andBetweenAction->setChecked(d->matchingCond != ImageFilterSettings::OrCondition);

    // restoring

    cmh.addAction(d->restoreTagFiltersAction);

    d->onRestoreTagFiltersAction->setChecked(d->restoreTagFilters == OnRestoreTagFilters);
    d->offRestoreTagFiltersAction->setChecked(d->restoreTagFilters != OnRestoreTagFilters);

}

void TagFilterView::handleCustomContextMenuAction(QAction *action, Album *album)
{
    TagCheckView::handleCustomContextMenuAction(action, album);

    if (!action)
    {
        return;
    }

    if (action == d->orBetweenAction)         // Or Between Tags.
    {
        d->matchingCond = ImageFilterSettings::OrCondition;
        emit matchingConditionChanged(d->matchingCond);
    }
    else if (action == d->andBetweenAction)        // And Between Tags.
    {
        d->matchingCond = ImageFilterSettings::AndCondition;
        emit matchingConditionChanged(d->matchingCond);
    }
    else if (action == d->onRestoreTagFiltersAction)        // Restore TagFilters ON.
    {
        d->restoreTagFilters = OnRestoreTagFilters;
    }
    else if (action == d->offRestoreTagFiltersAction)        // Restore TagFilters OFF.
    {
        d->restoreTagFilters = OffRestoreTagFilters;
    }

}

// -----------------------------------------------------------------------------

class TagFilterSideBarWidgetPriv
{
public:

    TagFilterSideBarWidgetPriv() :
        tagFilterView(0),
        tagFilterSearchBar(0)
    {
    }

    TagFilterView    *tagFilterView;
    SearchTextBar    *tagFilterSearchBar;

    TagModel *tagFilterModel;
    TagModificationHelper *tagModificationHelper;

    QCheckBox *withoutTagCheckBox;

};

TagFilterSideBarWidget::TagFilterSideBarWidget(QWidget *parent,
                TagModel *tagFilterModel,
                TagModificationHelper *tagModificationHelper) :
    QWidget(parent), StateSavingObject(this), d(new TagFilterSideBarWidgetPriv)
{

    setObjectName("TagFilter Sidebar");

    d->tagFilterModel = tagFilterModel;
    d->tagModificationHelper = tagModificationHelper;

    d->tagFilterView      = new TagFilterView(this, tagFilterModel, tagModificationHelper);
    d->tagFilterSearchBar = new SearchTextBar(this, "DigikamViewTagFilterSearchBar");
    d->tagFilterSearchBar->setModel(tagFilterModel, AbstractAlbumModel::AlbumIdRole);

    d->withoutTagCheckBox = new QCheckBox(i18n("Not Tagged"), this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(d->tagFilterView);
    layout->addWidget(d->withoutTagCheckBox);
    layout->addWidget(d->tagFilterSearchBar);

    // connection
    connect(d->tagFilterSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->tagFilterView, SLOT(setSearchTextSettings(const SearchTextSettings&)));

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(const QList<TAlbum*>&)),
            this, SLOT(slotCheckedTagsChanged(const QList<TAlbum*>&)));

    connect(d->tagFilterView, SIGNAL(matchingConditionChanged(const ImageFilterSettings::MatchingCondition&)),
            this, SLOT(slotMatchingConditionChanged(const ImageFilterSettings::MatchingCondition&)));

    connect(d->withoutTagCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWithoutTagChanged(int)));

}

TagFilterSideBarWidget::~TagFilterSideBarWidget()
{
}

void TagFilterSideBarWidget::slotResetTagFilters()
{
    d->tagFilterView->slotResetCheckState();
}

void TagFilterSideBarWidget::slotMatchingConditionChanged(const ImageFilterSettings::MatchingCondition &condition)
{
    Q_UNUSED(condition);
    filterChanged();
}

void TagFilterSideBarWidget::slotCheckedTagsChanged(const QList<TAlbum*> &tags)
{
    Q_UNUSED(tags);
    filterChanged();
}

void TagFilterSideBarWidget::slotWithoutTagChanged(int newState)
{

    bool showUntagged = newState == Qt::Checked;
    d->tagFilterView->setEnabled(!showUntagged);
    d->tagFilterSearchBar->setEnabled(!showUntagged);

    filterChanged();
}

void TagFilterSideBarWidget::filterChanged()
{

    bool showUntagged = d->withoutTagCheckBox->checkState() == Qt::Checked;

    QList<int> tagIds;
    if (!showUntagged)
    {
        foreach(TAlbum *tag, d->tagFilterView->getCheckedTags())
        {
            if (tag)
            {
                tagIds << tag->id();
            }
        }
    }

    emit tagFilterChanged(tagIds, d->tagFilterView->getMatchingCondition(), showUntagged);

}

void TagFilterSideBarWidget::setConfigGroup(KConfigGroup group)
{
    StateSavingObject::setConfigGroup(group);
    d->tagFilterView->setConfigGroup(group);
}

void TagFilterSideBarWidget::doLoadState()
{
    d->tagFilterView->loadState();
}

void TagFilterSideBarWidget::doSaveState()
{
    d->tagFilterView->saveState();
}

}
