/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2003-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2015      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "itemdescedittab.h"

// Qt includes

#include <QTextEdit>
#include <QStyle>
#include <QGridLayout>
#include <QScrollArea>
#include <QSignalMapper>
#include <QTimer>
#include <QToolButton>
#include <QApplication>
#include <QPushButton>
#include <QMenu>
#include <QIcon>
#include <QCheckBox>
#include <QMessageBox>
#include <QPointer>

// Local includes

#include "digikam_debug.h"
#include "addtagslineedit.h"
#include "applicationsettings.h"
#include "albumthumbnailloader.h"
#include "captionedit.h"
#include "collectionscanner.h"
#include "coredbtransaction.h"
#include "dnotificationwrapper.h"
#include "ddatetimeedit.h"
#include "digikamapp.h"
#include "fileactionmngr.h"
#include "ratingwidget.h"
#include "scancontroller.h"
#include "tagcheckview.h"
#include "templateselector.h"
#include "templateviewer.h"
#include "itemattributeswatch.h"
#include "statusprogressbar.h"
#include "tagmodificationhelper.h"
#include "template.h"
#include "iteminfolist.h"
#include "iteminfo.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "fileactionprogress.h"
#include "tagsmanager.h"
#include "tagtreeview.h"
#include "searchtextbar.h"
#include "disjointmetadata.h"
#include "altlangstredit.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemDescEditTab::Private
{

public:

    enum DescEditTab
    {
        DESCRIPTIONS = 0,
        TAGS,
        INFOS
    };

    explicit Private()
    {
        modified                   = false;
        ignoreItemAttributesWatch = false;
        ignoreTagChanges           = false;
        togglingSearchSettings     = false;
        recentTagsBtn              = 0;
        titleEdit                  = 0;
        captionsEdit               = 0;
        tagsSearchBar              = 0;
        dateTimeEdit               = 0;
        tagCheckView               = 0;
        ratingWidget               = 0;
        assignedTagsBtn            = 0;
        applyBtn                   = 0;
        moreButton                 = 0;
        revertBtn                  = 0;
        openTagMngr                = 0;
        moreMenu                   = 0;
        applyToAllVersionsButton   = 0;
        recentTagsMapper           = 0;
        newTagEdit                 = 0;
        lastSelectedWidget         = 0;
        templateSelector           = 0;
        templateViewer             = 0;
        tabWidget                  = 0;
        tagModel                   = 0;
        tagCheckView               = 0;
        colorLabelSelector         = 0;
        pickLabelSelector          = 0;
        metadataChangeTimer        = 0;
    }

    bool                 modified;
    bool                 ignoreItemAttributesWatch;
    bool                 ignoreTagChanges;
    bool                 togglingSearchSettings;

    QToolButton*         recentTagsBtn;
    QToolButton*         assignedTagsBtn;
    QToolButton*         revertBtn;
    QPushButton*         openTagMngr;

    QMenu*               moreMenu;

    QSignalMapper*       recentTagsMapper;

    QPushButton*         applyBtn;
    QPushButton*         moreButton;
    QPushButton*         applyToAllVersionsButton;

    QWidget*             lastSelectedWidget;

    AltLangStrEdit*      titleEdit;

    CaptionEdit*         captionsEdit;

    DDateTimeEdit*       dateTimeEdit;

    QTabWidget*          tabWidget;

    SearchTextBar*       tagsSearchBar;
    AddTagsLineEdit*     newTagEdit;

    ItemInfoList        currInfos;

    TagCheckView*        tagCheckView;

    TemplateSelector*    templateSelector;
    TemplateViewer*      templateViewer;

    RatingWidget*        ratingWidget;
    ColorLabelSelector*  colorLabelSelector;
    PickLabelSelector*   pickLabelSelector;

    DisjointMetadata     hub;

    TagModel*            tagModel;

    QTimer*              metadataChangeTimer;
    QList<int>           metadataChangeIds;
};

