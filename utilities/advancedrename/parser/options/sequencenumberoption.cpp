/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
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

#include "sequencenumberoption.moc"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <klocale.h>
#include <knuminput.h>

// Local includes

#include "ui_sequencenumberoptiondialogwidget.h"

namespace Digikam
{

SequenceNumberDialog::SequenceNumberDialog(Rule* parent)
    : RuleDialog(parent), ui(new Ui::SequenceNumberOptionDialogWidget())
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
             "accessories-calculator")
{
    addToken("#",                                 i18n("Sequence number"));
    addToken("#[||options||]",                    i18n("Sequence number (||options||: ||e|| = extension aware, ||f|| = folder aware)"));
    addToken("#[||options||,||start||]",          i18n("Sequence number (custom start)"));
    addToken("#[||options||,||start||,||step||]", i18n("Sequence number (custom start + step)"));

    QRegExp reg("(#+)(\\[(e?f?,?)?((-?\\d+)(,(-?\\d+))?)?\\])?");
    setRegExp(reg);
}

void SequenceNumberOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QPointer<SequenceNumberDialog> dlg = new SequenceNumberDialog(this);

    QString result;

    if (dlg->exec() == KDialog::Accepted)
    {
        int digits          = dlg->ui->digits->value();
        int start           = dlg->ui->start->value();
        int step            = dlg->ui->step->value();
        bool extensionAware = dlg->ui->extensionAware->isChecked();
        bool folderAware    = dlg->ui->folderAware->isChecked();

        result = QString("%1").arg("#", digits, QChar('#'));

        if (start > 1 || step > 1 || extensionAware || folderAware)
        {
            result.append(QChar('['));

            if (extensionAware)
            {
                result.append(QChar('e'));
            }

            if (folderAware)
            {
                result.append(QChar('f'));
            }

            if (start > 1 || step > 1)
            {
                if (extensionAware)
                {
                    result.append(QChar(','));
                }

                result.append(QString::number(start));
            }

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
    QString result;
    const QRegExp& reg = regExp();

    int slength      = 0;
    int start        = 0;
    int step         = 0;
    int number       = 0;
    int index        = 0;

    if (settings.manager)
    {
        bool extAware    = !reg.cap(3).isEmpty() && reg.cap(3).contains(QChar('e'));
        bool folderAware = !reg.cap(3).isEmpty() && reg.cap(3).contains(QChar('f'));

        index = settings.manager->indexOfFile(settings.fileUrl.toLocalFile());

        if (extAware)
        {
            index = settings.manager->indexOfFileGroup(settings.fileUrl.toLocalFile());
        }

        if (folderAware)
        {
            index = settings.manager->indexOfFolder(settings.fileUrl.toLocalFile());
        }
    }

    // --------------------------------------------------------

    slength = reg.cap(1).length();
    start   = reg.cap(5).isEmpty() ? settings.startIndex : reg.cap(5).toInt();
    step    = reg.cap(7).isEmpty() ? 1 : reg.cap(7).toInt();

    if (start < 1)
    {
        start = settings.startIndex;
    }

    if (step < 1)
    {
        step = 1;
    }

    number  = start + ((index - 1) * step);
    result  = QString("%1").arg(number, slength, 10, QChar('0'));

    return result;
}

} // namespace Digikam
