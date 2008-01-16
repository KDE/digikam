/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : Time line sidebar tab contents.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qtimer.h>
#include <qframe.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qhbuttongroup.h> 
#include <qvaluelist.h>
#include <qmap.h>
#include <qscrollbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE include.

#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
#include <kmessagebox.h>

#if KDE_IS_VERSION(3,2,0)
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
#include "searchtextbar.h"
#include "timelinefolderview.h"
#include "timelineview.h"
#include "timelineview.moc"

namespace Digikam
{

class TimeLineViewPriv
{

public:

    TimeLineViewPriv()
    {
        dateModeCB            = 0;
        scaleBG               = 0;
        cursorDateLabel       = 0;
        cursorCountLabel      = 0;
        totalLabel            = 0;
        timeLineWidget        = 0;
        timer                 = 0;
        resetButton           = 0;
        saveButton            = 0;
        scrollBar             = 0;
        timeLineFolderView    = 0;
        nameEdit              = 0;
        searchDateBar         = 0;
    }

    QScrollBar         *scrollBar;

    QTimer             *timer;

    QComboBox          *dateModeCB;

    QHButtonGroup      *scaleBG;

    QPushButton        *resetButton;
    QPushButton        *saveButton;

    KLineEdit          *nameEdit;

    KSqueezedTextLabel *cursorDateLabel;
    KSqueezedTextLabel *cursorCountLabel;
    KSqueezedTextLabel *totalLabel;

    SearchTextBar      *searchDateBar;

    TimeLineWidget     *timeLineWidget;

    TimeLineFolderView *timeLineFolderView;
};

TimeLineView::TimeLineView(QWidget *parent)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineViewPriv;
    d->timer = new QTimer(this);

    QVBoxLayout *vlay = new QVBoxLayout(this);
    QFrame *panel     = new QFrame(this);
    panel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    panel->setLineWidth(1);

    QGridLayout *grid = new QGridLayout(panel, 5, 3);

    // ---------------------------------------------------------------

    QWidget *hbox1    = new QWidget(panel);
    QHBoxLayout *hlay = new QHBoxLayout(hbox1);

    QLabel *label1 = new QLabel(i18n("Time Unit:"), hbox1);
    d->dateModeCB  = new QComboBox(false, hbox1);
    d->dateModeCB->insertItem(i18n("Day"),   TimeLineWidget::Day);
    d->dateModeCB->insertItem(i18n("Week"),  TimeLineWidget::Week);
    d->dateModeCB->insertItem(i18n("Month"), TimeLineWidget::Month);
    d->dateModeCB->insertItem(i18n("Year"),  TimeLineWidget::Year);
    d->dateModeCB->setCurrentItem((int)TimeLineWidget::Month);
    d->dateModeCB->setFocusPolicy(QWidget::NoFocus);