ItemDescEditTab::ItemDescEditTab(QWidget* const parent)
    : DVBox(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    setContentsMargins(QMargins());
    setSpacing(spacing);
    d->tabWidget           = new QTabWidget(this);

    d->metadataChangeTimer = new QTimer(this);
    d->metadataChangeTimer->setSingleShot(true);
    d->metadataChangeTimer->setInterval(250);

    // Captions/Date/Rating view -----------------------------------

    QScrollArea* const sv          = new QScrollArea(d->tabWidget);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);

    QWidget* const captionTagsArea = new QWidget(sv->viewport());
    QGridLayout* const grid1       = new QGridLayout(captionTagsArea);
    sv->setWidget(captionTagsArea);

    d->titleEdit    = new AltLangStrEdit(captionTagsArea);
    d->titleEdit->setTitle(i18n("Title:"));
    d->titleEdit->setPlaceholderText(i18n("Enter title here."));

    d->captionsEdit = new CaptionEdit(captionTagsArea);

    DHBox* const dateBox = new DHBox(captionTagsArea);
    new QLabel(i18n("Date:"), dateBox);
    d->dateTimeEdit      = new DDateTimeEdit(dateBox, QLatin1String("datepicker"));

    DHBox* const pickBox = new DHBox(captionTagsArea);
    new QLabel(i18n("Pick Label:"), pickBox);
    d->pickLabelSelector = new PickLabelSelector(pickBox);
    pickBox->layout()->setAlignment(d->pickLabelSelector, Qt::AlignVCenter | Qt::AlignRight);

    DHBox* const colorBox = new DHBox(captionTagsArea);
    new QLabel(i18n("Color Label:"), colorBox);
    d->colorLabelSelector = new ColorLabelSelector(colorBox);
    colorBox->layout()->setAlignment(d->colorLabelSelector, Qt::AlignVCenter | Qt::AlignRight);

    DHBox* const rateBox  = new DHBox(captionTagsArea);
    new QLabel(i18n("Rating:"), rateBox);
    d->ratingWidget       = new RatingWidget(rateBox);
    rateBox->layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignRight);

    // Buttons -----------------------------------------

    DHBox* const applyButtonBox = new DHBox(this);
    applyButtonBox->setSpacing(spacing);

    d->applyBtn = new QPushButton(i18n("Apply"), applyButtonBox);
    d->applyBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->applyBtn->setEnabled(false);
    d->applyBtn->setToolTip( i18n("Apply all changes to images"));
    //buttonsBox->setStretchFactor(d->applyBtn, 10);

    DHBox* const buttonsBox = new DHBox(this);
    buttonsBox->setSpacing(spacing);

    d->revertBtn = new QToolButton(buttonsBox);
    d->revertBtn->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->revertBtn->setToolTip( i18n("Revert all changes"));
    d->revertBtn->setEnabled(false);

    d->applyToAllVersionsButton = new QPushButton(i18n("Apply to all versions"), buttonsBox);
    d->applyToAllVersionsButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->applyToAllVersionsButton->setEnabled(false);
    d->applyToAllVersionsButton->setToolTip(i18n("Apply all changes to all versions of this image"));

    d->moreButton = new QPushButton(i18n("More"), buttonsBox);
    d->moreMenu   = new QMenu(captionTagsArea);
    d->moreButton->setMenu(d->moreMenu);

    // --------------------------------------------------

    grid1->addWidget(d->titleEdit,    0, 0, 1, 2);
    grid1->addWidget(d->captionsEdit, 1, 0, 1, 2);
    grid1->addWidget(dateBox,         2, 0, 1, 2);
    grid1->addWidget(pickBox,         3, 0, 1, 2);
    grid1->addWidget(colorBox,        4, 0, 1, 2);
    grid1->addWidget(rateBox,         5, 0, 1, 2);
    grid1->setRowStretch(1, 10);
    grid1->setContentsMargins(spacing, spacing, spacing, spacing);
    grid1->setSpacing(spacing);

    d->tabWidget->insertTab(Private::DESCRIPTIONS, sv, i18n("Description"));

    // Tags view ---------------------------------------------------

    QScrollArea* const sv3    = new QScrollArea(d->tabWidget);
    sv3->setFrameStyle(QFrame::NoFrame);
    sv3->setWidgetResizable(true);

    QWidget* const tagsArea   = new QWidget(sv3->viewport());
    QGridLayout* const grid3  = new QGridLayout(tagsArea);
    sv3->setWidget(tagsArea);

    d->tagModel     = new TagModel(AbstractAlbumModel::IncludeRootAlbum, this);
    d->tagModel->setCheckable(true);
    d->tagModel->setRootCheckable(false);
    d->tagCheckView = new TagCheckView(tagsArea, d->tagModel);
    d->tagCheckView->setCheckNewTags(true);

    d->openTagMngr = new QPushButton( i18n("Open Tag Manager"));

    d->newTagEdit  = new AddTagsLineEdit(tagsArea);
    d->newTagEdit->setSupportingTagModel(d->tagModel);
    d->newTagEdit->setTagTreeView(d->tagCheckView);
    //, "ItemDescEditTabNewTagEdit",
    //d->newTagEdit->setCaseSensitive(false);
    d->newTagEdit->setPlaceholderText(i18n("Enter tag here."));
    d->newTagEdit->setWhatsThis(i18n("Enter the text used to create tags here. "
                                     "'/' can be used to create a hierarchy of tags. "
                                     "',' can be used to create more than one hierarchy at the same time."));

    DHBox* const tagsSearch = new DHBox(tagsArea);
    tagsSearch->setSpacing(spacing);

    d->tagsSearchBar   = new SearchTextBar(tagsSearch, QLatin1String("ItemDescEditTabTagsSearchBar"));
    d->tagsSearchBar->setModel(d->tagCheckView->filteredModel(),
                               AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagsSearchBar->setFilterModel(d->tagCheckView->albumFilterModel());

    d->assignedTagsBtn = new QToolButton(tagsSearch);
    d->assignedTagsBtn->setToolTip( i18n("Tags already assigned"));
    d->assignedTagsBtn->setIcon(QIcon::fromTheme(QLatin1String("tag-assigned")));
    d->assignedTagsBtn->setCheckable(true);

    d->recentTagsBtn            = new QToolButton(tagsSearch);
    QMenu* const recentTagsMenu = new QMenu(d->recentTagsBtn);
    d->recentTagsBtn->setToolTip( i18n("Recent Tags"));
    d->recentTagsBtn->setIcon(QIcon::fromTheme(QLatin1String("tag-recents")));
    d->recentTagsBtn->setIconSize(QSize(16, 16));
    d->recentTagsBtn->setMenu(recentTagsMenu);
    d->recentTagsBtn->setPopupMode(QToolButton::InstantPopup);
    d->recentTagsMapper = new QSignalMapper(this);

    grid3->addWidget(d->openTagMngr,  0, 0, 1, 2);
    grid3->addWidget(d->newTagEdit,   1, 0, 1, 2);
    grid3->addWidget(d->tagCheckView, 2, 0, 1, 2);
    grid3->addWidget(tagsSearch,      3, 0, 1, 2);
    grid3->setRowStretch(1, 10);

    d->tabWidget->insertTab(Private::TAGS, sv3, i18n("Tags"));

    // Information Management View --------------------------------------

    QScrollArea* const sv2    = new QScrollArea(d->tabWidget);
    sv2->setFrameStyle(QFrame::NoFrame);
    sv2->setWidgetResizable(true);

    QWidget* const infoArea   = new QWidget(sv2->viewport());
    QGridLayout* const grid2  = new QGridLayout(infoArea);
    sv2->setWidget(infoArea);

    d->templateSelector = new TemplateSelector(infoArea);
    d->templateViewer   = new TemplateViewer(infoArea);
    d->templateViewer->setObjectName(QLatin1String("ItemDescEditTab Expander"));

    grid2->addWidget(d->templateSelector, 0, 0, 1, 2);
    grid2->addWidget(d->templateViewer,   1, 0, 1, 2);
    grid2->setRowStretch(1, 10);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(spacing);

    d->tabWidget->insertTab(Private::INFOS, sv2, i18n("Information"));

    // --------------------------------------------------

    connect(d->openTagMngr, SIGNAL(clicked()),
            this, SLOT(slotOpenTagsManager()));

    connect(d->tagCheckView->checkableModel(), SIGNAL(checkStateChanged(Album*,Qt::CheckState)),
            this, SLOT(slotTagStateChanged(Album*,Qt::CheckState)));

    connect(d->titleEdit, SIGNAL(signalModified(QString,QString)),
            this, SLOT(slotTitleChanged()));

    connect(d->titleEdit, SIGNAL(signalValueAdded(QString,QString)),
            this, SLOT(slotTitleChanged()));

    connect(d->titleEdit, SIGNAL(signalValueDeleted(QString)),
            this, SLOT(slotTitleChanged()));

    connect(d->captionsEdit, SIGNAL(signalModified()),
            this, SLOT(slotCommentChanged()));

    connect(d->dateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)),
            this, SLOT(slotDateTimeChanged(QDateTime)));

    connect(d->pickLabelSelector, SIGNAL(signalPickLabelChanged(int)),
            this, SLOT(slotPickLabelChanged(int)));

    connect(d->colorLabelSelector, SIGNAL(signalColorLabelChanged(int)),
            this, SLOT(slotColorLabelChanged(int)));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged(int)));

    connect(d->templateSelector, SIGNAL(signalTemplateSelected()),
            this, SLOT(slotTemplateSelected()));

    connect(d->tagsSearchBar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotTagsSearchChanged(SearchTextSettings)));

    connect(d->assignedTagsBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotAssignedTagsToggled(bool)));

    connect(d->newTagEdit, SIGNAL(taggingActionActivated(TaggingAction)),
            this, SLOT(slotTaggingActionActivated(TaggingAction)));

    connect(d->applyBtn, SIGNAL(clicked()),
            this, SLOT(slotApplyAllChanges()));

    connect(d->applyToAllVersionsButton, SIGNAL(clicked()),
            this, SLOT(slotApplyChangesToAllVersions()));

    connect(d->revertBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertAllChanges()));

    connect(d->moreMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotMoreMenu()));

    connect(d->recentTagsMapper, SIGNAL(mapped(int)),
            this, SLOT(slotRecentTagsMenuActivated(int)));

    connect(d->metadataChangeTimer, SIGNAL(timeout()),
            this, SLOT(slotReloadForMetadataChange()));

    connect(this, SIGNAL(askToApplyChanges(QList<ItemInfo>,DisjointMetadata*)),
            this, SLOT(slotAskToApplyChanges(QList<ItemInfo>,DisjointMetadata*)),
            Qt::QueuedConnection);

    // Initialize ---------------------------------------------

    d->titleEdit->textEdit()->installEventFilter(this);
    d->captionsEdit->textEdit()->installEventFilter(this);

    d->dateTimeEdit->installEventFilter(this);
    d->pickLabelSelector->installEventFilter(this);
    d->colorLabelSelector->installEventFilter(this);
    d->ratingWidget->installEventFilter(this);
    // TODO update, what does this filter?
    d->tagCheckView->installEventFilter(this);
    updateRecentTags();

    // Connect to attribute watch ------------------------------

    ItemAttributesWatch* const watch = ItemAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageTagsChanged(qlonglong)),
            this, SLOT(slotImageTagsChanged(qlonglong)));

    connect(watch, SIGNAL(signalImagesChanged(int)),
            this, SLOT(slotImagesChanged(int)));

    connect(watch, SIGNAL(signalImageRatingChanged(qlonglong)),
            this, SLOT(slotImageRatingChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageDateChanged(qlonglong)),
            this, SLOT(slotImageDateChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageCaptionChanged(qlonglong)),
            this, SLOT(slotImageCaptionChanged(qlonglong)));
}

