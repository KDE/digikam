/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-30-08
 * Description :a progress dialog for digiKam
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

#include <qlayout.h>
#include <qwhatsthis.h>
#include <qheader.h>
#include <qlabel.h>
#include <qpushbutton.h>

// KDE includes.

#include <klocale.h>
#include <kprogress.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kstandarddirs.h>

// Local includes

#include "dprogressdlg.h"

namespace Digikam
{

class DProgressDlgPriv
{
public:

    DProgressDlgPriv()
    {
        progress    = 0;
        actionsList = 0;
        logo        = 0;
        message     = 0;
    }

    QLabel    *logo;
    QLabel    *message;
    
    KListView *actionsList;
   
    KProgress *progress;
};

DProgressDlg::DProgressDlg(QWidget *parent, const QString &caption)
            : KDialogBase(parent, 0, true, caption, Cancel)
{
    d = new DProgressDlgPriv;
    
    QFrame *page      = makeMainWidget();
    QGridLayout* grid = new QGridLayout(page, 1, 1, 0, spacingHint());
    QVBoxLayout *vlay = new QVBoxLayout();
    d->actionsList    = new KListView(page);
    d->message        = new QLabel(page);
    d->logo           = new QLabel(page);
    d->progress       = new KProgress(page);
    vlay->addWidget(d->logo);
    vlay->addWidget(d->progress);
    vlay->addStretch();

    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    d->logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));
     
    d->actionsList->addColumn("Status");  // no i18n here: column hiden
    d->actionsList->setSorting(-1);
    d->actionsList->setItemMargin(1);
    d->actionsList->header()->hide();
    d->actionsList->setResizeMode(QListView::LastColumn);

    grid->addMultiCellLayout(vlay, 0, 1, 0, 0);
    grid->addMultiCellWidget(d->message, 0, 0, 1, 1);
    grid->addMultiCellWidget(d->actionsList, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setColStretch(1, 10);
}

DProgressDlg::~DProgressDlg()
{
    delete d;
}

void DProgressDlg::setButtonText(const QString &text)
{
    KDialogBase::setButtonText(Cancel, text);
}

void DProgressDlg::addedAction(const QString &text)
{
    KListViewItem *item = new KListViewItem(d->actionsList, d->actionsList->lastItem(), text);
    d->actionsList->ensureItemVisible(item);
}

void DProgressDlg::reset()
{
    d->actionsList->clear();
    d->progress->setValue(0);
}

void DProgressDlg::setTotalSteps(int total)
{
    d->progress->setTotalSteps(total);
}
    
void DProgressDlg::setValue(int value)
{
    d->progress->setValue(value);
}

void DProgressDlg::advance(int value)
{
    d->progress->advance(value);
}

void DProgressDlg::setLabel(const QString &text)
{
    d->message->setText(text);
}

}  // NameSpace Digikam

#include "dprogressdlg.moc"
