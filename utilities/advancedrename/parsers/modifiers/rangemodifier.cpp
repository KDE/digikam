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

// Qt includes

#include <QPointer>
#include <QGridLayout>

// KDE includes

#include <klocale.h>
#include <knuminput.h>
#include <kiconloader.h>

namespace Digikam
{

class RangeDialogPriv
{
public:

    RangeDialogPriv()
    {
        start  = 0;
        stop   = 0;
    }

    KIntNumInput* start;
    KIntNumInput* stop;
};

RangeDialog::RangeDialog()
             : KDialog(0), d(new RangeDialogPriv)
{
    setCaption("Specify a text range", "Range");

    const int minRange = 1;
    const int maxRange = 999999;

    d->start = new KIntNumInput(this);
    d->start->setMinimum(minRange);
    d->start->setMaximum(maxRange);
    d->start->setLabel(i18nc("Beginning of the text range", "From:"));

    d->stop = new KIntNumInput(this);
    d->stop->setMinimum(minRange);
    d->stop->setMaximum(maxRange);
    d->stop->setLabel(i18nc("End of the text range", "To:"));

    QWidget*     mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(d->start, 0, 0);
    mainLayout->addWidget(d->stop,  1, 0);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setRowStretch(2, 10);
    mainWidget->setLayout(mainLayout);

    setMainWidget(mainWidget);

    d->start->setFocus();
}

RangeDialog::~RangeDialog()
{
    delete d;
}

int RangeDialog::start() const
{
    return d->start->value();
}

int RangeDialog::stop() const
{
    return d->stop->value();
}

// --------------------------------------------------------

RangeModifier::RangeModifier()
             : Modifier(i18n("Range..."), i18n("Add only a specific range of a string"),
                        SmallIcon("measure"))
{
    setUseTokenMenu(false);

    addTokenDescription(QString("{<i>index</i>}"), i18n("Index"),
             i18n("Extract the character at the given index"));

    addTokenDescription(QString("{<i>from</i> - <i>to</i>}"), i18n("Range"),
             i18n("Extract a specific range ('<i>to'</i> = end of string, if omitted)"));

    setRegExp("\\{\\s*(\\d+)\\s*(-\\s*((-1|\\d+)\\s*)?)?\\}");
}

void RangeModifier::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    QString tmp;

    QPointer<RangeDialog> dlg = new RangeDialog;
    if (dlg->exec() == KDialog::Accepted)
    {
        int start = dlg->start();
        int stop  = dlg->stop();
        tmp       = QString("{%1-%2}").arg(QString::number(start))
                                      .arg(QString::number(stop));
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