ItemDescEditTab::~ItemDescEditTab()
{
    delete d;
}

void ItemDescEditTab::readSettings(KConfigGroup& group)
{
    d->tabWidget->setCurrentIndex(group.readEntry(QLatin1String("ItemDescEdit Tab"), (int)Private::DESCRIPTIONS));
    d->titleEdit->setCurrentLanguageCode(group.readEntry(QLatin1String("ItemDescEditTab TitleLang"), QString()));
    d->captionsEdit->setCurrentLanguageCode(group.readEntry(QLatin1String("ItemDescEditTab CaptionsLang"), QString()));

    d->templateViewer->readSettings(group);

    d->tagCheckView->setConfigGroup(group);
    d->tagCheckView->setEntryPrefix(QLatin1String("ItemDescEditTab TagCheckView"));
    d->tagCheckView->loadState();
    d->tagsSearchBar->setConfigGroup(group);
    d->tagsSearchBar->setEntryPrefix(QLatin1String("ItemDescEditTab SearchBar"));
    d->tagsSearchBar->loadState();
}

void ItemDescEditTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(QLatin1String("ItemDescEdit Tab"),             d->tabWidget->currentIndex());
    group.writeEntry(QLatin1String("ItemDescEditTab TitleLang"),    d->titleEdit->currentLanguageCode());
    group.writeEntry(QLatin1String("ItemDescEditTab CaptionsLang"), d->captionsEdit->currentLanguageCode());

    d->templateViewer->writeSettings(group);

    d->tagCheckView->saveState();
    d->tagsSearchBar->saveState();
}

