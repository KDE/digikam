/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : Time line sidebar tab contents.
 *
 * Copyright (C) 2007-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "timelineview.h"
#include "timelineview.moc"

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QLayout>
#include <QMap>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "searchtextbar.h"
#include "searchxml.h"
#include "timelinefolderview.h"
#include "timelinewidget.h"

namespace Digikam
{

class TimeLineViewPriv
{

public:

    TimeLineViewPriv()
    {
        timeUnitCB            = 0;
        scaleBG               = 0;
        cursorDateLabel       = 0;
        cursorCountLabel      = 0;
        timeLineWidget        = 0;
        timer                 = 0;
        scrollBar             = 0;
        timeLineFolderView    = 0;
        resetButton           = 0;
        saveButton            = 0;
        nameEdit              = 0;
        searchDateBar         = 0;
    }

    QScrollBar         *scrollBar;

    QTimer             *timer;

    KComboBox          *timeUnitCB;

    QButtonGroup       *scaleBG;

    QLabel             *cursorCountLabel;

    QToolButton        *resetButton;
    QToolButton        *saveButton;

    KLineEdit          *nameEdit;

    KSqueezedTextLabel *cursorDateLabel;

    SearchTextBar      *searchDateBar;

    TimeLineWidget     *timeLineWidget;

    TimeLineFolderView *timeLineFolderView;
};

TimeLineView::TimeLineView(QWidget *parent)
            : QWidget(parent), d(new TimeLineViewPriv)
{
    d->timer = new QTimer(this);
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vlay = new QVBoxLayout(this);
    QFrame *panel     = new QFrame(this);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    panel->setLineWidth(1);

    QGridLayout *grid = new QGridLayout(panel);

    // ---------------------------------------------------------------

    QWidget *hbox1    = new QWidget(panel);
    QHBoxLayout *hlay = new QHBoxLayout(hbox1);

    QLabel *label1 = new QLabel(i18n("Time Unit:"), hbox1);
    d->timeUnitCB  = new KComboBox(hbox1);
    d->timeUnitCB->addItem(i18n("Day"),   TimeLineWidget::Day);
    d->timeUnitCB->addItem(i18n("Week"),  TimeLineWidget::Week);
    d->timeUnitCB->addItem(i18n("Month"), TimeLineWidget::Month);
    d->timeUnitCB->addItem(i18n("Year"),  TimeLineWidget::Year);
    d->timeUnitCB->setCurrentIndex((int)TimeLineWidget::Month);
    d->timeUnitCB->setFocusPolicy(Qt::NoFocus);
    d->timeUnitCB->setWhatsThis(i18n("<p>Select the histogram time unit.</p>"
                                     "<p>You can change the graph decade to zoom in or zoom out over time.</p>"));

    QWidget *scaleBox  = new QWidget(hbox1);
    QHBoxLayout *hlay2 = new QHBoxLayout(scaleBox);
    d->scaleBG         = new QButtonGroup(scaleBox);
    d->scaleBG->setExclusive(true);
    scaleBox->setWhatsThis( i18n("<p>Select the histogram scale.</p>"
                                  "<p>If the date's maximal counts are small, you can use the linear scale.</p>"
                                  "<p>Logarithmic scale can be used when the maximal counts are big; "
                                  "if it is used, all values (small and large) will be visible on the "
                                  "graph.</p>"));

    QToolButton *linHistoButton = new QToolButton(scaleBox);
    linHistoButton->setToolTip( i18n( "Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    d->scaleBG->addButton(linHistoButton, TimeLineWidget::LinScale);

    QToolButton *logHistoButton = new QToolButton(scaleBox);
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

    KHBox *hbox2 = new KHBox(panel);
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

    d->timeLineFolderView = new TimeLineFolderView(this);
    d->searchDateBar      = new SearchTextBar(this, "TimeLineViewSearchDateBar");

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

    connect(d->timeLineFolderView, SIGNAL(signalAlbumSelected(SAlbum*)),
            this, SLOT(slotAlbumSelected(SAlbum*)));

    connect(d->timeLineFolderView, SIGNAL(signalRenameAlbum(SAlbum*)),
            this, SLOT(slotRenameAlbum(SAlbum*)));

    connect(d->timeLineFolderView, SIGNAL(signalTextSearchFilterMatch(bool)),
            d->searchDateBar, SLOT(slotSearchResult(bool)));

    connect(d->searchDateBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            d->timeLineFolderView, SLOT(slotTextSearchFilterChanged(const SearchTextSettings&)));

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

TimeLineView::~TimeLineView()
{
    writeConfig();
    delete d->timer;
    delete d;
}

TimeLineFolderView* TimeLineView::folderView() const
{
    return d->timeLineFolderView;
}

SearchTextBar* TimeLineView::searchBar() const
{
    return d->searchDateBar;
}

void TimeLineView::slotInit()
{
    // Date Maps are loaded from AlbumManager to TimeLineWidget after than GUI is initialized.
    // AlbumManager query Date KIO slave to stats items from database and it can take a while.
    // We waiting than TimeLineWidget is ready before to set last config from users.

    readConfig();

    disconnect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
               this, SLOT(slotInit()));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotCursorPositionChanged()));
}

void TimeLineView::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("TimeLine SideBar"));

    d->timeUnitCB->setCurrentIndex(group.readEntry("Histogram TimeUnit", (int)TimeLineWidget::Month));
    slotTimeUnitChanged(d->timeUnitCB->currentIndex());

    int id = group.readEntry("Histogram Scale", (int)TimeLineWidget::LinScale);
    if ( d->scaleBG->button( id ) )
       d->scaleBG->button( id )->setChecked(true);
    slotScaleChanged(d->scaleBG->checkedId());

    QDateTime now = QDateTime::currentDateTime();
    d->timeLineWidget->setCursorDateTime(group.readEntry("Cursor Position", now));
    d->timeLineWidget->setCurrentIndex(d->timeLineWidget->indexForCursorDateTime());
}

