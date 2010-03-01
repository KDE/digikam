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
        tagFilterModel(0),
        restoreTagFiltersAction(0),
        onRestoreTagFiltersAction(0),
        offRestoreTagFiltersAction(0)
    {
    }

    TagModel *tagFilterModel;

    KSelectAction *restoreTagFiltersAction;
    QAction *onRestoreTagFiltersAction;
    QAction *offRestoreTagFiltersAction;

};

TagFilterView::TagFilterView(QWidget *parent, TagModel *tagFilterModel) :
    TagCheckView(parent, tagFilterModel),
    d(new TagFilterViewPriv)
{

    d->tagFilterModel = tagFilterModel;

    d->restoreTagFiltersAction = new KSelectAction(i18n("Restore Tag Filters"), this);
    d->onRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("On"));
    d->offRestoreTagFiltersAction = d->restoreTagFiltersAction->addAction(i18n("Off"));

}

TagFilterView::~TagFilterView()
{
    delete d;
}

void TagFilterView::addCustomContextMenuActions(ContextMenuHelper &cmh, Album *album)
{
    TagCheckView::addCustomContextMenuActions(cmh, album);

    // restoring
    cmh.addAction(d->restoreTagFiltersAction);

    d->onRestoreTagFiltersAction->setChecked(isRestoreCheckState());
    d->offRestoreTagFiltersAction->setChecked(!isRestoreCheckState());

}

void TagFilterView::handleCustomContextMenuAction(QAction *action, AlbumPointer<Album> album)
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

}

// -----------------------------------------------------------------------------

class TagFilterSideBarWidgetPriv
{
public:

    TagFilterSideBarWidgetPriv() :
        configLastShowUntaggedEntry("Show Untagged"),
        configMatchingConditionEntry("Matching Condition"),
        tagFilterView(0),
        tagFilterSearchBar(0)
    {
    }

    QString configLastShowUntaggedEntry;
    QString configMatchingConditionEntry;

    TagFilterView    *tagFilterView;
    SearchTextBar    *tagFilterSearchBar;

    TagModel *tagFilterModel;

    QCheckBox *withoutTagCheckBox;
    KComboBox *matchingConditionComboBox;

};

TagFilterSideBarWidget::TagFilterSideBarWidget(QWidget *parent,
                TagModel *tagFilterModel) :
    QWidget(parent), StateSavingObject(this), d(new TagFilterSideBarWidgetPriv)
{

    setObjectName("TagFilter Sidebar");

    d->tagFilterModel = tagFilterModel;

    d->tagFilterView      = new TagFilterView(this, tagFilterModel);
    d->tagFilterView->setObjectName("DigikamViewTagFilterView");
    d->tagFilterSearchBar = new SearchTextBar(this, "DigikamViewTagFilterSearchBar");
    d->tagFilterSearchBar->setModel(tagFilterModel, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagFilterSearchBar->setFilterModel(d->tagFilterView->albumFilterModel());

    const QString notTaggedTitle = i18n("Images Without Tag");
    d->withoutTagCheckBox = new QCheckBox(notTaggedTitle, this);
    d->withoutTagCheckBox->setWhatsThis(i18n("Show images without a tag."));

    QLabel *matchingConditionLabel = new QLabel(i18n("Matching Condition:"), this);
    matchingConditionLabel->setWhatsThis(i18n(
                    "Defines in which way the selected tags are combined to filter the images. "
                    "This also includes the '%1' check box.", notTaggedTitle));

    d->matchingConditionComboBox = new KComboBox(this);
    d->matchingConditionComboBox->setWhatsThis(matchingConditionLabel->whatsThis());
    d->matchingConditionComboBox->addItem(i18n("AND"), ImageFilterSettings::AndCondition);
    d->matchingConditionComboBox->addItem(i18n("OR"), ImageFilterSettings::OrCondition);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(d->tagFilterView);
    layout->addWidget(d->tagFilterSearchBar);
    layout->addWidget(d->withoutTagCheckBox);
    layout->addWidget(matchingConditionLabel);
    layout->addWidget(d->matchingConditionComboBox);

    // connection

    connect(d->tagFilterView, SIGNAL(checkedTagsChanged(const QList<TAlbum*>&)),
            this, SLOT(slotCheckedTagsChanged(const QList<TAlbum*>&)));

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

void TagFilterSideBarWidget::slotCheckedTagsChanged(const QList<TAlbum*> &tags)
{
    Q_UNUSED(tags);
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

    emit tagFilterChanged(tagIds,
                          (ImageFilterSettings::MatchingCondition)d->matchingConditionComboBox->itemData(
                                          d->matchingConditionComboBox->currentIndex()).toInt(),
                          showUntagged);

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
