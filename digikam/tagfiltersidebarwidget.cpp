/*
 * TagFilterSideBarWidget.cpp
 *
 *  Created on: 26.11.2009
 *      Author: languitar
 */

#include "tagfiltersidebarwidget.moc"

// Qt includes
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

    // TODO update, implement this
    ImageFilterSettings::MatchingCondition matchingCond;
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

void TagFilterView::loadViewState(KConfigGroup &group, QString prefix)
{
    TagCheckView::loadViewState(group, prefix);

    // TODO update
//    KSharedConfig::Ptr config = KGlobal::config();
//    KConfigGroup group = config->group(objectName());
//    d->matchingCond = (ImageFilterSettings::MatchingCondition)
//                      (group.readEntry("Matching Condition", (int)ImageFilterSettings::OrCondition));
//
//    d->restoreTagFilters = (RestoreTagFilters)
//                           (group.readEntry("Restore Tag Filters", (int)OffRestoreTagFilters));
}

void TagFilterView::saveViewState(KConfigGroup &group, QString prefix)
{
    TagCheckView::saveViewState(group, prefix);

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

void TagFilterView::addCustomContextMenuActions(ContextMenuHelper &cmh, TAlbum *tag)
{
    TagCheckView::addCustomContextMenuActions(cmh, tag);

    // matching condition

    cmh.addAction(d->matchingCondAction);

    d->orBetweenAction->setChecked(d->matchingCond == ImageFilterSettings::OrCondition);
    d->andBetweenAction->setChecked(d->matchingCond != ImageFilterSettings::OrCondition);

    // restoring

    cmh.addAction(d->restoreTagFiltersAction);

    d->onRestoreTagFiltersAction->setChecked(d->restoreTagFilters == OnRestoreTagFilters);
    d->offRestoreTagFiltersAction->setChecked(d->restoreTagFilters != OnRestoreTagFilters);

}

void TagFilterView::handleCustomContextMenuAction(QAction *action, TAlbum *tag)
{
    TagCheckView::handleCustomContextMenuAction(action, tag);

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

};

TagFilterSideBarWidget::TagFilterSideBarWidget(QWidget *parent,
                TagModel *tagFilterModel,
                TagModificationHelper *tagModificationHelper) :
    QWidget(parent), d(new TagFilterSideBarWidgetPriv)
{

    d->tagFilterModel = tagFilterModel;
    d->tagModificationHelper = tagModificationHelper;

    d->tagFilterView      = new TagFilterView(this, tagFilterModel, tagModificationHelper);
    d->tagFilterSearchBar = new SearchTextBar(this, "DigikamViewTagFilterSearchBar");
    d->tagFilterSearchBar->setModel(tagFilterModel, AbstractAlbumModel::AlbumIdRole);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(d->tagFilterView);
    layout->addWidget(d->tagFilterSearchBar);

    // connection
    connect(d->tagFilterSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->tagFilterView, SLOT(setSearchTextSettings(const SearchTextSettings&)));

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(const QList<TAlbum*>&)),
            this, SLOT(slotCheckedTagsChanged(const QList<TAlbum*>&)));

    connect(d->tagFilterView, SIGNAL(matchingConditionChanged(const ImageFilterSettings::MatchingCondition&)),
            this, SLOT(slotMatchingConditionChanged(const ImageFilterSettings::MatchingCondition&)));

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

void TagFilterSideBarWidget::filterChanged()
{

    QList<int> tagIds;
    foreach(TAlbum *tag, d->tagFilterView->getCheckedTags())
    {
        if (tag)
        {
            tagIds << tag->id();
        }
    }

    // TODO enalbe without tags argument
    emit tagFilterChanged(tagIds, d->tagFilterView->getMatchingCondition(), false);

}

}
