/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : export Tool dialog
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tooldialog.h"

// Qt includes

#include <QIcon>
#include <QAction>
#include <QDialog>
#include <QMenu>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class ToolDialog::Private
{
public:
    Private()
        : buttonBox(0),
          startButton(0),
          mainWidget(0),
          propagateReject(true)
    {
    }

    QDialogButtonBox* buttonBox;
    QPushButton*      startButton;
    QWidget*          mainWidget;

    bool              propagateReject;
};

ToolDialog::ToolDialog(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    d->buttonBox   = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Help, this);
    d->startButton = new QPushButton(i18nc("@action:button", "&Start"), this);
    d->startButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-start")));
    d->buttonBox->addButton(d->startButton, QDialogButtonBox::ActionRole);
    d->buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(d->buttonBox);
    setLayout(mainLayout);

    connect(d->buttonBox, &QDialogButtonBox::rejected,
            this, &ToolDialog::slotCloseClicked);
}

ToolDialog::~ToolDialog()
{
    delete d;
}

void ToolDialog::setMainWidget(QWidget* const widget)
{
    if (d->mainWidget == widget)
    {
        return;
    }

    layout()->removeWidget(d->buttonBox);

    if (d->mainWidget)
    {
        // Replace existing widget
        layout()->removeWidget(d->mainWidget);
        delete d->mainWidget;
    }

    d->mainWidget = widget;
    layout()->addWidget(d->mainWidget);
    layout()->addWidget(d->buttonBox);
}

void ToolDialog::setRejectButtonMode(QDialogButtonBox::StandardButton button)
{
    if (button == QDialogButtonBox::Close)
    {
        d->buttonBox->button(QDialogButtonBox::Close)->setText(i18n("Close"));
        d->buttonBox->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QString::fromLatin1("window-close")));
        d->buttonBox->button(QDialogButtonBox::Close)->setToolTip(i18n("Close window"));
        d->propagateReject = true;
    }
    else if (button == QDialogButtonBox::Cancel)
    {
        d->buttonBox->button(QDialogButtonBox::Close)->setText(i18n("Cancel"));
        d->buttonBox->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QString::fromLatin1("dialog-cancel")));
        d->buttonBox->button(QDialogButtonBox::Close)->setToolTip(i18n("Cancel current operation"));
        d->propagateReject = false;
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Unexpected button mode passed";
    }
}

QPushButton* ToolDialog::startButton() const
{
    return d->startButton;
}

void ToolDialog::addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role)
{
    d->buttonBox->addButton(button, role);
}

void ToolDialog::slotCloseClicked()
{
    if (d->propagateReject)
    {
        reject();
    }
    else
    {
        emit cancelClicked();
    }
}

} // namespace Digikam
