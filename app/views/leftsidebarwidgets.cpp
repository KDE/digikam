/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : left sidebar widgets
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "leftsidebarwidgets.h"

// Qt includes

#include <QButtonGroup>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QRadioButton>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albummodificationhelper.h"
#include "albumselectiontreeview.h"
#include "applicationsettings.h"
#include "datefolderview.h"
#include "editablesearchtreeview.h"
#include "fuzzysearchview.h"
#include "searchfolderview.h"
#include "searchtabheader.h"
#include "searchtextbar.h"
#include "coredbsearchxml.h"
#include "tagfolderview.h"
#include "timelinewidget.h"
#include "facescandialog.h"
#include "facesdetector.h"
#include "tagsmanager.h"
#include "albumlabelstreeview.h"
#include "coredb.h"
#include "dexpanderbox.h"

namespace Digikam
{

class AlbumFolderViewSideBarWidget::Private
{
public:

    Private() :
        albumModificationHelper(0),
        albumFolderView(0),
        searchTextBar(0)
    {
    }

    AlbumModificationHelper* albumModificationHelper;
    AlbumSelectionTreeView*  albumFolderView;
    SearchTextBar*           searchTextBar;
};

AlbumFolderViewSideBarWidget::AlbumFolderViewSideBarWidget(QWidget* const parent, AlbumModel* const model,
                                                           AlbumModificationHelper* const albumModificationHelper)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("AlbumFolderView Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F1);
    d->albumModificationHelper = albumModificationHelper;

    QVBoxLayout* const layout  = new QVBoxLayout(this);
    d->albumFolderView         = new AlbumSelectionTreeView(this, model, d->albumModificationHelper);
    d->albumFolderView->setObjectName(QLatin1String("AlbumFolderView"));
    d->albumFolderView->setConfigGroup(getConfigGroup());
    d->albumFolderView->setExpandNewCurrentItem(true);
    d->albumFolderView->setAlbumManagerCurrentAlbum(true);
    d->searchTextBar   = new SearchTextBar(this, QLatin1String("DigikamViewFolderSearchBar"));
    d->searchTextBar->setHighlightOnResult(true);
    d->searchTextBar->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchTextBar->setFilterModel(d->albumFolderView->albumFilterModel());

    layout->addWidget(d->albumFolderView);
    layout->addWidget(d->searchTextBar);

    // setup connection
    connect(d->albumFolderView, SIGNAL(signalFindDuplicates(PAlbum*)),
            this, SIGNAL(signalFindDuplicates(PAlbum*)));
}

AlbumFolderViewSideBarWidget::~AlbumFolderViewSideBarWidget()
{
    delete d;
}

void AlbumFolderViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->albumFolderView->currentAlbum());
    }
}

void AlbumFolderViewSideBarWidget::doLoadState()
{
    d->albumFolderView->loadState();
}

void AlbumFolderViewSideBarWidget::doSaveState()
{
    d->albumFolderView->saveState();
}

void AlbumFolderViewSideBarWidget::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();
    d->albumFolderView->setEnableToolTips(settings->getShowAlbumToolTips());
}

void AlbumFolderViewSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    d->albumFolderView->setCurrentAlbums(album);
}

AlbumPointer<PAlbum> AlbumFolderViewSideBarWidget::currentAlbum() const
{
    return AlbumPointer<PAlbum> (d->albumFolderView->currentAlbum());
}

void AlbumFolderViewSideBarWidget::setCurrentAlbum(PAlbum* album)
{
    // Change the current album in list view.
    d->albumFolderView->setCurrentAlbums(QList<Album*>() << album);
}

const QIcon AlbumFolderViewSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("folder-pictures"));
}

const QString AlbumFolderViewSideBarWidget::getCaption()
{
    return i18n("Albums");
}

// -----------------------------------------------------------------------------

class TagViewSideBarWidget::Private
{
public:

    enum TagsSource
    {
        NoTags = 0,
        ExistingTags
    };

    Private() :
        openTagMngr(0),
        tagSearchBar(0),
        tagFolderView(0),
        btnGroup(0),
        noTagsBtn(0),
        tagsBtn(0),
        noTagsWasChecked(false),
        ExistingTagsWasChecked(false)
    {
    }

    QPushButton*   openTagMngr;
    SearchTextBar* tagSearchBar;
    TagFolderView* tagFolderView;
    QButtonGroup*  btnGroup;
    QRadioButton*  noTagsBtn;
    QRadioButton*  tagsBtn;

    bool           noTagsWasChecked;
    bool           ExistingTagsWasChecked;

    QString        noTagsSearchXml;

    static const QString configTagsSourceEntry;
};

const QString TagViewSideBarWidget::Private::configTagsSourceEntry(QLatin1String("TagsSource"));

