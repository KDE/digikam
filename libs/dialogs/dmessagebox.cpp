/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-01-19
 * Description : message box notification settings
 *
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QSignalMapper>

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

void DMessageBox::showInformationList(QMessageBox::Icon icon,
                                      QWidget* const parent,
                                      const QString& caption,
                                      const QString& text,
                                      const QStringList& items,
                                      const QString& dontShowAgainName)
{
    showInformationWidget(icon, parent, caption, text, createWidgetList(items), dontShowAgainName);
}

void DMessageBox::showInformationWidget(QMessageBox::Icon icon,
                                        QWidget* const parent,
                                        const QString& caption,
                                        const QString& text,
                                        QWidget* const listWidget,
                                        const QString& dontShowAgainName)
{
    if (!readMsgBoxShouldBeShown(dontShowAgainName))
    {
        return;
    }

    QDialog* const dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption);
    dialog->setObjectName(QLatin1String("showInformation"));
    dialog->setModal(true);

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok, dialog);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Escape);

    QObject::connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
                     dialog, SLOT(accept()));

    bool  checkboxResult = false;

    createMessageBox(dialog, buttons, createIcon(icon), text, listWidget,
                     dontShowAgainName.isEmpty() ? QString() : i18n("Do not show this message again"),
                     &checkboxResult);

    saveMsgBoxShouldBeShown(dontShowAgainName, checkboxResult);
}

// --------------------------------------------------------------------------------------------------------

int DMessageBox::showContinueCancel(QMessageBox::Icon icon,
                                    QWidget* const parent,
                                    const QString& caption,
                                    const QString& text,
                                    const QString& dontAskAgainName)
{
    return (showContinueCancelList(icon, parent, caption, text, QStringList(), dontAskAgainName));
}

int DMessageBox::showContinueCancelList(QMessageBox::Icon icon,
                                        QWidget* const parent,
                                        const QString& caption,
                                        const QString& text,
                                        const QStringList& items,
                                        const QString& dontAskAgainName)
{
    return (showContinueCancelWidget(icon, parent, caption, text, createWidgetList(items), dontAskAgainName));
}

int DMessageBox::showContinueCancelWidget(QMessageBox::Icon icon,
                                          QWidget* const parent,
                                          const QString& caption,
                                          const QString& text,
                                          QWidget* const listWidget,
                                          const QString& dontAskAgainName)
{
    if (!readMsgBoxShouldBeShown(dontAskAgainName))
    {
        return QMessageBox::Yes;
    }

    QDialog* const dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption);
    dialog->setObjectName(QLatin1String("showContinueCancel"));
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
    const int result    = createMessageBox(dialog, buttons, createIcon(icon), text, listWidget,
                                           dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                                           &checkboxResult);

    if (result != QDialog::Accepted)
    {
        return QMessageBox::Cancel;
    }

    saveMsgBoxShouldBeShown(dontAskAgainName, checkboxResult);

    return QMessageBox::Yes;
}

// --------------------------------------------------------------------------------------------------------

int DMessageBox::showYesNo(QMessageBox::Icon icon,
                           QWidget* const parent,
                           const QString& caption,
                           const QString& text,
                           const QString& dontAskAgainName)
{
    return (showYesNoList(icon, parent, caption, text, QStringList(), dontAskAgainName));
}

int DMessageBox::showYesNoList(QMessageBox::Icon icon,
                               QWidget* const parent,
                               const QString& caption,
                               const QString& text,
                               const QStringList& items,
                               const QString& dontAskAgainName)
{
    return (showYesNoWidget(icon, parent, caption, text, createWidgetList(items), dontAskAgainName));
}

int DMessageBox::showYesNoWidget(QMessageBox::Icon icon,
                                 QWidget* const parent,
                                 const QString& caption,
                                 const QString& text,
                                 QWidget* const listWidget,
                                 const QString& dontAskAgainName)
{
    if (!readMsgBoxShouldBeShown(dontAskAgainName))
    {
        return QMessageBox::Yes;
    }

    QDialog* const dialog = new QDialog(parent, Qt::Dialog);
    dialog->setWindowTitle(caption);
    dialog->setObjectName(QLatin1String("showYesNo"));
    dialog->setModal(true);

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, dialog);
    buttons->button(QDialogButtonBox::No)->setDefault(true);
    buttons->button(QDialogButtonBox::No)->setShortcut(Qt::Key_Escape);

    QSignalMapper* const signalMapper = new QSignalMapper(buttons);
    signalMapper->setMapping(buttons->button(QDialogButtonBox::Yes), QDialogButtonBox::Yes);
    signalMapper->setMapping(buttons->button(QDialogButtonBox::No),  QDialogButtonBox::No);

    QObject::connect(buttons->button(QDialogButtonBox::Yes), SIGNAL(clicked()),
                     signalMapper, SLOT(map()));

    QObject::connect(buttons->button(QDialogButtonBox::No), SIGNAL(clicked()),
                     signalMapper, SLOT(map()));

    QObject::connect(signalMapper, SIGNAL(mapped(int)),
                     dialog, SLOT(done(int)));

    bool checkboxResult = false;
    int result          = createMessageBox(dialog, buttons, createIcon(icon), text, listWidget,
                                           dontAskAgainName.isEmpty() ? QString() : i18n("Do not ask again"),
                                           &checkboxResult);

    if (result == QDialogButtonBox::Yes)
    {
        saveMsgBoxShouldBeShown(dontAskAgainName, checkboxResult);
        return QMessageBox::Yes;
    }
    else if (result == QDialogButtonBox::No)
    {
        saveMsgBoxShouldBeShown(dontAskAgainName, checkboxResult);
        return QMessageBox::No;
    }

    return QMessageBox::Cancel;
}

// --------------------------------------------------------------------------------------------------------

int DMessageBox::createMessageBox(QDialog* const dialog,
                                  QDialogButtonBox* const buttons,
                                  const QIcon& icon,
                                  const QString& text,
                                  QWidget* const listWidget,
                                  const QString& ask,
                                  bool* checkboxReturn)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const mainWidget     = new QWidget(dialog);
    QVBoxLayout* const mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    QHBoxLayout* const hLayout    = new QHBoxLayout();
    hLayout->setContentsMargins(QMargins());
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
    hLayout->addSpacing(spacing);

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

    if (listWidget)
    {
        listWidget->setParent(mainWidget);
        mainLayout->addWidget(listWidget, 50);
    }

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

QIcon DMessageBox::createIcon(QMessageBox::Icon icon)
{
    QIcon tmpIcon;

    switch (icon)
    {
        case QMessageBox::Warning:
            tmpIcon = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, qApp->activeWindow());
            break;

        case QMessageBox::Critical:
            tmpIcon = qApp->style()->standardIcon(QStyle::SP_MessageBoxCritical, 0, qApp->activeWindow());
            break;

        case QMessageBox::Question:
            tmpIcon = qApp->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, qApp->activeWindow());
            break;

        case QMessageBox::Information:
        default:
            tmpIcon = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation, 0, qApp->activeWindow());
            break;
    }

    int iconSize = qApp->style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, qApp->activeWindow());

    return tmpIcon.pixmap(iconSize);
}


QListWidget* DMessageBox::createWidgetList(const QStringList& items)
{
    QListWidget* listWidget = 0;

    if (!items.isEmpty())
    {
        listWidget = new QListWidget();
        listWidget->addItems(items);
    }

    return listWidget;
}

} // namespace Digikam
