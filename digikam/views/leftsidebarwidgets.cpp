/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2000-12-05
 * Description : left sidebar widgets
 *
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol dot de>
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

#include "leftsidebarwidgets.moc"

// QT includes

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qtimer.h>
#include <qtoolbutton.h>

// KDE includes

#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <ksqueezedtextlabel.h>

// Local includes

#include "albummanager.h"
#include "albummodificationhelper.h"
#include "albumselectiontreeview.h"
#include "albumsettings.h"
#include "datefolderview.h"
#include "editablesearchtreeview.h"
#include "fuzzysearchview.h"
#include "searchfolderview.h"
#include "searchtabheader.h"
#include "searchtextbar.h"
#include "searchxml.h"
#include "tagfolderview.h"
#include "timelinewidget.h"
#include "peoplewidget.h"

namespace Digikam
{

class AlbumFolderViewSideBarWidgetPriv
{
public:

    AlbumFolderViewSideBarWidgetPriv() :
        albumModificationHelper(0),
        albumFolderView(0),
        searchTextBar(0)
    {
    }

    AlbumModificationHelper* albumModificationHelper;
    AlbumSelectionTreeView*  albumFolderView;
    SearchTextBar*           searchTextBar;
};

AlbumFolderViewSideBarWidget::AlbumFolderViewSideBarWidget(QWidget* parent, AlbumModel* model,
                                                           AlbumModificationHelper* albumModificationHelper)
    : SidebarWidget(parent), d(new AlbumFolderViewSideBarWidgetPriv)
{
    setObjectName("AlbumFolderView Sidebar");

    d->albumModificationHelper = albumModificationHelper;

    QVBoxLayout* layout = new QVBoxLayout(this);

    d->albumFolderView = new AlbumSelectionTreeView(this, model, d->albumModificationHelper);
    d->albumFolderView->setObjectName("AlbumFolderView");
    d->albumFolderView->setConfigGroup(getConfigGroup());
    d->albumFolderView->setExpandNewCurrentItem(true);
    d->albumFolderView->setAlbumManagerCurrentAlbum(true);
    d->searchTextBar   = new SearchTextBar(this, "DigikamViewFolderSearchBar");
    d->searchTextBar->setHighlightOnResult(true);
    d->searchTextBar->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->searchTextBar->setFilterModel(d->albumFolderView->albumFilterModel());

    layout->addWidget(d->albumFolderView);
    layout->addWidget(d->searchTextBar);

    // setup connection
    connect(d->albumFolderView, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SIGNAL(signalFindDuplicatesInAlbum(Album*)));
}

AlbumFolderViewSideBarWidget::~AlbumFolderViewSideBarWidget()
{
    delete d;
}

void AlbumFolderViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbum(d->albumFolderView->currentAlbum());
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
    AlbumSettings* settings = AlbumSettings::instance();
    d->albumFolderView->setEnableToolTips(settings->getShowAlbumToolTips());
}

void AlbumFolderViewSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->albumFolderView->setCurrentAlbum(dynamic_cast<PAlbum*>(album));
}

AlbumPointer<PAlbum> AlbumFolderViewSideBarWidget::currentAlbum() const
{
    return AlbumPointer<PAlbum> (d->albumFolderView->currentAlbum());
}

void AlbumFolderViewSideBarWidget::setCurrentAlbum(PAlbum* album)
{
    // Change the current album in list view.
    d->albumFolderView->setCurrentAlbum(album);
}

QPixmap AlbumFolderViewSideBarWidget::getIcon()
{
    return SmallIcon("folder-image");
}

QString AlbumFolderViewSideBarWidget::getCaption()
{
    return i18n("Albums");
}

// -----------------------------------------------------------------------------

class TagViewSideBarWidgetPriv
{
public:

    TagViewSideBarWidgetPriv() :
        tagModel(0),
        tagSearchBar(0),
        tagFolderView(0)
    {
    }

    TagModel*      tagModel;
    SearchTextBar* tagSearchBar;
    TagFolderView* tagFolderView;
};