TagViewSideBarWidget::TagViewSideBarWidget(QWidget* const parent, TagModel* const model)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("TagView Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F2);

    QVBoxLayout* const layout = new QVBoxLayout(this);

    d->openTagMngr = new QPushButton( i18n("Open Tag Manager"));
    d->noTagsBtn   = new QRadioButton(i18n("No Tags"), this);
    d->tagsBtn     = new QRadioButton(i18n("Existing Tags"), this);
    d->btnGroup    = new QButtonGroup(this);
    d->btnGroup->addButton(d->noTagsBtn);
    d->btnGroup->addButton(d->tagsBtn);
    d->btnGroup->setId(d->noTagsBtn, 0);
    d->btnGroup->setId(d->tagsBtn, 1);
    d->btnGroup->setExclusive(true);

    d->tagFolderView = new TagFolderView(this, model);
    d->tagFolderView->setConfigGroup(getConfigGroup());
    d->tagFolderView->setExpandNewCurrentItem(true);
    d->tagFolderView->setAlbumManagerCurrentAlbum(true);

    d->tagSearchBar  = new SearchTextBar(this, QLatin1String("DigikamViewTagSearchBar"));
    d->tagSearchBar->setHighlightOnResult(true);
    d->tagSearchBar->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagFolderView->albumFilterModel());

    layout->addWidget(d->openTagMngr);
    layout->addWidget(d->noTagsBtn);
    layout->addWidget(d->tagsBtn);
    layout->addWidget(d->tagFolderView);
    layout->addWidget(d->tagSearchBar);

    connect(d->openTagMngr, SIGNAL(clicked()),
            this,SLOT(slotOpenTagManager()));

    connect(d->tagFolderView, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SIGNAL(signalFindDuplicates(QList<TAlbum*>)));

    connect(d->btnGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotToggleTagsSelection(int)));
}

TagViewSideBarWidget::~TagViewSideBarWidget()
{
    delete d;
}

void TagViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        if(d->noTagsBtn->isChecked())
        {
            setNoTagsAlbum();
        }
        else
        {
            AlbumManager::instance()->setCurrentAlbums(d->tagFolderView->selectedTags());
        }
    }
}

void TagViewSideBarWidget::doLoadState()
{
    KConfigGroup group = getConfigGroup();

    bool noTagsBtnWasChecked  = group.readEntry(d->configTagsSourceEntry, false);
    d->noTagsBtn->setChecked(noTagsBtnWasChecked);
    d->tagsBtn->setChecked(!noTagsBtnWasChecked);
    d->noTagsWasChecked       = noTagsBtnWasChecked;
    d->ExistingTagsWasChecked = !noTagsBtnWasChecked;

    d->tagFolderView->loadState();
    d->tagFolderView->setDisabled(noTagsBtnWasChecked);
}

void TagViewSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(d->configTagsSourceEntry, d->noTagsBtn->isChecked());

    d->tagFolderView->saveState();

    group.sync();
}

void TagViewSideBarWidget::applySettings()
{
}

void TagViewSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    if(album.first()->type() == Album::TAG)
    {
        d->tagsBtn->setChecked(true);
        d->tagFolderView->setEnabled(true);
        d->ExistingTagsWasChecked = true;
        d->noTagsWasChecked = false;
        d->tagFolderView->setCurrentAlbums(album);
    }
    else
    {
        d->noTagsBtn->setChecked(true);
        d->tagFolderView->setDisabled(true);
        d->noTagsWasChecked = true;
        d->ExistingTagsWasChecked = false;
    }
}

AlbumPointer<TAlbum> TagViewSideBarWidget::currentAlbum() const
{
    return AlbumPointer<TAlbum> (d->tagFolderView->currentAlbum());
}

void TagViewSideBarWidget::setNoTagsAlbum()
{
    if(d->noTagsSearchXml.isEmpty())
    {
        SearchXmlWriter writer;
        writer.setFieldOperator((SearchXml::standardFieldOperator()));
        writer.writeGroup();
        writer.writeField(QLatin1String("notag"), SearchXml::Equal);
        writer.finishField();
        writer.finishGroup();
        writer.finish();
        d->noTagsSearchXml = writer.xml();
    }

    QString title = SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch);
    SAlbum* album = AlbumManager::instance()->findSAlbum(title);

    int id;

    if (album)
    {
        id = album->id();
        CoreDbAccess().db()->updateSearch(id,DatabaseSearch::AdvancedSearch,
                                            SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), d->noTagsSearchXml);
    }
    else
    {
        id = CoreDbAccess().db()->addSearch(DatabaseSearch::AdvancedSearch,
                                              SAlbum::getTemporaryTitle(DatabaseSearch::AdvancedSearch), d->noTagsSearchXml);
    }

    album = new SAlbum(i18n("No Tags Album"), id);

    if (album)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << album);
    }
}

const QIcon TagViewSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("tag"));
}

const QString TagViewSideBarWidget::getCaption()
{
    return i18n("Tags");
}

void TagViewSideBarWidget::setCurrentAlbum(TAlbum* album)
{
    d->tagFolderView->setCurrentAlbums(QList<Album*>() << album);
}

void TagViewSideBarWidget::slotOpenTagManager()
{
    TagsManager* const tagMngr = TagsManager::instance();
    tagMngr->show();
    tagMngr->activateWindow();
    tagMngr->raise();
}

