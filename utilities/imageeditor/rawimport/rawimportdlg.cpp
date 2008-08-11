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
#include "managedloadsavethread.h"
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
        previewWidget = 0;
        thread        = 0;
        settingsBox   = 0; 
    }

    RawSettingsBox        *settingsBox;

    KURL                   url;

    ManagedLoadSaveThread *thread;

    RawPreview            *previewWidget;
};

RawImportDlg::RawImportDlg(const KURL& url, QWidget *parent)
            : KDialogBase(parent, 0, false, QString(),
                          Help|Default|User1|User2|Ok|Cancel, Cancel, true)
{
    d = new RawImportDlgPriv;
    d->thread = new ManagedLoadSaveThread;
    d->url    = url;

    setHelp("rawimport.anchor", "digikam");
    setCaption(i18n("Raw Import - %1").arg(d->url.fileName()));

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
    d->previewWidget        = new RawPreview(page, d->thread);
    d->settingsBox          = new RawSettingsBox(page);
    d->settingsBox->setUrl(d->url);

    // ---------------------------------------------------------------

    mainLayout->addMultiCellWidget(d->previewWidget, 0, 1, 0, 0);
    mainLayout->addMultiCellWidget(d->settingsBox,   0, 0, 1, 1);
    mainLayout->setColStretch(0, 10);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setSpacing(spacingHint());
    mainLayout->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalImageLoaded(const DImg&)),
            this, SLOT(slotImageLoaded(const DImg&)));

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalLoadingFailed()),
            this, SLOT(slotLoadingFailed()));

    connect(d->previewWidget, SIGNAL(signalLoadingProgress(float)),
            this, SLOT(slotLoadingProgress(float)));

    connect(d->thread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
            this, SLOT(slotImageLoaded(const LoadingDescription&, const DImg&)));

    // ---------------------------------------------------------------

    readSettings();

    // Load image for curve widget.
    busy(true);
    enableButton (User2, false);
    d->settingsBox->histogram()->setDataLoading();

    d->settingsBox->curve()->setDataLoading();
    DRawDecoding settings;
    settings.optimizeTimeLoading();
    d->thread->load(LoadingDescription(d->url.path(), settings), 
                    ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
}

RawImportDlg::~RawImportDlg()
{
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
    d->settingsBox->setEnabled(!val);
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

void RawImportDlg::slotImageLoaded(const LoadingDescription& desc, const DImg& img)
{
    if (desc.filePath != d->url.path())
        return;

    disconnect(d->thread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
               this, SLOT(slotImageLoaded(const LoadingDescription&, const DImg&)));

    d->settingsBox->setCurveImage(img);
    slotUser1();
}

// 'Preview' dialog button.
void RawImportDlg::slotUser1()
{
    DRawDecoding settings = rawDecodingSettings();
    // We will load an half size image to speed up preview computing.
    settings.halfSizeColorImage = true;

    d->previewWidget->setDecodingSettings(d->url, settings);
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
    busy(true);
}

void RawImportDlg::slotLoadingProgress(float /*progress*/)
{
}

void RawImportDlg::slotImageLoaded(const DImg& img)
{
    d->settingsBox->histogram()->stopHistogramComputation();
    d->settingsBox->histogram()->updateData(img.bits(), img.width(), img.height(), img.sixteenBit(), 0, 0, 0, true);
    busy(false);
}

void RawImportDlg::slotLoadingFailed()
{
    d->settingsBox->histogram()->setLoadingFailed();
    busy(false);
}

} // NameSpace Digikam