TagViewSideBarWidget::TagViewSideBarWidget(QWidget* parent, TagModel* model)
    : SidebarWidget(parent), d(new TagViewSideBarWidgetPriv)
{
    setObjectName("TagView Sidebar");

    d->tagModel = model;

    QVBoxLayout* layout = new QVBoxLayout(this);

    d->tagFolderView = new TagFolderView(this, model);
    d->tagFolderView->setConfigGroup(getConfigGroup());
    d->tagFolderView->setExpandNewCurrentItem(true);
    d->tagFolderView->setAlbumManagerCurrentAlbum(true);
    d->tagSearchBar  = new SearchTextBar(this, "DigikamViewTagSearchBar");
    d->tagSearchBar->setHighlightOnResult(true);
    d->tagSearchBar->setModel(model, AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagFolderView->albumFilterModel());

    layout->addWidget(d->tagFolderView);
    layout->addWidget(d->tagSearchBar);

    connect(d->tagFolderView, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SIGNAL(signalFindDuplicatesInAlbum(Album*)));
}

TagViewSideBarWidget::~TagViewSideBarWidget()
{
    delete d;
}

void TagViewSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbum(d->tagFolderView->currentAlbum());
    }
}

void TagViewSideBarWidget::doLoadState()
{
    d->tagFolderView->loadState();
}

void TagViewSideBarWidget::doSaveState()
{
    d->tagFolderView->saveState();
}

void TagViewSideBarWidget::applySettings()
{
}

void TagViewSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->tagFolderView->setCurrentAlbum(dynamic_cast<TAlbum*>(album));
}

AlbumPointer<TAlbum> TagViewSideBarWidget::currentAlbum() const
{
    return AlbumPointer<TAlbum> (d->tagFolderView->currentAlbum());
}

QPixmap TagViewSideBarWidget::getIcon()
{
    return SmallIcon("tag");
}

QString TagViewSideBarWidget::getCaption()
{
    return i18n("Tags");
}

void TagViewSideBarWidget::setCurrentAlbum(TAlbum* album)
{
    d->tagFolderView->setCurrentAlbum(album);
}

// -----------------------------------------------------------------------------

class DateFolderViewSideBarWidgetPriv
{
public:

    DateFolderViewSideBarWidgetPriv() :
        dateFolderView(0)
    {
    }

    DateFolderView* dateFolderView;
};

DateFolderViewSideBarWidget::DateFolderViewSideBarWidget(QWidget* parent, DateAlbumModel* model,
                                                         ImageAlbumFilterModel* imageFilterModel)
    : SidebarWidget(parent), d(new DateFolderViewSideBarWidgetPriv)
{
    setObjectName("DateFolderView Sidebar");

    QVBoxLayout* layout = new QVBoxLayout(this);

    d->dateFolderView   = new DateFolderView(this, model);
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

void DateFolderViewSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->dateFolderView->changeAlbumFromHistory(dynamic_cast<DAlbum*>(album));
}

AlbumPointer<DAlbum> DateFolderViewSideBarWidget::currentAlbum() const
{
    return d->dateFolderView->currentAlbum();
}

void DateFolderViewSideBarWidget::gotoDate(const QDate& date)
{
    d->dateFolderView->gotoDate(date);
}

QPixmap DateFolderViewSideBarWidget::getIcon()
{
    return SmallIcon("view-calendar-list");
}

QString DateFolderViewSideBarWidget::getCaption()
{
    return i18n("Calendar");
}

// -----------------------------------------------------------------------------

class TimelineSideBarWidgetPriv
{
public:

    TimelineSideBarWidgetPriv() :
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
        searchModel(0),
        searchModificationHelper(0)
    {}

    static const QString      configHistogramTimeUnitEntry;
    static const QString      configHistogramScaleEntry;
    static const QString      configCursorPositionEntry;

    QButtonGroup*             scaleBG;
    QLabel*                   cursorCountLabel;
    QScrollBar*               scrollBar;
    QTimer*                   timer;
    QToolButton*              resetButton;
    QToolButton*              saveButton;

    KComboBox*                timeUnitCB;
    KLineEdit*                nameEdit;
    KSqueezedTextLabel*       cursorDateLabel;

    SearchTextBar*            searchDateBar;
    EditableSearchTreeView*   timeLineFolderView;
    TimeLineWidget*           timeLineWidget;

    SearchModel*              searchModel;

