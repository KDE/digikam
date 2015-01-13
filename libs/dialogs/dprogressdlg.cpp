/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : a progress dialog for digiKam
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <ksqueezedtextlabel.h>

namespace Digikam
{

class DProgressDlg::Private
{
public:

    Private() :
        allowCancel(true),
        cancelled(false),
        logo(0),
        title(0),
        label(0),
        actionPix(0),
        actionLabel(0),
        progress(0)
    {
    }

    bool                allowCancel;
    bool                cancelled;

    QLabel*             logo;
    QLabel*             title;
    QLabel*             label;

    QLabel*             actionPix;
    KSqueezedTextLabel* actionLabel;

    QProgressBar*       progress;
};

DProgressDlg::DProgressDlg(QWidget* const parent, const QString& caption)
    : KDialog(parent), d(new Private)
{
    setCaption(caption);
    setButtons(Cancel);
    setDefaultButton(Cancel);
    setModal(true);

    QWidget* page     = new QWidget(this);
    setMainWidget(page);

    QGridLayout* grid = new QGridLayout(page);

    d->actionPix      = new QLabel(page);
    d->actionLabel    = new KSqueezedTextLabel(page);
    d->logo           = new QLabel(page);
    d->progress       = new QProgressBar(page);
    d->title          = new QLabel(page);
    d->label          = new QLabel(page);
    d->label->setWordWrap(true);
    d->actionPix->setFixedSize(QSize(32, 32));

    d->logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                       .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    grid->addWidget(d->logo,        0, 0, 3, 1);
    grid->addWidget(d->label,       0, 1, 1, 2);
    grid->addWidget(d->actionPix,   1, 1, 1, 1);
    grid->addWidget(d->actionLabel, 1, 2, 1, 1);
    grid->addWidget(d->progress,    2, 1, 1, 2);
    grid->addWidget(d->title,       3, 1, 1, 2);
    grid->setSpacing(spacingHint());
    grid->setMargin(0);
    grid->setColumnStretch(2, 10);

    setInitialSize(QSize(500, 150));

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

    emit signalCancelPressed();

    if (d->allowCancel)
    {
        close();
    }
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

    if (pix.isNull())
    {
        pix = DesktopIcon("image-missing", KIconLoader::SizeMedium);    // 32x32 px
    }
    else
    {
        pix = pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    d->actionPix->setPixmap(pix);
    d->actionLabel->setText(text);
}

void DProgressDlg::reset()
{
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

int DProgressDlg::value() const
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

}  // namespace Digikam
