/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a time line control view
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
        infoLabel      = 0;
        timeLineWidget = 0;
    }

    QToolButton        *backBtn;
    QToolButton        *prevBtn;
    QToolButton        *nextBtn;
    QToolButton        *forwBtn;

    QComboBox          *dateModeCB;

    KSqueezedTextLabel *infoLabel;

    TimeLineWidget     *timeLineWidget;
};

TimeLineView::TimeLineView(QWidget *parent)
            : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineViewPriv;

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QGridLayout *grid = new QGridLayout(this, 1, 6);
    d->timeLineWidget = new TimeLineWidget(this);
    d->infoLabel      = new KSqueezedTextLabel(0, this);
    d->backBtn        = new QToolButton(this);
    d->prevBtn        = new QToolButton(this);
    d->nextBtn        = new QToolButton(this);
    d->forwBtn        = new QToolButton(this);
    d->dateModeCB     = new QComboBox(false, this);
    d->dateModeCB->insertItem(i18n("Day"),   TimeLineWidget::Day);
    d->dateModeCB->insertItem(i18n("Week"),  TimeLineWidget::Week);
    d->dateModeCB->insertItem(i18n("Month"), TimeLineWidget::Month);
    d->dateModeCB->insertItem(i18n("Year"),  TimeLineWidget::Year);
    d->dateModeCB->setCurrentItem((int)d->timeLineWidget->dateMode());
    d->dateModeCB->setFocusPolicy(QWidget::NoFocus);

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

    grid->addMultiCellWidget(d->timeLineWidget, 0, 0, 0, 5);
    grid->addMultiCellWidget(d->backBtn,        1, 1, 0, 0);
    grid->addMultiCellWidget(d->prevBtn,        1, 1, 1, 1);
    grid->addMultiCellWidget(d->infoLabel,      1, 1, 2, 2);
    grid->addMultiCellWidget(d->dateModeCB,     1, 1, 3, 3);
    grid->addMultiCellWidget(d->nextBtn,        1, 1, 4, 4);
    grid->addMultiCellWidget(d->forwBtn,        1, 1, 5, 5);
    grid->setRowStretch(6, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

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
}

void TimeLineView::slotScaleChanged(int mode)
{
    d->timeLineWidget->setDateMode((TimeLineWidget::DateMode)mode);
}

void TimeLineView::slotSelectionChanged()
{
    QDateTime start, end;
    int val = d->timeLineWidget->currentSelectionInfo(start, end);

    QString txt = i18n("%1 to %2 : %3")
                  .arg(KGlobal::locale()->formatDate(start.date(), true))
                  .arg(KGlobal::locale()->formatDate(end.date(), true))
                  .arg(val);

    d->infoLabel->setText(txt);

    QValueList<QDateTime> days;
    QDateTime current = start;
    current.setTime(QTime());

    do
    {
        days.append(current);
        current = current.addDays(1);
    }
    while (current < end);

    emit signalSelectionChanged(days);
}

}  // NameSpace Digikam