    SearchModificationHelper* searchModificationHelper;
};
const QString TimelineSideBarWidgetPriv::configHistogramTimeUnitEntry("Histogram TimeUnit");
const QString TimelineSideBarWidgetPriv::configHistogramScaleEntry("Histogram Scale");
const QString TimelineSideBarWidgetPriv::configCursorPositionEntry("Cursor Position");

// --------------------------------------------------------

TimelineSideBarWidget::TimelineSideBarWidget(QWidget* parent, SearchModel* searchModel,
                                             SearchModificationHelper* searchModificationHelper)
    : SidebarWidget(parent), d(new TimelineSideBarWidgetPriv)
{
    setObjectName("TimeLine Sidebar");

    d->searchModificationHelper = searchModificationHelper;
    d->timer                    = new QTimer(this);
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout* vlay = new QVBoxLayout(this);
    QFrame* panel     = new QFrame(this);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    panel->setLineWidth(1);

    QGridLayout* grid = new QGridLayout(panel);

    // ---------------------------------------------------------------

    QWidget* hbox1    = new QWidget(panel);
    QHBoxLayout* hlay = new QHBoxLayout(hbox1);

    QLabel* label1 = new QLabel(i18n("Time Unit:"), hbox1);
    d->timeUnitCB  = new KComboBox(hbox1);
    d->timeUnitCB->addItem(i18n("Day"),   TimeLineWidget::Day);
    d->timeUnitCB->addItem(i18n("Week"),  TimeLineWidget::Week);
    d->timeUnitCB->addItem(i18n("Month"), TimeLineWidget::Month);
    d->timeUnitCB->addItem(i18n("Year"),  TimeLineWidget::Year);
    d->timeUnitCB->setCurrentIndex((int)TimeLineWidget::Month);
    d->timeUnitCB->setFocusPolicy(Qt::NoFocus);
    d->timeUnitCB->setWhatsThis(i18n("<p>Select the histogram time unit.</p>"
                                     "<p>You can change the graph decade to zoom in or zoom out over time.</p>"));

    QWidget* scaleBox  = new QWidget(hbox1);
    QHBoxLayout* hlay2 = new QHBoxLayout(scaleBox);
    d->scaleBG         = new QButtonGroup(scaleBox);
    d->scaleBG->setExclusive(true);
    scaleBox->setWhatsThis( i18n("<p>Select the histogram scale.</p>"
                                 "<p>If the date's maximal counts are small, you can use the linear scale.</p>"
                                 "<p>Logarithmic scale can be used when the maximal counts are big; "
                                 "if it is used, all values (small and large) will be visible on the "
                                 "graph.</p>"));

    QToolButton* linHistoButton = new QToolButton(scaleBox);
    linHistoButton->setToolTip( i18n( "Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    d->scaleBG->addButton(linHistoButton, TimeLineWidget::LinScale);

    QToolButton* logHistoButton = new QToolButton(scaleBox);
    logHistoButton->setToolTip( i18n( "Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    d->scaleBG->addButton(logHistoButton, TimeLineWidget::LogScale);

    hlay2->setMargin(0);
    hlay2->setSpacing(0);
    hlay2->addWidget(linHistoButton);
    hlay2->addWidget(logHistoButton);

    hlay->setMargin(0);
    hlay->setSpacing(KDialog::spacingHint());
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

    d->cursorDateLabel  = new KSqueezedTextLabel(0, panel);
    d->cursorCountLabel = new QLabel(panel);
    d->cursorCountLabel->setAlignment(Qt::AlignRight);

    // ---------------------------------------------------------------

    KHBox* hbox2 = new KHBox(panel);
    hbox2->setMargin(0);
    hbox2->setSpacing(KDialog::spacingHint());

    d->resetButton = new QToolButton(hbox2);
    d->resetButton->setIcon(SmallIcon("document-revert"));
    d->resetButton->setToolTip(i18n("Clear current selection"));
    d->resetButton->setWhatsThis(i18n("If you press this button, the current date selection on the time-line will be cleared."));
    d->nameEdit    = new KLineEdit(hbox2);
    d->nameEdit->setClearButtonShown(true);
    d->nameEdit->setWhatsThis(i18n("Enter the name of the current dates search to save in the "
                                   "\"My Date Searches\" view"));

    d->saveButton  = new QToolButton(hbox2);
    d->saveButton->setIcon(SmallIcon("document-save"));
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
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    d->timeLineFolderView = new EditableSearchTreeView(this, searchModel, searchModificationHelper);
    d->timeLineFolderView->setConfigGroup(getConfigGroup());
    d->timeLineFolderView->filteredModel()->listTimelineSearches();
    d->timeLineFolderView->filteredModel()->setListTemporarySearches(false);
    d->timeLineFolderView->setAlbumManagerCurrentAlbum(true);
    d->searchDateBar      = new SearchTextBar(this, "TimeLineViewSearchDateBar");
    d->searchDateBar->setModel(d->timeLineFolderView->filteredModel(),
                               AbstractAlbumModel::AlbumIdRole,
                               AbstractAlbumModel::AlbumTitleRole);
    d->searchDateBar->setFilterModel(d->timeLineFolderView->albumFilterModel());

    vlay->addWidget(panel);
    vlay->addWidget(d->timeLineFolderView);
    vlay->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                  QSizePolicy::Minimum, QSizePolicy::Minimum));
    vlay->addWidget(d->searchDateBar);
    vlay->setMargin(0);
    vlay->setSpacing(0);

    // ---------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalDatesMapDirty(const QMap<QDateTime, int>&)),
            d->timeLineWidget, SLOT(slotDatesMap(const QMap<QDateTime, int>&)));

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

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckAboutSelection()));

    connect(d->nameEdit, SIGNAL(returnPressed(const QString&)),
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
        AlbumManager::instance()->setCurrentAlbum(d->timeLineFolderView->currentAlbum());
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

void TimelineSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->timeLineFolderView->setCurrentAlbum(dynamic_cast<SAlbum*>(album));
}