    d->scaleBG = new QHButtonGroup(hbox1);
    d->scaleBG->setExclusive(true);
    d->scaleBG->setFrameShape(QFrame::NoFrame);
    d->scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( d->scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                      "If the date count's maximal values are small, you can use the linear scale.<p>"
                                      "Logarithmic scale can be used when the maximal values are big; "
                                      "if it is used, all values (small and large) will be visible on the "
                                      "graph."));

    QPushButton *linHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    d->scaleBG->insert(linHistoButton, TimeLineWidget::LinScale);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    d->scaleBG->insert(logHistoButton, TimeLineWidget::LogScale);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);

    hlay->setMargin(0);
    hlay->setSpacing(KDialog::spacingHint());
    hlay->addWidget(label1);
    hlay->addWidget(d->dateModeCB);
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hlay->addWidget(d->scaleBG);

    // ---------------------------------------------------------------

    d->timeLineWidget = new TimeLineWidget(panel);
    d->scrollBar      = new QScrollBar(panel);
    d->scrollBar->setOrientation(Qt::Horizontal);
    d->scrollBar->setMinValue(0);
    d->scrollBar->setLineStep(1);

    d->cursorDateLabel  = new KSqueezedTextLabel(0, panel);
    d->cursorCountLabel = new KSqueezedTextLabel(0, panel);
    d->cursorCountLabel->setAlignment(Qt::AlignRight);

    QLabel *label4 = new QLabel(i18n("Total:"), panel);
    d->totalLabel  = new KSqueezedTextLabel(0, panel);
    d->totalLabel->setAlignment(Qt::AlignRight);

    // ---------------------------------------------------------------

    QHBox *hbox2 = new QHBox(panel);
    hbox2->setMargin(0);
    hbox2->setSpacing(KDialog::spacingHint());

    d->resetButton = new QPushButton(hbox2);
    d->resetButton->setPixmap(SmallIcon("reload_page"));
    QToolTip::add(d->resetButton, i18n("Clear current selection"));
    QWhatsThis::add(d->resetButton, i18n("<p>If you press this button, current "
                                        "dates selection from time-line will be "
                                        "clear."));
    d->nameEdit    = new KLineEdit(hbox2);
    QWhatsThis::add(d->nameEdit, i18n("<p>Enter the name of the current dates search to save in the "
                                      "\"My Date Searches\" view"));

    d->saveButton  = new QPushButton(hbox2);
    d->saveButton->setPixmap(SmallIcon("filesave"));
    d->saveButton->setEnabled(false);
    QToolTip::add(d->saveButton, i18n("Save current selection to a new virtual Album"));
    QWhatsThis::add(d->saveButton, i18n("<p>If you press this button, current "
                                        "dates selection from time-line will be "
                                        "saved to a new search virtual Album using name "
                                        "set on the left side."));

    // ---------------------------------------------------------------

    grid->addMultiCellWidget(hbox1,               0, 0, 0, 3);
    grid->addMultiCellWidget(d->timeLineWidget,   1, 1, 0, 3);
    grid->addMultiCellWidget(d->scrollBar,        2, 2, 0, 3);
    grid->addMultiCellWidget(d->cursorDateLabel,  3, 3, 0, 2);
    grid->addMultiCellWidget(d->cursorCountLabel, 3, 3, 3, 3);
    grid->addMultiCellWidget(label4,              4, 4, 0, 0);
    grid->addMultiCellWidget(d->totalLabel,       4, 4, 3, 3);
    grid->addMultiCellWidget(hbox2,               5, 5, 0, 3);
    grid->setColStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    d->timeLineFolderView = new TimeLineFolderView(this);
    d->searchDateBar      = new SearchTextBar(this);

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

    connect(d->timeLineFolderView, SIGNAL(signalSearchFilterMatch(bool)),
            d->searchDateBar, SLOT(slotSearchResult(bool)));

    connect(d->searchDateBar, SIGNAL(signalTextChanged(const QString&)),
            d->timeLineFolderView, SLOT(slotSearchFilterChanged(const QString&)));

    connect(d->dateModeCB, SIGNAL(activated(int)),
            this, SLOT(slotDateUnitChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotInit()));

    connect(d->timeLineWidget, SIGNAL(signalCursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalRefDateTimeChanged()),
            this, SLOT(slotRefDateTimeChanged()));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotDateMapChanged()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotUpdateCurrentDateSearchAlbum()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetSelection()));

    connect(d->saveButton, SIGNAL(clicked()),
            this, SLOT(slotSaveSelection()));

    connect(d->scrollBar, SIGNAL(valueChanged(int)),
            this, SLOT(slotScrollBarValueChanged(int)));

    connect(d->nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotCheckSaveButton()));
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

void TimeLineView::slotInit()
{
    // Date Maps are loaded from AlbumManager to TimeLineWidget after than GUI is initialized.
    // AlbumManager query Date KIO slave to stats items from database and it can take a while.
    // We waiting than TimeLineWidget is ready before to set last config from users.

    readConfig();

    disconnect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
               this, SLOT(slotInit()));
}

