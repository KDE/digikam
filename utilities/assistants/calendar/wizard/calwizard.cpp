/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : main dialog.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2012-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "calwizard.h"

// Qt includes

#include <QDate>
#include <QPrintDialog>
#include <QPrinter>
#include <QStringList>
#include <QMenu>
#include <QPushButton>

// KDE includes

#include <kcalendarsystem.h>
#include <klocalizedstring.h>

// Local includes

#include "calprinter.h"
#include "calsettings.h"
#include "caltemplate.h"
#include "digikam_debug.h"
#include "ui_calevents.h"
#include "ui_calprogress.h"

namespace Digikam
{

class CalWizard::Private
{
public:

    Private()
    {
        cSettings     = 0;
        wTemplate     = 0;
        wEvents       = 0;
        wPrintLabel   = 0;
        wFinish       = 0;
        wTemplatePage = 0;
        wEventsPage   = 0;
        wPrintPage    = 0;
        wFinishPage   = 0;
        printer       = 0;
        printThread   = 0;
    }

    CalSettings*     cSettings;
    CalTemplate*     wTemplate;
    QWidget*         wEvents;
    Ui::CalEvents    calEventsUI;
    Ui::CalProgress  calProgressUI;

    QLabel*          wPrintLabel;
    QWidget*         wFinish;

    DWizardPage*     wTemplatePage;
    DWizardPage*     wEventsPage;
    DWizardPage*     wPrintPage;
    DWizardPage*     wFinishPage;

    QPrinter*        printer;

    CalPrinter*      printThread;

    QMap<int, QUrl>  months;
};

CalWizard::CalWizard(const QList<QUrl>& urlList, QWidget* const parent)
    : QWizard(parent),
      d(new Private)
{
    setWindowTitle(i18n("Create Calendar"));
    d->cSettings     = CalSettings::instance(this);

    // ---------------------------------------------------------------

    d->wTemplate     = new CalTemplate(urlList, this);
    d->wTemplatePage = new DWizardPage(this, i18n("Create Template for Calendar"));
    d->wTemplatePage->setPageWidget(d->wTemplate);
    d->wTemplatePage->setShowLeftView(false);

    // ---------------------------------------------------------------

    d->wEvents     = new QWidget(this);
    d->calEventsUI.setupUi(d->wEvents);
    d->wEventsPage = new DWizardPage(this, i18n("Choose events to show on the Calendar"));
    d->wEventsPage->setPageWidget(d->wEvents);
    d->wEventsPage->setShowLeftView(false);

    // ---------------------------------------------------------------

    d->wPrintLabel = new QLabel(this);
    d->wPrintLabel->setIndent(20);
    d->wPrintLabel->setWordWrap(true);
    d->wPrintPage = new DWizardPage(this, i18n("Print Calendar"));
    d->wPrintPage->setPageWidget(d->wPrintLabel);
    d->wPrintPage->setShowLeftView(false);

    // ---------------------------------------------------------------

    d->wFinish     = new QWidget(this);
    d->calProgressUI.setupUi(d->wFinish);
    d->wFinishPage = new DWizardPage(this, i18n("Printing in Progress"));
    d->wFinishPage->setPageWidget(d->wFinish);
    d->wFinishPage->setShowLeftView(false);

    // ---------------------------------------------------------------

    d->calEventsUI.ohUrlRequester->setFileDlgFilter(i18n("%1|Calendar Data File", QLatin1String("*.ics")));
    d->calEventsUI.ohUrlRequester->setFileDlgTitle(i18n("Select Calendar Data File"));
    d->calEventsUI.ohUrlRequester->setFileDlgMode(QFileDialog::ExistingFile);

    d->calEventsUI.fhUrlRequester->setFileDlgFilter(i18n("%1|Calendar Data File", QLatin1String("*.ics")));
    d->calEventsUI.fhUrlRequester->setFileDlgTitle(i18n("Select Calendar Data File"));
    d->calEventsUI.fhUrlRequester->setFileDlgMode(QFileDialog::ExistingFile);

    // ------------------------------------------

    d->printThread = 0;
    d->printer     = 0;

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotPageSelected(int)));
}

CalWizard::~CalWizard()
{
    if (d->printThread)
    {
        d->printThread->cancel();
        d->printThread->wait();
        delete d->printThread;
    }

    delete d->printer;
    delete d;
}

