/*
 * TagFilterSideBarWidget.cpp
 *
 *  Created on: 26.11.2009
 *      Author: languitar
 */

#include "tagfiltersidebarwidget.moc"

// Qt includes
#include <qlayout.h>

// Local includes
#include "albummodel.h"
#include "tagfilterview.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

class TagFilterSideBarWidgetPriv
{
public:

    TagFilterSideBarWidgetPriv() :
        tagFilterView(0),
        tagFilterSearchBar(0)
    {
    }

    TagFilterViewNew *tagFilterView;
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

    d->tagFilterView      = new TagFilterViewNew(this, tagFilterModel, tagModificationHelper);
    d->tagFilterSearchBar = new SearchTextBar(this, "DigikamViewTagFilterSearchBar");
    d->tagFilterSearchBar->setModel(tagFilterModel, AbstractAlbumModel::AlbumIdRole);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(d->tagFilterView);
    layout->addWidget(d->tagFilterSearchBar);

    // connection
    connect(d->tagFilterSearchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->tagFilterView, SLOT(setSearchTextSettings(const SearchTextSettings&)));

    connect(d->tagFilterView,
            SIGNAL(tagFilterChanged(const QList<int>&, ImageFilterSettings::MatchingCondition, bool)),
            this,
            SIGNAL(tagFilterChanged(const QList<int>&, ImageFilterSettings::MatchingCondition, bool)));

}

TagFilterSideBarWidget::~TagFilterSideBarWidget()
{
}

void TagFilterSideBarWidget::slotResetTagFilters()
{
    d->tagFilterView->slotResetTagFilters();
}

}
