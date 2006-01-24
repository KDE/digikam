/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-24
 * Description : a progress bar used to display io file acess 
 *               progressor or the current file name.
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
  
#include "qlabel.h"
 
// KDE includes.

#include "kprogress.h"

// Local includes.

#include "iofileprogressbar.h"

namespace Digikam
{

class IOFileProgressBarPriv
{
    
public:

    enum WidgetVisible
    {
        FileNameLabel=0,      
        FileAcessProgressBar
    };

    IOFileProgressBarPriv()
    {
        fileNameLabel        = 0; 
        fileAcessProgressBar = 0;
    }

    QLabel    *fileNameLabel;

    KProgress *fileAcessProgressBar;
};

IOFileProgressBar::IOFileProgressBar(QWidget *parent)
                 : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new IOFileProgressBarPriv;
    d->fileNameLabel        = new QLabel(this);
    d->fileAcessProgressBar = new KProgress(this);
    d->fileAcessProgressBar->setTotalSteps(100);

    addWidget(d->fileNameLabel, IOFileProgressBarPriv::FileNameLabel);
    addWidget(d->fileAcessProgressBar, IOFileProgressBarPriv::FileAcessProgressBar);
    progressBarVisible(false);
}      

IOFileProgressBar::~IOFileProgressBar()
{
    delete d;
}
    
void IOFileProgressBar::setText( QString text )
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

void IOFileProgressBar::progressBarVisible(bool v)
{
    if (v)
    {
        raiseWidget(IOFileProgressBarPriv::FileAcessProgressBar);
    }
    else
        raiseWidget(IOFileProgressBarPriv::FileNameLabel);
}

}  // namespace Digikam

#include "iofileprogressbar.moc"
