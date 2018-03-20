/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : filters view for the right sidebar
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2011 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C)      2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "filtersidebarwidget.h"

// Qt includes

#include <QLayout>
#include <QCheckBox>
#include <QGridLayout>
#include <QToolButton>
#include <QMenu>
#include <QIcon>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "digikam_debug.h"
#include "applicationsettings.h"
#include "colorlabelfilter.h"
#include "geolocationfilter.h"
#include "picklabelfilter.h"
#include "ratingfilter.h"
#include "mimefilter.h"
#include "tagfilterview.h"

namespace Digikam
{

class FilterSideBarWidget::Private
{
public:

    Private() :
        space(0),
        expanderVlay(0),
        tagFilterView(0),
        tagFilterSearchBar(0),
        tagOptionsBtn(0),
        tagOptionsMenu(0),
        tagFilterModel(0),
        tagOrCondAction(0),
        tagAndCondAction(0),
        tagMatchCond(ImageFilterSettings::OrCondition),
        colorLabelFilter(0),
        geolocationFilter(0),
        pickLabelFilter(0),
        ratingFilter(0),
        mimeFilter(0),
        textFilter(0),
        withoutTagCheckBox(0),
        expbox(0)
    {
    }

    static const QString                   configSearchTextFilterFieldsEntry;
    static const QString                   configLastShowUntaggedEntry;
    static const QString                   configMatchingConditionEntry;

    QWidget*                               space;
    QVBoxLayout*                           expanderVlay;

    TagFilterView*                         tagFilterView;
    SearchTextBar*                         tagFilterSearchBar;
    QToolButton*                           tagOptionsBtn;
    QMenu*                                 tagOptionsMenu;
    TagModel*                              tagFilterModel;
    QAction*                               tagOrCondAction;
    QAction*                               tagAndCondAction;
    ImageFilterSettings::MatchingCondition tagMatchCond;

    ColorLabelFilter*                      colorLabelFilter;
    GeolocationFilter*                     geolocationFilter;
    PickLabelFilter*                       pickLabelFilter;
    RatingFilter*                          ratingFilter;
    MimeFilter*                            mimeFilter;
    TextFilter*                            textFilter;

    QCheckBox*                             withoutTagCheckBox;

