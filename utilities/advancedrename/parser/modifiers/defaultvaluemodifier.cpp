/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-11
 * Description : a modifier for setting a default value if option parsing failed
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "defaultvaluemodifier.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DefaultValueDialog::DefaultValueDialog(Rule* parent)
    : RuleDialog(parent),
      valueInput(0)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QString defaultValueStr = i18n("Default Value");

    QLabel* const srcLabel = new QLabel(defaultValueStr + QLatin1Char(':'));
    valueInput             = new QLineEdit(this);
    valueInput->setToolTip(i18n("<p>Set a default value for empty strings.<br/>"
                                "When applied to a renaming option, "
                                "an empty string will be replaced by the value you specify here.</p>"));

    QWidget* const mainWidget     = new QWidget(this);
    QGridLayout* const mainLayout = new QGridLayout(this);
    mainLayout->addWidget(srcLabel,   0, 0);
    mainLayout->addWidget(valueInput, 0, 1);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainLayout->setRowStretch(1, 10);
    mainWidget->setLayout(mainLayout);

    setSettingsWidget(mainWidget);

    valueInput->setFocus();
}

DefaultValueDialog::~DefaultValueDialog()
{
}

// --------------------------------------------------------

DefaultValueModifier::DefaultValueModifier()
    : Modifier(i18nc("default value for empty strings", "Default Value..."),
               i18n("Set a default value for empty strings"),
               QLatin1String("edit-undo"))
{
    addToken(QLatin1String("{default:\"||value||\"}"), description());

    QRegExp reg(QLatin1String("\\{default:\"(.+)\"\\}"));
    reg.setMinimal(true);
    setRegExp(reg);
}

void DefaultValueModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<DefaultValueDialog> dlg = new DefaultValueDialog(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        QString valueStr = dlg->valueInput->text();

        if (!valueStr.isEmpty())
        {
            result = QString::fromUtf8("{default:\"%1\"}").arg(valueStr);
        }
    }

    delete dlg;

    emit signalTokenTriggered(result);
}

QString DefaultValueModifier::parseOperation(ParseSettings& settings)
{
    if (!settings.str2Modify.isEmpty())
    {
        return settings.str2Modify;
    }

    const QRegExp& reg = regExp();

    QString defaultStr = reg.cap(1).isEmpty() ? QString() : reg.cap(1);

    return defaultStr;
}

} // namespace Digikam
