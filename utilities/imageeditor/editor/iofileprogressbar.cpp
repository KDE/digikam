/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-24
 * Description : a progress bar used to display io file access
 *               progress or the current file name.
 *
 * Copyright 2006 by Gilles Caulier
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

#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpushbutton.h>

// KDE includes.

#include <kprogress.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcursor.h>

// Local includes.

#include "iofileprogressbar.h"

namespace Digikam
{

class IOFileProgressBarPriv
{

public:

    enum WidgetStackEnum
    {
        FileNameLabel=0,
        FileAcessProgressBar
    };

    IOFileProgressBarPriv()
    {
        fileNameLabel        = 0;
        fileAcessProgressBar = 0;
        progressWidget       = 0;
        cancelButton         = 0;
    }

    QLabel      *fileNameLabel;

    QWidget     *progressWidget;

    QPushButton *cancelButton;

    KProgress   *fileAcessProgressBar;
};

IOFileProgressBar::IOFileProgressBar(QWidget *parent)
                 : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new IOFileProgressBarPriv;

    d->fileNameLabel        = new QLabel(this);
    d->progressWidget       = new QWidget(this);
    QHBoxLayout *hBox       = new QHBoxLayout(d->progressWidget);
    d->fileAcessProgressBar = new KProgress(d->progressWidget);
    d->fileAcessProgressBar->setTotalSteps(100);
    d->cancelButton = new QPushButton(d->progressWidget);
    d->cancelButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    d->cancelButton->setPixmap( SmallIcon( "cancel" ) );

    // Parent widget will probably have the wait cursor set.
    // Set arrow cursor to indicate the button can be clicked
    d->cancelButton->setCursor( KCursor::arrowCursor() );

    hBox->addWidget(d->fileAcessProgressBar);
    hBox->addWidget(d->cancelButton);

    addWidget(d->fileNameLabel, IOFileProgressBarPriv::FileNameLabel);
    addWidget(d->progressWidget, IOFileProgressBarPriv::FileAcessProgressBar);
    setMaximumHeight( fontMetrics().height() );

    connect( d->cancelButton, SIGNAL( clicked() ),
             this, SIGNAL( signalCancelButtonPressed() ) );

    progressBarMode(FileNameMode);
}

IOFileProgressBar::~IOFileProgressBar()
{
    delete d;
}

void IOFileProgressBar::setText( const QString& text )
{
    d->fileNameLabel->setText(text);
}

void IOFileProgressBar::setAlignment(int a)
{
    d->fileNameLabel->setAlignment(a);
}

void IOFileProgressBar::setProgressValue( int v )
{
    d->fileAcessProgressBar->setProgress(v);
}

void IOFileProgressBar::setProgressText( const QString& text )
{
    d->fileAcessProgressBar->setFormat( text + QString ("%p%") );
}

void IOFileProgressBar::progressBarMode( int mode, const QString& text )
{
    if ( mode == FileNameMode)
    {
        raiseWidget(IOFileProgressBarPriv::FileNameLabel);
        setProgressValue(0);
    }
    else if ( mode == ProgressBarMode )
    {
        d->cancelButton->hide();
        raiseWidget(IOFileProgressBarPriv::FileAcessProgressBar);
    }
    else  // CancelProgressBarMode
    {
        d->cancelButton->show();
        raiseWidget(IOFileProgressBarPriv::FileAcessProgressBar);
    }

    setProgressText( text );
}

}  // namespace Digikam

#include "iofileprogressbar.moc"