    DExpanderBox*                          expbox;
};

const QString FilterSideBarWidget::Private::configSearchTextFilterFieldsEntry(QLatin1String("Search Text Filter Fields"));
const QString FilterSideBarWidget::Private::configLastShowUntaggedEntry(QLatin1String("Show Untagged"));
const QString FilterSideBarWidget::Private::configMatchingConditionEntry(QLatin1String("Matching Condition"));

// ---------------------------------------------------------------------------------------------------

FilterSideBarWidget::FilterSideBarWidget(QWidget* const parent, TagModel* const tagFilterModel)
    : DVBox(parent), StateSavingObject(this), d(new Private)
{
    setObjectName(QLatin1String("TagFilter Sidebar"));

    d->expbox = new DExpanderBox(this);
    d->expbox->setObjectName(QLatin1String("FilterSideBarWidget Expander"));

    // --------------------------------------------------------------------------------------------------------

    d->textFilter = new TextFilter(d->expbox);
    d->expbox->addItem(d->textFilter, QIcon::fromTheme(QLatin1String("text-field")),
                       i18n("Text Filter"), QLatin1String("TextFilter"), true);

    // --------------------------------------------------------------------------------------------------------

    d->mimeFilter = new MimeFilter(d->expbox);
    d->expbox->addItem(d->mimeFilter, QIcon::fromTheme(QLatin1String("folder-open")),
                       i18n("MIME Type Filter"), QLatin1String("TypeMimeFilter"), true);

    // --------------------------------------------------------------------------------------------------------

    d->geolocationFilter = new GeolocationFilter(d->expbox);
    d->expbox->addItem(d->geolocationFilter, QIcon::fromTheme(QLatin1String("globe")),
                       i18n("Geolocation Filter"), QLatin1String("TypeGeolocationFilter"), true);

    // --------------------------------------------------------------------------------------------------------

    QWidget* const box3   = new QWidget(d->expbox);
    d->tagFilterModel     = tagFilterModel;
    d->tagFilterView      = new TagFilterView(box3, tagFilterModel);
    d->tagFilterView->setObjectName(QLatin1String("DigikamViewTagFilterView"));
    d->tagFilterSearchBar = new SearchTextBar(box3, QLatin1String("DigikamViewTagFilterSearchBar"));
    d->tagFilterSearchBar->setModel(d->tagFilterView->filteredModel(),
                                    AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagFilterSearchBar->setFilterModel(d->tagFilterView->albumFilterModel());

    const QString notTaggedTitle   = i18n("Images Without Tags");
    d->withoutTagCheckBox          = new QCheckBox(notTaggedTitle, box3);
    d->withoutTagCheckBox->setWhatsThis(i18n("Show images without a tag."));

    d->tagOptionsBtn = new QToolButton(box3);
    d->tagOptionsBtn->setToolTip( i18n("Tags Matching Condition"));
    d->tagOptionsBtn->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    d->tagOptionsBtn->setPopupMode(QToolButton::InstantPopup);
    d->tagOptionsBtn->setWhatsThis(i18n("Defines in which way the selected tags are combined "
                                        "to filter the images. This also includes the '%1' check box.",
                                        notTaggedTitle));

    d->tagOptionsMenu  = new QMenu(d->tagOptionsBtn);
    d->tagOrCondAction = d->tagOptionsMenu->addAction(i18n("OR"));
    d->tagOrCondAction->setCheckable(true);
    d->tagAndCondAction = d->tagOptionsMenu->addAction(i18n("AND"));
    d->tagAndCondAction->setCheckable(true);
    d->tagOptionsBtn->setMenu(d->tagOptionsMenu);

    QGridLayout* const lay3 = new QGridLayout(box3);
    lay3->addWidget(d->tagFilterView,      0, 0, 1, 3);
    lay3->addWidget(d->tagFilterSearchBar, 1, 0, 1, 3);
    lay3->addWidget(d->withoutTagCheckBox, 2, 0, 1, 1);
    lay3->addWidget(d->tagOptionsBtn,      2, 2, 1, 1);
    lay3->setRowStretch(0, 100);
    lay3->setColumnStretch(1, 10);
    lay3->setContentsMargins(QMargins());
    lay3->setSpacing(0);

    d->expbox->addItem(box3, QIcon::fromTheme(QLatin1String("tag-assigned")), i18n("Tags Filter"), QLatin1String("TagsFilter"), true);

    // --------------------------------------------------------------------------------------------------------

    QWidget* const box4 = new QWidget(d->expbox);
    d->colorLabelFilter = new ColorLabelFilter(box4);
    d->pickLabelFilter  = new PickLabelFilter(box4);
    d->ratingFilter     = new RatingFilter(box4);

    QGridLayout* const lay4 = new QGridLayout(box4);
    lay4->addWidget(d->colorLabelFilter, 0, 0, 1, 3);
    lay4->addWidget(d->pickLabelFilter,  1, 0, 1, 1);
    lay4->addWidget(d->ratingFilter,     1, 2, 1, 1);
    lay4->setColumnStretch(2, 1);
    lay4->setColumnStretch(3, 10);
    lay4->setContentsMargins(QMargins());
    lay4->setSpacing(0);

    d->expbox->addItem(box4, QIcon::fromTheme(QLatin1String("folder-favorites")), i18n("Labels Filter"), QLatin1String("LabelsFilter"), true);

    d->expanderVlay = dynamic_cast<QVBoxLayout*>(dynamic_cast<QScrollArea*>(d->expbox)->widget()->layout());
    d->space        = new QWidget();
    d->expanderVlay->addWidget(d->space);

    // --------------------------------------------------------------------------------------------------------

    connect(d->expbox, SIGNAL(signalItemExpanded(int,bool)),
            this, SLOT(slotItemExpanded(int,bool)));

    connect(d->mimeFilter, SIGNAL(activated(int)),
            this, SIGNAL(signalMimeTypeFilterChanged(int)));

    connect(d->geolocationFilter, SIGNAL(signalFilterChanged(ImageFilterSettings::GeolocationCondition)),
            this, SIGNAL(signalGeolocationFilterChanged(ImageFilterSettings::GeolocationCondition)));

    connect(d->textFilter, SIGNAL(signalSearchTextFilterSettings(SearchTextFilterSettings)),
            this, SIGNAL(signalSearchTextFilterChanged(SearchTextFilterSettings)));

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(QList<TAlbum*>,QList<TAlbum*>)),
            this, SLOT(slotCheckedTagsChanged(QList<TAlbum*>,QList<TAlbum*>)));

