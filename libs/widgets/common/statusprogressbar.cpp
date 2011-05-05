/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-24
 * Description : a progress bar used to display file access
 *               progress or a text in status bar.
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class StatusProgressBar::StatusProgressBarPriv
{

public:

    enum WidgetStackEnum
    {
        TextLabel=0,
        ProgressBar
    };

    StatusProgressBarPriv() :
        progressWidget(0),
        cancelButton(0),
        progressBar(0),
        textLabel(0)
    {
    }

    QWidget*            progressWidget;
    QPushButton*        cancelButton;
    QProgressBar*       progressBar;

    KSqueezedTextLabel* textLabel;
};

StatusProgressBar::StatusProgressBar(QWidget* parent)
    : QStackedWidget(parent), d(new StatusProgressBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->textLabel      = new KSqueezedTextLabel(this);
    d->progressWidget = new QWidget(this);
    QHBoxLayout* hBox = new QHBoxLayout(d->progressWidget);
    d->progressBar    = new QProgressBar(d->progressWidget);
    d->cancelButton   = new QPushButton(d->progressWidget);
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

    insertWidget(StatusProgressBarPriv::TextLabel,   d->textLabel);
    insertWidget(StatusProgressBarPriv::ProgressBar, d->progressWidget);

    connect(d->cancelButton, SIGNAL( clicked() ),
            this, SIGNAL( signalCancelButtonPressed() ) );

    progressBarMode(TextMode);
}

StatusProgressBar::~StatusProgressBar()
{
    delete d;
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
}

int StatusProgressBar::progressTotalSteps() const
{
    return d->progressBar->maximum();
}

void StatusProgressBar::setProgressTotalSteps(int v)
{
    d->progressBar->setMaximum(v);
}

void StatusProgressBar::setProgressText(const QString& text)
{
    d->progressBar->setFormat(text + QString("%p%"));
    d->progressBar->update();
}

void StatusProgressBar::progressBarMode(int mode, const QString& text)
{
    if (mode == TextMode)
    {
        setCurrentIndex(StatusProgressBarPriv::TextLabel);
        setProgressValue(0);
        setText(text);
    }
    else if (mode == ProgressBarMode)
    {
        d->cancelButton->hide();
        setCurrentIndex(StatusProgressBarPriv::ProgressBar);
        setProgressText(text);
    }
    else // CancelProgressBarMode
    {
        d->cancelButton->show();
        setCurrentIndex(StatusProgressBarPriv::ProgressBar);
        setProgressText(text);
    }
}

}  // namespace Digikam
