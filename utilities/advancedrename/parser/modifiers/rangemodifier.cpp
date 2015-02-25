/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for displaying only a range of a token result
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

#include "rangemodifier.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ui_rangemodifierdialogwidget.h"

namespace Digikam
{

RangeDialog::RangeDialog(Rule* const parent)
    : RuleDialog(parent),
      ui(new Ui::RangeModifierDialogWidget())
{
    QWidget* const mainWidget = new QWidget(this);
    ui->setupUi(mainWidget);
    setSettingsWidget(mainWidget);
    ui->startInput->setFocus();

    slotToTheEndChecked(true);

    connect(ui->toTheEndCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToTheEndChecked(bool)));
}

RangeDialog::~RangeDialog()
{
    delete ui;
}

void RangeDialog::slotToTheEndChecked(bool checked)
{
    ui->stopInput->setEnabled(!checked);
}

// --------------------------------------------------------

RangeModifier::RangeModifier()
    : Modifier(i18n("Range..."),
               i18n("Add only a specific range of a renaming option"),
               QLatin1String("measure"))
{
    addToken(QLatin1String("{range:||from||,||to||}"),
             i18n("Extract a specific range (if '||to||' is omitted, go to the end of string)"));

    QRegExp reg(QLatin1String("\\{range(:(-?\\d+)(,((-1|\\d+))?)?)\\}"));
    reg.setMinimal(true);
    setRegExp(reg);
}

void RangeModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString result;

    QPointer<RangeDialog> dlg = new RangeDialog(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        int start = dlg->ui->startInput->value();
        int stop  = dlg->ui->stopInput->value();

        if (dlg->ui->toTheEndCheckBox->isChecked())
        {
            result = QString::fromUtf8("{range:%1,}").arg(QString::number(start));
        }
        else
        {
            result = QString::fromUtf8("{range:%1,%2}").arg(QString::number(start))
                     .arg(QString::number(stop));
        }
    }

    delete dlg;

    emit signalTokenTriggered(result);
}

QString RangeModifier::parseOperation(ParseSettings& settings)
{
    const QRegExp& reg = regExp();
    bool ok = false;

    // if the start parameter can not be extracted or is a negative value, set it to 1
    int start = reg.cap(2).simplified().toInt(&ok);

    if (!ok || start < 0)
    {
        start = 1;
    }

    // If no range is defined at all ({start}), set stop = start.
    // If the stop parameter is omitted ({start-}), set stop = -1 (end of string)
    ok = false;
    int stop;

    if (!reg.cap(3).isEmpty())
    {
        ok   = true;
        stop = (reg.cap(4).isEmpty()) ? -1 : reg.cap(5).simplified().toInt(&ok);
    }
    else
    {
        stop = start;
    }

    if (!ok)
    {
        stop = start;
    }

    // --------------------------------------------------------

    // replace the string according to the given range
    if (start > settings.str2Modify.count())
    {
        return QString();
    }

    if (stop > settings.str2Modify.count())
    {
        stop = -1;
    }

    --start;

    if (stop != -1)
    {
        --stop;
    }

    if ((start < 0) || (stop < -1))
    {
        return QString();
    }

    if (stop == -1)
    {
        stop = settings.str2Modify.count() - 1;
    }

    QString result;

    for (int i = start; i <= stop; ++i)
    {
        result.append(settings.str2Modify.at(i));
    }

    return result;
}

} // namespace Digikam