void ItemDescEditTab::setFocusToLastSelectedWidget()
{
    if (d->lastSelectedWidget)
    {
        d->lastSelectedWidget->setFocus();
    }

    d->lastSelectedWidget = 0;
}

void ItemDescEditTab::setFocusToTagsView()
{
    d->lastSelectedWidget = qobject_cast<QWidget*>(d->tagCheckView);
    d->tagCheckView->setFocus();
    d->tabWidget->setCurrentIndex(Private::TAGS);
}

void ItemDescEditTab::setFocusToNewTagEdit()
{
    //select "Tags" tab and focus the NewTagLineEdit widget
    d->tabWidget->setCurrentIndex(Private::TAGS);
    d->newTagEdit->setFocus();
}

void ItemDescEditTab::setFocusToTitlesEdit()
{
    d->tabWidget->setCurrentIndex(Private::DESCRIPTIONS);
    d->titleEdit->textEdit()->setFocus();
}

void ItemDescEditTab::setFocusToCommentsEdit()
{
    d->tabWidget->setCurrentIndex(Private::DESCRIPTIONS);
    d->captionsEdit->textEdit()->setFocus();
}

void ItemDescEditTab::activateAssignedTagsButton()
{
    d->tabWidget->setCurrentIndex(Private::TAGS);
    d->assignedTagsBtn->click();
}

bool ItemDescEditTab::singleSelection() const
{
    return (d->currInfos.count() == 1);
}

void ItemDescEditTab::slotChangingItems()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    if (!ApplicationSettings::instance()->getApplySidebarChangesDirectly())
    {
        // Open dialog via queued connection out-of-scope, see bug 302311
        emit askToApplyChanges(d->currInfos, new DisjointMetadata(d->hub));
        reset();
    }
    else
    {
        slotApplyAllChanges();
    }
}

void ItemDescEditTab::slotAskToApplyChanges(const QList<ItemInfo>& infos, DisjointMetadata* hub)
{
    int changedFields = 0;

    if (hub->titlesChanged())
    {
        ++changedFields;
    }

    if (hub->commentsChanged())
    {
        ++changedFields;
    }

    if (hub->dateTimeChanged())
    {
        ++changedFields;
    }

    if (hub->ratingChanged())
    {
        ++changedFields;
    }

    if (hub->pickLabelChanged())
    {
        ++changedFields;
    }

    if (hub->colorLabelChanged())
    {
        ++changedFields;
    }

    if (hub->tagsChanged())
    {
        ++changedFields;
    }

    QString text;

    if (changedFields == 1)
    {
        if (hub->commentsChanged())
            text = i18np("You have edited the image caption. ",
                         "You have edited the captions of %1 images. ",
                         infos.count());
        else if (hub->titlesChanged())
            text = i18np("You have edited the image title. ",
                         "You have edited the titles of %1 images. ",
                         infos.count());
        else if (hub->dateTimeChanged())
            text = i18np("You have edited the date of the image. ",
                         "You have edited the date of %1 images. ",
                         infos.count());
        else if (hub->pickLabelChanged())
            text = i18np("You have edited the pick label of the image. ",
                         "You have edited the pick label of %1 images. ",
                         infos.count());
        else if (hub->colorLabelChanged())
            text = i18np("You have edited the color label of the image. ",
                         "You have edited the color label of %1 images. ",
                         infos.count());
        else if (hub->ratingChanged())
            text = i18np("You have edited the rating of the image. ",
                         "You have edited the rating of %1 images. ",
                         infos.count());
        else if (hub->tagsChanged())
            text = i18np("You have edited the tags of the image. ",
                         "You have edited the tags of %1 images. ",
                         infos.count());

        text += i18n("Do you want to apply your changes?");
    }
    else
    {
        text = i18np("<p>You have edited the metadata of the image: </p><p><ul>",
                     "<p>You have edited the metadata of %1 images: </p><p><ul>",
                     infos.count());

        if (hub->titlesChanged())
        {
            text += i18n("<li>title</li>");
        }

        if (hub->commentsChanged())
        {
            text += i18n("<li>caption</li>");
        }

        if (hub->dateTimeChanged())
        {
            text += i18n("<li>date</li>");
        }

        if (hub->pickLabelChanged())
        {
            text += i18n("<li>pick label</li>");
        }

        if (hub->colorLabelChanged())
        {
            text += i18n("<li>color label</li>");
        }

        if (hub->ratingChanged())
        {
            text += i18n("<li>rating</li>");
        }

        if (hub->tagsChanged())
        {
            text += i18n("<li>tags</li>");
        }

        text += QLatin1String("</ul></p>");

        text += i18n("<p>Do you want to apply your changes?</p>");
    }

    QCheckBox* const alwaysCBox  = new QCheckBox(i18n("Always apply changes without confirmation"));

    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Information,
                                                   i18n("Apply changes?"),
                                                   text,
                                                   QMessageBox::Yes | QMessageBox::No,
                                                   qApp->activeWindow());
    msgBox->setCheckBox(alwaysCBox);
    msgBox->setDefaultButton(QMessageBox::No);
    msgBox->setEscapeButton(QMessageBox::No);

    // Pop-up a message in desktop notification manager
    DNotificationWrapper(QString(), i18n("Apply changes?"),
                         DigikamApp::instance(), DigikamApp::instance()->windowTitle());

    int returnCode   = msgBox->exec();
    bool alwaysApply = msgBox->checkBox()->isChecked();
    delete msgBox;

    if (alwaysApply)
    {
        ApplicationSettings::instance()->setApplySidebarChangesDirectly(true);
    }

    if (returnCode == QMessageBox::No)
    {
        delete hub;
        return;
    }

    // otherwise apply:
    FileActionMngr::instance()->applyMetadata(infos, hub);
}

