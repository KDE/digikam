/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : tag filter view for the right sidebar
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

// KDE includes

#include <kselectaction.h>

// Local includes

#include "albummodel.h"
#include "contextmenuhelper.h"
#include "tagcheckview.h"

namespace Digikam
{

class TagFilterViewPriv
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

    QAction*        onRestoreTagFiltersAction;
    QAction*        offRestoreTagFiltersAction;
    QAction*        ignoreTagAction;
    QAction*        includeTagAction;
    QAction*        excludeTagAction;

    KSelectAction*  restoreTagFiltersAction;
    KSelectAction*  tagFilterModeAction;

    TagModel*       tagFilterModel;
};

TagFilterView::TagFilterView(QWidget* parent, TagModel* tagFilterModel) :
    TagCheckView(parent, tagFilterModel),
    d(new TagFilterViewPriv)
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
    else if (action == d->offRestoreTagFiltersAction)        // Restore TagFilters OFF.
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

// -----------------------------------------------------------------------------

class TagFilterSideBarWidgetPriv
{
public:

    TagFilterSideBarWidgetPriv() :
        tagFilterView(0),
        tagFilterSearchBar(0),
        tagFilterModel(0),
        withoutTagCheckBox(0),
        matchingConditionComboBox(0)
    {
    }

    static const QString configLastShowUntaggedEntry;
    static const QString configMatchingConditionEntry;

    TagFilterView*       tagFilterView;
    SearchTextBar*       tagFilterSearchBar;

    TagModel*            tagFilterModel;

    QCheckBox*           withoutTagCheckBox;
    KComboBox*           matchingConditionComboBox;

};
const QString TagFilterSideBarWidgetPriv::configLastShowUntaggedEntry("Show Untagged");
const QString TagFilterSideBarWidgetPriv::configMatchingConditionEntry("Matching Condition");

// --------------------------------------------------------

TagFilterSideBarWidget::TagFilterSideBarWidget(QWidget* parent,
        TagModel* tagFilterModel) :
    QWidget(parent), StateSavingObject(this), d(new TagFilterSideBarWidgetPriv)
{

    setObjectName("TagFilter Sidebar");

    d->tagFilterModel = tagFilterModel;

    d->tagFilterView      = new TagFilterView(this, tagFilterModel);
    d->tagFilterView->setObjectName("DigikamViewTagFilterView");
    d->tagFilterSearchBar = new SearchTextBar(this, "DigikamViewTagFilterSearchBar");
    d->tagFilterSearchBar->setModel(tagFilterModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagFilterSearchBar->setFilterModel(d->tagFilterView->albumFilterModel());

    const QString notTaggedTitle = i18n("Images Without Tags");
    d->withoutTagCheckBox = new QCheckBox(notTaggedTitle, this);
    d->withoutTagCheckBox->setWhatsThis(i18n("Show images without a tag."));

    QLabel* matchingConditionLabel = new QLabel(i18n("Matching Condition:"), this);
    matchingConditionLabel->setWhatsThis(i18n(
            "Defines in which way the selected tags are combined to filter the images. "
            "This also includes the '%1' check box.", notTaggedTitle));

    d->matchingConditionComboBox = new KComboBox(this);
    d->matchingConditionComboBox->setWhatsThis(matchingConditionLabel->whatsThis());
    d->matchingConditionComboBox->addItem(i18n("AND"), ImageFilterSettings::AndCondition);
    d->matchingConditionComboBox->addItem(i18n("OR"), ImageFilterSettings::OrCondition);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(d->tagFilterView);
    layout->addWidget(d->tagFilterSearchBar);
    layout->addWidget(d->withoutTagCheckBox);
    layout->addWidget(matchingConditionLabel);
    layout->addWidget(d->matchingConditionComboBox);

    // connection

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(const QList<TAlbum*>&, const QList<TAlbum*>&)),
            this, SLOT(slotCheckedTagsChanged(const QList<TAlbum*>&, const QList<TAlbum*>&)));

    connect(d->withoutTagCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWithoutTagChanged(int)));

    connect(d->matchingConditionComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMatchingConditionChanged(int)));

}

TagFilterSideBarWidget::~TagFilterSideBarWidget()
{
}

void TagFilterSideBarWidget::slotResetTagFilters()
{
    d->tagFilterView->slotResetCheckState();
    d->withoutTagCheckBox->setChecked(false);
}

void TagFilterSideBarWidget::slotMatchingConditionChanged(int index)
{
    Q_UNUSED(index);
    filterChanged();
}

void TagFilterSideBarWidget::slotCheckedTagsChanged(const QList<TAlbum*> &includedTags, const QList<TAlbum*> &excludedTags)
{
    Q_UNUSED(includedTags);
    Q_UNUSED(excludedTags);
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

    if (!showUntagged || matchCond == ImageFilterSettings::OrCondition)
    {
        foreach(TAlbum *tag, d->tagFilterView->getCheckedTags())
        {
            if (tag)
            {
                includedTagIds << tag->id();
            }
        }
        foreach(TAlbum *tag, d->tagFilterView->getPartiallyCheckedTags())
        {
            if (tag)
            {
                excludedTagIds << tag->id();
            }
        }
    }

    emit tagFilterChanged(includedTagIds, excludedTagIds, matchCond, showUntagged);
}

void TagFilterSideBarWidget::setConfigGroup(KConfigGroup group)
{
    StateSavingObject::setConfigGroup(group);
    d->tagFilterView->setConfigGroup(group);
}

void TagFilterSideBarWidget::doLoadState()
{
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
    getConfigGroup().writeEntry(entryName(d->configMatchingConditionEntry),
                                d->matchingConditionComboBox->currentIndex());
    d->tagFilterView->saveState();
    getConfigGroup().writeEntry(entryName(d->configLastShowUntaggedEntry),
                                d->withoutTagCheckBox->isChecked());
    getConfigGroup().sync();
}

}
