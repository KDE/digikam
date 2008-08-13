/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : Raw import dialog
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qstring.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>

// Local includes.

#include "ddebug.h"
#include "drawdecoding.h"
#include "histogramwidget.h"
#include "curveswidget.h"
#include "imagehistogram.h"
#include "rawsettingsbox.h"
#include "rawpreview.h"
#include "rawimportdlg.h"
#include "rawimportdlg.moc"

namespace Digikam
{

class RawImportDlgPriv
{
public:

    RawImportDlgPriv()
    {
        timer         = 0;
        previewWidget = 0;
        settingsBox   = 0; 
    }

    QTimer         *timer;

    RawSettingsBox *settingsBox;

    RawPreview     *previewWidget;
};

RawImportDlg::RawImportDlg(const KURL& url, QWidget *parent)
            : KDialogBase(parent, 0, false, QString(),
                          Help|Default|User1|User2|Ok|Cancel, Cancel, true)
{
    d = new RawImportDlgPriv;

    setHelp("rawimport.anchor", "digikam");
    setCaption(i18n("Raw Import - %1").arg(url.fileName()));

    setButtonGuiItem(User1, KGuiItem(i18n("&Preview"), "run"));
    setButtonTip(User1, i18n("<p>Generate a Raw image preview using current settings."));

    setButtonGuiItem(User2, KGuiItem(i18n("&Abort"), "stop"));
    setButtonTip(User2, i18n("<p>Abort the current Raw image preview"));

    setButtonText(Ok, i18n("&Import"));
    setButtonTip(Ok, i18n("<p>Import image to editor using current settings."));

    setButtonGuiItem(Cancel, KGuiItem(i18n("&Use Default"), "gohome"));
    setButtonTip(Cancel, i18n("<p>Use general Raw decoding settings to load this image in editor."));

    setButtonGuiItem(Default, KGuiItem(i18n("&Reset"), "reload_page"));
    setButtonTip(Default, i18n("<p>Reset these settings to default values."));

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout *mainLayout = new QGridLayout(page, 1, 1);
    d->previewWidget        = new RawPreview(url, page);
    d->settingsBox          = new RawSettingsBox(url, page);

    // ---------------------------------------------------------------

    mainLayout->addMultiCellWidget(d->previewWidget, 0, 1, 0, 0);
    mainLayout->addMultiCellWidget(d->settingsBox,   0, 0, 1, 1);
    mainLayout->setColStretch(0, 10);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setSpacing(spacingHint());
    mainLayout->setMargin(0);

    readSettings();

    // ---------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalDemosaicedImage()),
            this, SLOT(slotDemosaicedImage()));

    connect(d->previewWidget, SIGNAL(signalPostProcessedImage()),
            this, SLOT(slotPostProcessedImage()));

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalLoadingFailed()),
            this, SLOT(slotLoadingFailed()));

    connect(d->settingsBox, SIGNAL(signalPostProcessingChanged()),
            this, SLOT(slotPostProcessing()));

    // ---------------------------------------------------------------

    busy(true);
    enableButton (User2, false);
    slotUser1();
}

RawImportDlg::~RawImportDlg()
{
    delete d->timer;
    delete d;
}

void RawImportDlg::closeEvent(QCloseEvent *e)
{
    if (!e) return;
    saveSettings();
    e->accept();
}

void RawImportDlg::slotClose()
{
    saveSettings();
    KDialogBase::slotClose();
}

void RawImportDlg::slotDefault()
{
    d->settingsBox->setDefaultSettings();
}

void RawImportDlg::slotOk()
{
    saveSettings();
    KDialogBase::slotOk();
}

void RawImportDlg::busy(bool val)
{
    if (val) d->previewWidget->setCursor(KCursor::waitCursor());
    else d->previewWidget->unsetCursor();
    d->settingsBox->setBusy(val);
    enableButton (Default, !val);
    enableButton (Ok,      !val);
    enableButton (Close,   !val);
    enableButton (User1,   !val);
    enableButton (User2,   val);
}

void RawImportDlg::readSettings()
{
    KConfig* config = kapp->config();
    resize(configDialogSize(*config, QString("RAW Import Dialog")));
    d->settingsBox->readSettings();
}

void RawImportDlg::saveSettings()
{
    KConfig* config = kapp->config();
    saveDialogSize(*config, QString("RAW Import Dialog"));
    d->settingsBox->saveSettings();
    config->sync();
}

DRawDecoding RawImportDlg::rawDecodingSettings()
{
    return d->settingsBox->settings();
}

// 'Preview' dialog button.
void RawImportDlg::slotUser1()
{
    DRawDecoding settings = rawDecodingSettings();
    // We will load an half size image to speed up preview computing.
    settings.halfSizeColorImage = true;

    d->previewWidget->setDecodingSettings(settings);
}

// 'Abort' dialog button.
void RawImportDlg::slotUser2()
{
    d->previewWidget->cancelLoading();
    d->settingsBox->histogram()->stopHistogramComputation();
    busy(false);
}

void RawImportDlg::slotLoadingStarted()
{
    d->settingsBox->histogram()->setDataLoading();
    d->settingsBox->curve()->setDataLoading();
    busy(true);
}

void RawImportDlg::slotDemosaicedImage()
{
    d->settingsBox->setDemosaicedImage(d->previewWidget->demosaicedImage());
}

void RawImportDlg::slotPostProcessedImage()
{
    d->settingsBox->setPostProcessedImage(d->previewWidget->postProcessedImage());
    busy(false);
}

void RawImportDlg::slotLoadingFailed()
{
    d->settingsBox->histogram()->setLoadingFailed();
    busy(false);
}

void RawImportDlg::slotTimer()
{
    if (d->timer)
    {
       d->timer->stop();
       delete d->timer;
    }

    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()),
             this, SLOT(slotPostProcessing()) );
    d->timer->start(500, true);
}

void RawImportDlg::slotPostProcessing()
{
    d->previewWidget->setPostProcessingSettings(rawDecodingSettings());
}

} // NameSpace Digikam
