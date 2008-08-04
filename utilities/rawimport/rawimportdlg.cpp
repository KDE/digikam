/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
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

#include <qtimer.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qfileinfo.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qfile.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapplication.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawsettingswidget.h>

// Local includes.

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
        previewWidget       = 0;
        decodingSettingsBox = 0;
    }

    ImageInfo                         info;

    RawPreview                       *previewWidget;

    KDcrawIface::DcrawSettingsWidget *decodingSettingsBox;
};

RawImportDlg::RawImportDlg(const ImageInfo& info, QWidget */*parent*/)
            : KDialogBase(0, 0, false, i18n("Raw Import"),
                          Help|Default|User1|User2|User3|Close, Close, true,
                          i18n("&Preview"), i18n("&Load"), i18n("&Abort"))
{
    d = new RawImportDlgPriv;
    d->info = info;

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout *mainLayout = new QGridLayout(page, 1, 1, 0, spacingHint());
    d->previewWidget        = new RawPreview(page);
    d->decodingSettingsBox  = new KDcrawIface::DcrawSettingsWidget(page, true, true, false);

    mainLayout->addMultiCellWidget(d->previewWidget,       0, 1, 0, 0);
    mainLayout->addMultiCellWidget(d->decodingSettingsBox, 0, 0, 1, 1);
    mainLayout->setColStretch(0, 10);
    mainLayout->setRowStretch(1, 10);

    // ---------------------------------------------------------------

    setButtonTip( User1, i18n("<p>Generate a Preview from current settings. "
                              "Uses a simple bilinear interpolation for "
                              "quick results."));

    setButtonTip( User2, i18n("<p>Convert the Raw Image from current settings. "
                              "This uses a high-quality adaptive algorithm."));

    setButtonTip( User3, i18n("<p>Abort the current Raw file conversion"));

    setButtonTip( Close, i18n("<p>Exit Raw Converter"));

    // ---------------------------------------------------------------

    busy(false);
    readSettings();
    d->previewWidget->setImageInfo(&d->info);
}

RawImportDlg::~RawImportDlg()
{
    delete d;
}

void RawImportDlg::closeEvent(QCloseEvent *e)
{
    if (!e) return;
//    m_thread->cancel();
    saveSettings();
    e->accept();
}

void RawImportDlg::slotClose()
{
 //   m_thread->cancel();
    saveSettings();
    KDialogBase::slotClose();
}

void RawImportDlg::slotDefault()
{
    d->decodingSettingsBox->setDefaultSettings();
}

void RawImportDlg::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    d->decodingSettingsBox->setWhiteBalance((KDcrawIface::RawDecodingSettings::WhiteBalance)
                                            config->readNumEntry("White Balance",
                                            KDcrawIface::RawDecodingSettings::CAMERA));
    d->decodingSettingsBox->setCustomWhiteBalance(config->readNumEntry("Custom White Balance", 6500));
    d->decodingSettingsBox->setCustomWhiteBalanceGreen(config->readDoubleNumEntry("Custom White Balance Green", 1.0));
    d->decodingSettingsBox->setFourColor(config->readBoolEntry("Four Color RGB", false));
    d->decodingSettingsBox->setUnclipColor(config->readNumEntry("Unclip Color", 0));
    d->decodingSettingsBox->setDontStretchPixels(config->readBoolEntry("Dont Stretch Pixels", false));
    d->decodingSettingsBox->setNoiseReduction(config->readBoolEntry("Use Noise Reduction", false));
    d->decodingSettingsBox->setBrightness(config->readDoubleNumEntry("Brightness Multiplier", 1.0));
    d->decodingSettingsBox->setUseBlackPoint(config->readBoolEntry("Use Black Point", false));
    d->decodingSettingsBox->setBlackPoint(config->readNumEntry("Black Point", 0));
    d->decodingSettingsBox->setNRThreshold(config->readNumEntry("NR Threshold", 100));
    d->decodingSettingsBox->setUseCACorrection(config->readBoolEntry("EnableCACorrection", false));
    d->decodingSettingsBox->setcaRedMultiplier(config->readDoubleNumEntry("caRedMultiplier", 1.0));
    d->decodingSettingsBox->setcaBlueMultiplier(config->readDoubleNumEntry("caBlueMultiplier", 1.0));

    d->decodingSettingsBox->setQuality(
        (KDcrawIface::RawDecodingSettings::DecodingQuality)config->readNumEntry("Decoding Quality", 
            (int)(KDcrawIface::RawDecodingSettings::BILINEAR))); 

    d->decodingSettingsBox->setOutputColorSpace(
        (KDcrawIface::RawDecodingSettings::OutputColorSpace)config->readNumEntry("Output Color Space", 
            (int)(KDcrawIface::RawDecodingSettings::SRGB))); 

    resize(configDialogSize(*config, QString("Raw Import Dialog")));
}