void TagViewSideBarWidget::slotToggleTagsSelection(int radioClicked)
{
    switch (Private::TagsSource(radioClicked))
    {
        case Private::NoTags:
        {
            if (!d->noTagsWasChecked)
            {
                setNoTagsAlbum();
                d->tagFolderView->setDisabled(true);
                d->noTagsWasChecked = d->noTagsBtn->isChecked();
                d->ExistingTagsWasChecked = d->tagsBtn->isChecked();
            }
            break;
        }
        case Private::ExistingTags:
        {
            if (!d->ExistingTagsWasChecked)
            {
                d->tagFolderView->setEnabled(true);
                setActive(true);
                d->noTagsWasChecked = d->noTagsBtn->isChecked();
                d->ExistingTagsWasChecked = d->tagsBtn->isChecked();
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------------

class LabelsSideBarWidget::Private
{

public:

    Private() :
        labelsTree(0)
    {
    }

    AlbumLabelsTreeView* labelsTree;
};

LabelsSideBarWidget::LabelsSideBarWidget(QWidget* const parent) :
    SidebarWidget(parent),
    d(new Private)
{
    setObjectName(QLatin1String("Labels Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F3);

    QVBoxLayout* const layout = new QVBoxLayout(this);

    d->labelsTree = new AlbumLabelsTreeView(this);
    d->labelsTree->setConfigGroup(getConfigGroup());

    layout->addWidget(d->labelsTree);
}

LabelsSideBarWidget::~LabelsSideBarWidget()
{
    delete d;
}

AlbumLabelsTreeView *LabelsSideBarWidget::labelsTree()
{
    return d->labelsTree;
}

void LabelsSideBarWidget::setActive(bool active)
{
    if (active)
    {
        d->labelsTree->setCurrentAlbum();
    }
}

void LabelsSideBarWidget::applySettings()
{
}

void LabelsSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    Q_UNUSED(album);
}

void LabelsSideBarWidget::doLoadState()
{
    d->labelsTree->doLoadState();
}

void LabelsSideBarWidget::doSaveState()
{
    d->labelsTree->doSaveState();
}

const QIcon LabelsSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("folder-favorites"));
}

const QString LabelsSideBarWidget::getCaption()
{
    return i18n("Labels");
}

QHash<AlbumLabelsTreeView::Labels, QList<int> > LabelsSideBarWidget::selectedLabels()
{
    return d->labelsTree->selectedLabels();
}

// -----------------------------------------------------------------------------

class DateFolderViewSideBarWidget::Private
{
public:

    Private() :
        dateFolderView(0)
    {
    }

    DateFolderView* dateFolderView;
};

DateFolderViewSideBarWidget::DateFolderViewSideBarWidget(QWidget* const parent, DateAlbumModel* const model,
                                                         ImageAlbumFilterModel* const imageFilterModel)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("DateFolderView Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F4);

    QVBoxLayout* const layout = new QVBoxLayout(this);

    d->dateFolderView         = new DateFolderView(this, model);
    d->dateFolderView->setConfigGroup(getConfigGroup());
    d->dateFolderView->setImageModel(imageFilterModel);

    layout->addWidget(d->dateFolderView);
}

DateFolderViewSideBarWidget::~DateFolderViewSideBarWidget()
{
    delete d;
}

void DateFolderViewSideBarWidget::setActive(bool active)
{
    d->dateFolderView->setActive(active);
}

void DateFolderViewSideBarWidget::doLoadState()
{
    d->dateFolderView->loadState();
}

void DateFolderViewSideBarWidget::doSaveState()
{
    d->dateFolderView->saveState();
}

void DateFolderViewSideBarWidget::applySettings()
{
}

void DateFolderViewSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    d->dateFolderView->changeAlbumFromHistory(dynamic_cast<DAlbum*>(album.first()));
}

AlbumPointer<DAlbum> DateFolderViewSideBarWidget::currentAlbum() const
{
    return d->dateFolderView->currentAlbum();
}

void DateFolderViewSideBarWidget::gotoDate(const QDate& date)
{
    d->dateFolderView->gotoDate(date);
}

const QIcon DateFolderViewSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("view-calendar-list"));
}

const QString DateFolderViewSideBarWidget::getCaption()
{
    return i18n("Dates");
}

// -----------------------------------------------------------------------------

class TimelineSideBarWidget::Private
{
public:

    Private() :
        scaleBG(0),
        cursorCountLabel(0),
        scrollBar(0),
        timer(0),
        resetButton(0),
        saveButton(0),
        timeUnitCB(0),
        nameEdit(0),
        cursorDateLabel(0),
        searchDateBar(0),
        timeLineFolderView(0),
        timeLineWidget(0),
        searchModificationHelper(0)
    {
    }

    static const QString      configHistogramTimeUnitEntry;
    static const QString      configHistogramScaleEntry;
    static const QString      configCursorPositionEntry;

    QButtonGroup*             scaleBG;
    QLabel*                   cursorCountLabel;
    QScrollBar*               scrollBar;
    QTimer*                   timer;
    QToolButton*              resetButton;
    QToolButton*              saveButton;

    QComboBox*                timeUnitCB;
    QLineEdit*                nameEdit;
    DAdjustableLabel*       cursorDateLabel;

    SearchTextBar*            searchDateBar;
    EditableSearchTreeView*   timeLineFolderView;
    TimeLineWidget*           timeLineWidget;

    SearchModificationHelper* searchModificationHelper;

    AlbumPointer<SAlbum>      currentTimelineSearch;
};

