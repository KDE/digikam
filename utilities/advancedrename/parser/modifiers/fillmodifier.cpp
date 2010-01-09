/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a modifier to fill a string with a character to
 *               match a certain length
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

#include "fillmodifier.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPointer>
#include <QRegExp>
#include <QRegExpValidator>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "ui_fillmodifierdialogwidget.h"

namespace Digikam
{
FillDialog::FillDialog(ParseObject* parent)
          : ParseObjectDialog(parent), ui(new Ui::FillModifierDialogWidget())
{
    QWidget* mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);

    QRegExp validateReg("\\w{1}");
    QRegExpValidator* validator = new QRegExpValidator(validateReg, this);
    ui->charInput->setValidator(validator);

    ui->alignBox->insertItem(Left,  "Left");
    ui->alignBox->insertItem(Right, "Right");

    setSettingsWidget(mainWidget);
    ui->digitsInput->setFocus();
}

FillDialog::~FillDialog()
{
    delete ui;
}

// --------------------------------------------------------

FillModifier::FillModifier()
            : Modifier(i18n("Fill..."), i18n("Fill the string with a character to match a specific length"),
                       SmallIcon("format-fill-color"))
{
    QString tooltip = QString("%1 %2").arg(description())
                                      .arg(i18n("(||align||: ||l|| = left, ||r|| = right)"));

    addToken("{fill:||length||,||align||,\"||char||\"}", tooltip);

    QRegExp reg("\\{fill:(\\d+)(,([lr]{1}))?(,\"(\\w)?\")?\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString FillModifier::modifyOperation(const ParseSettings& settings, const QString& str2Modify)
{
    Q_UNUSED(settings);

    QString result;
    QRegExp reg      = regExp();
    QString alignstr = reg.cap(3);
    QString charstr  = reg.cap(5);

    bool ok;
    int length = reg.cap(1).toInt(&ok);
    if (!ok)
    {
        length = 1;
    }

    if (alignstr != QString('r'))
    {
        length *= -1;
    }

    QChar character = (charstr.length() == 1) ? charstr.at(0) : QChar('_');
    result = QString("%1").arg(str2Modify, length, character);
    return result;
}

void FillModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;
    QString alignStr;
    QString charStr;
    int     length = 1;

    QPointer<FillDialog> dlg = new FillDialog(this);
    if (dlg->exec() == KDialog::Accepted)
    {
        alignStr = (dlg->ui->alignBox->currentIndex() == FillDialog::Left) ? QString('l') : QString('r');
        length   = dlg->ui->digitsInput->value();
        charStr  = dlg->ui->charInput->text();
    }
    delete dlg;

    QString options;

    if (!alignStr.isNull())
    {
        options.append(',');
        options.append(alignStr);
    }

    if (!charStr.isEmpty())
    {
        options.append(",\"");
        options.append(charStr);
        options.append("\"");
    }

    result = QString("{fill:%1").arg(QString::number(length));
    if (!options.isEmpty())
    {
        result.append(options);
    }
    result.append('}');

    emit signalTokenTriggered(result);
}

} // namespace Digikam