void RawImportDlg::saveSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");
    config->writeEntry("White Balance", d->decodingSettingsBox->whiteBalance());
    config->writeEntry("Custom White Balance", d->decodingSettingsBox->customWhiteBalance());
    config->writeEntry("Custom White Balance Green", d->decodingSettingsBox->customWhiteBalanceGreen());
    config->writeEntry("Four Color RGB", d->decodingSettingsBox->useFourColor());
    config->writeEntry("Unclip Color", d->decodingSettingsBox->unclipColor());
    config->writeEntry("Dont Stretch Pixels", d->decodingSettingsBox->useDontStretchPixels());
    config->writeEntry("Use Noise Reduction", d->decodingSettingsBox->useNoiseReduction());
    config->writeEntry("Brightness Multiplier", d->decodingSettingsBox->brightness());
    config->writeEntry("Use Black Point", d->decodingSettingsBox->useBlackPoint());
    config->writeEntry("Black Point", d->decodingSettingsBox->blackPoint());
    config->writeEntry("NR Threshold", d->decodingSettingsBox->NRThreshold());
    config->writeEntry("EnableCACorrection", d->decodingSettingsBox->useCACorrection());
    config->writeEntry("caRedMultiplier", d->decodingSettingsBox->caRedMultiplier());
    config->writeEntry("caBlueMultiplier", d->decodingSettingsBox->caBlueMultiplier());
    config->writeEntry("Decoding Quality", (int)d->decodingSettingsBox->quality());
    config->writeEntry("Output Color Space", (int)d->decodingSettingsBox->outputColorSpace());

    saveDialogSize(*config, QString("Raw Import Dialog"));
    config->sync();
}

void RawImportDlg::slotHelp()
{
    KApplication::kApplication()->invokeHelp("rawimport", "digikam");
}

// 'Preview' dialog button.
void RawImportDlg::slotUser1()
{
    KDcrawIface::RawDecodingSettings rawDecodingSettings;
    rawDecodingSettings.whiteBalance               = d->decodingSettingsBox->whiteBalance();
    rawDecodingSettings.customWhiteBalance         = d->decodingSettingsBox->customWhiteBalance();
    rawDecodingSettings.customWhiteBalanceGreen    = d->decodingSettingsBox->customWhiteBalanceGreen();
    rawDecodingSettings.RGBInterpolate4Colors      = d->decodingSettingsBox->useFourColor();
    rawDecodingSettings.unclipColors               = d->decodingSettingsBox->unclipColor();
    rawDecodingSettings.DontStretchPixels          = d->decodingSettingsBox->useDontStretchPixels();
    rawDecodingSettings.enableNoiseReduction       = d->decodingSettingsBox->useNoiseReduction();
    rawDecodingSettings.brightness                 = d->decodingSettingsBox->brightness();
    rawDecodingSettings.enableBlackPoint           = d->decodingSettingsBox->useBlackPoint();
    rawDecodingSettings.blackPoint                 = d->decodingSettingsBox->blackPoint();
    rawDecodingSettings.NRThreshold                = d->decodingSettingsBox->NRThreshold();
    rawDecodingSettings.enableCACorrection         = d->decodingSettingsBox->useCACorrection();
    rawDecodingSettings.caMultiplier[0]            = d->decodingSettingsBox->caRedMultiplier();
    rawDecodingSettings.caMultiplier[1]            = d->decodingSettingsBox->caBlueMultiplier();
    rawDecodingSettings.RAWQuality                 = d->decodingSettingsBox->quality();
    rawDecodingSettings.outputColorSpace           = d->decodingSettingsBox->outputColorSpace();

    d->previewWidget->setDecodingSettings(rawDecodingSettings);
}

