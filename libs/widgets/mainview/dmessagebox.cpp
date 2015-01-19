/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-01-19
 * Description : message box notification settings
 *
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dmessagebox.h"

// Qt includes

#include <QListWidget>
#include <QPointer>
#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QLabel>
#include <QHeaderView>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QIcon>
#include <QDialog>
#include <QPushButton>
#include <QDialogButtonBox>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

namespace Digikam
{

bool DMessageBox::readMsgBoxShouldBeShown(const QString& dontShowAgainName)
{
    if (dontShowAgainName.isEmpty()) return true;

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group("Notification Messages");
    bool value              = group.readEntry(dontShowAgainName, true);

    return value;
}
    
void DMessageBox::saveMsgBoxShouldBeShown(const QString& dontShowAgainName, bool value)
{
    if (dontShowAgainName.isEmpty()) return;

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group("Notification Messages");
    
    group.writeEntry(dontShowAgainName, value);
    config->sync();
}

// --------------------------------------------------------------------------------------------------------

void DMessageBox::showList(QMessageBox::Icon icon,
                           QWidget* const parent,
                           const QString& caption,
                           const QString& text,
                           const QStringList& items,
                           const QString& dontShowAgainName)
{
    if (!readMsgBoxShouldBeShown(dontShowAgainName))
    {
        return;
    }

    QDialog* const dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Information") : caption);
    dialog->setObjectName("information");
    dialog->setModal(true);

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok, dialog);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Escape);

    QObject::connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
                     dialog, SLOT(accept()));

    bool  checkboxResult = false;
    QIcon tmpIcon;

    switch (icon)
    {
        case QMessageBox::Information:
            tmpIcon = dialog->style()->standardIcon(QStyle::SP_MessageBoxInformation, 0, dialog);
            break;
        case QMessageBox::Warning:
            tmpIcon = dialog->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, dialog);
            break;
        case QMessageBox::Critical:
            tmpIcon = dialog->style()->standardIcon(QStyle::SP_MessageBoxCritical, 0, dialog);
            break;
        case QMessageBox::Question:
            tmpIcon = dialog->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, dialog);
        default:
            break;
    }
    
    int iconSize = dialog->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, dialog);

    createMessageBox(dialog, buttons, tmpIcon.pixmap(iconSize), text, items,
                     dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                     &checkboxResult);

    saveMsgBoxShouldBeShown(dontShowAgainName, checkboxResult);
}

int DMessageBox::warningContinueCancelList(QWidget* const parent,
                                           const QString& caption,
                                           const QString& text,
                                           const QStringList& items,
                                           const QString& dontAskAgainName)
{
    if (!readMsgBoxShouldBeShown(dontAskAgainName))
    {
        return QMessageBox::Yes;
    }

    QDialog* const dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption.isEmpty() ? i18n("Warning") : caption);
    dialog->setObjectName("warningYesNo");
    dialog->setModal(true);

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::Cancel, dialog);
    buttons->button(QDialogButtonBox::Yes)->setDefault(true);
    buttons->button(QDialogButtonBox::Yes)->setText(i18n("Continue"));
    buttons->button(QDialogButtonBox::Cancel)->setShortcut(Qt::Key_Escape);
    
    QObject::connect(buttons->button(QDialogButtonBox::Yes), SIGNAL(clicked()),
                     dialog, SLOT(accept()));

    QObject::connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
                     dialog, SLOT(reject()));

    bool checkboxResult = false;
    const int result    = createMessageBox(dialog, buttons, QIcon::fromTheme("dialog-warning"), text, items,
                                           dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                                           &checkboxResult);

    if (result != QDialog::Accepted)
    {
        return QMessageBox::Cancel;
    }

    saveMsgBoxShouldBeShown(dontAskAgainName, checkboxResult);

    return QMessageBox::Yes;
}

int DMessageBox::createMessageBox(QDialog* const dialog,
                                  QDialogButtonBox* const buttons,
                                  const QIcon& icon,
                                  const QString& text,
                                  const QStringList& items,
                                  const QString& ask,
                                  bool* checkboxReturn)
{
    QWidget* const mainWidget     = new QWidget(dialog);
    QVBoxLayout* const mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    mainLayout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QHBoxLayout* const hLayout    = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(-1); // use default spacing
    mainLayout->addLayout(hLayout, 5);

    //--------------------------------------------------------------------------------

    QLabel* const iconLabel       = new QLabel(mainWidget);
    QStyleOption option;
    option.initFrom(mainWidget);
    iconLabel->setPixmap(icon.pixmap(mainWidget->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, &option, mainWidget)));

    QVBoxLayout* const iconLayout = new QVBoxLayout();
    iconLayout->addStretch(1);
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch(5);
    hLayout->addLayout(iconLayout, 0);
    hLayout->addSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    //--------------------------------------------------------------------------------

    QLabel* const messageLabel    = new QLabel(text, mainWidget);
    messageLabel->setOpenExternalLinks(true);
    messageLabel->setWordWrap(true);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QPalette messagePal(messageLabel->palette());
    messagePal.setColor(QPalette::Window, Qt::transparent);
    messageLabel->setPalette(messagePal);
    hLayout->addWidget(messageLabel, 5);

    //--------------------------------------------------------------------------------

    QListWidget* const listWidget = new QListWidget(mainWidget);
    listWidget->addItems(items);
    mainLayout->addWidget(listWidget, 50);

    //--------------------------------------------------------------------------------

    QPointer<QCheckBox> checkbox = 0;

    if (!ask.isEmpty())
    {
        checkbox = new QCheckBox(ask, mainWidget);
        mainLayout->addWidget(checkbox);

        if (checkboxReturn)
        {
            checkbox->setChecked(*checkboxReturn);
        }
    }

    //--------------------------------------------------------------------------------

    mainLayout->addWidget(buttons);
    dialog->setLayout(mainLayout);

    //--------------------------------------------------------------------------------

    // We use a QPointer because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the QPointer will reset to 0.
    QPointer<QDialog> guardedDialog = dialog;

    const int result = guardedDialog->exec();

    if (checkbox && checkboxReturn)
    {
        *checkboxReturn = checkbox->isChecked();
    }

    delete(QDialog*) guardedDialog;
    return result;
}


}  // namespace Digikam
