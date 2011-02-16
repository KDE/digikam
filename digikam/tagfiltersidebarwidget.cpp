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

#include "tagfiltersidebarwidget.moc"

// Qt includes

#include <QLabel>
#include <QLayout>
#include <QCheckBox>

// KDE includes

#include <kselectaction.h>
#include <khbox.h>

// Local includes

#include "albumsettings.h"
#include "albummodel.h"
#include "contextmenuhelper.h"
#include "tagcheckview.h"
#include "colorlabelfilter.h"
#include "picklabelfilter.h"
#include "ratingfilter.h"

namespace Digikam
{

class TagFilterView::TagFilterViewPriv
{
public:

    TagFilterViewPriv() :
        onRestoreTagFiltersAction(0),
        offRestoreTagFiltersAction(0),
        ignoreTagAction(0),
        includeTagAction(0),
        excludeTagAction(0),
        restoreTagFiltersAction(0),
        tagFilterModeAction(0),
        tagFilterModel(0)
    {
    }

    QAction*       onRestoreTagFiltersAction;
    QAction*       offRestoreTagFiltersAction;
    QAction*       ignoreTagAction;
    QAction*       includeTagAction;
    QAction*       excludeTagAction;

    KSelectAction* restoreTagFiltersAction;
    KSelectAction* tagFilterModeAction;

    TagModel*      tagFilterModel;
};

TagFilterView::TagFilterView(QWidget* parent, TagModel* tagFilterModel)
    : TagCheckView(parent, tagFilterModel), d(new TagFilterViewPriv)
{
    d->tagFilterModel             = tagFilterModel;

    d->restoreTagFiltersAction    = new KSelectAction(i18n("Restore Tag Filters"), this);
    d->onRestoreTagFiltersAction  = d->restoreTagFiltersAction->addAction(i18n("On"));
    d->offRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("Off"));

    d->tagFilterModeAction        = new KSelectAction(i18n("Tag Filter Mode"), this);
    d->ignoreTagAction            = d->tagFilterModeAction->addAction(i18n("Ignore This Tag"));
    d->includeTagAction           = d->tagFilterModeAction->addAction(KIcon("list-add"), i18n("Must Have This Tag"));
    d->excludeTagAction           = d->tagFilterModeAction->addAction(KIcon("list-remove"), i18n("Must Not Have This Tag"));
}

TagFilterView::~TagFilterView()
{
    delete d;
}

void TagFilterView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    TagCheckView::addCustomContextMenuActions(cmh, album);

    // restoring
    cmh.addAction(d->restoreTagFiltersAction);

    Qt::CheckState state = d->tagFilterModel->checkState(album);

    switch (state)
    {
        case Qt::Unchecked:
            d->tagFilterModeAction->setCurrentAction(d->ignoreTagAction);
            break;
        case Qt::PartiallyChecked:
            d->tagFilterModeAction->setCurrentAction(d->excludeTagAction);
            break;
        case Qt::Checked:
            d->tagFilterModeAction->setCurrentAction(d->includeTagAction);
            break;
    }

    cmh.addAction(d->tagFilterModeAction);

    d->onRestoreTagFiltersAction->setChecked(isRestoreCheckState());
    d->offRestoreTagFiltersAction->setChecked(!isRestoreCheckState());
}

void TagFilterView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    TagCheckView::handleCustomContextMenuAction(action, album);

    if (!action)
    {
        return;
    }

    if (action == d->onRestoreTagFiltersAction)        // Restore TagFilters ON.
    {
        setRestoreCheckState(true);
    }
    else if (action == d->offRestoreTagFiltersAction)  // Restore TagFilters OFF.
    {
        setRestoreCheckState(false);
    }
    else if (action == d->ignoreTagAction)
    {
        albumModel()->setCheckState(album, Qt::Unchecked);
    }
    else if (action == d->includeTagAction)
    {
        albumModel()->setCheckState(album, Qt::Checked);
    }
    else if (action == d->excludeTagAction)
    {
        albumModel()->setCheckState(album, Qt::PartiallyChecked);
    }
}

// --------------------------------------------------------------------------------------------------------

class TagFilterSideBarWidget::TagFilterSideBarWidgetPriv
{
public:

    TagFilterSideBarWidgetPriv() :
        tagFilterView(0),
        tagFilterSearchBar(0),
        tagFilterModel(0),
        colorLabelFilter(0),
        pickLabelFilter(0),
        ratingFilter(0),
        withoutTagCheckBox(0),
        matchingConditionComboBox(0)
    {
    }