// 'Convert' dialog button.
void RawImportDlg::slotUser2()
{
    KDcrawIface::RawDecodingSettings rawDecodingSettings;
    rawDecodingSettings.whiteBalance               = d->decodingSettingsBox->whiteBalance();
    rawDecodingSettings.customWhiteBalance         = d->decodingSettingsBox->customWhiteBalance();
    rawDecodingSettings.customWhiteBalanceGreen    = d->decodingSettingsBox->customWhiteBalanceGreen();
    rawDecodingSettings.RGBInterpolate4Colors      = d->decodingSettingsBox->useFourColor();
    rawDecodingSettings.unclipColors               = d->decodingSettingsBox->unclipColor();
    rawDecodingSettings.DontStretchPixels          = d->decodingSettingsBox->useDontStretchPixels();
    rawDecodingSettings.enableNoiseReduction       = d->decodingSettingsBox->useNoiseReduction();
    rawDecodingSettings.brightness                 = d->decodingSettingsBox->brightness();
    rawDecodingSettings.enableBlackPoint           = d->decodingSettingsBox->useBlackPoint();
    rawDecodingSettings.blackPoint                 = d->decodingSettingsBox->blackPoint();
    rawDecodingSettings.NRThreshold                = d->decodingSettingsBox->NRThreshold();
    rawDecodingSettings.enableCACorrection         = d->decodingSettingsBox->useCACorrection();
    rawDecodingSettings.caMultiplier[0]            = d->decodingSettingsBox->caRedMultiplier();
    rawDecodingSettings.caMultiplier[1]            = d->decodingSettingsBox->caBlueMultiplier();
    rawDecodingSettings.RAWQuality                 = d->decodingSettingsBox->quality();
    rawDecodingSettings.outputColorSpace           = d->decodingSettingsBox->outputColorSpace();
/*
    d->thread->setRawDecodingSettings(rawDecodingSettings, d->saveSettingsBox->fileFormat());
    d->thread->processRawFile(KURL(d->inputFile));
    if (!d->thread->running())
        d->thread->start();*/
}

// 'Abort' dialog button.
void RawImportDlg::slotUser3()
{
//    d->thread->cancel();
}

void RawImportDlg::slotIdentify()
{
/*    d->thread->identifyRawFile(KURL(d->inputFile), true);
    if (!d->thread->running())
        d->thread->start();*/
}

void RawImportDlg::busy(bool val)
{
    d->decodingSettingsBox->setEnabled(!val);
    enableButton (User1, !val);
    enableButton (User2, !val);
    enableButton (User3, val);
    enableButton (Close, !val);
}

void RawImportDlg::identified(const QString&, const QString& identity, const QPixmap& preview)
{
//    d->previewWidget->setInfo(d->inputFileName + QString(" :\n") + identity, Qt::white, preview);
}

void RawImportDlg::previewing(const QString&)
{
/*
    d->previewWidget->setCursor( KCursor::waitCursor() );
*/
}

void RawImportDlg::previewed(const QString&, const QString& tmpFile)
{
/*    d->previewWidget->unsetCursor();
    d->blinkPreviewTimer->stop();
    d->previewWidget->load(tmpFile);
    ::remove(QFile::encodeName(tmpFile));*/
}

void RawImportDlg::previewFailed(const QString&)
{
/*    d->previewWidget->unsetCursor();
    d->blinkPreviewTimer->stop();
    d->previewWidget->setInfo(i18n("Failed to generate preview"), Qt::red);*/
}

void RawImportDlg::processing(const QString&)
{
/*    d->convertBlink = false;
    d->previewWidget->setCursor( KCursor::waitCursor() );
    d->blinkConvertTimer->start(200);*/
}

