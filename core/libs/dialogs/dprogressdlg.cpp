/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : a progress dialog for digiKam
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dprogressdlg.h"

// Qt includes

#include <QHeaderView>
#include <QStyle>
#include <QLabel>
#include <QImage>
#include <QGridLayout>
#include <QProgressBar>
#include <QTreeWidget>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// Local includes

#include "dexpanderbox.h"

namespace Digikam
{

class DProgressDlg::Private
{
public:

    Private() :
        logo(0),
        title(0),
        label(0),
        actionPix(0),
        actionLabel(0),
        progress(0),
        buttons(0)
    {
    }

    QLabel*           logo;
    QLabel*           title;
    QLabel*           label;

    QLabel*           actionPix;
    DAdjustableLabel* actionLabel;

    QProgressBar*     progress;

    QDialogButtonBox* buttons;
};

DProgressDlg::DProgressDlg(QWidget* const parent, const QString& caption)
    : QDialog(parent),
      d(new Private)
{
    setModal(true);
    setWindowTitle(caption);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);

    QWidget* const page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(page);

    d->actionPix      = new QLabel(page);
    d->actionLabel    = new DAdjustableLabel(page);
    d->logo           = new QLabel(page);
    d->progress       = new QProgressBar(page);
    d->title          = new QLabel(page);
    d->label          = new QLabel(page);
    d->actionPix->setFixedSize(QSize(32, 32));

    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    grid->addWidget(d->logo,        0, 0, 3, 1);
    grid->addWidget(d->label,       0, 1, 1, 2);
    grid->addWidget(d->actionPix,   1, 1, 1, 1);
    grid->addWidget(d->actionLabel, 1, 2, 1, 1);
    grid->addWidget(d->progress,    2, 1, 1, 2);
    grid->addWidget(d->title,       3, 1, 1, 2);
    grid->setSpacing(style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->setContentsMargins(QMargins());
    grid->setColumnStretch(2, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(slotCancel()));

    adjustSize();
    reset();
}

DProgressDlg::~DProgressDlg()
{
    delete d;
}

void DProgressDlg::slotCancel()
{
    emit signalCancelPressed();

    close();
}

void DProgressDlg::setButtonText(const QString& text)
{
    d->buttons->button(QDialogButtonBox::Cancel)->setText(text);
}

void DProgressDlg::addedAction(const QPixmap& itemPix, const QString& text)
{
    QPixmap pix = itemPix;

    if (pix.isNull())
    {
        pix = QIcon::fromTheme(QLatin1String("image-missing")).pixmap(32);
    }
    else
    {
        pix = pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    d->actionPix->setPixmap(pix);
    d->actionLabel->setAdjustedText(text);
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

}  // namespace Digikam