void ItemDescEditTab::reset()
{
    d->modified = false;
    d->hub.resetChanged();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);
    d->applyToAllVersionsButton->setEnabled(false);
}

void ItemDescEditTab::slotApplyAllChanges()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    FileActionMngr::instance()->applyMetadata(d->currInfos, d->hub);
    reset();
}

void ItemDescEditTab::slotRevertAllChanges()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    setInfos(d->currInfos);
}

void ItemDescEditTab::setItem(const ItemInfo& info)
{
    slotChangingItems();
    ItemInfoList list;

    if (!info.isNull())
    {
        list << info;
    }

    setInfos(list);
}

void ItemDescEditTab::setItems(const ItemInfoList& infos)
{
    slotChangingItems();
    setInfos(infos);
}

void ItemDescEditTab::setInfos(const ItemInfoList& infos)
{
    if (infos.isEmpty())
    {
        d->hub = DisjointMetadata();
        d->captionsEdit->blockSignals(true);
        d->captionsEdit->reset();
        d->captionsEdit->blockSignals(false);
        d->titleEdit->blockSignals(true);
        d->titleEdit->reset();
        d->titleEdit->blockSignals(false);
        d->currInfos.clear();
        resetMetadataChangeInfo();
        setEnabled(false);
        return;
    }

    setEnabled(true);
    d->currInfos = infos;
    d->modified  = false;
    resetMetadataChangeInfo();
    d->hub       = DisjointMetadata();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);

    if (d->currInfos.count() > 1000)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    // First, we read all tags of the items into the cache with one SQL query.
    // This is faster than each item individually.
    d->currInfos.loadTagIds();

    foreach (const ItemInfo& info, d->currInfos)
    {
        d->hub.load(info);
    }

    updateComments();
    updatePickLabel();
    updateColorLabel();
    updateRating();
    updateDate();
    updateTemplate();
    updateTagsView();
    updateRecentTags();
    setFocusToLastSelectedWidget();

    if (d->currInfos.count() > 1000)
    {
        QApplication::restoreOverrideCursor();
    }
}

void ItemDescEditTab::slotReadFromFileMetadataToDatabase()
{
    initProgressIndicator();

    emit signalProgressMessageChanged(i18n("Reading metadata from files. Please wait..."));

    d->ignoreItemAttributesWatch = true;
    int i                        = 0;

    ScanController::instance()->suspendCollectionScan();

    CollectionScanner scanner;

    foreach (const ItemInfo& info, d->currInfos)
    {
        scanner.scanFile(info, CollectionScanner::Rescan);

        emit signalProgressValueChanged(i++/(float)d->currInfos.count());

        qApp->processEvents();
    }

    ScanController::instance()->resumeCollectionScan();
    d->ignoreItemAttributesWatch = false;

    emit signalProgressFinished();

    // reload everything
    setInfos(d->currInfos);
}

void ItemDescEditTab::slotWriteToFileMetadataFromDatabase()
{
    initProgressIndicator();

    emit signalProgressMessageChanged(i18n("Writing metadata to files. Please wait..."));

    int i = 0;

    foreach (const ItemInfo& info, d->currInfos)
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info.filePath());

        emit signalProgressValueChanged(i++ / (float)d->currInfos.count());
        qApp->processEvents();
    }

    emit signalProgressFinished();
}

bool ItemDescEditTab::eventFilter(QObject* o, QEvent* e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent* const k = static_cast<QKeyEvent*>(e);

        if (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return)
        {
            if (k->modifiers() == Qt::ControlModifier)
            {
                d->lastSelectedWidget = qobject_cast<QWidget*>(o);
                emit signalNextItem();
                return true;
            }
            else if (k->modifiers() == Qt::ShiftModifier)
            {
                d->lastSelectedWidget = qobject_cast<QWidget*>(o);
                emit signalPrevItem();
                return true;
            }
        }

        if (k->key() == Qt::Key_PageUp)
        {
            d->lastSelectedWidget = qobject_cast<QWidget*>(o);
            emit signalPrevItem();
            return true;
        }

        if (k->key() == Qt::Key_PageDown)
        {
            d->lastSelectedWidget = qobject_cast<QWidget*>(o);
            emit signalNextItem();
            return true;
        }
    }

    return DVBox::eventFilter(o, e);
}

void ItemDescEditTab::populateTags()
{
    // TODO update, this wont work... crashes
    //KConfigGroup group;
    //d->tagCheckView->loadViewState(group);
}

void ItemDescEditTab::slotTagStateChanged(Album* album, Qt::CheckState checkState)
{
    TAlbum* const tag = dynamic_cast<TAlbum*>(album);

    if (!tag || d->ignoreTagChanges)
    {
        return;
    }

    switch (checkState)
    {
        case Qt::Checked:
            d->hub.setTag(tag->id());
            break;
        default:
            d->hub.setTag(tag->id(), DisjointMetadata::MetadataInvalid);
            break;
    }

    slotModified();
}

void ItemDescEditTab::slotCommentChanged()
{
    d->hub.setComments(d->captionsEdit->values());
    setMetadataWidgetStatus(d->hub.commentsStatus(), d->captionsEdit);
    slotModified();
}