const QString TimelineSideBarWidget::Private::configHistogramTimeUnitEntry(QLatin1String("Histogram TimeUnit"));
const QString TimelineSideBarWidget::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));
const QString TimelineSideBarWidget::Private::configCursorPositionEntry(QLatin1String("Cursor Position"));

// --------------------------------------------------------

TimelineSideBarWidget::TimelineSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                                             SearchModificationHelper* const searchModificationHelper)
    : SidebarWidget(parent), d(new Private)
{
    setObjectName(QLatin1String("TimeLine Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F5);

    d->searchModificationHelper = searchModificationHelper;
    d->timer                    = new QTimer(this);
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QVBoxLayout* const vlay = new QVBoxLayout(this);
    QFrame* const panel     = new QFrame(this);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    panel->setLineWidth(1);

    QGridLayout* const grid = new QGridLayout(panel);

    // ---------------------------------------------------------------

    QWidget* const hbox1    = new QWidget(panel);
    QHBoxLayout* const hlay = new QHBoxLayout(hbox1);

    QLabel* const label1    = new QLabel(i18n("Time Unit:"), hbox1);
    d->timeUnitCB           = new QComboBox(hbox1);
    d->timeUnitCB->addItem(i18n("Day"),   TimeLineWidget::Day);
    d->timeUnitCB->addItem(i18n("Week"),  TimeLineWidget::Week);
    d->timeUnitCB->addItem(i18n("Month"), TimeLineWidget::Month);
    d->timeUnitCB->addItem(i18n("Year"),  TimeLineWidget::Year);
    d->timeUnitCB->setCurrentIndex((int)TimeLineWidget::Month);
    d->timeUnitCB->setFocusPolicy(Qt::NoFocus);
    d->timeUnitCB->setWhatsThis(i18n("<p>Select the histogram time unit.</p>"
                                     "<p>You can change the graph decade to zoom in or zoom out over time.</p>"));

    QWidget* const scaleBox  = new QWidget(hbox1);
    QHBoxLayout* const hlay2 = new QHBoxLayout(scaleBox);
    d->scaleBG               = new QButtonGroup(scaleBox);
    d->scaleBG->setExclusive(true);
    scaleBox->setWhatsThis( i18n("<p>Select the histogram scale.</p>"
                                 "<p>If the date's maximal counts are small, you can use the linear scale.</p>"
                                 "<p>Logarithmic scale can be used when the maximal counts are big; "
                                 "if it is used, all values (small and large) will be visible on the "
                                 "graph.</p>"));

    QToolButton* const linHistoButton = new QToolButton(scaleBox);
    linHistoButton->setToolTip( i18n( "Linear" ) );
    linHistoButton->setIcon(QIcon::fromTheme(QLatin1String("view-object-histogram-linear")));
    linHistoButton->setCheckable(true);
    d->scaleBG->addButton(linHistoButton, TimeLineWidget::LinScale);

    QToolButton* const logHistoButton = new QToolButton(scaleBox);
    logHistoButton->setToolTip( i18n( "Logarithmic" ) );
    logHistoButton->setIcon(QIcon::fromTheme(QLatin1String("view-object-histogram-logarithmic")));
    logHistoButton->setCheckable(true);
    d->scaleBG->addButton(logHistoButton, TimeLineWidget::LogScale);

    hlay2->setContentsMargins(QMargins());
    hlay2->setSpacing(0);
    hlay2->addWidget(linHistoButton);
    hlay2->addWidget(logHistoButton);

    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(spacing);
    hlay->addWidget(label1);
    hlay->addWidget(d->timeUnitCB);
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hlay->addWidget(scaleBox);

    // ---------------------------------------------------------------

    d->timeLineWidget = new TimeLineWidget(panel);
    d->scrollBar      = new QScrollBar(panel);
    d->scrollBar->setOrientation(Qt::Horizontal);
    d->scrollBar->setMinimum(0);
    d->scrollBar->setSingleStep(1);

    d->cursorDateLabel  = new DAdjustableLabel(panel);
    d->cursorCountLabel = new QLabel(panel);
    d->cursorCountLabel->setAlignment(Qt::AlignRight);

    // ---------------------------------------------------------------

    DHBox* const hbox2 = new DHBox(panel);
    hbox2->setContentsMargins(QMargins());
    hbox2->setSpacing(spacing);

    d->resetButton = new QToolButton(hbox2);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18n("Clear current selection"));
    d->resetButton->setWhatsThis(i18n("If you press this button, the current date selection on the time-line will be cleared."));
    d->nameEdit    = new QLineEdit(hbox2);
    d->nameEdit->setClearButtonEnabled(true);
    d->nameEdit->setWhatsThis(i18n("Enter the name of the current dates search to save in the "
                                   "\"Searches\" view"));

    d->saveButton  = new QToolButton(hbox2);
    d->saveButton->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    d->saveButton->setEnabled(false);
    d->saveButton->setToolTip(i18n("Save current selection to a new virtual Album"));
    d->saveButton->setWhatsThis(i18n("If you press this button, the dates selected on the time-line will be "
                                     "saved to a new search virtual Album using the name set on the left."));

    // ---------------------------------------------------------------

    grid->addWidget(hbox1,               0, 0, 1, 4);
    grid->addWidget(d->cursorDateLabel,  1, 0, 1, 3);
    grid->addWidget(d->cursorCountLabel, 1, 3, 1, 1);
    grid->addWidget(d->timeLineWidget,   2, 0, 1, 4);
    grid->addWidget(d->scrollBar,        3, 0, 1, 4);
    grid->addWidget(hbox2,               4, 0, 1, 4);
    grid->setColumnStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // ---------------------------------------------------------------

    d->timeLineFolderView = new EditableSearchTreeView(this, searchModel, searchModificationHelper);
    d->timeLineFolderView->setConfigGroup(getConfigGroup());
    d->timeLineFolderView->filteredModel()->listTimelineSearches();
    d->timeLineFolderView->filteredModel()->setListTemporarySearches(false);
    d->timeLineFolderView->setAlbumManagerCurrentAlbum(false);
    d->searchDateBar      = new SearchTextBar(this, QLatin1String("TimeLineViewSearchDateBar"));
    d->searchDateBar->setModel(d->timeLineFolderView->filteredModel(),
                               AbstractAlbumModel::AlbumIdRole,
                               AbstractAlbumModel::AlbumTitleRole);
    d->searchDateBar->setFilterModel(d->timeLineFolderView->albumFilterModel());

    vlay->addWidget(panel);
    vlay->addWidget(d->timeLineFolderView);
    vlay->addItem(new QSpacerItem(spacing, spacing, QSizePolicy::Minimum, QSizePolicy::Minimum));
    vlay->addWidget(d->searchDateBar);
    vlay->setContentsMargins(QMargins());
    vlay->setSpacing(0);

    // ---------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalDatesMapDirty(QMap<QDateTime,int>)),
            d->timeLineWidget, SLOT(slotDatesMap(QMap<QDateTime,int>)));

    connect(d->timeLineFolderView, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->timeUnitCB, SIGNAL(activated(int)),
            this, SLOT(slotTimeUnitChanged(int)));

    connect(d->scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotInit()));

    connect(d->timeLineWidget, SIGNAL(signalCursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalRefDateTimeChanged()),
            this, SLOT(slotRefDateTimeChanged()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotUpdateCurrentDateSearchAlbum()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetSelection()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveSelection()));

    connect(d->scrollBar, SIGNAL(valueChanged(int)),
            this, SLOT(slotScrollBarValueChanged(int)));

    connect(d->nameEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotCheckAboutSelection()));

    connect(d->nameEdit, SIGNAL(returnPressed()),
            d->saveButton, SLOT(animateClick()));
}