    static const QString configLastShowUntaggedEntry;
    static const QString configMatchingConditionEntry;

    TagFilterView*       tagFilterView;
    SearchTextBar*       tagFilterSearchBar;

    TagModel*            tagFilterModel;

    ColorLabelFilter*    colorLabelFilter;
    PickLabelFilter*     pickLabelFilter;
    RatingFilter*        ratingFilter;

    QCheckBox*           withoutTagCheckBox;
    KComboBox*           matchingConditionComboBox;
};

const QString TagFilterSideBarWidget::TagFilterSideBarWidgetPriv::configLastShowUntaggedEntry("Show Untagged");
const QString TagFilterSideBarWidget::TagFilterSideBarWidgetPriv::configMatchingConditionEntry("Matching Condition");

// ---------------------------------------------------------------------------------------------------

TagFilterSideBarWidget::TagFilterSideBarWidget(QWidget* parent, TagModel* tagFilterModel)
    : QWidget(parent), StateSavingObject(this), d(new TagFilterSideBarWidgetPriv)
{
    setObjectName("TagFilter Sidebar");

    d->tagFilterModel     = tagFilterModel;

    d->tagFilterView      = new TagFilterView(this, tagFilterModel);
    d->tagFilterView->setObjectName("DigikamViewTagFilterView");
    d->tagFilterSearchBar = new SearchTextBar(this, "DigikamViewTagFilterSearchBar");
    d->tagFilterSearchBar->setModel(d->tagFilterView->filteredModel(),
                                    AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagFilterSearchBar->setFilterModel(d->tagFilterView->albumFilterModel());

    const QString notTaggedTitle   = i18n("Images Without Tags");
    d->withoutTagCheckBox          = new QCheckBox(notTaggedTitle, this);
    d->withoutTagCheckBox->setWhatsThis(i18n("Show images without a tag."));

    KHBox* hbox                    = new KHBox(this);
    QLabel* matchingConditionLabel = new QLabel(i18n("Matching Condition:"), hbox);
    d->matchingConditionComboBox   = new KComboBox(hbox);
    d->matchingConditionComboBox->setWhatsThis(matchingConditionLabel->whatsThis());
    d->matchingConditionComboBox->addItem(i18n("AND"), ImageFilterSettings::AndCondition);
    d->matchingConditionComboBox->addItem(i18n("OR"),  ImageFilterSettings::OrCondition);
    d->matchingConditionComboBox->setWhatsThis(i18n(
            "Defines in which way the selected tags are combined to filter the images. "
            "This also includes the '%1' check box.", notTaggedTitle));

    // --------------------------------------------------------------------------------------------------------

    QLabel* fLabel      = new QLabel(i18n("Label Filters:"));
    KHBox* hbox2        = new KHBox(this);
    d->colorLabelFilter = new ColorLabelFilter(hbox2);
    QLabel* space2      = new QLabel(hbox2);
    hbox2->setStretchFactor(space2, 10);
    hbox2->setSpacing(0);
    hbox2->setMargin(0);

    KHBox* hbox3        = new KHBox(this);
    d->pickLabelFilter  = new PickLabelFilter(hbox3);
    d->ratingFilter     = new RatingFilter(hbox3);
    QLabel* space3      = new QLabel(hbox3);
    hbox3->layout()->setAlignment(d->ratingFilter, Qt::AlignVCenter|Qt::AlignRight);
    hbox3->setStretchFactor(space3, 10);
    hbox3->setSpacing(0);
    hbox3->setMargin(0);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(d->tagFilterView);
    layout->addWidget(d->tagFilterSearchBar);
    layout->addWidget(d->withoutTagCheckBox);
    layout->addWidget(hbox);
    layout->addWidget(fLabel);
    layout->addWidget(hbox2);
    layout->addWidget(hbox3);
    layout->setStretchFactor(d->tagFilterView, 10);

    // signals/slots connections

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(const QList<TAlbum*>&, const QList<TAlbum*>&)),
            this, SLOT(slotCheckedTagsChanged(const QList<TAlbum*>&, const QList<TAlbum*>&)));

    connect(d->colorLabelFilter, SIGNAL(signalColorLabelSelectionChanged(const QList<ColorLabel>&)),
            this, SLOT(slotColorLabelFilterChanged(const QList<ColorLabel>&)));

    connect(d->pickLabelFilter, SIGNAL(signalPickLabelSelectionChanged(const QList<PickLabel>&)),
            this, SLOT(slotPickLabelFilterChanged(const QList<PickLabel>&)));

    connect(d->withoutTagCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWithoutTagChanged(int)));

    connect(d->matchingConditionComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMatchingConditionChanged(int)));

    connect(d->ratingFilter, SIGNAL(signalRatingFilterChanged(int, ImageFilterSettings::RatingCondition)),
            this, SIGNAL(signalRatingFilterChanged(int, ImageFilterSettings::RatingCondition)));
}