void ItemDescEditTab::slotTitleChanged()
{
    CaptionsMap titles;

    titles.fromAltLangMap(d->titleEdit->values());
    d->hub.setTitles(titles);
    setMetadataWidgetStatus(d->hub.titlesStatus(), d->titleEdit);
    slotModified();
}

void ItemDescEditTab::slotDateTimeChanged(const QDateTime& dateTime)
{
    d->hub.setDateTime(dateTime);
    setMetadataWidgetStatus(d->hub.dateTimeStatus(), d->dateTimeEdit);
    slotModified();
}

void ItemDescEditTab::slotTemplateSelected()
{
    d->hub.setMetadataTemplate(d->templateSelector->getTemplate());
    d->templateViewer->setTemplate(d->templateSelector->getTemplate());
    setMetadataWidgetStatus(d->hub.templateStatus(), d->templateSelector);
    slotModified();
}

void ItemDescEditTab::slotPickLabelChanged(int pickId)
{
    d->hub.setPickLabel(pickId);
    // no handling for MetadataDisjoint needed for pick label,
    // we set it to 0 when disjoint, see below
    slotModified();
}

void ItemDescEditTab::slotColorLabelChanged(int colorId)
{
    d->hub.setColorLabel(colorId);
    // no handling for MetadataDisjoint needed for color label,
    // we set it to 0 when disjoint, see below
    slotModified();
}

void ItemDescEditTab::slotRatingChanged(int rating)
{
    d->hub.setRating(rating);
    // no handling for MetadataDisjoint needed for rating,
    // we set it to 0 when disjoint, see below
    slotModified();
}

void ItemDescEditTab::slotModified()
{
    d->modified = true;
    d->applyBtn->setEnabled(true);
    d->revertBtn->setEnabled(true);

    if (d->currInfos.size() == 1)
    {
        d->applyToAllVersionsButton->setEnabled(true);
    }
}

void ItemDescEditTab::slotCreateNewTag()
{
    if (d->newTagEdit->text().isEmpty())
    {
        return;
    }

    TAlbum* const created = d->tagCheckView->tagModificationHelper()->
                            slotTagNew(d->tagCheckView->currentAlbum(), d->newTagEdit->text());

    if (created)
    {
        //d->tagCheckView->slotSelectAlbum(created);
        d->newTagEdit->clear();
    }
}

void ItemDescEditTab::slotTaggingActionActivated(const TaggingAction& action)
{
    TAlbum* assigned = 0;

    if (action.shallAssignTag())
    {
        assigned = AlbumManager::instance()->findTAlbum(action.tagId());

        if (assigned)
        {
            d->tagModel->setChecked(assigned, true);
        }
    }
    else if (action.shallCreateNewTag())
    {
        TAlbum* const parent = AlbumManager::instance()->findTAlbum(action.parentTagId());
        // tag is assigned automatically
        assigned = d->tagCheckView->tagModificationHelper()->slotTagNew(parent, action.newTagName());
    }

    if (assigned)
    {
        d->tagCheckView->scrollTo(d->tagCheckView->albumFilterModel()->indexForAlbum(assigned));
        QTimer::singleShot(0, d->newTagEdit, SLOT(clear()));
    }
}

void ItemDescEditTab::assignPickLabel(int pickId)
{
    d->pickLabelSelector->setPickLabel((PickLabel)pickId);
}

void ItemDescEditTab::assignColorLabel(int colorId)
{
    d->colorLabelSelector->setColorLabel((ColorLabel)colorId);
}

void ItemDescEditTab::assignRating(int rating)
{
    d->ratingWidget->setRating(rating);
}

void ItemDescEditTab::setTagState(TAlbum* const tag, DisjointMetadata::Status status)
{
    if (!tag)
    {
        return;
    }

    switch (status)
    {
        case DisjointMetadata::MetadataDisjoint:
            d->tagModel->setCheckState(tag, Qt::PartiallyChecked);
            break;
        case DisjointMetadata::MetadataAvailable:
            d->tagModel->setChecked(tag, true);
            break;
        case DisjointMetadata::MetadataInvalid:
            d->tagModel->setChecked(tag, false);
            break;
        default:
            qCWarning(DIGIKAM_GENERAL_LOG) << "Untreated tag status enum value " << status;
            d->tagModel->setCheckState(tag, Qt::PartiallyChecked);
            break;
    }
}

void ItemDescEditTab::updateTagsView()
{
    // avoid that the automatic tag toggling handles these calls and
    // modification is indicated to this widget
    TagCheckView::ToggleAutoTags toggle = d->tagCheckView->getToggleAutoTags();
    d->tagCheckView->setToggleAutoTags(TagCheckView::NoToggleAuto);
    d->ignoreTagChanges                 = true;

    // first reset the tags completely
    d->tagModel->resetAllCheckedAlbums();

    // then update checked state for all tags of the currently selected images
    const QMap<int, DisjointMetadata::Status> hubMap = d->hub.tags();

    for (QMap<int, DisjointMetadata::Status>::const_iterator it = hubMap.begin() ;
         it != hubMap.end() ; ++it)
    {
        TAlbum* tag = AlbumManager::instance()->findTAlbum(it.key());
        setTagState(tag, it.value());
    }

    d->ignoreTagChanges = false;
    d->tagCheckView->setToggleAutoTags(toggle);

    // The condition is a temporary fix not to destroy name filtering on image change.
    // See comments in these methods.
    if (d->assignedTagsBtn->isChecked())
    {
        slotAssignedTagsToggled(d->assignedTagsBtn->isChecked());
    }
}