void CalWizard::slotPageSelected(int curr)
{
    DWizardPage* const current = dynamic_cast<DWizardPage*>(page(curr));

    if (current == d->wPrintPage)
    {
        d->months.clear();
        QUrl        image;
        QString     month;
        QStringList printList;
        QDate       date;
        KLocale::global()->calendar()->setDate(date, d->cSettings->year(), 1, 1);

        for (int i = 1; i <= KLocale::global()->calendar()->monthsInYear(date); ++i)
        {
            month = KLocale::global()->calendar()->monthName(i, d->cSettings->year(), KCalendarSystem::LongName);
            image = d->cSettings->image(i);

            if (!image.isEmpty())
            {
                d->months.insert(i, image);
                printList.append(month);
            }
        }

        if (d->months.empty())
        {
            d->wPrintLabel->setText(QString::fromLatin1("<qt>") +
                                  i18n("No valid images selected for months<br/>"
                                       "Click Back to select images") + QString::fromLatin1("</qt>"));
            d->wFinishPage->setComplete(false);
        }
        else
        {
            int year = d->cSettings->year();

            QString extra;

            if ((KLocale::global()->calendar()->month(QDate::currentDate()) >= 6    &&
                 KLocale::global()->calendar()->year(QDate::currentDate()) == year) ||
                 KLocale::global()->calendar()->year(QDate::currentDate()) > year)
                extra = QString::fromLatin1("<br/><br/><b>") +
                    i18n("Please note that you are making a "
                         "calendar for<br/>the current year or a year in the "
                         "past.") + QString::fromLatin1("</b>");

            QString year_locale = KLocale::global()->calendar()->formatDate(date, KLocale::Year, KLocale::LongNumber);

            d->wPrintLabel->setText(i18n("Click Next to start Printing<br/><br/>"
                                       "Following months will be printed for year %1:<br/>", year_locale)
                                  + printList.join(QString::fromLatin1(" - ")) + extra);
            d->wPrintLabel->setTextFormat(Qt::RichText);

            d->wFinishPage->setComplete(true);
        }
    }

    else if (current == d->wFinishPage)
    {
        d->calProgressUI.finishLabel->clear();
        d->calProgressUI.currentProgress->reset();
        d->calProgressUI.totalProgress->reset();

        button(QWizard::BackButton)->setEnabled(false);
        button(QWizard::NextButton)->setEnabled(false);

        // Set printer settings ---------------------------------------

        if (!d->printer)
        {
            d->printer = new QPrinter(d->cSettings->resolution());
        }

        CalParams& params = d->cSettings->params;

        // Orientation
        switch (params.imgPos)
        {
            case (CalParams::Top):
                d->printer->setOrientation(QPrinter::Portrait);
                break;

            default:
                d->printer->setOrientation(QPrinter::Landscape);
                break;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "printing...";

        // PageSize
        d->printer->setPageSize(QPageSize(params.pageSize));

        QPrintDialog* const printDialog = new QPrintDialog(d->printer, this);
        printDialog->setWindowTitle(i18n("Print Calendar"));

        if (printDialog->exec() == QDialog::Accepted)
        {
            print();
        }
        else
        {
            d->calProgressUI.finishLabel->setText(i18n("Printing Cancelled"));
            button(QWizard::BackButton)->setEnabled(true);
        }

        delete printDialog;
    }
}

void CalWizard::print()
{
    d->calProgressUI.totalProgress->setMaximum(d->months.count());
    d->calProgressUI.totalProgress->setValue(0);
    d->calProgressUI.totalProgress->progressScheduled(i18n("Create calendar"), false, true);
    d->calProgressUI.totalProgress->progressThumbnailChanged(QIcon::fromTheme(QString::fromLatin1("view-pim-calendar")).pixmap(22, 22));

    if (d->printThread)
    {
        d->printThread->cancel();
        d->printThread->wait();
        delete d->printThread;
    }

    d->cSettings->clearSpecial();
    d->cSettings->loadSpecial(QUrl::fromLocalFile(d->calEventsUI.ohUrlRequester->lineEdit()->text()), Qt::red);
    d->cSettings->loadSpecial(QUrl::fromLocalFile(d->calEventsUI.fhUrlRequester->lineEdit()->text()), Qt::darkGreen);

    d->printThread = new CalPrinter(d->printer, d->months, this);

    connect(d->printThread, SIGNAL(pageChanged(int)),
            this,         SLOT(updatePage(int)));

    connect(d->printThread, SIGNAL(pageChanged(int)),
            d->calProgressUI.totalProgress, SLOT(setValue(int)));

    connect(d->printThread, SIGNAL(totalBlocks(int)),
            d->calProgressUI.currentProgress, SLOT(setMaximum(int)));

    connect(d->printThread, SIGNAL(blocksFinished(int)),
            d->calProgressUI.currentProgress, SLOT(setValue(int)));

    d->calProgressUI.totalProgress->setMaximum(d->months.count());
    d->printThread->start();
}

void CalWizard::updatePage(int page)
{
    const int year = d->cSettings->year();
    QDate date(year, 1, 1);

    if (page >= d->months.count())
    {
        printComplete();
        return;
    }

    int month = d->months.keys().at(page);

    d->calProgressUI.finishLabel->setText(i18n("Printing calendar page for %1 of %2",
        KLocale::global()->calendar()->monthName(month, year, KCalendarSystem::LongName),
        KLocale::global()->calendar()->formatDate(date, QString::fromLatin1("%Y"))));
}

void CalWizard::printComplete()
{
    d->calProgressUI.totalProgress->progressCompleted();
    button(QWizard::BackButton)->setEnabled(true);
    button(QWizard::NextButton)->setEnabled(true);
    d->calProgressUI.finishLabel->setText(i18n("Printing Complete"));
}

}  // Namespace Digikam
