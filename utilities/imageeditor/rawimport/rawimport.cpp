/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Raw import tool
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

#include "rawimport.h"
#include "rawimport.moc"

// Qt includes

#include <QString>
#include <QLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "drawdecoding.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "curveswidget.h"
#include "imagehistogram.h"
#include "rawsettingsbox.h"
#include "rawpostprocessing.h"
#include "editortooliface.h"
#include "rawpreview.h"

namespace Digikam
{

class RawImportPriv
{
public:

    RawImportPriv()
    {
        previewWidget = 0;
        settingsBox   = 0;
    }

    RawSettingsBox* settingsBox;
    RawPreview*     previewWidget;
};

RawImport::RawImport(const KUrl& url, QObject *parent)
         : EditorToolThreaded(parent), d(new RawImportPriv)
{
    d->previewWidget = new RawPreview(url, 0);
    d->settingsBox   = new RawSettingsBox(url, 0);

    setToolName(i18n("Raw Import"));
    setToolIcon(SmallIcon("kdcraw"));
    setProgressMessage(i18n("Post Processing"));
    setToolView(d->previewWidget);
    setToolSettings(d->settingsBox);
    init();
}

RawImport::~RawImport()
{
    delete d;
}

void RawImport::slotInit()
{
    EditorToolThreaded::slotInit();

    // ---------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalLoadingStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->previewWidget, SIGNAL(signalLoadingProgress(float)),
            this, SLOT(slotLoadingProgress(float)));

    connect(d->previewWidget, SIGNAL(signalLoadingFailed()),
            this, SLOT(slotLoadingFailed()));

    connect(d->previewWidget, SIGNAL(signalDemosaicedImage()),
            this, SLOT(slotDemosaicedImage()));

    connect(d->settingsBox, SIGNAL(signalDemosaicingChanged()),
            this, SLOT(slotDemosaicingChanged()));

    connect(d->settingsBox, SIGNAL(signalPostProcessingChanged()),
            this, SLOT(slotTimer()));

    connect(d->settingsBox, SIGNAL(signalUpdatePreview()),
            this, SLOT(slotUpdatePreview()));

    connect(d->settingsBox, SIGNAL(signalAbortPreview()),
            this, SLOT(slotAbort()));

    // ---------------------------------------------------------------

    setBusy(true);
    slotUpdatePreview();
}

void RawImport::setBusy(bool val)
{
    if (val) d->previewWidget->setCursor(Qt::WaitCursor);
    else d->previewWidget->unsetCursor();
    d->settingsBox->setBusy(val);
}

DRawDecoding RawImport::rawDecodingSettings()
{
    return d->settingsBox->settings();
}

void RawImport::slotUpdatePreview()
{
    DRawDecoding settings = rawDecodingSettings();
    // We will load an half size image to speed up preview computing.
    settings.halfSizeColorImage = true;

    d->previewWidget->setDecodingSettings(settings);
}

void RawImport::slotAbort()
{
    // If preview loading, don't play with threaded filter interface.
    if (renderingMode() == EditorToolThreaded::NoneRendering)
    {
        d->previewWidget->cancelLoading();
        d->settingsBox->histogramBox()->histogram()->stopHistogramComputation();
        EditorToolIface::editorToolIface()->setToolStopProgress();
        setBusy(false);
        return;
    }

    EditorToolThreaded::slotAbort();
}

void RawImport::slotLoadingStarted()
{
    d->settingsBox->enableUpdateBtn(false);
    d->settingsBox->histogramBox()->histogram()->setDataLoading();
    d->settingsBox->curve()->setDataLoading();
    EditorToolIface::editorToolIface()->setToolStartProgress(i18n("Raw Decoding"));
    setBusy(true);
}

void RawImport::slotDemosaicedImage()
{
    d->settingsBox->setDemosaicedImage(d->previewWidget->demosaicedImage());
    slotEffect();
}

void RawImport::prepareEffect()
{
    DImg postImg = d->previewWidget->demosaicedImage();
    setFilter(dynamic_cast<DImgThreadedFilter*>(new RawPostProcessing(&postImg, this, rawDecodingSettings())));
}

void RawImport::putPreviewData()
{
    d->previewWidget->setPostProcessedImage(filter()->getTargetImage());
    d->settingsBox->setPostProcessedImage(d->previewWidget->postProcessedImage());
    EditorToolIface::editorToolIface()->setToolStopProgress();
    setBusy(false);
}

void RawImport::slotLoadingFailed()
{
    d->settingsBox->histogramBox()->histogram()->setLoadingFailed();
    EditorToolIface::editorToolIface()->setToolStopProgress();
    setBusy(false);
}

void RawImport::slotDemosaicingChanged()
{
    d->settingsBox->enableUpdateBtn(true);
}

void RawImport::slotLoadingProgress(float v)
{
    EditorToolIface::editorToolIface()->setToolProgress((int)(v*100));
}

void RawImport::slotOk()
{
    EditorTool::slotOk();
}

void RawImport::slotCancel()
{
    EditorTool::slotCancel();
}

} // namespace Digikam