TimelineSideBarWidget::~TimelineSideBarWidget()
{
    delete d;
}

void TimelineSideBarWidget::slotInit()
{
    // Date Maps are loaded from AlbumManager to TimeLineWidget after than GUI is initialized.
    // AlbumManager query Date KIO slave to stats items from database and it can take a while.
    // We waiting than TimeLineWidget is ready before to set last config from users.

    loadState();

    disconnect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
               this, SLOT(slotInit()));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotCursorPositionChanged()));
}

void TimelineSideBarWidget::setActive(bool active)
{
    if (active)
    {
        if (!d->currentTimelineSearch)
        {
            d->currentTimelineSearch = d->timeLineFolderView->currentAlbum();
        }
        if (d->currentTimelineSearch)
        {
            AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->currentTimelineSearch);
        }
        else
        {
            slotUpdateCurrentDateSearchAlbum();
        }
    }
}

void TimelineSideBarWidget::doLoadState()
{

    KConfigGroup group = getConfigGroup();

    d->timeUnitCB->setCurrentIndex(group.readEntry(d->configHistogramTimeUnitEntry, (int)TimeLineWidget::Month));
    slotTimeUnitChanged(d->timeUnitCB->currentIndex());

    int id = group.readEntry(d->configHistogramScaleEntry, (int)TimeLineWidget::LinScale);

    if ( d->scaleBG->button( id ) )
    {
        d->scaleBG->button( id )->setChecked(true);
    }

    slotScaleChanged(d->scaleBG->checkedId());

    QDateTime now = QDateTime::currentDateTime();
    d->timeLineWidget->setCursorDateTime(group.readEntry(d->configCursorPositionEntry, now));
    d->timeLineWidget->setCurrentIndex(d->timeLineWidget->indexForCursorDateTime());

    d->timeLineFolderView->loadState();
}

void TimelineSideBarWidget::doSaveState()
{
    KConfigGroup group = getConfigGroup();

    group.writeEntry(d->configHistogramTimeUnitEntry, d->timeUnitCB->currentIndex());
    group.writeEntry(d->configHistogramScaleEntry,    d->scaleBG->checkedId());
    group.writeEntry(d->configCursorPositionEntry,    d->timeLineWidget->cursorDateTime());

    d->timeLineFolderView->saveState();

    group.sync();
}

void TimelineSideBarWidget::applySettings()
{
    // nothing to do here right now
}

void TimelineSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    d->timeLineFolderView->setCurrentAlbums(album);
}

const QIcon TimelineSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("player-time"));
}

const QString TimelineSideBarWidget::getCaption()
{
    return i18n("Timeline");
}