void TimeLineView::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("TimeLine SideBar"));
    group.writeEntry("Histogram TimeUnit", d->timeUnitCB->currentIndex());
    group.writeEntry("Histogram Scale", d->scaleBG->checkedId());
    group.writeEntry("Cursor Position", d->timeLineWidget->cursorDateTime());
    group.sync();
}

void TimeLineView::setActive(bool val)
{
    if (d->timeLineFolderView->selectedItem())
    {
        d->timeLineFolderView->setActive(val);
    }
    else if (val)
    {
        int totalCount = 0;
        DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);
        if (list.isEmpty())
        {
            AlbumManager::instance()->setCurrentAlbum(0);
        }
        else
        {
            AlbumList sList = AlbumManager::instance()->allSAlbums();
            for (AlbumList::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
            {
                SAlbum* salbum = (SAlbum*)(*it);
                if (salbum->title() == d->timeLineFolderView->currentTimeLineSearchName())
                    AlbumManager::instance()->setCurrentAlbum(salbum);
            }
        }
    }
}

void TimeLineView::slotRefDateTimeChanged()
{
    d->scrollBar->blockSignals(true);
    d->scrollBar->setMaximum(d->timeLineWidget->totalIndex()-1);
    d->scrollBar->setValue(d->timeLineWidget->indexForRefDateTime()-1);
    d->scrollBar->blockSignals(false);
}

void TimeLineView::slotTimeUnitChanged(int mode)
{
    d->timeLineWidget->setTimeUnit((TimeLineWidget::TimeUnit)mode);
}

void TimeLineView::slotScrollBarValueChanged(int val)
{
    d->timeLineWidget->setCurrentIndex(val);
}

void TimeLineView::slotScaleChanged(int mode)
{
    d->timeLineWidget->setScaleMode((TimeLineWidget::ScaleMode)mode);
}