void RawImportDlg::processed(const QString&, const QString& tmpFile)
{
/*    d->previewWidget->unsetCursor();
    d->blinkConvertTimer->stop();
    d->previewWidget->load(tmpFile);
    QString filter("*.");
    QString ext;

    switch(d->saveSettingsBox->fileFormat())
    {
        case SaveSettingsWidget::OUTPUT_JPEG:
            ext = "jpg";
            break;
        case SaveSettingsWidget::OUTPUT_TIFF:
            ext = "tif";
            break;
        case SaveSettingsWidget::OUTPUT_PPM:
            ext = "ppm";
            break;
        case SaveSettingsWidget::OUTPUT_PNG:
            ext = "png";
            break;
    }

    filter += ext;
    QFileInfo fi(d->inputFile);
    QString destFile = fi.dirPath(true) + QString("/") + fi.baseName() + QString(".") + ext;

    if (d->saveSettingsBox->conflictRule() != SaveSettingsWidget::OVERWRITE)
    {
        struct stat statBuf;
        if (::stat(QFile::encodeName(destFile), &statBuf) == 0) 
        {
            KIO::RenameDlg dlg(this, i18n("Save Raw Image converted from '%1' as").arg(fi.fileName()),
                               tmpFile, destFile,
                               KIO::RenameDlg_Mode(KIO::M_SINGLE | KIO::M_OVERWRITE | KIO::M_SKIP));

            switch (dlg.exec())
            {
                case KIO::R_CANCEL:
                case KIO::R_SKIP:
                {
                    destFile = QString();
                    break;
                }
                case KIO::R_RENAME:
                {
                    destFile = dlg.newDestURL().path();
                    break;
                }
                default:    // Overwrite.
                    break;
            }
        }
    }

    if (!destFile.isEmpty()) 
    {
        if (::rename(QFile::encodeName(tmpFile), QFile::encodeName(destFile)) != 0)
        {
            KMessageBox::error(this, i18n("Failed to save image %1").arg( destFile ));
        }
    }*/
}

void RawImportDlg::processingFailed(const QString&)
{
/*    d->previewWidget->unsetCursor();
    d->blinkConvertTimer->stop();
    d->previewWidget->setInfo(i18n("Failed to convert Raw image"), Qt::red);*/
}

void RawImportDlg::slotPreviewBlinkTimerDone()
{
/*    QString preview = i18n("Generating Preview...");

    if (d->previewBlink)
        d->previewWidget->setInfo(preview, Qt::green);
    else
        d->previewWidget->setInfo(preview, Qt::darkGreen);

    d->previewBlink = !d->previewBlink;
    d->blinkPreviewTimer->start(200);*/
}

void RawImportDlg::slotConvertBlinkTimerDone()
{
/*    QString convert = i18n("Converting Raw Image...");

    if (d->convertBlink)
        d->previewWidget->setInfo(convert, Qt::green);
    else
        d->previewWidget->setInfo(convert, Qt::darkGreen);

    d->convertBlink = !d->convertBlink;
    d->blinkConvertTimer->start(200);*/
}

void RawImportDlg::customEvent(QCustomEvent *event)
{
/*    if (!event) return;

    EventData *d = (EventData*) event->data();
    if (!d) return;

    QString text;

    if (d->starting)            // Something have been started...
    {
        switch (d->action) 
        {
            case(IDENTIFY_FULL): 
                break;
            case(PREVIEW):
            {
                busy(true);
                previewing(d->filePath);
                break;
            }
            case(PROCESS):
            {
                busy(true);
                processing(d->filePath);
                break;
            }
            default: 
            {
                kdWarning( 51000 ) << "KIPIRawConverterPlugin: Unknown event" << endl;
                break;
            }
        }
    }
    else
    {
        if (!d->success)        // Something is failed...
        {
            switch (d->action) 
            {
                case(IDENTIFY_FULL): 
                    break;
                case(PREVIEW):
                {
                    previewFailed(d->filePath);
                    busy(false);
                    break;
                }
                case(PROCESS):
                {
                    processingFailed(d->filePath);
                    busy(false);
                    break;
                }
                default: 
                {
                    kdWarning( 51000 ) << "KIPIRawConverterPlugin: Unknown event" << endl;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (d->action)
            {
                case(IDENTIFY_FULL): 
                {
                    QPixmap pix = QPixmap(d->image.scale(256, 256, QImage::ScaleMin));
                    identified(d->filePath, d->message, pix);
                    busy(false);
                    break;
                }
                case(PREVIEW):
                {
                    previewed(d->filePath, d->destPath);
                    busy(false);
                    break;
                }
                case(PROCESS):
                {
                    processed(d->filePath, d->destPath);
                    busy(false);
                    break;
                }
                default: 
                {
                    kdWarning( 51000 ) << "KIPIRawConverterPlugin: Unknown event" << endl;
                    break;
                }
            }
        }
    }

    delete d;*/
}

} // NameSpace Digikam
