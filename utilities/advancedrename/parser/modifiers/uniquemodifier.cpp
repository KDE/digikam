/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : a modifier for setting an additional string to a renaming option
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

#include "uniquemodifier.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>

namespace Digikam
{

//UniqueDialog::UniqueDialog(ParseObject* parent)
//                  : ParseObjectDialog(parent),
//                    valueInput(0)
//{
//    QString defaultValueStr = i18n("Unique Value");
//
//    QLabel* srcLabel = new QLabel(defaultValueStr + ':');
//    valueInput       = new KLineEdit(this);
//    valueInput->setToolTip(i18n("<p>Set a unique value for empty strings.<br/>"
//                                "When applied to a renaming option, "
//                                "an empty string will be replaced by the value you specify here.</p>"));
//
//    QWidget*     mainWidget = new QWidget(this);
//    QGridLayout* mainLayout = new QGridLayout(this);
//    mainLayout->addWidget(srcLabel,   0, 0);
//    mainLayout->addWidget(valueInput, 0, 1);
//    mainLayout->setSpacing(KDialog::spacingHint());
//    mainLayout->setMargin(KDialog::spacingHint());
//    mainLayout->setRowStretch(1, 10);
//    mainWidget->setLayout(mainLayout);
//
//    setSettingsWidget(mainWidget);
//
//    valueInput->setFocus();
//}
//
//UniqueDialog::~UniqueDialog()
//{
//}

// --------------------------------------------------------

UniqueModifier::UniqueModifier()
              : Modifier(i18nc("unique value for duplicate strings", "Unique"),
                         i18n("Add a prefix number to have unique values"),
                         SmallIcon("button_more")),
                counter(0),
                MAXCOUNT(2)
{
//    addToken("{u:\"|unique|\"}", description());
    addToken("{unique}", description());

    QRegExp reg("\\{unique\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

//void UniqueModifier::slotTokenTriggered(const QString& token)
//{
//    Q_UNUSED(token)
//
//    QString tmp;
//
//    QPointer<UniqueDialog> dlg = new UniqueDialog(this);
//    if (dlg->exec() == KDialog::Accepted)
//    {
//        QString valueStr = dlg->valueInput->text();
//        if (!valueStr.isEmpty())
//        {
//            tmp = QString("{u:\"%1\"}").arg(valueStr);
//        }
//    }
//    delete dlg;
//
//    emit signalTokenTriggered(tmp);
//}

QString UniqueModifier::modifyOperation(const QString& parseString, const QString& result)
{
    if (cache.count(result) < MAXCOUNT)
    {
        cache << result;
    }

    QRegExp reg = regExp();
    int pos     = 0;
    pos         = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        if (cache.count(result) >= MAXCOUNT)
        {
            ++counter;
            QString tmp  = result;
            tmp         += QString("_%1").arg(QString::number(counter));
            return tmp;
        }
    }
    return result;
}

void UniqueModifier::reset()
{
    cache.clear();
    counter = 0;
}

} // namespace Digikam