void TimeLineView::slotCursorPositionChanged()
{
    QString txt;
    int val = d->timeLineWidget->cursorInfo(txt);
    d->cursorDateLabel->setText(txt);
    d->cursorCountLabel->setText(QString::number(val));
}

void TimeLineView::slotSelectionChanged()
{
    d->timer->setSingleShot(true);
    d->timer->start(100);
}

/** Called from d->timer event.*/
void TimeLineView::slotUpdateCurrentDateSearchAlbum()
{
    slotCheckAboutSelection();
    createNewDateSearchAlbum(d->timeLineFolderView->currentTimeLineSearchName());
}

void TimeLineView::slotSaveSelection()
{
    QString name = d->nameEdit->text();
    if (!checkName(name))
        return;
    createNewDateSearchAlbum(name);
}

void TimeLineView::createNewDateSearchAlbum(const QString& name)
{
    int totalCount = 0;
    QDateTime start, end;
    DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);

    AlbumManager::instance()->setCurrentAlbum(0);

    if (list.isEmpty())
        return;

    d->timeLineFolderView->blockSignals(true);
    d->timeLineFolderView->clearSelection();
    d->timeLineFolderView->blockSignals(false);

    // Create an XML search query for the list of date ranges

    SearchXmlWriter writer;

    // for each range, write a group with two fields
    for (int i=0; i<list.size(); i++)
    {
        writer.writeGroup();
        writer.writeField("creationdate", SearchXml::GreaterThan);
        writer.writeValue(list[i].first);
        writer.finishField();
        writer.writeField("creationdate", SearchXml::LessThan);
        writer.writeValue(list[i].second);
        writer.finishField();
        writer.finishGroup();
    }
    writer.finish();

    kDebug(50003) << "Date search XML:\n" << writer.xml();

    SAlbum* album = AlbumManager::instance()->createSAlbum(name, DatabaseSearch::TimeLineSearch, writer.xml());
    AlbumManager::instance()->setCurrentAlbum(album);
}

void TimeLineView::slotAlbumSelected(SAlbum* salbum)
{
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
                    break;

                if (reader.isFieldElement())
                {
                    if (numberOfFields == 0)
                        start = reader.valueToDateTime();
                    else if (numberOfFields == 1)
                        end = reader.valueToDateTime();
                    numberOfFields++;
                }
            }
            if (numberOfFields)
                list << DateRange(start, end);
        }
    }

    d->timeLineWidget->setSelectedDateRange(list);
    AlbumManager::instance()->setCurrentAlbum(salbum);
}

void TimeLineView::slotResetSelection()
{
    d->timeLineWidget->slotResetSelection();
    slotCheckAboutSelection();
    AlbumManager::instance()->setCurrentAlbum(0);
}

bool TimeLineView::checkName(QString& name)
{
    bool checked = checkAlbum(name);

    while (!checked)
    {
        QString label = i18n( "Search name already exists.\n"
                              "Please enter a new name:" );
        bool ok;
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label, name, &ok, this);
        if (!ok) return false;

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

bool TimeLineView::checkAlbum(const QString& name) const
{
    const AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void TimeLineView::slotCheckAboutSelection()
{
    int totalCount     = 0;
    DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);
    if (!list.isEmpty())
    {
        d->nameEdit->setEnabled(true);

        if (!d->nameEdit->text().isEmpty())
            d->saveButton->setEnabled(true);
    }
    else
    {
        d->nameEdit->setEnabled(false);
        d->saveButton->setEnabled(false);
    }
}

void TimeLineView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    QString oldName(salbum->title());
    bool    ok;

    QString name = KInputDialog::getText(i18n("Rename Album (%1)",oldName),
                                          i18n("Enter new album name:"),
                                          oldName, &ok, this);

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    AlbumManager::instance()->updateSAlbum(salbum, salbum->query(), name);
}

}  // namespace Digikam