void TimelineSideBarWidget::slotRefDateTimeChanged()
{
    d->scrollBar->blockSignals(true);
    d->scrollBar->setMaximum(d->timeLineWidget->totalIndex()-1);
    d->scrollBar->setValue(d->timeLineWidget->indexForRefDateTime()-1);
    d->scrollBar->blockSignals(false);
}

void TimelineSideBarWidget::slotTimeUnitChanged(int mode)
{
    d->timeLineWidget->setTimeUnit((TimeLineWidget::TimeUnit)mode);
}

void TimelineSideBarWidget::slotScrollBarValueChanged(int val)
{
    d->timeLineWidget->setCurrentIndex(val);
}

void TimelineSideBarWidget::slotScaleChanged(int mode)
{
    d->timeLineWidget->setScaleMode((TimeLineWidget::ScaleMode)mode);
}

void TimelineSideBarWidget::slotCursorPositionChanged()
{
    QString txt;
    int val = d->timeLineWidget->cursorInfo(txt);
    d->cursorDateLabel->setAdjustedText(txt);
    d->cursorCountLabel->setText((val == 0) ? i18n("no item") : i18np("1 item", "%1 items", val));
}

void TimelineSideBarWidget::slotSelectionChanged()
{
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

/** Called from d->timer event.*/
void TimelineSideBarWidget::slotUpdateCurrentDateSearchAlbum()
{
    slotCheckAboutSelection();
    int totalCount           = 0;
    DateRangeList dateRanges = d->timeLineWidget->selectedDateRange(totalCount);
    d->currentTimelineSearch = d->searchModificationHelper->
        slotCreateTimeLineSearch(SAlbum::getTemporaryTitle(DatabaseSearch::TimeLineSearch), dateRanges, true);
    d->timeLineFolderView->setCurrentAlbum(0); // "temporary" search is not listed in view
}

void TimelineSideBarWidget::slotSaveSelection()
{
    QString name             = d->nameEdit->text();
    int totalCount           = 0;
    DateRangeList dateRanges = d->timeLineWidget->selectedDateRange(totalCount);
    d->currentTimelineSearch = d->searchModificationHelper->slotCreateTimeLineSearch(name, dateRanges);
}

void TimelineSideBarWidget::slotAlbumSelected(Album* album)
{
    if (d->currentTimelineSearch == album)
    {
        return;
    }

    SAlbum* const salbum = dynamic_cast<SAlbum*>(album);

    if (!salbum)
    {
        return;
    }

    d->currentTimelineSearch = salbum;
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << salbum);

    SearchXmlReader reader(salbum->query());

    // The timeline query consists of groups, with two date time fields each
    DateRangeList list;

    while (!reader.atEnd())
    {
        // read groups
        if (reader.readNext() == SearchXml::Group)
        {
            QDateTime start, end;
            int numberOfFields = 0;

            while (!reader.atEnd())
            {
                // read fields
                reader.readNext();

                if (reader.isEndElement())
                {
                    break;
                }

                if (reader.isFieldElement())
                {
                    if (numberOfFields == 0)
                    {
                        start = reader.valueToDateTime();
                    }
                    else if (numberOfFields == 1)
                    {
                        end = reader.valueToDateTime();
                    }

                    ++numberOfFields;
                }
            }

            if (numberOfFields)
            {
                list << DateRange(start, end);
            }
        }
    }

    d->timeLineWidget->setSelectedDateRange(list);
}

void TimelineSideBarWidget::slotResetSelection()
{
    d->timeLineWidget->slotResetSelection();
    slotCheckAboutSelection();
    AlbumManager::instance()->setCurrentAlbums(QList<Album*>());
}

void TimelineSideBarWidget::slotCheckAboutSelection()
{
    int totalCount     = 0;
    DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);

    if (!list.isEmpty())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
        {
            d->saveButton->setEnabled(true);
        }
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveButton->setEnabled(false);
    }
}

// -----------------------------------------------------------------------------

class SearchSideBarWidget::Private
{
public:

    Private() :
        searchSearchBar(0),
        searchTreeView(0),
        searchTabHeader(0)
    {
    }

    SearchTextBar*        searchSearchBar;
    NormalSearchTreeView* searchTreeView;
    SearchTabHeader*      searchTabHeader;
};

SearchSideBarWidget::SearchSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                                         SearchModificationHelper* const searchModificationHelper)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("Search Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F6);

    QVBoxLayout* const layout = new QVBoxLayout(this);

    d->searchTabHeader  = new SearchTabHeader(this);
    d->searchTreeView   = new NormalSearchTreeView(this, searchModel, searchModificationHelper);
    d->searchTreeView->setConfigGroup(getConfigGroup());
    d->searchTreeView->filteredModel()->listNormalSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(true);
    d->searchTreeView->setAlbumManagerCurrentAlbum(true);
    d->searchSearchBar  = new SearchTextBar(this, QLatin1String("DigikamViewSearchSearchBar"));
    d->searchSearchBar->setModel(d->searchTreeView->filteredModel(),
                                 AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchSearchBar->setFilterModel(d->searchTreeView->albumFilterModel());

    layout->addWidget(d->searchTabHeader);
    layout->addWidget(d->searchTreeView);
    layout->setStretchFactor(d->searchTreeView, 1);
    layout->addWidget(d->searchSearchBar);

    connect(d->searchTreeView, SIGNAL(newSearch()),
            d->searchTabHeader, SLOT(newAdvancedSearch()));

    connect(d->searchTreeView, SIGNAL(editSearch(SAlbum*)),
            d->searchTabHeader, SLOT(editSearch(SAlbum*)));

    connect(d->searchTreeView, SIGNAL(currentAlbumChanged(Album*)),
            d->searchTabHeader, SLOT(selectedSearchChanged(Album*)));

    connect(d->searchTabHeader, SIGNAL(searchShallBeSelected(QList<Album*>)),
            d->searchTreeView, SLOT(setCurrentAlbums(QList<Album*>)));
}

SearchSideBarWidget::~SearchSideBarWidget()
{
    delete d;
}

void SearchSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->searchTreeView->currentAlbum());
    }
}