void TimeLineView::readConfig()
{
    KConfig* config = kapp->config();
    config->setGroup("TimeLine SideBar");
    d->dateModeCB->setCurrentItem(config->readNumEntry("Histogram TimeUnit", TimeLineWidget::Month));
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", TimeLineWidget::LinScale));
    slotDateUnitChanged(d->dateModeCB->currentItem());
    slotScaleChanged(d->scaleBG->selectedId());
}

void TimeLineView::writeConfig()
{
    KConfig* config = kapp->config();
    config->setGroup("TimeLine SideBar");
    config->writeEntry("Histogram TimeUnit", d->dateModeCB->currentItem());
    config->writeEntry("Histogram Scale", d->scaleBG->selectedId());
    config->sync();
}

void TimeLineView::slotDateMapChanged()
{
    slotDateUnitChanged(d->dateModeCB->currentItem());
}

void TimeLineView::slotRefDateTimeChanged()
{
    d->scrollBar->setMaxValue(d->timeLineWidget->totalIndex());
    d->scrollBar->setValue(d->timeLineWidget->indexForRefDateTime());
}

void TimeLineView::slotDateUnitChanged(int mode)
{
    d->timeLineWidget->setDateMode((TimeLineWidget::DateMode)mode);
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
    QDateTime start, end;
    int val = d->timeLineWidget->cursorInfo(start, end);

    QString txt = i18n("%1 to %2:")
                  .arg(KGlobal::locale()->formatDate(start.date(), true))
                  .arg(KGlobal::locale()->formatDate(end.date(), true));

    d->cursorDateLabel->setText(txt);
    d->cursorCountLabel->setText(QString::number(val));
}

void TimeLineView::slotSelectionChanged()
{
    d->timer->start(100, true);
}

void TimeLineView::slotUpdateCurrentDateSearchAlbum()
{
    slotCheckSaveButton();
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
    d->totalLabel->setText(QString::number(totalCount));

    if (list.isEmpty())
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        return;
    }

    // We will make now the Url for digiKam Search KIO-Slave

    KURL url;
    url.setProtocol("digikamsearch");

    int grp = list.count();
    QString path("1 AND 2");

    if (grp > 1 )
    {
        for (int i = 1 ; i < grp; i++)
        {
            path.append(" OR ");
            path.append(QString("%1 AND %2").arg(i*2+1).arg(i*2+2));
        }
    }
    url.setPath(path);

    int i = 0;
    DateRangeList::iterator it;
    for (it = list.begin() ; it != list.end(); ++it)
    {
        start = (*it).first;
        end   = (*it).second;
        url.addQueryItem(QString("%1.key").arg(i*2+1), QString("imagedate"));
        url.addQueryItem(QString("%1.op").arg(i*2+1),  QString("GT"));
        url.addQueryItem(QString("%1.val").arg(i*2+1), start.date().toString(Qt::ISODate));
        url.addQueryItem(QString("%1.key").arg(i*2+2), QString("imagedate"));
        url.addQueryItem(QString("%1.op").arg(i*2+2),  QString("LT"));
        url.addQueryItem(QString("%1.val").arg(i*2+2), end.date().toString(Qt::ISODate));
        i++;
    }

    url.addQueryItem("name", name);
    url.addQueryItem("count", QString::number(grp*2));
    url.addQueryItem("type", QString("datesearch"));

    DDebug() << url << endl;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url, false);
    AlbumManager::instance()->setCurrentAlbum(album);
}

