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

#include <kseparator.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
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
        dateModeCB         = 0;
        scaleBG            = 0;
        dRangeLabel        = 0;
        itemsLabel         = 0;
        totalLabel         = 0;
        timeLineWidget     = 0;
        timer              = 0;
        resetButton        = 0;
        scrollBar          = 0;
        timeLineFolderView = 0;
    }

    QScrollBar         *scrollBar;

    QTimer             *timer;

    QComboBox          *dateModeCB;

    QHButtonGroup      *scaleBG;

    QPushButton        *resetButton;

    KSqueezedTextLabel *dRangeLabel;
    KSqueezedTextLabel *itemsLabel;
    KSqueezedTextLabel *totalLabel;

    TimeLineWidget     *timeLineWidget;

    TimeLineFolderView *timeLineFolderView;
};

TimeLineView::TimeLineView(QWidget *parent)
            : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineViewPriv;
    d->timer = new QTimer(this);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QGridLayout *grid = new QGridLayout(this, 4, 3);

    QLabel *label1 = new QLabel(i18n("Time Unit:"), this);
    d->dateModeCB  = new QComboBox(false, this);
    d->dateModeCB->insertItem(i18n("Day"),   TimeLineWidget::Day);
    d->dateModeCB->insertItem(i18n("Week"),  TimeLineWidget::Week);
    d->dateModeCB->insertItem(i18n("Month"), TimeLineWidget::Month);
    d->dateModeCB->insertItem(i18n("Year"),  TimeLineWidget::Year);
    d->dateModeCB->setCurrentItem((int)TimeLineWidget::Month);
    d->dateModeCB->setFocusPolicy(QWidget::NoFocus);

    d->scaleBG = new QHButtonGroup(this);
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

    d->timeLineWidget = new TimeLineWidget(this);
    d->scrollBar      = new QScrollBar(this);
    d->scrollBar->setOrientation(Qt::Horizontal);
    d->scrollBar->setMinValue(0);
    d->scrollBar->setLineStep(1);

    d->timeLineFolderView = new TimeLineFolderView(this);

    // ---------------------------------------------------------------

    QWidget *info      = new QWidget(this);
    QGridLayout *grid2 = new QGridLayout(info, 4, 2);

    QLabel *label2 = new QLabel(i18n("Date:"), info);
    d->dRangeLabel = new KSqueezedTextLabel(0, info);
    d->dRangeLabel->setAlignment(Qt::AlignRight);

    QLabel *label3 = new QLabel(i18n("Items:"), info);
    d->itemsLabel  = new KSqueezedTextLabel(0, info);
    d->itemsLabel->setAlignment(Qt::AlignRight);

    KSeparator *line2 = new KSeparator(Horizontal, info);

    QLabel *label4 = new QLabel(i18n("Total:"), info);
    d->totalLabel  = new KSqueezedTextLabel(0, info);
    d->totalLabel->setAlignment(Qt::AlignRight);

    d->resetButton = new QPushButton(i18n("&Reset Selection"), info);

    grid2->addMultiCellWidget(label2,          0, 0, 0, 0);
    grid2->addMultiCellWidget(d->dRangeLabel,  0, 0, 1, 2);
    grid2->addMultiCellWidget(label3,          1, 1, 0, 0);
    grid2->addMultiCellWidget(d->itemsLabel ,  1, 1, 2, 2);
    grid2->addMultiCellWidget(line2,           2, 2, 0, 2);
    grid2->addMultiCellWidget(label4,          3, 3, 0, 0);
    grid2->addMultiCellWidget(d->totalLabel ,  3, 3, 2, 2);
    grid2->addMultiCellWidget(d->resetButton , 4, 4, 0, 0);
    grid2->setColStretch(1, 10);
    grid2->setMargin(0);
    grid2->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    grid->addMultiCellWidget(label1,                0, 0, 0, 0);
    grid->addMultiCellWidget(d->dateModeCB,         0, 0, 1, 1);
    grid->addMultiCellWidget(d->scaleBG,            0, 0, 3, 3);
    grid->addMultiCellWidget(d->timeLineWidget,     1, 1, 0, 3);
    grid->addMultiCellWidget(d->scrollBar,          2, 2, 0, 3);
    grid->addMultiCellWidget(info,                  3, 3, 0, 3);
    grid->addMultiCellWidget(d->timeLineFolderView, 4, 4, 0, 3);
    grid->setColStretch(2, 10);
    grid->setRowStretch(4, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotDateMapLoaded()));

    connect(AlbumManager::instance(), SIGNAL(signalDatesMapDirty(const QMap<QDateTime, int>&)),
            d->timeLineWidget, SLOT(slotDatesMap(const QMap<QDateTime, int>&)));

    connect(d->dateModeCB, SIGNAL(activated(int)),
            this, SLOT(slotDateUnitChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->timeLineWidget, SIGNAL(signalCursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalRefDateTimeChanged()),
            this, SLOT(slotRefDateTimeChanged()));

    connect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
            this, SLOT(slotDateMapChanged()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotQuerySearchKIOSlave()));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetSelection()));

    connect(d->scrollBar, SIGNAL(valueChanged(int)),
            this, SLOT(slotScrollBarValueChanged(int)));

    // ---------------------------------------------------------------

    readConfig();
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

void TimeLineView::readConfig()
{
    KConfig* config = kapp->config();
    config->setGroup("TimeLine SideBar");
    d->dateModeCB->setCurrentItem(config->readNumEntry("Histogram TimeUnit", TimeLineWidget::Month));
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", TimeLineWidget::LinScale));
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

    QString txt = i18n("%1 to %2")
                  .arg(KGlobal::locale()->formatDate(start.date(), true))
                  .arg(KGlobal::locale()->formatDate(end.date(), true));

    d->dRangeLabel->setText(txt);
    d->itemsLabel->setText(QString::number(val));
}

void TimeLineView::slotSelectionChanged()
{
    d->timer->start(100, true);
}

void TimeLineView::slotQuerySearchKIOSlave()
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

    url.addQueryItem("name", QString("TimeLineSelection"));
    url.addQueryItem("count", QString::number(grp*2));

    DDebug() << url << endl;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url, false);
    AlbumManager::instance()->setCurrentAlbum(album);
}

void TimeLineView::dateSearchUrlToDateRangeList()
{
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

    SAlbum *salbum  = 0;
    AlbumList sList = AlbumManager::instance()->allSAlbums();
    AlbumList::iterator it;
    for (it = sList.begin(); it != sList.end(); ++it)
    {
        salbum = (SAlbum*)(*it);
        if (salbum)
        {
            if (salbum->title() == QString("TimeLineSelection"))
                break;
        }
    }

    if (it == sList.end()) return;

    KURL url = salbum->kurl();
    QMap<QString, QString> queries = url.queryItems();
    if (queries.isEmpty()) return;

    DDebug() << url << endl;

    QMap<QString, QString>::iterator it2;
    QString       key;
    QDateTime     start, end;
    DateRangeList list;
    for (uint i = 1 ; i <= (queries.count()/2)-4 ; i+=2)
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
}

void TimeLineView::slotDateMapLoaded()
{
    dateSearchUrlToDateRangeList();
    disconnect(d->timeLineWidget, SIGNAL(signalDateMapChanged()),
               this, SLOT(slotDateMapLoaded()));
}

void TimeLineView::slotResetSelection()
{
    d->timeLineWidget->slotResetSelection();
    AlbumManager::instance()->setCurrentAlbum(0);
}

}  // NameSpace Digikam
