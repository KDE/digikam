/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a time line control view
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
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qpushbutton.h>

// KDE include.

#include <kseparator.h>
#include <klocale.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "album.h"
#include "albummanager.h"
#include "ddebug.h"
#include "timelinewidget.h"
#include "timelineview.h"
#include "timelineview.moc"

namespace Digikam
{

class TimeLineViewPriv
{

public:

    TimeLineViewPriv()
    {
        backBtn        = 0;
        prevBtn        = 0;
        nextBtn        = 0;
        forwBtn        = 0;
        dateModeCB     = 0;
        dRangeLabel    = 0;
        itemsLabel     = 0;
        totalLabel     = 0;
        timeLineWidget = 0;
        timer          = 0;
        resetButton    = 0;
    }

    QToolButton        *backBtn;
    QToolButton        *prevBtn;
    QToolButton        *nextBtn;
    QToolButton        *forwBtn;

    QTimer             *timer;

    QComboBox          *dateModeCB;

    QPushButton        *resetButton;

    KSqueezedTextLabel *dRangeLabel;
    KSqueezedTextLabel *itemsLabel;
    KSqueezedTextLabel *totalLabel;

    TimeLineWidget     *timeLineWidget;
};

TimeLineView::TimeLineView(QWidget *parent)
            : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineViewPriv;
    d->timer = new QTimer(this);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QGridLayout *grid = new QGridLayout(this, 3, 5);
    d->timeLineWidget = new TimeLineWidget(this);
    d->backBtn        = new QToolButton(this);
    d->prevBtn        = new QToolButton(this);
    d->nextBtn        = new QToolButton(this);
    d->forwBtn        = new QToolButton(this);

    d->backBtn->setIconSet(SmallIconSet("2leftarrow"));
    d->backBtn->setAutoRaise(true);
    d->backBtn->setAutoRepeat(true);
    d->backBtn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    d->prevBtn->setIconSet(SmallIconSet("1leftarrow"));
    d->prevBtn->setAutoRaise(true);
    d->prevBtn->setAutoRepeat(true);
    d->prevBtn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    d->nextBtn->setIconSet(SmallIconSet("1rightarrow"));
    d->nextBtn->setAutoRaise(true);
    d->nextBtn->setAutoRepeat(true);
    d->nextBtn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    d->forwBtn->setIconSet(SmallIconSet("2rightarrow"));
    d->forwBtn->setAutoRaise(true);
    d->forwBtn->setAutoRepeat(true);
    d->forwBtn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    // ---------------------------------------------------------------

    QWidget *info      = new QWidget(this);
    QGridLayout *grid2 = new QGridLayout(info, 6, 2);

    QLabel *label1 = new QLabel(i18n("Time Units:"), info);
    d->dateModeCB  = new QComboBox(false, info);
    d->dateModeCB->insertItem(i18n("Day"),   TimeLineWidget::Day);
    d->dateModeCB->insertItem(i18n("Week"),  TimeLineWidget::Week);
    d->dateModeCB->insertItem(i18n("Month"), TimeLineWidget::Month);
    d->dateModeCB->insertItem(i18n("Year"),  TimeLineWidget::Year);
    d->dateModeCB->setCurrentItem((int)d->timeLineWidget->dateMode());
    d->dateModeCB->setFocusPolicy(QWidget::NoFocus);

    KSeparator *line1 = new KSeparator(Horizontal, info);

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

    grid2->addMultiCellWidget(label1,          0, 0, 0, 0);
    grid2->addMultiCellWidget(d->dateModeCB,   0, 0, 2, 2);
    grid2->addMultiCellWidget(line1,           1, 1, 0, 2);
    grid2->addMultiCellWidget(label2,          2, 2, 0, 0);
    grid2->addMultiCellWidget(d->dRangeLabel,  2, 2, 1, 2);
    grid2->addMultiCellWidget(label3,          3, 3, 0, 0);
    grid2->addMultiCellWidget(d->itemsLabel ,  3, 3, 2, 2);
    grid2->addMultiCellWidget(line2,           4, 4, 0, 2);
    grid2->addMultiCellWidget(label4,          5, 5, 0, 0);
    grid2->addMultiCellWidget(d->totalLabel ,  5, 5, 2, 2);
    grid2->addMultiCellWidget(d->resetButton , 6, 6, 0, 0);
    grid2->setColStretch(1, 10);
    grid2->setMargin(0);
    grid2->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    grid->addMultiCellWidget(d->timeLineWidget, 0, 0, 0, 5);
    grid->addMultiCellWidget(d->backBtn,        1, 1, 0, 0);
    grid->addMultiCellWidget(d->prevBtn,        1, 1, 1, 1);
    grid->addMultiCellWidget(d->nextBtn,        1, 1, 3, 3);
    grid->addMultiCellWidget(d->forwBtn,        1, 1, 4, 4);
    grid->addMultiCellWidget(info,              2, 2, 0, 4);
    grid->setColStretch(2, 10);
    grid->setRowStretch(3, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // ---------------------------------------------------------------

    connect(AlbumManager::instance(), SIGNAL(signalDatesMapDirty(const QMap<QDateTime, int>&)),
            d->timeLineWidget, SLOT(slotDatesMap(const QMap<QDateTime, int>&)));

    connect(d->dateModeCB, SIGNAL(activated(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->backBtn, SIGNAL(clicked()),
            d->timeLineWidget, SLOT(slotBackward()));

    connect(d->prevBtn, SIGNAL(clicked()),
            d->timeLineWidget, SLOT(slotPrevious()));

    connect(d->nextBtn, SIGNAL(clicked()),
            d->timeLineWidget, SLOT(slotNext()));

    connect(d->forwBtn, SIGNAL(clicked()),
            d->timeLineWidget, SLOT(slotForward()));

    connect(d->timeLineWidget, SIGNAL(signalCursorPositionChanged()),
            this, SLOT(slotCursorPositionChanged()));

    connect(d->timeLineWidget, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotQuerySearchKIOSlave()));

    connect(d->resetButton, SIGNAL(clicked()),
            d->timeLineWidget, SLOT(slotResetSelection()));
}

TimeLineView::~TimeLineView()
{
    delete d->timer;
    delete d;
}

void TimeLineView::setCurrentDateTime(const QDateTime& dateTime)
{
    d->timeLineWidget->setCurrentDateTime(dateTime);
    d->timeLineWidget->setRefDateTime(dateTime);
}

void TimeLineView::slotScaleChanged(int mode)
{
    d->timeLineWidget->setDateMode((TimeLineWidget::DateMode)mode);
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
    d->timer->start(500, true);
}

void TimeLineView::slotQuerySearchKIOSlave()
{
    int totalCount = 0;
    QDateTime start, end;
    DateRangeList list = d->timeLineWidget->currentSelectedDateRange(totalCount);
    d->totalLabel->setText(QString::number(totalCount));

    if (list.isEmpty())
    {
        AlbumManager::instance()->setCurrentAlbum(0);
        return;
    }

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

}  // NameSpace Digikam