TagFilterSideBarWidget::~TagFilterSideBarWidget()
{
    delete d;
}

void TagFilterSideBarWidget::slotResetFilters()
{
    d->tagFilterView->slotResetCheckState();
    d->withoutTagCheckBox->setChecked(false);
    d->colorLabelFilter->reset();
    d->pickLabelFilter->reset();
    d->ratingFilter->setRating(0);
    d->ratingFilter->setRatingFilterCondition(ImageFilterSettings::GreaterEqualCondition);
}

void TagFilterSideBarWidget::slotMatchingConditionChanged(int index)
{
    Q_UNUSED(index);
    filterChanged();
}

void TagFilterSideBarWidget::slotCheckedTagsChanged(const QList<TAlbum*>& includedTags,
                                                    const QList<TAlbum*>& excludedTags)
{
    Q_UNUSED(includedTags);
    Q_UNUSED(excludedTags);
    filterChanged();
}

void TagFilterSideBarWidget::slotColorLabelFilterChanged(const QList<ColorLabel>& list)
{
    Q_UNUSED(list);
    filterChanged();
}

void TagFilterSideBarWidget::slotPickLabelFilterChanged(const QList<PickLabel>& list)
{
    Q_UNUSED(list);
    filterChanged();
}

void TagFilterSideBarWidget::slotWithoutTagChanged(int newState)
{
    Q_UNUSED(newState);
    filterChanged();
}

void TagFilterSideBarWidget::filterChanged()
{
    bool showUntagged = d->withoutTagCheckBox->checkState() == Qt::Checked;
    ImageFilterSettings::MatchingCondition matchCond =
        (ImageFilterSettings::MatchingCondition)d->matchingConditionComboBox->itemData(
            d->matchingConditionComboBox->currentIndex()).toInt();

    QList<int> includedTagIds;
    QList<int> excludedTagIds;
    QList<int> clTagIds;
    QList<int> plTagIds;

    if (!showUntagged || matchCond == ImageFilterSettings::OrCondition)
    {
        foreach (TAlbum* tag, d->tagFilterView->getCheckedTags())
        {
            if (tag)
            {
                includedTagIds << tag->id();
            }
        }
        foreach (TAlbum* tag, d->tagFilterView->getPartiallyCheckedTags())
        {
            if (tag)
            {
                excludedTagIds << tag->id();
            }
        }
        foreach (TAlbum* tag, d->colorLabelFilter->getCheckedColorLabelTags())
        {
            if (tag)
            {
                clTagIds << tag->id();
            }
        }
        foreach (TAlbum* tag, d->pickLabelFilter->getCheckedPickLabelTags())
        {
            if (tag)
            {
                plTagIds << tag->id();
            }
        }
    }

    emit signalTagFilterChanged(includedTagIds, excludedTagIds, matchCond, showUntagged, clTagIds, plTagIds);
}

void TagFilterSideBarWidget::setConfigGroup(KConfigGroup group)
{
    StateSavingObject::setConfigGroup(group);
    d->tagFilterView->setConfigGroup(group);
}

void TagFilterSideBarWidget::doLoadState()
{
    d->ratingFilter->setRatingFilterCondition((Digikam::ImageFilterSettings::RatingCondition)
            (AlbumSettings::instance()->getRatingFilterCond()));

    d->matchingConditionComboBox->setCurrentIndex(getConfigGroup().readEntry(
                entryName(d->configMatchingConditionEntry), 0));
    d->tagFilterView->loadState();

    if (d->tagFilterView->isRestoreCheckState())
    {
        d->withoutTagCheckBox->setChecked(getConfigGroup().readEntry(entryName(
                                              d->configLastShowUntaggedEntry), false));
    }

    filterChanged();
}

void TagFilterSideBarWidget::doSaveState()
{
    AlbumSettings::instance()->setRatingFilterCond(d->ratingFilter->ratingFilterCondition());

    getConfigGroup().writeEntry(entryName(d->configMatchingConditionEntry),
                                d->matchingConditionComboBox->currentIndex());
    d->tagFilterView->saveState();
    getConfigGroup().writeEntry(entryName(d->configLastShowUntaggedEntry),
                                d->withoutTagCheckBox->isChecked());
    getConfigGroup().sync();
}

} // namespace Digikam
