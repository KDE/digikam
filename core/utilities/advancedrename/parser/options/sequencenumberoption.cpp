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

#include "sequencenumberoption.h"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ui_sequencenumberoptiondialogwidget.h"

namespace Digikam
{

SequenceNumberDialog::SequenceNumberDialog(Rule* const parent)
    : RuleDialog(parent),
      ui(new Ui::SequenceNumberOptionDialogWidget())
{
    QWidget* const mainWidget = new QWidget(this);
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
    : Option(i18nc("Sequence Number", "Number..."),
             i18n("Add a sequence number"),
             QLatin1String("accessories-calculator"))
{
    addToken(QLatin1String("#"),                                 i18n("Sequence number"));
    addToken(QLatin1String("#[||options||]"),                    i18n("Sequence number (||options||: ||e|| = extension aware, ||f|| = folder aware)"));
    addToken(QLatin1String("#[||options||,||start||]"),          i18n("Sequence number (custom start)"));
    addToken(QLatin1String("#[||options||,||start||,||step||]"), i18n("Sequence number (custom start + step)"));

    QRegExp reg(QLatin1String("(#+)(\\[(e?f?,?)?((-?\\d+)(,(-?\\d+))?)?\\])?"));
    setRegExp(reg);
}

SequenceNumberOption::~SequenceNumberOption()
{
}

void SequenceNumberOption::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QPointer<SequenceNumberDialog> dlg = new SequenceNumberDialog(this);

    QString result;

    if (dlg->exec() == QDialog::Accepted)
    {
        int digits          = dlg->ui->digits->value();
        int start           = dlg->ui->start->value();
        int step            = dlg->ui->step->value();
        bool extensionAware = dlg->ui->extensionAware->isChecked();
        bool folderAware    = dlg->ui->folderAware->isChecked();

        result = QString::fromUtf8("%1").arg(QLatin1String("#"), digits, QLatin1Char('#'));

        if (start > 1 || step > 1 || extensionAware || folderAware)
        {
            result.append(QLatin1Char('['));

            if (extensionAware)
            {
                result.append(QLatin1Char('e'));
            }

            if (folderAware)
            {
                result.append(QLatin1Char('f'));
            }

            if (start > 1 || step > 1)
            {
                if (extensionAware)
                {
                    result.append(QLatin1Char(','));
                }

                result.append(QString::number(start));
            }

            if (step > 1)
            {
                result.append(QString::fromUtf8(",%1").arg(QString::number(step)));
            }

            result.append(QLatin1Char(']'));
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
        bool extAware    = !reg.cap(3).isEmpty() && reg.cap(3).contains(QLatin1Char('e'));
        bool folderAware = !reg.cap(3).isEmpty() && reg.cap(3).contains(QLatin1Char('f'));

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
    result  = QString::fromUtf8("%1").arg(number, slength, 10, QLatin1Char('0'));

    return result;
}

} // namespace Digikam
