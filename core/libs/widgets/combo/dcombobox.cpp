/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-16
 * Description : a combo box widget re-implemented with a
 *               reset button to switch to a default item
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dcombobox.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QToolButton>
#include <QHBoxLayout>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN DComboBox::Private
{

public:

    Private()
    {
        defaultIndex = 0;
        resetButton  = 0;
        combo        = 0;
    }

    int          defaultIndex;

    QToolButton* resetButton;
    QComboBox*   combo;
};

DComboBox::DComboBox(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QHBoxLayout* const hlay = new QHBoxLayout(this);
    d->combo                = new QComboBox(this);
    d->resetButton          = new QToolButton(this);
    d->resetButton->setAutoRaise(true);
    d->resetButton->setFocusPolicy(Qt::NoFocus);
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip(i18nc("@info:tooltip", "Reset to default value"));

    hlay->addWidget(d->combo);
    hlay->addWidget(d->resetButton);
    hlay->setStretchFactor(d->combo, 10);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // -------------------------------------------------------------

    connect(d->resetButton, &QToolButton::clicked,
            this, &DComboBox::slotReset);

    connect(d->combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &DComboBox::slotItemActivated);

    connect(d->combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DComboBox::slotCurrentIndexChanged);
}

DComboBox::~DComboBox()
{
    delete d;
}

QComboBox* DComboBox::combo() const
{
    return d->combo;
}

void DComboBox::addItem(const QString& t, int index)
{
    d->combo->addItem(t, index);
}

void DComboBox::insertItem(int index, const QString& t)
{
    d->combo->insertItem(index, t);
}

int DComboBox::currentIndex() const
{
    return d->combo->currentIndex();
}

void DComboBox::setCurrentIndex(int v)
{
    d->combo->setCurrentIndex(v);
}

int DComboBox::defaultIndex() const
{
    return d->defaultIndex;
}

void DComboBox::setDefaultIndex(int v)
{
    d->defaultIndex = v;
    d->combo->setCurrentIndex(d->defaultIndex);
    slotItemActivated(v);
}

void DComboBox::slotReset()
{
    d->combo->setCurrentIndex(d->defaultIndex);
    d->resetButton->setEnabled(false);
    slotItemActivated(d->defaultIndex);
    emit reset();
}

void DComboBox::slotItemActivated(int v)
{
    d->resetButton->setEnabled(v != d->defaultIndex);
    emit activated(v);
}

void DComboBox::slotCurrentIndexChanged(int v)
{
    d->resetButton->setEnabled(v != d->defaultIndex);
    emit currentIndexChanged(v);
}

}  // namespace Digikam