void SearchSideBarWidget::doLoadState()
{
    d->searchTreeView->loadState();
}

void SearchSideBarWidget::doSaveState()
{
    d->searchTreeView->saveState();
}

void SearchSideBarWidget::applySettings()
{
}

void SearchSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    d->searchTreeView->setCurrentAlbums(album);
}

const QIcon SearchSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("edit-find"));
}

const QString SearchSideBarWidget::getCaption()
{
    return i18nc("Avanced search images, access stored searches", "Search");
}

void SearchSideBarWidget::newKeywordSearch()
{
    d->searchTabHeader->newKeywordSearch();
}

void SearchSideBarWidget::newAdvancedSearch()
{
    d->searchTabHeader->newAdvancedSearch();
}

// -----------------------------------------------------------------------------

class FuzzySearchSideBarWidget::Private
{
public:
    Private() :
        fuzzySearchView(0),
        searchModificationHelper(0)
    {
    }

    FuzzySearchView*          fuzzySearchView;
    SearchModificationHelper* searchModificationHelper;
};

FuzzySearchSideBarWidget::FuzzySearchSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                                                   SearchModificationHelper* const searchModificationHelper)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("Fuzzy Search Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F7);

    d->fuzzySearchView        = new FuzzySearchView(searchModel, searchModificationHelper, this);
    d->fuzzySearchView->setConfigGroup(getConfigGroup());

    QVBoxLayout* const layout = new QVBoxLayout(this);

    layout->addWidget(d->fuzzySearchView);
}

FuzzySearchSideBarWidget::~FuzzySearchSideBarWidget()
{
    delete d;
}

void FuzzySearchSideBarWidget::setActive(bool active)
{
    d->fuzzySearchView->setActive(active);

    if (active)
    {
        AlbumManager::instance()->setCurrentAlbums(QList<Album*>() << d->fuzzySearchView->currentAlbum());
    }
    emit signalActive(active);
}

void FuzzySearchSideBarWidget::doLoadState()
{
    d->fuzzySearchView->loadState();
}

void FuzzySearchSideBarWidget::doSaveState()
{
    d->fuzzySearchView->saveState();
}

void FuzzySearchSideBarWidget::applySettings()
{
}

void FuzzySearchSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    SAlbum* const salbum = dynamic_cast<SAlbum*>(album.first());
    d->fuzzySearchView->setCurrentAlbum(salbum);
}

const QIcon FuzzySearchSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("tools-wizard"));
}

const QString FuzzySearchSideBarWidget::getCaption()
{
    return i18nc("Fuzzy Search images, as dupplicates, sketch, searches by similarities", "Similarity");
}

void FuzzySearchSideBarWidget::newDuplicatesSearch(PAlbum* album)
{
    d->fuzzySearchView->newDuplicatesSearch(album);
}

void FuzzySearchSideBarWidget::newDuplicatesSearch(QList<PAlbum*> albums)
{
    d->fuzzySearchView->newDuplicatesSearch(albums);
}

void FuzzySearchSideBarWidget::newDuplicatesSearch(QList<TAlbum*> albums)
{
    d->fuzzySearchView->newDuplicatesSearch(albums);
}

void FuzzySearchSideBarWidget::newSimilarSearch(const ImageInfo& imageInfo)
{
    if (imageInfo.isNull())
    {
        return;
    }

    d->fuzzySearchView->setImageInfo(imageInfo);
}

// -----------------------------------------------------------------------------

#ifdef HAVE_MARBLE

class GPSSearchSideBarWidget::Private
{
public:
    Private() :
        gpsSearchView(0)
    {
    }

    GPSSearchView* gpsSearchView;
};

GPSSearchSideBarWidget::GPSSearchSideBarWidget(QWidget* const parent, SearchModel* const searchModel,
                                               SearchModificationHelper* const searchModificationHelper,
                                               ImageFilterModel* const imageFilterModel,  QItemSelectionModel* const itemSelectionModel)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("GPS Search Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F8);

    d->gpsSearchView = new GPSSearchView(this, searchModel, searchModificationHelper, imageFilterModel, itemSelectionModel);
    d->gpsSearchView->setConfigGroup(getConfigGroup());

    QScrollArea* const scrollArea = new QScrollArea(this);
    QVBoxLayout* const layout     = new QVBoxLayout(this);

    layout->addWidget(scrollArea);
    scrollArea->setWidget(d->gpsSearchView);
    scrollArea->setWidgetResizable(true);

    connect(d->gpsSearchView, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)),
            this, SIGNAL(signalMapSoloItems(QList<qlonglong>,QString)));
}

