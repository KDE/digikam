/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : trimmed token modifier
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
#include <QPointer>
#include <QRegExp>
#include <QRegExpValidator>
#include <QLabel>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>

namespace Digikam
{
FillDialog::FillDialog(ParseObject* parent)
          : ParseObjectDialog(parent),
            charInput(0), digitsInput(0)
{
    QLabel* digitsLabel = new QLabel(i18nc("Length of the string", "Length:"), this);
    digitsInput = new KIntNumInput(this);
    digitsInput->setMinimum(1);

    QLabel* charLabel = new QLabel(i18nc("character to fill string with", "Character:"), this);
    QRegExp validateReg("\\w{1}");
    QRegExpValidator* validator = new QRegExpValidator(validateReg, this);
    charInput = new KLineEdit(this);
    charInput->setValidator(validator);

    QLabel* alignLabel = new QLabel(i18n("Alignment:"), this);
    alignBox = new KComboBox(this);
    alignBox->insertItem(Left,  "Left");
    alignBox->insertItem(Right, "Right");

    // --------------------------------------------------------

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(digitsLabel, 0, 0, 1, 1);
    mainLayout->addWidget(digitsInput, 0, 1, 1, 1);
    mainLayout->addWidget(charLabel,   1, 0, 1, 1);
    mainLayout->addWidget(charInput,   1, 1, 1, 1);
    mainLayout->addWidget(alignLabel,  2, 0, 1, 1);
    mainLayout->addWidget(alignBox,    2, 1, 1, 1);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(3, 10);
    mainWidget->setLayout(mainLayout);

    setSettingsWidget(mainWidget);

    // --------------------------------------------------------

    digitsInput->setFocus();
}

FillDialog::~FillDialog()
{
}

// --------------------------------------------------------

FillModifier::FillModifier()
            : Modifier(i18n("Fill..."), i18n("Fill the string to match a specific length"),
                       SmallIcon("format-fill-color"))
{
    QString tooltipA = QString("%1 %2").arg(description())
                                       .arg(i18n("(||align||: ||l|| = left, ||r|| = right)"));
    QString tooltipB = QString("%1 %2").arg(description())
                                       .arg(i18n("(||char|| = fill with a custom character)"));

    addToken("{fill:||n||,||align||}",          tooltipA);
    addToken("{fill:||n||,||align||,||char||}", tooltipB);

    QRegExp reg("\\{fill:(\\d+)(,([lr]{1}))?(,\"(\\w)?\")?\\}");
    reg.setMinimal(true);
    setRegExp(reg);
}

QString FillModifier::modifyOperation(const ParseSettings& settings, const QString& str2Modify)
{
    Q_UNUSED(settings);

    QString result;
    QRegExp reg       = regExp();
    QString alignstr  = reg.cap(3);
    QString charstr   = reg.cap(5);

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
    int     length;

    QPointer<FillDialog> dlg = new FillDialog(this);
    if (dlg->exec() == KDialog::Accepted)
    {
        alignStr = (dlg->alignBox->currentIndex() == FillDialog::Left) ? QString('l') : QString('r');
        length   = dlg->digitsInput->value();
        charStr  = dlg->charInput->text();
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
