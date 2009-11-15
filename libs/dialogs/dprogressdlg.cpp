/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : a progress dialog for digiKam
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dprogressdlg.moc"

// Qt includes

#include <QHeaderView>
#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTreeWidget>

// KDE includes


#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>


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
        title       = 0;
        label       = 0;
        allowCancel = true;
        cancelled   = false;
    }

    bool          allowCancel;
    bool          cancelled;

    QLabel       *logo;
    QLabel       *title;
    QLabel       *label;

    QTreeWidget  *actionsList;

    QProgressBar *progress;
};

DProgressDlg::DProgressDlg(QWidget *parent, const QString& caption)
            : KDialog(parent), d(new DProgressDlgPriv)
{
    setCaption(caption);
    setButtons(Cancel);
    setDefaultButton(Cancel);
    setModal(true);

    QWidget *page     = new QWidget(this);
    setMainWidget(page);

    QGridLayout* grid = new QGridLayout(page);

    QVBoxLayout *vlay = new QVBoxLayout();
    d->actionsList    = new QTreeWidget(page);
    d->label          = new QLabel(page);
    d->title          = new QLabel(page);
    d->logo           = new QLabel(page);
    d->progress       = new QProgressBar(page);
    d->label->setWordWrap(true);

    vlay->addWidget(d->logo);
    vlay->addWidget(d->progress);
    vlay->addWidget(d->title);
    vlay->addStretch();

    d->logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                       .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->actionsList->setSortingEnabled(false);
    d->actionsList->setRootIsDecorated(false);
    d->actionsList->setSelectionMode(QAbstractItemView::NoSelection);
    d->actionsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->actionsList->setAllColumnsShowFocus(true);
    d->actionsList->setColumnCount(2);
    d->actionsList->header()->hide();
    d->actionsList->setColumnWidth(0, 40);
    d->actionsList->setIconSize(QSize(32, 32));

    QStringList labels;
    labels.append("Thumb");        // no i18n here: hidden header
    labels.append("Status");       // no i18n here: hidden header
    d->actionsList->setHeaderLabels(labels);

    grid->addLayout(vlay,           0, 0, 2, 1);
    grid->addWidget(d->label,       0, 1, 1, 1);
    grid->addWidget(d->actionsList, 1, 1, 1, 1);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setRowStretch(1, 10);
    grid->setColumnStretch(1, 10);

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancel()));

    reset();
}

DProgressDlg::~DProgressDlg()
{
    delete d;
}

void DProgressDlg::slotCancel()
{
    d->cancelled = true;

    if (d->allowCancel)
    {
        close();
    }

    emit signalCancelPressed();
}

void DProgressDlg::setButtonText(const QString& text)
{
    KDialog::setButtonText(Cancel, text);
}

void DProgressDlg::setButtonGuiItem(const KGuiItem& item)
{
    KDialog::setButtonGuiItem(Cancel, item);
}

void DProgressDlg::addedAction(const QPixmap& itemPix, const QString& text)
{
    QPixmap pix = itemPix;
    QTreeWidgetItem *item = new QTreeWidgetItem(d->actionsList, QStringList() << QString() << text);

    if (pix.isNull())
    {
        pix = DesktopIcon("image-missing", KIconLoader::SizeMedium);    // 32x32 px
    }
    else
    {
        pix = pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    item->setIcon(0, pix);
    d->actionsList->scrollToItem(item);
}

void DProgressDlg::reset()
{
    d->actionsList->clear();
    d->progress->setValue(0);
}

void DProgressDlg::setMaximum(int max)
{
    d->progress->setMaximum(max);
}

void DProgressDlg::incrementMaximum(int added)
{
    d->progress->setMaximum(d->progress->maximum() + added);
}

void DProgressDlg::setValue(int value)
{
    d->progress->setValue(value);
}

void DProgressDlg::advance(int offset)
{
    d->progress->setValue(d->progress->value() + offset);
}

int DProgressDlg::value()
{
    return d->progress->value();
}

void DProgressDlg::setLabel(const QString& text)
{
    d->label->setText(text);
}

void DProgressDlg::setTitle(const QString& text)
{
    d->title->setText(text);
}

void DProgressDlg::showCancelButton(bool show)
{
    showButton(Cancel, show);
}

void DProgressDlg::setAllowCancel(bool allowCancel)
{
    d->allowCancel = allowCancel;
    showButton(Cancel, allowCancel);
}

bool DProgressDlg::allowCancel() const
{
    return d->allowCancel;
}

bool DProgressDlg::wasCancelled() const
{
    return d->cancelled;
}

void DProgressDlg::setActionListVSBarVisible(bool visible)
{
    if (!visible)
        d->actionsList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    else
        d->actionsList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

}  // namespace Digikam