void TimeLineView::slotAlbumSelected(SAlbum* salbum)
{
    if (!salbum) 
    {
        slotResetSelection();
        return;
    }

    // Date Search url for KIO-Slave is something like that :
    // digikamsearch:1 AND 2 OR 3 AND 4 OR 5 AND 6?
    //               1.key=imagedate&1.op=GT&1.val=2006-02-06&
    //               2.key=imagedate&2.op=LT&2.val=2006-02-07&
    //               3.key=imagedate&3.op=GT&3.val=2006-02-10&
    //               4.key=imagedate&4.op=LT&4.val=2006-02-11&
    //               5.key=imagedate&5.op=GT&5.val=2006-02-12&
    //               6.key=imagedate&6.op=LT&6.val=2006-02-13&
    //               name=TimeLineSelection&
    //               count=6
    //               type=datesearch

    // Check if a special url query exist to identify a SAlbum dedicaced to Date Search
    KURL url = salbum->kurl();
    QMap<QString, QString> queries = url.queryItems();
    if (queries.isEmpty()) return;

    QString type = url.queryItem("type");
    if (type != QString("datesearch")) return;

    bool ok   = false;
    int count = url.queryItem("count").toInt(&ok);
    if (!ok || count <= 0) return;

    DDebug() << url << endl;

    QMap<QString, QString>::iterator it2;
    QString       key;
    QDateTime     start, end;
    DateRangeList list;
    for (int i = 1 ; i <= count/2 ; i+=2)
    {
        key = QString("%1.val").arg(QString::number(i));
        it2 = queries.find(key);
        if (it2 != queries.end())
            start = QDateTime(QDate::fromString(it2.data(), Qt::ISODate));

        DDebug() << key << " :: " << it2.data() << endl;

        key = QString("%1.val").arg(QString::number(i+1));
        it2 = queries.find(key);
        if (it2 != queries.end())
            end = QDateTime(QDate::fromString(it2.data(), Qt::ISODate));

        DDebug() << key << " :: " << it2.data() << endl;

        list.append(DateRange(start, end));
    }

    DateRangeList::iterator it3;
    for (it3 = list.begin() ; it3 != list.end(); ++it3)
        DDebug() << (*it3).first.date().toString(Qt::ISODate) << " :: " 
                 << (*it3).second.date().toString(Qt::ISODate) << endl;

    d->timeLineWidget->setSelectedDateRange(list);
    AlbumManager::instance()->setCurrentAlbum(salbum);
}

void TimeLineView::slotResetSelection()
{
    d->timeLineWidget->slotResetSelection();
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
#if KDE_IS_VERSION(3,2,0)
        QString newTitle = KInputDialog::getText(i18n("Name exists"), label, name, &ok, this);
#else
        QString newTitle = KLineEditDlg::getText(i18n("Name exists"), label, name, ok, this);
#endif
        if (!ok) return false;

        name    = newTitle;
        checked = checkAlbum(name);
    }

    return true;
}

bool TimeLineView::checkAlbum(const QString& name) const
{
    AlbumList list = AlbumManager::instance()->allSAlbums();

    for (AlbumList::Iterator it = list.begin() ; it != list.end() ; ++it)
    {
        SAlbum *album = (SAlbum*)(*it);
        if ( album->title() == name )
            return false;
    }
    return true;
}

void TimeLineView::slotCheckSaveButton()
{
    int totalCount     = 0;
    DateRangeList list = d->timeLineWidget->selectedDateRange(totalCount);
    if (!list.isEmpty() && !d->nameEdit->text().isEmpty())
        d->saveButton->setEnabled(true);
    else
        d->saveButton->setEnabled(false);
}

void TimeLineView::slotRenameAlbum(SAlbum* salbum)
{
    if (!salbum) return;

    QString oldName(salbum->title());
    bool    ok;

#if KDE_IS_VERSION(3,2,0)
    QString name = KInputDialog::getText(i18n("Rename Album (%1)").arg(oldName), 
                                          i18n("Enter new album name:"),
                                          oldName, &ok, this);
#else
    QString name = KLineEditDlg::getText(i18n("Rename Item (%1)").arg(oldName), 
                                          i18n("Enter new album name:"),
                                          oldName, &ok, this);
#endif

    if (!ok || name == oldName || name.isEmpty()) return;

    if (!checkName(name)) return;

    KURL url = salbum->kurl();
    url.removeQueryItem("name");
    url.addQueryItem("name", name);
    AlbumManager::instance()->updateSAlbum(salbum, url);
}

}  // NameSpace Digikam
