/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-16
 * Description : Dialog to prompt users about versioning
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "versioningpromptusersavedlg.h"

// Qt includes

#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QStyle>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class VersioningPromptUserSaveDialog::Private
{
public:

    Private()
        : clicked(QDialogButtonBox::NoButton),
          buttons(0)
    {
    }

public:

    QDialogButtonBox::StandardButton clicked;
    QDialogButtonBox*                buttons;
};

VersioningPromptUserSaveDialog::VersioningPromptUserSaveDialog(QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setWindowTitle(i18nc("@title:window", "Save?"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Ok      |  // Save Changes
                                      QDialogButtonBox::Apply   |  // Save Changes as a New Version
                                      QDialogButtonBox::Discard |  // Discard Changes
                                      QDialogButtonBox::Cancel,
                                      this);

    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);

    d->buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button", "Save Changes"));
    d->buttons->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    d->buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@info:tooltip",
                                                               "Save the current changes. Note: The original image will never be overwritten."));

    d->buttons->button(QDialogButtonBox::Apply)->setText(i18nc("@action:button", "Save Changes as a New Version"));
    d->buttons->button(QDialogButtonBox::Apply)->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->buttons->button(QDialogButtonBox::Apply)->setToolTip(i18nc("@info:tooltip",
                                                                  "Save the current changes as a new version. "
                                                                  "The loaded file will remain unchanged, a new file will be created."));

    d->buttons->button(QDialogButtonBox::Discard)->setText(i18nc("@action:button", "Discard Changes"));
    d->buttons->button(QDialogButtonBox::Discard)->setIcon(QIcon::fromTheme(QLatin1String("task-reject")));
    d->buttons->button(QDialogButtonBox::Discard)->setToolTip(i18nc("@info:tooltip",
                                                                    "Discard the changes applied to the image during this editing session."));

    connect(d->buttons, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(slotButtonClicked(QAbstractButton*)));

    QWidget* const mainWidget = new QWidget;

    // -- Icon and Header --

    QLabel* const warningIcon = new QLabel;
    warningIcon->setPixmap(QIcon::fromTheme(QLatin1String("dialog-warning")).pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this)));
    QLabel* const question    = new QLabel;
    question->setTextFormat(Qt::RichText);
    question->setText(i18nc("@label",
                            "<qt>The current image has been changed.<br>"
                            "Do you wish to save your changes?</qt>"));
    QLabel* const editIcon    = new QLabel;
    editIcon->setPixmap(QIcon::fromTheme(QLatin1String("document-edit")).pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this)));

    QHBoxLayout* const headerLayout = new QHBoxLayout;
    headerLayout->addWidget(warningIcon);
    headerLayout->addWidget(question, 10, Qt::AlignCenter);
    headerLayout->addWidget(editIcon);
    mainWidget->setLayout(headerLayout);

    // -- Layouts --

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);
}

VersioningPromptUserSaveDialog::~VersioningPromptUserSaveDialog()
{
    delete d;
}

void VersioningPromptUserSaveDialog::slotButtonClicked(QAbstractButton* button)
{
    d->clicked = d->buttons->standardButton(button);

    if (d->clicked == QDialogButtonBox::Cancel)
    {
        reject();
        return;
    }

    accept();
}

bool VersioningPromptUserSaveDialog::shallSave() const
{
    return (d->clicked == QDialogButtonBox::Ok);
}

bool VersioningPromptUserSaveDialog::newVersion() const
{
    return (d->clicked == QDialogButtonBox::Apply);
}

bool VersioningPromptUserSaveDialog::shallDiscard() const
{
    return (d->clicked == QDialogButtonBox::Discard);
}

} // namespace Digikam
