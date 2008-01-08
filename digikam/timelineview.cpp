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

#include <qhbox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qcombobox.h>

// KDE include.

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
        dateSAlbumSet  = false;
        backBtn        = 0;
        prevBtn        = 0;
        nextBtn        = 0;
        forwBtn        = 0;
        dateModeCB     = 0;
        dRangeLabel    = 0;
        itemsLabel     = 0;
        timeLineWidget = 0;
    }

    bool                dateSAlbumSet;

    QToolButton        *backBtn;
    QToolButton        *prevBtn;
    QToolButton        *nextBtn;
    QToolButton        *forwBtn;

    QComboBox          *dateModeCB;

    KSqueezedTextLabel *dRangeLabel;
    KSqueezedTextLabel *itemsLabel;

    TimeLineWidget     *timeLineWidget;
};

TimeLineView::TimeLineView(QWidget *parent)
            : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineViewPriv;

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
    QGridLayout *grid2 = new QGridLayout(info, 3, 2);

    QLabel *label1 = new QLabel(i18n("Scale:"), info);
    d->dateModeCB  = new QComboBox(false, info);
    d->dateModeCB->insertItem(i18n("Day"),   TimeLineWidget::Day);
    d->dateModeCB->insertItem(i18n("Week"),  TimeLineWidget::Week);
    d->dateModeCB->insertItem(i18n("Month"), TimeLineWidget::Month);
    d->dateModeCB->insertItem(i18n("Year"),  TimeLineWidget::Year);
    d->dateModeCB->setCurrentItem((int)d->timeLineWidget->dateMode());
    d->dateModeCB->setFocusPolicy(QWidget::NoFocus);

    QLabel *label2 = new QLabel(i18n("Date:"), info);
    d->dRangeLabel = new KSqueezedTextLabel(0, info);
    d->dRangeLabel->setAlignment(Qt::AlignRight);

    QLabel *label3 = new QLabel(i18n("Items:"), info);
    d->itemsLabel  = new KSqueezedTextLabel(0, info);
    d->itemsLabel->setAlignment(Qt::AlignRight);

    grid2->addMultiCellWidget(label1,         0, 0, 0, 0);
    grid2->addMultiCellWidget(d->dateModeCB,  0, 0, 2, 2);
    grid2->addMultiCellWidget(label2,         1, 1, 0, 0);
    grid2->addMultiCellWidget(d->dRangeLabel, 1, 1, 1, 2);
    grid2->addMultiCellWidget(label3,         2, 2, 0, 0);
    grid2->addMultiCellWidget(d->itemsLabel , 2, 2, 2, 2);
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
            this, SLOT(slotDatesMap(const QMap<QDateTime, int>&)));

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

    connect(d->timeLineWidget, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));
}

TimeLineView::~TimeLineView()
{
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

void TimeLineView::slotSelectionChanged()
{
    QDateTime start, end;
    int val = d->timeLineWidget->currentSelectionInfo(start, end);

    QString txt = i18n("%1 to %2")
                  .arg(KGlobal::locale()->formatDate(start.date(), true))
                  .arg(KGlobal::locale()->formatDate(end.date(), true));

    d->dRangeLabel->setText(txt);
    d->itemsLabel->setText(QString::number(val));

    KURL url;
    url.setProtocol("digikamsearch");

    url.setPath("1 AND 2");
    url.addQueryItem(QString("1.key"), QString("imagedate"));
    url.addQueryItem(QString("1.op"),  QString("GT"));
    url.addQueryItem(QString("1.val"), start.date().toString(Qt::ISODate));
    url.addQueryItem(QString("2.key"), QString("imagedate"));
    url.addQueryItem(QString("2.op"),  QString("LT"));
    url.addQueryItem(QString("2.val"), end.date().toString(Qt::ISODate));
    url.addQueryItem("name", QString("TimeLineSelection"));
    url.addQueryItem("count", QString::number(2));

    DDebug() << url << endl;

    SAlbum* album = AlbumManager::instance()->createSAlbum(url, false);
    AlbumManager::instance()->setCurrentAlbum(album);
    d->dateSAlbumSet = true;
}

void TimeLineView::slotDatesMap(const QMap<QDateTime, int>& map)
{
    /** AlbumManager call refresh() when a SAlbum is created or updated.
        This way emit signalDatesMapDirty() witch reset all selection 
        in TimeLineWidget. We need to catch this signal/slot side effect. */

    if (!d->dateSAlbumSet)
        d->timeLineWidget->slotDatesMap(map);
    else
        d->dateSAlbumSet = false;
}

}  // NameSpace Digikam