void ItemDescEditTab::updateComments()
{
    d->captionsEdit->blockSignals(true);
    d->captionsEdit->setValues(d->hub.comments());
    setMetadataWidgetStatus(d->hub.commentsStatus(), d->captionsEdit);
    d->captionsEdit->blockSignals(false);

    d->titleEdit->blockSignals(true);
    d->titleEdit->setValues(d->hub.titles().toAltLangMap());
    setMetadataWidgetStatus(d->hub.titlesStatus(), d->titleEdit);
    d->titleEdit->blockSignals(false);
}

void ItemDescEditTab::updatePickLabel()
{
    d->pickLabelSelector->blockSignals(true);

    if (d->hub.pickLabelStatus() == DisjointMetadata::MetadataDisjoint)
    {
        d->pickLabelSelector->setPickLabel(NoPickLabel);
    }
    else
    {
        d->pickLabelSelector->setPickLabel((PickLabel)d->hub.pickLabel());
    }

    d->pickLabelSelector->blockSignals(false);
}

void ItemDescEditTab::updateColorLabel()
{
    d->colorLabelSelector->blockSignals(true);

    if (d->hub.colorLabelStatus() == DisjointMetadata::MetadataDisjoint)
    {
        d->colorLabelSelector->setColorLabel(NoColorLabel);
    }
    else
    {
        d->colorLabelSelector->setColorLabel((ColorLabel)d->hub.colorLabel());
    }

    d->colorLabelSelector->blockSignals(false);
}

void ItemDescEditTab::updateRating()
{
    d->ratingWidget->blockSignals(true);

    if (d->hub.ratingStatus() == DisjointMetadata::MetadataDisjoint)
    {
        d->ratingWidget->setRating(0);
    }
    else
    {
        d->ratingWidget->setRating(d->hub.rating());
    }

    d->ratingWidget->blockSignals(false);
}

void ItemDescEditTab::updateDate()
{
    d->dateTimeEdit->blockSignals(true);
    d->dateTimeEdit->setDateTime(d->hub.dateTime());
    setMetadataWidgetStatus(d->hub.dateTimeStatus(), d->dateTimeEdit);
    d->dateTimeEdit->blockSignals(false);
}

void ItemDescEditTab::updateTemplate()
{
    d->templateSelector->blockSignals(true);
    d->templateSelector->setTemplate(d->hub.metadataTemplate());
    d->templateViewer->setTemplate(d->hub.metadataTemplate());
    setMetadataWidgetStatus(d->hub.templateStatus(), d->templateSelector);
    d->templateSelector->blockSignals(false);
}

void ItemDescEditTab::setMetadataWidgetStatus(int status, QWidget* const widget)
{
    if (status == DisjointMetadata::MetadataDisjoint)
    {
        // For text widgets: Set text color to color of disabled text
        QPalette palette = widget->palette();
        palette.setColor(QPalette::Text, palette.color(QPalette::Disabled, QPalette::Text));
        widget->setPalette(palette);
    }
    else
    {
        widget->setPalette(QPalette());
    }
}

void ItemDescEditTab::slotMoreMenu()
{
    d->moreMenu->clear();

    if (singleSelection())
    {
        d->moreMenu->addAction(i18n("Read metadata from file to database"), this, SLOT(slotReadFromFileMetadataToDatabase()));
        QAction* const writeAction = d->moreMenu->addAction(i18n("Write metadata to each file"), this,
                                                            SLOT(slotWriteToFileMetadataFromDatabase()));
        // we do not need a "Write to file" action here because the apply button will do just that
        // if selection is a single file.
        // Adding the option will confuse users: Does the apply button not write to file?
        // Removing the option will confuse users: There is not option to write to file! (not visible in single selection)
        // Disabling will confuse users: Why is it disabled?
        writeAction->setEnabled(false);
    }
    else
    {
        // We need to make clear that this action is different from the Apply button,
        // which saves the same changes to all files. These batch operations operate on each single file.
        d->moreMenu->addAction(i18n("Read metadata from each file to database"), this, SLOT(slotReadFromFileMetadataToDatabase()));
        d->moreMenu->addAction(i18n("Write metadata to each file"), this, SLOT(slotWriteToFileMetadataFromDatabase()));
    }
}

void ItemDescEditTab::slotOpenTagsManager()
{
    TagsManager* const tagMngr = TagsManager::instance();
    tagMngr->show();
    tagMngr->activateWindow();
    tagMngr->raise();
}

void ItemDescEditTab::slotImagesChanged(int albumId)
{
    if (d->ignoreItemAttributesWatch || d->modified)
    {
        return;
    }

    Album* const a = AlbumManager::instance()->findAlbum(albumId);

    if (d->currInfos.isEmpty() || !a || a->isRoot() || a->type() != Album::TAG)
    {
        return;
    }

    setInfos(d->currInfos);
}

void ItemDescEditTab::slotImageTagsChanged(qlonglong imageId)
{
    metadataChange(imageId);
}

void ItemDescEditTab::slotImageRatingChanged(qlonglong imageId)
{
    metadataChange(imageId);
}

void ItemDescEditTab::slotImageCaptionChanged(qlonglong imageId)
{
    metadataChange(imageId);
}

void ItemDescEditTab::slotImageDateChanged(qlonglong imageId)
{
    metadataChange(imageId);
}

// private common code for above methods
void ItemDescEditTab::metadataChange(qlonglong imageId)
{
    if (d->ignoreItemAttributesWatch || d->modified)
    {
        // Don't lose modifications
        return;
    }

    d->metadataChangeIds << imageId;
    d->metadataChangeTimer->start();
}

void ItemDescEditTab::resetMetadataChangeInfo()
{
    d->metadataChangeTimer->stop();
    d->metadataChangeIds.clear();
}

