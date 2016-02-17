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

// KDE includes

#include <kcalendarsystem.h>
#include <klocalizedstring.h>

// Local includes

#include "calsettings.h"
#include "calmonthwidget.h"
#include "calpainter.h"
#include "digikam_debug.h"

#define MAX_MONTHS (13)

namespace Digikam
{

CalTemplate::CalTemplate(const QList<QUrl>& urlList, QWidget* const parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    CalSettings* const settings = CalSettings::instance();

    // set initial settings
    settings->setPaperSize(m_ui.paperSizeCombo->currentText());
    settings->setDrawLines(m_ui.drawLinesCheckBox->isChecked());
    settings->setRatio(m_ui.ratioSlider->value());
    settings->setFont(m_ui.fontCombo->currentText());
    settings->setResolution(m_ui.resolutionCombo->currentText());

    m_ui.calendarWidget->recreate();

    connect(m_ui.yearSpin, SIGNAL(valueChanged(int)),
            this, SLOT(yearChanged(int)));

    const KCalendarSystem* const cal = KLocale::global()->calendar();
    int currentYear                  = cal->year(QDate::currentDate());

    QDate d;
    cal->setDate(d, currentYear, 1, 1);
    int months     = cal->monthsInYear(d);
    // span the monthWidgets over 2 rows. inRow should usually be 6 or 7 (for 12 or 13 months)
    int inRow      = (months / 2) + ((months % 2) != 0);
    CalMonthWidget* w = 0;

    for (int i = 0; i < MAX_MONTHS; ++i)
    {
        w = new CalMonthWidget(m_ui.monthBox, i + 1);

        connect(w, SIGNAL(monthSelected(int)),
                this, SLOT(monthChanged(int)));

        if (i < urlList.count())
        {
            w->setImage(urlList[i]);
        }

        if (i < months)
        {
            m_ui.monthBoxLayout->addWidget(w, i / inRow, i % inRow);
        }
        else
        {
            w->hide();
        }

        m_wVector.insert(i, w);
    }

    m_ui.yearSpin->setRange(cal->year(cal->earliestValidDate()) + 1, cal->year(cal->latestValidDate()) - 1);
    m_ui.yearSpin->setValue(currentYear);
    
    QButtonGroup* const btnGrp = new QButtonGroup(m_ui.imagePosButtonGroup);
    btnGrp->addButton(m_ui.topRadio,   CalParams::Top);
    btnGrp->addButton(m_ui.leftRadio,  CalParams::Left);
    btnGrp->addButton(m_ui.rightRadio, CalParams::Right);
    btnGrp->setExclusive(true);

    connect(m_ui.paperSizeCombo, SIGNAL(currentIndexChanged(QString)),
            settings, SLOT(setPaperSize(QString)));

    connect(m_ui.resolutionCombo, SIGNAL(currentIndexChanged(QString)),
            settings, SLOT(setResolution(QString)));

    connect(btnGrp, SIGNAL(buttonClicked(int)),
            settings, SLOT(setImagePos(int)));

    connect(m_ui.drawLinesCheckBox, SIGNAL(toggled(bool)),
            settings, SLOT(setDrawLines(bool)));

    connect(m_ui.ratioSlider, SIGNAL(valueChanged(int)),
            settings, SLOT(setRatio(int)));

    connect(m_ui.fontCombo, SIGNAL(currentIndexChanged(QString)),
            settings, SLOT(setFont(QString)));

    connect(settings, SIGNAL(settingsChanged()),
            m_ui.calendarWidget, SLOT(recreate()));
}

CalTemplate::~CalTemplate()
{
}

void CalTemplate::monthChanged(int m)
{
  m_ui.calendarWidget->setCurrent(m);
}

void CalTemplate::yearChanged(int year)
{
    int months;
    QDate d, oldD;
    const KCalendarSystem* const cal = KLocale::global()->calendar();
    cal->setDate(d, year, 1, 1);
    cal->setDate(oldD, CalSettings::instance()->year(), 1, 1);
    months = cal->monthsInYear(d);

    if ((cal->monthsInYear(oldD) != months) && !m_wVector.isEmpty())
    {
        int i;

        // hide the last months that are not present on the current year
        for (i = months; (i < cal->monthsInYear(oldD)) && (i < m_wVector.count()); ++i)
        {
            m_wVector.at(i)->hide();
        }

        // span the monthWidgets over 2 rows. inRow should usually be 6 or 7 (for 12 or 13 months)
        int inRow = (months / 2) + ((months % 2) != 0);

        // remove all the monthWidgets, then readd the needed ones
        for (i = 0; i < cal->monthsInYear(oldD); ++i)
        {
            m_ui.monthBoxLayout->removeWidget(m_wVector.at(i));
        }

        for (i = 0; (i < months) && (i < m_wVector.count()); ++i)
        {
            m_ui.monthBoxLayout->addWidget(m_wVector.at(i), i / inRow, i % inRow);

            if (m_wVector.at(i)->isHidden())
            {
                m_wVector.at(i)->show();
            }

            m_wVector.at(i)->update();
        }
    }

    CalSettings::instance()->setYear(year);
}

}  // Namespace Digikam