    connect(d->colorLabelFilter, SIGNAL(signalColorLabelSelectionChanged(QList<ColorLabel>)),
            this, SLOT(slotColorLabelFilterChanged(QList<ColorLabel>)));

    connect(d->pickLabelFilter, SIGNAL(signalPickLabelSelectionChanged(QList<PickLabel>)),
            this, SLOT(slotPickLabelFilterChanged(QList<PickLabel>)));

    connect(d->withoutTagCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWithoutTagChanged(int)));

    connect(d->tagOptionsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotTagOptionsTriggered(QAction*)));

    connect(d->tagOptionsMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotTagOptionsMenu()));

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int,ImageFilterSettings::RatingCondition,bool)),
            this, SIGNAL(signalRatingFilterChanged(int,ImageFilterSettings::RatingCondition,bool)));
}

FilterSideBarWidget::~FilterSideBarWidget()
{
    delete d;
}

void FilterSideBarWidget::slotTagOptionsMenu()
{
    d->tagOrCondAction->setChecked(false);
    d->tagAndCondAction->setChecked(false);

    switch (d->tagMatchCond)
    {
        case ImageFilterSettings::OrCondition:
            d->tagOrCondAction->setChecked(true);
            break;
        case ImageFilterSettings::AndCondition:
            d->tagAndCondAction->setChecked(true);
            break;
    }
}

void FilterSideBarWidget::slotItemExpanded(int id, bool b)
{
    if (id == Digikam::TAGS)
    {
        d->expanderVlay->setStretchFactor(d->space, b ? 0 : 100);
    }
}

void FilterSideBarWidget::setFocusToTextFilter()
{
    d->textFilter->searchTextBar()->setFocus();
}

void FilterSideBarWidget::slotFilterMatchesForText(bool match)
{
    d->textFilter->searchTextBar()->slotSearchResult(match);
}

void FilterSideBarWidget::slotResetFilters()
{
    d->textFilter->reset();
    d->mimeFilter->setMimeFilter(MimeFilter::AllFiles);
    d->geolocationFilter->setGeolocationFilter(ImageFilterSettings::GeolocationNoFilter);
    d->tagFilterView->slotResetCheckState();
    d->withoutTagCheckBox->setChecked(false);
    d->colorLabelFilter->reset();
    d->pickLabelFilter->reset();
    d->ratingFilter->setRating(0);
    d->ratingFilter->setRatingFilterCondition(ImageFilterSettings::GreaterEqualCondition);
    d->ratingFilter->setExcludeUnratedItems(false);
    d->tagMatchCond = ImageFilterSettings::OrCondition;
}

void FilterSideBarWidget::slotTagOptionsTriggered(QAction* action)
{
    if (action)
    {
        if (action == d->tagOrCondAction)
        {
            d->tagMatchCond = ImageFilterSettings::OrCondition;
        }
        else if (action == d->tagAndCondAction)
        {
            d->tagMatchCond = ImageFilterSettings::AndCondition;
        }
    }

    filterChanged();
}

