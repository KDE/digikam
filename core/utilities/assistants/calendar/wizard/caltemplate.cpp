/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : template selection for calendar.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "caltemplate.h"

// Qt includes

#include <QButtonGroup>

// Local includes

#include "calsettings.h"
#include "calsystem.h"
#include "calmonthwidget.h"
#include "calpainter.h"
#include "digikam_debug.h"

namespace Digikam
{

class CalTemplate::Private
{
public:

    explicit Private()
      : MAX_MONTHS(13)
    {
    }

    const int                MAX_MONTHS;
    Ui::CalTemplate          ui;
    QVector<CalMonthWidget*> wVector;
};

CalTemplate::CalTemplate(const QList<QUrl>& urlList, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    d->ui.setupUi(this);

    CalSettings* const settings = CalSettings::instance();

    // set initial settings
    settings->setPaperSize(d->ui.paperSizeCombo->currentText());
    settings->setDrawLines(d->ui.drawLinesCheckBox->isChecked());
    settings->setRatio(d->ui.ratioSlider->value());
    settings->setFont(d->ui.fontCombo->currentText());
    settings->setResolution(d->ui.resolutionCombo->currentText());

    d->ui.calendarWidget->recreate();

    connect(d->ui.yearSpin, SIGNAL(valueChanged(int)),
            this, SLOT(yearChanged(int)));

    int currentYear   = CalSystem().year(QDate::currentDate());

    QDate date        = CalSystem().date(currentYear, 1, 1);
    int months        = CalSystem().monthsInYear(date);
    // span the monthWidgets over 2 rows. inRow should usually be 6 or 7 (for 12 or 13 months)
    int inRow         = (months / 2) + ((months % 2) != 0);
    CalMonthWidget* w = 0;

    for (int i = 0; i < d->MAX_MONTHS; ++i)
    {
        w = new CalMonthWidget(d->ui.monthBox, i + 1);

        connect(w, SIGNAL(monthSelected(int)),
                this, SLOT(monthChanged(int)));

        if (i < urlList.count())
        {
            w->setImage(urlList[i]);
        }

        if (i < months)
        {
            d->ui.monthBoxLayout->addWidget(w, i / inRow, i % inRow);
        }
        else
        {
            w->hide();
        }

        d->wVector.insert(i, w);
    }

    d->ui.yearSpin->setRange(CalSystem().year(CalSystem().earliestValidDate()) + 1,
                             CalSystem().year(CalSystem().latestValidDate()) - 1);
    d->ui.yearSpin->setValue(currentYear);

    QButtonGroup* const btnGrp = new QButtonGroup(d->ui.imagePosButtonGroup);
    btnGrp->addButton(d->ui.topRadio,   CalParams::Top);
    btnGrp->addButton(d->ui.leftRadio,  CalParams::Left);
    btnGrp->addButton(d->ui.rightRadio, CalParams::Right);
    btnGrp->setExclusive(true);

    connect(d->ui.paperSizeCombo, SIGNAL(currentIndexChanged(QString)),
            settings, SLOT(setPaperSize(QString)));

    connect(d->ui.resolutionCombo, SIGNAL(currentIndexChanged(QString)),
            settings, SLOT(setResolution(QString)));

    connect(btnGrp, SIGNAL(buttonClicked(int)),
            settings, SLOT(setImagePos(int)));

    connect(d->ui.drawLinesCheckBox, SIGNAL(toggled(bool)),
            settings, SLOT(setDrawLines(bool)));

    connect(d->ui.ratioSlider, SIGNAL(valueChanged(int)),
            settings, SLOT(setRatio(int)));

    connect(d->ui.fontCombo, SIGNAL(currentIndexChanged(QString)),
            settings, SLOT(setFont(QString)));

    connect(settings, SIGNAL(settingsChanged()),
            d->ui.calendarWidget, SLOT(recreate()));
}

CalTemplate::~CalTemplate()
{
    delete d;
}

void CalTemplate::monthChanged(int m)
{
  d->ui.calendarWidget->setCurrent(m);
}

void CalTemplate::yearChanged(int year)
{
    int months;
    QDate date = CalSystem().date(year, 1, 1);
    QDate oldD = CalSystem().date(CalSettings::instance()->year(), 1, 1);
    months     = CalSystem().monthsInYear(date);

    if ((CalSystem().monthsInYear(oldD) != months) && !d->wVector.isEmpty())
    {
        int i;

        // hide the last months that are not present on the current year
        for (i = months; (i < CalSystem().monthsInYear(oldD)) && (i < d->wVector.count()); ++i)
        {
            d->wVector.at(i)->hide();
        }

        // span the monthWidgets over 2 rows. inRow should usually be 6 or 7 (for 12 or 13 months)
        int inRow = (months / 2) + ((months % 2) != 0);

        // remove all the monthWidgets, then readd the needed ones
        for (i = 0; i < CalSystem().monthsInYear(oldD); ++i)
        {
            d->ui.monthBoxLayout->removeWidget(d->wVector.at(i));
        }

        for (i = 0; (i < months) && (i < d->wVector.count()); ++i)
        {
            d->ui.monthBoxLayout->addWidget(d->wVector.at(i), i / inRow, i % inRow);

            if (d->wVector.at(i)->isHidden())
            {
                d->wVector.at(i)->show();
            }

            d->wVector.at(i)->update();
        }
    }

    CalSettings::instance()->setYear(year);
}

} // Namespace Digikam
