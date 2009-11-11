/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-11
 * Description : a modifier for setting a default value if option parsing failed
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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
#include "defaultvaluemodifier.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>

namespace Digikam
{

DefaultValueDialog::DefaultValueDialog()
                  : KDialog(0)
{
    QString replace = i18n("Default Value");

    setCaption(replace);

    QLabel* srcLabel = new QLabel(replace + ':');
    valueInput       = new KLineEdit(this);

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(srcLabel,   0, 0);
    mainLayout->addWidget(valueInput, 0, 1);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(1, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    valueInput->setFocus();
}

DefaultValueDialog::~DefaultValueDialog()
{
}

// --------------------------------------------------------

DefaultValueModifier::DefaultValueModifier()
               : Modifier(i18nc("default value for replaced text", "Default Value..."),
                          i18n("Replace empty renaming options results with a default value"),
                          SmallIcon("document-edit"))
{
    addTokenDescription(QString("{\"<i>default</i>\"}"), i18nc("default value", "Default"), description());

    setRegExp("\\{\\s*\"(.+)\"\\s*\\}");
}

void DefaultValueModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString tmp;

    QPointer<DefaultValueDialog> dlg = new DefaultValueDialog;
    if (dlg->exec() == KDialog::Accepted)
    {
        QString valueStr = dlg->valueInput->text();
        if (!valueStr.isEmpty())
        {
            tmp = QString("{\"%1\"}").arg(valueStr);
        }
    }
    delete dlg;

    emit signalTokenTriggered(tmp);
}

QString DefaultValueModifier::modifyOperation(const QString& parseString, const QString& result)
{
    if (!result.isEmpty())
    {
        return result;
    }

    QRegExp reg = regExp();
    reg.setMinimal(true);

    int pos = 0;
    pos     = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        QString defaultStr = reg.cap(1).isEmpty() ? QString() : reg.cap(1);
        return defaultStr;
    }
    return QString();
}

} // namespace Digikam