void ItemDescEditTab::slotReloadForMetadataChange()
{
    // NOTE: What to do if d->modified? Reloading is no option.
    // It may be a little change the user wants to ignore, or a large conflict.
    if (d->currInfos.isEmpty() || d->modified)
    {
        resetMetadataChangeInfo();
        return;
    }

    if (singleSelection())
    {
        if (d->metadataChangeIds.contains(d->currInfos.first().id()))
        {
            setInfos(d->currInfos);
        }
    }
    else
    {
        // if image id is in our list, update
        foreach (const ItemInfo& info, d->currInfos)
        {
            if (d->metadataChangeIds.contains(info.id()))
            {
                setInfos(d->currInfos);
                break;
            }
        }
    }
}

void ItemDescEditTab::updateRecentTags()
{
    QMenu* const menu = dynamic_cast<QMenu*>(d->recentTagsBtn->menu());

    if (!menu)
    {
        return;
    }

    menu->clear();

    AlbumList recentTags = AlbumManager::instance()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        QAction* const noTagsAction = menu->addAction(i18n("No Recently Assigned Tags"));
        noTagsAction->setEnabled(false);
    }
    else
    {
        for (AlbumList::const_iterator it = recentTags.constBegin() ;
             it != recentTags.constEnd() ; ++it)
        {
            TAlbum* const album = static_cast<TAlbum*>(*it);

            if (album)
            {
                AlbumThumbnailLoader* const loader = AlbumThumbnailLoader::instance();
                QPixmap                     icon;

                if (!loader->getTagThumbnail(album, icon))
                {
                    if (icon.isNull())
                    {
                        icon = loader->getStandardTagIcon(album, AlbumThumbnailLoader::SmallerSize);
                    }
                }

                TAlbum* const parent = dynamic_cast<TAlbum*> (album->parent());

                if (parent)
                {
                    QString text          = album->title() + QLatin1String(" (") + parent->prettyUrl() + QLatin1Char(')');
                    QAction* const action = menu->addAction(icon, text, d->recentTagsMapper, SLOT(map()));
                    d->recentTagsMapper->setMapping(action, album->id());
                }
                else
                {
                    qCDebug(DIGIKAM_GENERAL_LOG) << "Tag" << album
                                                 << "do not have a valid parent";
                }
            }
        }
    }
}

void ItemDescEditTab::slotRecentTagsMenuActivated(int id)
{
    AlbumManager* const albumMan = AlbumManager::instance();

    if (id > 0)
    {
        TAlbum* const album = albumMan->findTAlbum(id);

        if (album)
        {
            d->tagModel->setChecked(album, true);
        }
    }
}

void ItemDescEditTab::slotTagsSearchChanged(const SearchTextSettings& settings)
{
    Q_UNUSED(settings);

    // if we filter, we should reset the assignedTagsBtn again
    if (d->assignedTagsBtn->isChecked() && !d->togglingSearchSettings)
    {
        d->togglingSearchSettings = true;
        d->assignedTagsBtn->setChecked(false);
        d->togglingSearchSettings = false;
    }
}

void ItemDescEditTab::slotAssignedTagsToggled(bool t)
{
    d->tagCheckView->checkableAlbumFilterModel()->setFilterChecked(t);
    d->tagCheckView->checkableAlbumFilterModel()->setFilterPartiallyChecked(t);
    d->tagCheckView->checkableAlbumFilterModel()->setFilterBehavior(t ? AlbumFilterModel::StrictFiltering
                                                                      : AlbumFilterModel::FullFiltering);

    if (t)
    {
        // if we filter by assigned, we should initially clear the normal search
        if (!d->togglingSearchSettings)
        {
            d->togglingSearchSettings = true;
            d->tagsSearchBar->clear();
            d->togglingSearchSettings = false;
        }

        // Only after above change, do this
        d->tagCheckView->expandMatches(d->tagCheckView->rootIndex());
   }
}

void ItemDescEditTab::slotApplyChangesToAllVersions()
{
    if (!d->modified)
    {
        return;
    }

    if (d->currInfos.isEmpty())
    {
        return;
    }

    QSet<qlonglong>                     tmpSet;
    QList<QPair<qlonglong, qlonglong> > relations;

    foreach (const ItemInfo& info, d->currInfos)
    {
        // Collect all ids in all image's relations
        relations.append(info.relationCloud());
    }

    if (relations.isEmpty())
    {
        slotApplyAllChanges();
        return;
    }

    for (int i = 0 ; i < relations.size() ; ++i)
    {
        // Use QSet to prevent duplicates
        tmpSet.insert(relations.at(i).first);
        tmpSet.insert(relations.at(i).second);
    }

    FileActionMngr::instance()->applyMetadata(ItemInfoList(tmpSet.toList()), d->hub);

    d->modified = false;
    d->hub.resetChanged();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);
    d->applyToAllVersionsButton->setEnabled(false);
}

void ItemDescEditTab::initProgressIndicator()
{
    if (!ProgressManager::instance()->findItembyId(QLatin1String("ItemDescEditTabProgress")))
    {
        FileActionProgress* const item = new FileActionProgress(QLatin1String("ItemDescEditTabProgress"));

        connect(this, SIGNAL(signalProgressMessageChanged(QString)),
                item, SLOT(slotProgressStatus(QString)));

        connect(this, SIGNAL(signalProgressValueChanged(float)),
                item, SLOT(slotProgressValue(float)));

        connect(this, SIGNAL(signalProgressFinished()),
                item, SLOT(slotCompleted()));
    }
}

AddTagsLineEdit* ItemDescEditTab::getNewTagEdit() const
{
    return d->newTagEdit;
}

} // namespace Digikam
