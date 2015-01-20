/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-09-16
 * Description : Dialog to prompt users about versioning
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QLabel>
#include <QIcon>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

VersioningPromptUserSaveDialog::VersioningPromptUserSaveDialog(QWidget* const parent, bool allowCancel)
    : TripleChoiceDialog(parent)
{
    setPlainCaption(i18nc("@title:window", "Save?"));
    setShowCancelButton(allowCancel);

    QWidget* const mainWidget = new QWidget;

    // -- Icon and Header --

    QLabel* const warningIcon = new QLabel;
    warningIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this)));
    QLabel* const editIcon    = new QLabel;
    editIcon->setPixmap(QIcon::fromTheme("document-edit").pixmap(iconSize()));
    QLabel* const question    = new QLabel;
    question->setTextFormat(Qt::RichText);
    question->setText(i18nc("@label", "<qt>The current image has been changed.<br>"
                            "Do you wish to save your changes?</qt>"));

    QHBoxLayout* const headerLayout = new QHBoxLayout;
    headerLayout->addWidget(question);
    headerLayout->addWidget(editIcon);

    // -- Central buttons --

    QToolButton* const saveCurrent = addChoiceButton(Ok, "dialog-ok-apply", i18nc("@action:button", "Save Changes"));
    saveCurrent->setToolTip(i18nc("@info:tooltip",
                                  "Save the current changes. Note: The original image will never be overwritten."));

    QToolButton* const saveVersion = addChoiceButton(Apply, "list-add", i18nc("@action:button", "Save Changes as a New Version"));
    saveVersion->setToolTip(i18nc("@info:tooltip",
                                  "Save the current changes as a new version. "
                                  "The loaded file will remain unchanged, a new file will be created."));

    QToolButton* const discard     = addChoiceButton(User1, "task-reject", i18nc("@action:button", "Discard Changes"));
    discard->setToolTip(i18nc("@info:tooltip",
                              "Discard the changes applied to the image during this editing session."));

    // -- Layout --

    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(warningIcon,       0, 0, 2, 1, Qt::AlignTop);
    mainLayout->addLayout(headerLayout,      0, 1);
    mainLayout->addWidget(buttonContainer(), 1, 1);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);
}

VersioningPromptUserSaveDialog::~VersioningPromptUserSaveDialog()
{
}

bool VersioningPromptUserSaveDialog::shallSave() const
{
    return (clickedButton() == Ok);
}

bool VersioningPromptUserSaveDialog::newVersion() const
{
    return (clickedButton() == Apply);
}

bool VersioningPromptUserSaveDialog::shallDiscard() const
{
    return (clickedButton() == User1);
}

} // namespace Digikam
