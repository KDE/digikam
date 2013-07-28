/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-24
 * Description : a progress bar used to display action
 *               progress or a text in status bar.
 *               Progress events are dispatched to ProgressManager.
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "statusprogressbar.moc"

// Qt includes

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QProgressBar>

// KDE includes

#include <ksqueezedtextlabel.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <kdebug.h>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class StatusProgressBar::Private
{

public:

    enum WidgetStackEnum
    {
        TextLabel=0,
        ProgressBar
    };

    Private() :
        notify(false),
        progressWidget(0),
        cancelButton(0),
        progressBar(0),
        textLabel(0)
    {
    }

    // For Progress Manager item
    bool                notify;
    QString             progressId;
    QString             title;
    QPixmap             icon;

    QWidget*            progressWidget;
    QPushButton*        cancelButton;
    QProgressBar*       progressBar;

    KSqueezedTextLabel* textLabel;
};

StatusProgressBar::StatusProgressBar(QWidget* const parent)
    : QStackedWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->textLabel            = new KSqueezedTextLabel(this);
    d->progressWidget       = new QWidget(this);
    QHBoxLayout* const hBox = new QHBoxLayout(d->progressWidget);
    d->progressBar          = new QProgressBar(d->progressWidget);
    d->cancelButton         = new QPushButton(d->progressWidget);
    d->cancelButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    d->cancelButton->setFocusPolicy(Qt::NoFocus);
    d->cancelButton->setIcon(SmallIcon("dialog-cancel"));
    setProgressTotalSteps(100);

    // Parent widget will probably have the wait cursor set.
    // Set arrow cursor to indicate the button can be clicked
    d->cancelButton->setCursor(Qt::ArrowCursor);

    hBox->addWidget(d->progressBar);
    hBox->addWidget(d->cancelButton);
    hBox->setMargin(0);
    hBox->setSpacing(0);

    insertWidget(Private::TextLabel,   d->textLabel);
    insertWidget(Private::ProgressBar, d->progressWidget);

    connect(d->cancelButton, SIGNAL(clicked()),
            this, SIGNAL(signalCancelButtonPressed()) );

    progressBarMode(TextMode);
}

StatusProgressBar::~StatusProgressBar()
{
    delete d;
}

void StatusProgressBar::setNotify(bool b)
{
    d->notify = b;
}

void StatusProgressBar::setNotificationTitle(const QString& title, const QPixmap& icon)
{
    d->title = title;
    d->icon  = icon;
}

void StatusProgressBar::setText(const QString& text)
{
    d->textLabel->setText(text);
}

void StatusProgressBar::setAlignment(Qt::Alignment a)
{
    d->textLabel->setAlignment(a);
}

int StatusProgressBar::progressValue() const
{
    return d->progressBar->value();
}

void StatusProgressBar::setProgressValue(int v)
{
    d->progressBar->setValue(v);

    if (d->notify)
    {
        ProgressItem* const item = currentProgressItem();
        if (item)
        {
            item->setCompletedItems(v);
            item->updateProgress();
        }
    }
}

int StatusProgressBar::progressTotalSteps() const
{
    return d->progressBar->maximum();
}

void StatusProgressBar::setProgressTotalSteps(int v)
{
    d->progressBar->setMaximum(v);

    if (d->notify)
    {
        ProgressItem* const item = currentProgressItem();
        if (item)
            item->setTotalItems(v);
    }
}

void StatusProgressBar::setProgressText(const QString& text)
{
    d->progressBar->setFormat(text + QString("%p%"));
    d->progressBar->update();

    if (d->notify)
    {
        ProgressItem* const item = currentProgressItem();
        if (item)
            item->setStatus(text);
    }
}

void StatusProgressBar::progressBarMode(int mode, const QString& text)
{
    if (mode == TextMode)
    {
        setCurrentIndex(Private::TextLabel);
        setProgressValue(0);
        setText(text);

        if (d->notify)
        {
            ProgressItem* const item = currentProgressItem();
            if (item)
                item->setComplete();
        }
    }
    else if (mode == ProgressBarMode)
    {
        d->cancelButton->hide();
        setCurrentIndex(Private::ProgressBar);
        setProgressText(text);

        if (d->notify)
        {
            ProgressItem* const item = ProgressManager::createProgressItem(d->title, QString(), false, !d->icon.isNull());
            item->setTotalItems(d->progressBar->maximum());
            item->setCompletedItems(d->progressBar->value());
            
            if (!d->icon.isNull())
                item->setThumbnail(d->icon);
            
            connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                    this, SIGNAL(signalCancelButtonPressed()));
            
            d->progressId = item->id();
        }
    }
    else // CancelProgressBarMode
    {
        d->cancelButton->show();
        setCurrentIndex(Private::ProgressBar);
        setProgressText(text);

        if (d->notify)
        {
            ProgressItem* const item = ProgressManager::createProgressItem(d->title, QString(), true, !d->icon.isNull());
            item->setTotalItems(d->progressBar->maximum());
            item->setCompletedItems(d->progressBar->value());
            
            if (!d->icon.isNull())
                item->setThumbnail(d->icon);
            
            connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                    this, SIGNAL(signalCancelButtonPressed()));
            
            d->progressId = item->id();
        }
    }
}

ProgressItem* StatusProgressBar::currentProgressItem() const
{
    return (ProgressManager::instance()->findItembyId(d->progressId));
}

}  // namespace Digikam
