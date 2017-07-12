/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "advprintfinalpage.h"

// Qt includes

#include <QImage>
#include <QIcon>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintthread.h"
#include "advprintwizard.h"
#include "advprintcaptionpage.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "dmetadata.h"
#include "dfileoperations.h"
#include "dimg.h"

namespace Digikam
{

class AdvPrintFinalPage::Private
{
public:

    Private(QWizard* const dialog)
      : FONT_HEIGHT_RATIO(0.8F),
        progressView(0),
        progressBar(0),
        wizard(0),
        settings(0),
        printThread(0),
        iface(0),
        complete(false)
    {
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = wizard->iface();
        }
    }

    const float       FONT_HEIGHT_RATIO;

    DHistoryView*     progressView;
    DProgressWdg*     progressBar;
    AdvPrintWizard*   wizard;
    AdvPrintSettings* settings;
    AdvPrintThread*   printThread;
    DInfoInterface*   iface;
    bool              complete;
};

AdvPrintFinalPage::AdvPrintFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("FinalPage"));

    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));
}

AdvPrintFinalPage::~AdvPrintFinalPage()
{
    delete d;
}

void AdvPrintFinalPage::initializePage()
{
    d->complete = false;
    emit completeChanged();
    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void AdvPrintFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Starting to render printing..."),
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("%1 items to process", d->settings->inputImages.count()),
                              DHistoryView::ProgressEntry);

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->photos.count());

    if (!d->wizard->prepareToPrint())
    {
        d->progressView->addEntry(i18n("Printing process aborted..."),
                                  DHistoryView::ErrorEntry);
    }

    d->printThread = new AdvPrintThread(this);

    connect(d->printThread, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->printThread, SIGNAL(signalMessage(QString, bool)),
            this, SLOT(slotMessage(QString, bool)));

    connect(d->printThread, SIGNAL(signalDone(bool)),
            this, SLOT(slotDone(bool)));

    d->printThread->setSettings(d->settings);
    d->printThread->start();
}

void AdvPrintFinalPage::cleanupPage()
{
    if (d->printThread)
        d->printThread->cancel();

    if (d->settings->gimpFiles.count() > 0)
    {
        removeGimpFiles();
    }
}

void AdvPrintFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

void AdvPrintFinalPage::slotDone(bool completed)
{
    d->progressBar->progressCompleted();
    d->complete = completed;

    if (!d->complete)
    {
        d->progressView->addEntry(i18n("Printing process is not completed"),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Printing process completed."),
                                  DHistoryView::ProgressEntry);

        if (d->settings->printerName == d->settings->outputName(AdvPrintSettings::FILES))
        {
            if (d->settings->openInFileBrowser)
            {
                QDesktopServices::openUrl(d->settings->outputDir);
                d->progressView->addEntry(i18n("Open destination directory in file-browser."),
                                          DHistoryView::ProgressEntry);
            }
        }
        else if(d->settings->printerName == d->settings->outputName(AdvPrintSettings::GIMP))
        {
            if (!d->settings->gimpFiles.isEmpty())
            {
                QStringList args;
                QString prog = d->settings->gimpPath;

                for (QStringList::ConstIterator it = d->settings->gimpFiles.constBegin() ;
                    it != d->settings->gimpFiles.constEnd() ; ++it)
                {
                    args << (*it);
                }

                QProcess process;
                process.setProcessEnvironment(adjustedEnvironmentForAppImage());

                if (!process.startDetached(prog, args))
                {
                    d->progressView->addEntry(i18n("There was an error to launch the external "
                                                "Gimp program. Please make sure it is properly "
                                                "installed."),
                                              DHistoryView::WarningEntry);
                    return;
                }
            }
        }
    }

    emit completeChanged();
}

bool AdvPrintFinalPage::isComplete() const
{
    return d->complete;
}

void AdvPrintFinalPage::removeGimpFiles()
{
    for (QStringList::ConstIterator it = d->settings->gimpFiles.constBegin() ;
         it != d->settings->gimpFiles.constEnd() ; ++it)
    {
        if (QFile::exists(*it))
        {
            if (QFile::remove(*it) == false)
            {
                QMessageBox::information(this,
                                         QString(),
                                         i18n("Could not remove the GIMP's temporary files."));
                break;
            }
        }
    }
}

} // namespace Digikam