QPixmap TimelineSideBarWidget::getIcon()
{
    return SmallIcon("player-time");
}

QString TimelineSideBarWidget::getCaption()
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
    d->cursorDateLabel->setText(txt);
    d->cursorCountLabel->setText(QString::number(val));
}

void TimelineSideBarWidget::slotSelectionChanged()
{
    d->timer->setSingleShot(true);
    d->timer->start(100);
}

/** Called from d->timer event.*/
void TimelineSideBarWidget::slotUpdateCurrentDateSearchAlbum()
{
    slotCheckAboutSelection();
    int totalCount           = 0;
    DateRangeList dateRanges = d->timeLineWidget->selectedDateRange(totalCount);
    d->searchModificationHelper->slotCreateTimeLineSearch(SAlbum::getTemporaryTitle(DatabaseSearch::TimeLineSearch),
                                                          dateRanges, true);
}

void TimelineSideBarWidget::slotSaveSelection()
{
    QString name             = d->nameEdit->text();
    int totalCount           = 0;
    DateRangeList dateRanges = d->timeLineWidget->selectedDateRange(totalCount);
    d->searchModificationHelper->slotCreateTimeLineSearch(name, dateRanges);
}

void TimelineSideBarWidget::slotAlbumSelected(Album* album)
{
    SAlbum* salbum = dynamic_cast<SAlbum*>(album);

    if (!salbum)
    {
        slotResetSelection();
        return;
    }

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
    AlbumManager::instance()->setCurrentAlbum(0);
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

class SearchSideBarWidgetPriv
{
public:
    SearchSideBarWidgetPriv() :
        searchSearchBar(0),
        searchTreeView(0),
        searchTabHeader(0),
        searchModel(0)
    {
    }

    SearchTextBar*        searchSearchBar;
    NormalSearchTreeView* searchTreeView;
    SearchTabHeader*      searchTabHeader;
    SearchModel*          searchModel;
};

SearchSideBarWidget::SearchSideBarWidget(QWidget* parent, SearchModel* searchModel,
                                         SearchModificationHelper* searchModeificationHelper)
    : SidebarWidget(parent), d(new SearchSideBarWidgetPriv)
{
    setObjectName("Search Sidebar");

    d->searchModel      = searchModel;

    QVBoxLayout* layout = new QVBoxLayout(this);

    d->searchTabHeader  = new SearchTabHeader(this);
    d->searchTreeView   = new NormalSearchTreeView(this, searchModel,
                                                   searchModeificationHelper);
    d->searchTreeView->setConfigGroup(getConfigGroup());
    d->searchTreeView->filteredModel()->listNormalSearches();
    d->searchTreeView->filteredModel()->setListTemporarySearches(true);
    d->searchTreeView->setAlbumManagerCurrentAlbum(true);
    d->searchSearchBar  = new SearchTextBar(this, "DigikamViewSearchSearchBar");
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

    connect(d->searchTabHeader, SIGNAL(searchShallBeSelected(SAlbum*)),
            d->searchTreeView, SLOT(setCurrentAlbum(SAlbum*)));
}

SearchSideBarWidget::~SearchSideBarWidget()
{
    delete d;
}

void SearchSideBarWidget::setActive(bool active)
{
    if (active)
    {
        AlbumManager::instance()->setCurrentAlbum(d->searchTreeView->currentAlbum());
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

void SearchSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->searchTreeView->setCurrentAlbum(dynamic_cast<SAlbum*>(album));
}

QPixmap SearchSideBarWidget::getIcon()
{
    return SmallIcon("edit-find");
}

QString SearchSideBarWidget::getCaption()
{
    return i18nc("Search images, access stored searches", "Search");
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

class FuzzySearchSideBarWidgetPriv
{
public:
    FuzzySearchSideBarWidgetPriv() :
        fuzzySearchView(0),
        searchModel(0),
        searchModificationHelper(0)
    {
    }

    FuzzySearchView*          fuzzySearchView;
    SearchModel*              searchModel;
    SearchModificationHelper* searchModificationHelper;
};

FuzzySearchSideBarWidget::FuzzySearchSideBarWidget(QWidget* parent, SearchModel* searchModel,
                                                   SearchModificationHelper* searchModificationHelper)
    : SidebarWidget(parent), d(new FuzzySearchSideBarWidgetPriv)
{
    setObjectName("Fuzzy Search Sidebar");

    d->searchModel      = searchModel;

    d->fuzzySearchView  = new FuzzySearchView(searchModel, searchModificationHelper, this);
    d->fuzzySearchView->setConfigGroup(getConfigGroup());

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(d->fuzzySearchView);

    connect(d->fuzzySearchView, SIGNAL(signalUpdateFingerPrints()),
            this, SIGNAL(signalUpdateFingerPrints()));

    connect(d->fuzzySearchView, SIGNAL(signalGenerateFingerPrintsFirstTime()),
            this, SIGNAL(signalGenerateFingerPrintsFirstTime()));
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
        AlbumManager::instance()->setCurrentAlbum(d->fuzzySearchView->currentAlbum());
    }
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

void FuzzySearchSideBarWidget::changeAlbumFromHistory(Album* album)
{
    SAlbum* salbum = dynamic_cast<SAlbum*>(album);
    d->fuzzySearchView->setCurrentAlbum(salbum);
}

QPixmap FuzzySearchSideBarWidget::getIcon()
{
    return SmallIcon("tools-wizard");
}

QString FuzzySearchSideBarWidget::getCaption()
{
    return i18n("Fuzzy Searches");
}

void FuzzySearchSideBarWidget::newDuplicatesSearch(Album* album)
{
    d->fuzzySearchView->newDuplicatesSearch(album);
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

class GPSSearchSideBarWidgetPriv
{
public:
    GPSSearchSideBarWidgetPriv() :
        gpsSearchView(0),
        searchModel(0)
    {
    }

    GPSSearchView* gpsSearchView;
    SearchModel*   searchModel;
};

GPSSearchSideBarWidget::GPSSearchSideBarWidget(QWidget* parent, SearchModel* searchModel,
                                               SearchModificationHelper* searchModificationHelper,
                                               ImageFilterModel* imageFilterModel,  QItemSelectionModel* itemSelectionModel)
    : SidebarWidget(parent), d(new GPSSearchSideBarWidgetPriv)
{
    setObjectName("GPS Search Sidebar");

    d->searchModel   = searchModel;

    d->gpsSearchView = new GPSSearchView(this, searchModel, searchModificationHelper, imageFilterModel, itemSelectionModel);
    d->gpsSearchView->setConfigGroup(getConfigGroup());

    QScrollArea* scrollArea = new QScrollArea(this);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(scrollArea);
    scrollArea->setWidget(d->gpsSearchView);
    scrollArea->setWidgetResizable(true);

    connect(d->gpsSearchView, SIGNAL(signalMapSoloItems(const QList<qlonglong>&, const QString&)),
            this, SIGNAL(signalMapSoloItems(const QList<qlonglong>&, const QString&)));
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

void GPSSearchSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->gpsSearchView->changeAlbumFromHistory(dynamic_cast<SAlbum*>(album));
}

QPixmap GPSSearchSideBarWidget::getIcon()
{
    return SmallIcon("applications-internet");
}

QString GPSSearchSideBarWidget::getCaption()
{
    return i18nc("Search images on a map", "Map Search");
}

// -----------------------------------------------------------------------------

class PeopleSideBarWidgetPriv : public TagViewSideBarWidgetPriv
{
public:

    PeopleSideBarWidgetPriv()
    {
        personIcon   = 0;
        textLabel    = 0;
        rescanButton = 0;
        searchModificationHelper = 0;
    }

    QLabel*      personIcon;
    QLabel*      textLabel;
    QPushButton* rescanButton;

    SearchModificationHelper* searchModificationHelper;
};

PeopleSideBarWidget::PeopleSideBarWidget(QWidget* parent, TagModel* model,
                                         SearchModificationHelper* searchModificationHelper)
    : SidebarWidget(parent), d(new PeopleSideBarWidgetPriv)
{
    setObjectName("People Sidebar");

    d->tagModel = model;
    d->searchModificationHelper = searchModificationHelper;

    QVBoxLayout* layout = new QVBoxLayout;
    QHBoxLayout* hlay = new QHBoxLayout;

    d->tagFolderView = new TagFolderView(this, model);
    d->tagFolderView->setConfigGroup(getConfigGroup());
    d->tagFolderView->setExpandNewCurrentItem(true);
    d->tagFolderView->setAlbumManagerCurrentAlbum(true);

    d->tagFolderView->filteredModel()->listOnlyTagsWithProperty("person");
    d->tagFolderView->filteredModel()->setFilterBehavior(AlbumFilterModel::StrictFiltering);

    d->tagSearchBar  = new SearchTextBar(this, "DigikamViewPeopleSearchBar");
    d->tagSearchBar->setHighlightOnResult(true);
    d->tagSearchBar->setModel(d->tagFolderView->filteredModel(),
                              AbstractAlbumModel::AlbumIdRole, AbstractAlbumModel::AlbumTitleRole);
    d->tagSearchBar->setFilterModel(d->tagFolderView->albumFilterModel());

    d->rescanButton = new QPushButton;
    d->rescanButton->setText("Scan collection for faces");

    QString s("user-identity");
    KIcon* icon       = new KIcon(s);
    QPixmap personPix = icon->pixmap(QSize(48, 48));
    d->personIcon = new QLabel;
    d->personIcon->setPixmap(personPix);

    d->textLabel = new QLabel("People Tags");

    hlay->addWidget(d->personIcon);
    hlay->addWidget(d->textLabel);

    layout->addLayout(hlay);
    layout->addWidget(d->rescanButton);
    layout->addWidget(d->tagFolderView);
    layout->addWidget(d->tagSearchBar);

    setLayout(layout);

    connect(d->tagFolderView, SIGNAL(signalFindDuplicatesInAlbum(Album*)),
            this, SIGNAL(signalFindDuplicatesInAlbum(Album*)));

    connect(d->rescanButton, SIGNAL(pressed()),
            this, SIGNAL( signalDetectFaces() ) );
}

QPixmap PeopleSideBarWidget::getIcon()
{
    return SmallIcon("user-identity");
}

QString PeopleSideBarWidget::getCaption()
{
    return i18nc("Browse images sorted by depicted people", "People");
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
        d->tagFolderView->setCurrentAlbum(d->tagFolderView->currentAlbum());
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

void PeopleSideBarWidget::changeAlbumFromHistory(Album* album)
{
    d->tagFolderView->setCurrentAlbum(dynamic_cast<TAlbum*>(album));
}

PeopleSideBarWidget::~PeopleSideBarWidget()
{
    delete d;
}

} // namespace Digikam