void FilterSideBarWidget::slotCheckedTagsChanged(const QList<TAlbum*>& includedTags,
                                                 const QList<TAlbum*>& excludedTags)
{
    Q_UNUSED(includedTags);
    Q_UNUSED(excludedTags);
    filterChanged();
}

void FilterSideBarWidget::slotColorLabelFilterChanged(const QList<ColorLabel>& list)
{
    Q_UNUSED(list);
    filterChanged();
}

void FilterSideBarWidget::slotPickLabelFilterChanged(const QList<PickLabel>& list)
{
    Q_UNUSED(list);
    filterChanged();
}

void FilterSideBarWidget::slotWithoutTagChanged(int newState)
{
    Q_UNUSED(newState);
    filterChanged();
}

void FilterSideBarWidget::filterChanged()
{
    bool showUntagged = d->withoutTagCheckBox->checkState() == Qt::Checked;

    QList<int> includedTagIds;
    QList<int> excludedTagIds;
    QList<int> clTagIds;
    QList<int> plTagIds;

    if (!showUntagged || d->tagMatchCond == ImageFilterSettings::OrCondition)
    {
        foreach(TAlbum* tag, d->tagFilterView->getCheckedTags())
        {
            if (tag)
            {
                includedTagIds << tag->id();
            }
        }
        foreach(TAlbum* tag, d->tagFilterView->getPartiallyCheckedTags())
        {
            if (tag)
            {
                excludedTagIds << tag->id();
            }
        }
        foreach(TAlbum* tag, d->colorLabelFilter->getCheckedColorLabelTags())
        {
            if (tag)
            {
                clTagIds << tag->id();
            }
        }
        foreach(TAlbum* tag, d->pickLabelFilter->getCheckedPickLabelTags())
        {
            if (tag)
            {
                plTagIds << tag->id();
            }
        }
    }

    emit signalTagFilterChanged(includedTagIds, excludedTagIds, d->tagMatchCond, showUntagged, clTagIds, plTagIds);
}

void FilterSideBarWidget::setConfigGroup(const KConfigGroup& group)
{
    StateSavingObject::setConfigGroup(group);
    d->tagFilterView->setConfigGroup(group);
}

void FilterSideBarWidget::doLoadState()
{
    /// @todo mime type and geolocation filter states are not loaded/saved

    KConfigGroup group = getConfigGroup();

    d->expbox->readSettings(group);

    d->textFilter->setsearchTextFields((SearchTextFilterSettings::TextFilterFields)
                                       (group.readEntry(entryName(d->configSearchTextFilterFieldsEntry),
                                                                   (int)SearchTextFilterSettings::All)));


    d->ratingFilter->setRatingFilterCondition((ImageFilterSettings::RatingCondition)
                                              (ApplicationSettings::instance()->getRatingFilterCond()));

    d->tagMatchCond = (ImageFilterSettings::MatchingCondition)
                      (group.readEntry(entryName(d->configMatchingConditionEntry),
                                                  (int)ImageFilterSettings::OrCondition));

    d->tagFilterView->loadState();

    if (d->tagFilterView->isRestoreCheckState())
    {
        d->withoutTagCheckBox->setChecked(group.readEntry(entryName(d->configLastShowUntaggedEntry), false));
    }

    filterChanged();
}

void FilterSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    d->expbox->writeSettings(group);

    group.writeEntry(entryName(d->configSearchTextFilterFieldsEntry), (int)d->textFilter->searchTextFields());

    ApplicationSettings::instance()->setRatingFilterCond(d->ratingFilter->ratingFilterCondition());

    group.writeEntry(entryName(d->configMatchingConditionEntry), (int)d->tagMatchCond);

    d->tagFilterView->saveState();
    group.writeEntry(entryName(d->configLastShowUntaggedEntry), d->withoutTagCheckBox->isChecked());
    group.sync();
}

} // namespace Digikam