GPSSearchSideBarWidget::~GPSSearchSideBarWidget()
{
    delete d;
}

void GPSSearchSideBarWidget::setActive(bool active)
{
    d->gpsSearchView->setActive(active);
}

void GPSSearchSideBarWidget::doLoadState()
{
    d->gpsSearchView->loadState();
}

void GPSSearchSideBarWidget::doSaveState()
{
    d->gpsSearchView->saveState();
}

void GPSSearchSideBarWidget::applySettings()
{
}

void GPSSearchSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    d->gpsSearchView->changeAlbumFromHistory(dynamic_cast<SAlbum*>(album.first()));
}

const QIcon GPSSearchSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("globe"));
}

const QString GPSSearchSideBarWidget::getCaption()
{
    return i18nc("Search images on a map", "Map");
}

#endif // HAVE_MARBLE

// -----------------------------------------------------------------------------

class PeopleSideBarWidget::Private : public TagViewSideBarWidget::Private
{
public:

    Private()
    {
        personIcon               = 0;
        textLabel                = 0;
        rescanButton             = 0;
        searchModificationHelper = 0;
    }

    QLabel*                   personIcon;
    QLabel*                   textLabel;
    QPushButton*              rescanButton;

    SearchModificationHelper* searchModificationHelper;
};

PeopleSideBarWidget::PeopleSideBarWidget(QWidget* const parent, TagModel* const model,
                                         SearchModificationHelper* const searchModificationHelper)
    : SidebarWidget(parent),
      d(new Private)
{
    setObjectName(QLatin1String("People Sidebar"));
    setProperty("Shortcut", Qt::META + Qt::CTRL + Qt::Key_F9);

    d->searchModificationHelper = searchModificationHelper;
    QVBoxLayout* const layout   = new QVBoxLayout;
    QHBoxLayout* const hlay     = new QHBoxLayout;
    d->tagFolderView            = new TagFolderView(this, model);
    d->tagFolderView->setConfigGroup(getConfigGroup());
    d->tagFolderView->setExpandNewCurrentItem(true);
    d->tagFolderView->setAlbumManagerCurrentAlbum(true);
    d->tagFolderView->setShowDeleteFaceTagsAction(true);

    d->tagFolderView->filteredModel()->listOnlyTagsWithProperty(TagPropertyName::person());
    d->tagFolderView->filteredModel()->setFilterBehavior(AlbumFilterModel::StrictFiltering);

    d->tagSearchBar   = new SearchTextBar(this, QLatin1String("DigikamViewPeopleSearchBar"));
    d->tagSearchBar->setHighlightOnResult(true);
    d->tagSearchBar->setModel(d->tagFolderView->filteredModel(),
                              AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagFolderView->albumFilterModel());

    d->rescanButton   = new QPushButton;
    d->rescanButton->setText(i18n("Scan collection for faces"));

    d->personIcon     = new QLabel;
    d->personIcon->setPixmap(QIcon::fromTheme(QLatin1String("edit-image-face-show")).pixmap(48));

    d->textLabel      = new QLabel(i18n("People Tags"));

    hlay->addWidget(d->personIcon);
    hlay->addWidget(d->textLabel);

    layout->addLayout(hlay);
    layout->addWidget(d->rescanButton);
    layout->addWidget(d->tagFolderView);
    layout->addWidget(d->tagSearchBar);

    setLayout(layout);

    connect(d->tagFolderView, SIGNAL(signalFindDuplicates(QList<TAlbum*>)),
            this, SIGNAL(signalFindDuplicates(QList<TAlbum*>)));

    connect(d->rescanButton, SIGNAL(pressed()),
            this, SLOT(slotScanForFaces()) );
}

PeopleSideBarWidget::~PeopleSideBarWidget()
{
    delete d;
}

void PeopleSideBarWidget::slotInit()
{
    loadState();
}

void PeopleSideBarWidget::setActive(bool active)
{
    emit requestFaceMode(active);

    if (active)
    {
        d->tagFolderView->setCurrentAlbums(QList<Album*>() << d->tagFolderView->currentAlbum());
    }
}

void PeopleSideBarWidget::doLoadState()
{
    d->tagFolderView->loadState();
}

void PeopleSideBarWidget::doSaveState()
{
    d->tagFolderView->saveState();
}

void PeopleSideBarWidget::applySettings()
{
}

void PeopleSideBarWidget::changeAlbumFromHistory(QList<Album*> album)
{
    d->tagFolderView->setCurrentAlbums(album);
}

void PeopleSideBarWidget::slotScanForFaces()
{
    FaceScanDialog dialog;

    if (dialog.exec() == QDialog::Accepted)
    {
        FacesDetector* const tool = new FacesDetector(dialog.settings());
        tool->start();
    }
}

const QIcon PeopleSideBarWidget::getIcon()
{
    return QIcon::fromTheme(QLatin1String("edit-image-face-show"));
}

const QString PeopleSideBarWidget::getCaption()
{
    return i18nc("Browse images sorted by depicted people", "People");
}

} // namespace Digikam
