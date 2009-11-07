/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for displaying only a range of a token result
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

#include "rangemodifier.h"
#include "rangemodifier.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

namespace Digikam
{

RangeDialog::RangeDialog()
             : KDialog(0), startInput(0), stopInput(0), toTheEndCheckBox(0)
{
    setCaption("Specify a text range", "Range");

    // --------------------------------------------------------

    const int minRange = 1;
    const int maxRange = 999999;

    startInput = new KIntNumInput(this);
    startInput->setMinimum(minRange);
    startInput->setMaximum(maxRange);
    startInput->setLabel(i18nc("Beginning of the text range", "From:"));

    stopInput = new KIntNumInput(this);
    stopInput->setMinimum(minRange);
    stopInput->setMaximum(maxRange);
    stopInput->setLabel(i18nc("end of the text range", "To:"));

    toTheEndCheckBox = new QCheckBox(i18nc("range is specified until the end of the string", "to the end"));
    toTheEndCheckBox->setChecked(true);
    slotToTheEndChecked(true);

    // --------------------------------------------------------

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(startInput,          0, 0);
    mainLayout->addWidget(toTheEndCheckBox,    1, 0);
    mainLayout->addWidget(stopInput,           2, 0);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(3, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    // --------------------------------------------------------

    startInput->setFocus();

    connect(toTheEndCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotToTheEndChecked(bool)));
}

RangeDialog::~RangeDialog()
{
}

void RangeDialog::slotToTheEndChecked(bool checked)
{
    stopInput->setEnabled(!checked);
}

// --------------------------------------------------------

RangeModifier::RangeModifier()
             : Modifier(i18n("Range..."), i18n("Add only a specific range of a string"),
                        SmallIcon("measure"))
{
    setUseTokenMenu(false);

    addTokenDescription(QString("{<i>from</i> - <i>to</i>}"), i18n("Range"),
             i18n("Extract a specific range (if omitted, '<i>to</i>' = end of string)"));

    setRegExp("\\{\\s*(\\d+)\\s*(-\\s*((-1|\\d+)\\s*)?)?\\}");
}

void RangeModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString tmp;

    QPointer<RangeDialog> dlg = new RangeDialog;
    if (dlg->exec() == KDialog::Accepted)
    {
        int start = dlg->startInput->value();
        int stop  = dlg->stopInput->value();

        if (dlg->toTheEndCheckBox->isChecked())
        {
            tmp = QString("{%1-}").arg(QString::number(start));
        }
        else
        {
            tmp = QString("{%1-%2}").arg(QString::number(start))
                                    .arg(QString::number(stop));
        }
    }
    delete dlg;

    emit signalTokenTriggered(tmp);
}

QString RangeModifier::modifyOperation(const QString& parseString, const QString& result)
{
    QRegExp reg = regExp();

    int pos = 0;
    pos     = reg.indexIn(parseString, pos);
    if (pos > -1)
    {
        /*
         * extract range parameters
         */

        bool ok = false;

        // if the start parameter can not be extracted, set it to 1
        int start = reg.cap(1).simplified().toInt(&ok);
        if (!ok)
        {
            start = 1;
        }

        // If no range is defined at all ({start}), set stop = start.
        // If the stop parameter is omitted ({start-}), set stop = -1 (end of string)
        ok = false;
        int stop;
        if (!reg.cap(2).isEmpty())
        {
            ok   = true;
            stop = (reg.cap(3).isEmpty()) ? -1 : reg.cap(4).simplified().toInt(&ok);
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

        /*
         * replace the string according to the given range
         */

        if (start > result.count())
        {
            return QString();
        }

        if (stop > result.count())
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
            stop = result.count() - 1;
        }

        QString tmp;
        for (int i = start; i <= stop; ++i)
        {
            tmp.append(result.at(i));
        }
        return tmp;
    }
    return QString();
}

} // namespace Digikam
