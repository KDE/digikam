/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : Web Service Tool dialog
 *
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "wstooldialog.h"

// Qt includes

#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dpluginaboutdlg.h"

namespace Digikam
{

class Q_DECL_HIDDEN WSToolDialog::Private
{
public:

    explicit Private()
        : startButton(0),
          mainWidget(0),
          propagateReject(true)
    {
    }

    QPushButton*      startButton;
    QWidget*          mainWidget;

    bool              propagateReject;
};

WSToolDialog::WSToolDialog(QWidget* const parent, const QString& objName)
    : DPluginDialog(parent, objName),
      d(new Private)
{
    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->button(QDialogButtonBox::Close)->setDefault(true);
    d->startButton = new QPushButton(i18nc("@action:button", "&Start"), this);
    d->startButton->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
    m_buttons->addButton(d->startButton, QDialogButtonBox::ActionRole);

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_buttons);
    setLayout(mainLayout);

    connect(m_buttons, &QDialogButtonBox::rejected,
            this, &WSToolDialog::slotCloseClicked);
}

WSToolDialog::~WSToolDialog()
{
    delete d;
}

void WSToolDialog::setMainWidget(QWidget* const widget)
{
    if (d->mainWidget == widget)
    {
        return;
    }

    layout()->removeWidget(m_buttons);

    if (d->mainWidget)
    {
        // Replace existing widget
        layout()->removeWidget(d->mainWidget);
        delete d->mainWidget;
    }

    d->mainWidget = widget;
    layout()->addWidget(d->mainWidget);
    layout()->addWidget(m_buttons);
}

void WSToolDialog::setRejectButtonMode(QDialogButtonBox::StandardButton button)
{
    if (button == QDialogButtonBox::Close)
    {
        m_buttons->button(QDialogButtonBox::Close)->setText(i18n("Close"));
        m_buttons->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QLatin1String("window-close")));
        m_buttons->button(QDialogButtonBox::Close)->setToolTip(i18n("Close window"));
        d->propagateReject = true;
    }
    else if (button == QDialogButtonBox::Cancel)
    {
        m_buttons->button(QDialogButtonBox::Close)->setText(i18n("Cancel"));
        m_buttons->button(QDialogButtonBox::Close)->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));
        m_buttons->button(QDialogButtonBox::Close)->setToolTip(i18n("Cancel current operation"));
        d->propagateReject = false;
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Unexpected button mode passed";
    }
}

QPushButton* WSToolDialog::startButton() const
{
    return d->startButton;
}

void WSToolDialog::addButton(QAbstractButton* button, QDialogButtonBox::ButtonRole role)
{
    m_buttons->addButton(button, role);
}

void WSToolDialog::slotCloseClicked()
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
