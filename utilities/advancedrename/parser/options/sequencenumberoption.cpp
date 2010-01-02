/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
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

#include "sequencenumberoption.moc"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "ui_sequencenumberoptiondialogwidget.h"

namespace Digikam
{

SequenceNumberDialog::SequenceNumberDialog(ParseObject* parent)
                    : ParseObjectDialog(parent), ui(new Ui::SequenceNumberOptionDialogWidget())
{
    QWidget* mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);
    setSettingsWidget(mainWidget);
    ui->digits->setFocus();
}

SequenceNumberDialog::~SequenceNumberDialog()
{
    delete ui;
}

// --------------------------------------------------------

SequenceNumberOption::SequenceNumberOption()
                    : Option(i18nc("Sequence Number", "Number..."), i18n("Add a sequence number"),
                             SmallIcon("accessories-calculator"))
{
    addToken("#",                     i18n("Sequence number"));
    addToken("#[||start||]",          i18n("Sequence number (custom start)"));
    addToken("#[||start||,||step||]", i18n("Sequence number (custom start + step)"));

    QRegExp reg("(#+)(\\[\\s*(\\d+)\\s*,?\\s*(\\d+)?\\s*\\])?");
    setRegExp(reg);
}

void SequenceNumberOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QPointer<SequenceNumberDialog> dlg = new SequenceNumberDialog(this);

    QString result;
    if (dlg->exec() == KDialog::Accepted)
    {
        int digits = dlg->ui->digits->value();
        int start  = dlg->ui->start->value();
        int step   = dlg->ui->step->value();

        result = QString("%1").arg("#", digits, QChar('#'));
        if (start > 1)
        {
            result.append(QString("[%1").arg(QString::number(start)));

            if (step > 1)
            {
                result.append(QString(",%1").arg(QString::number(step)));
            }

            result.append(QChar(']'));
        }
    }

    delete dlg;

    emit signalTokenTriggered(result);
}

QString SequenceNumberOption::parseOperation(ParseSettings& settings)
{
    int slength = 0;
    int start   = 0;
    int step    = 0;
    int number  = 0;
    int index   = settings.currentIndex;

    // --------------------------------------------------------

    QString result;
    const QRegExp& reg = regExp();
    slength            = reg.cap(1).length();
    start              = reg.cap(3).isEmpty() ? settings.startIndex : reg.cap(3).toInt();
    step               = reg.cap(4).isEmpty() ? 1 : reg.cap(4).toInt();

    number             = start + ((index - 1) * step);
    result             = QString("%1").arg(number, slength, 10, QChar('0'));

    return result;
}

} // namespace Digikam
