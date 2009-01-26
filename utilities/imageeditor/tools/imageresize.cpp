/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageresize.h"
#include "imageresize.moc"

// C++ includes.

#include <cmath>

// Qt includes.

#include <QBrush>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCustomEvent>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

// KDE includes.

#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "dimg.h"
#include "imageiface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationiface.h"
#include "greycstorationwidget.h"
#include "greycstorationsettings.h"

using namespace KDcrawIface;

namespace Digikam
{

class ImageResizePriv
{
public:

    enum RunningMode
    {
        NoneRendering=0,
        FinalRendering
    };

    ImageResizePriv()
    {
        currentRenderingMode = NoneRendering;
        parent               = 0;
        preserveRatioBox     = 0;
        useGreycstorationBox = 0;
        mainTab              = 0;
        wInput               = 0;
        hInput               = 0;
        wpInput              = 0;
        hpInput              = 0;
        progressBar          = 0;
        greycstorationIface  = 0;
        settingsWidget       = 0;
        cimgLogoLabel        = 0;
        restorationTips      = 0;
    }

    int                   currentRenderingMode;
    int                   orgWidth;
    int                   orgHeight;
    int                   prevW;
    int                   prevH;

    double                prevWP;
    double                prevHP;

    QWidget              *parent;

    QLabel               *restorationTips;

    QCheckBox            *preserveRatioBox;
    QCheckBox            *useGreycstorationBox;

    KTabWidget           *mainTab;

    QProgressBar         *progressBar;

    KUrlLabel            *cimgLogoLabel;

    RIntNumInput         *wInput;
    RIntNumInput         *hInput;

    RDoubleNumInput      *wpInput;
    RDoubleNumInput      *hpInput;

    GreycstorationIface  *greycstorationIface;
    GreycstorationWidget *settingsWidget;
};

ImageResize::ImageResize(QWidget* parent)
           : KDialog(parent), d(new ImageResizePriv)
{
    d->parent = parent;

    setDefaultButton(Ok);
    setButtons(Help|Default|User2|User3|Ok|Cancel);
    setCaption(i18n("Resize Image"));
    setModal(true);
    setButtonText(User2,i18n("&Save As..."));
    setButtonText(User3,i18n("&Load..."));
    setHelp("resizetool.anchor", "digikam");
    setButtonWhatsThis ( Default, i18n("Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("Save all filter parameters to settings text file.") );
    enableButton(Ok, false);

    ImageIface iface(0, 0);
    d->orgWidth  = iface.originalWidth();
    d->orgHeight = iface.originalHeight();
    d->prevW     = d->orgWidth;
    d->prevH     = d->orgHeight;
    d->prevWP    = 100.0;
    d->prevHP    = 100.0;

    // -------------------------------------------------------------

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QVBoxLayout *vlay  = new QVBoxLayout();
    vlay->setSpacing(spacingHint());
    page->setLayout(vlay);

    d->mainTab         = new KTabWidget( page );
    QWidget* firstPage = new QWidget( d->mainTab );
    QGridLayout* grid  = new QGridLayout(firstPage);

    d->mainTab->addTab( firstPage, i18n("New Size") );

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    d->wInput      = new RIntNumInput(firstPage);
    d->wInput->setSliderEnabled(true);
    d->wInput->setRange(1, qMax(d->orgWidth * 10, 9999), 1);
    d->wInput->setDefaultValue(d->orgWidth);
    d->wInput->setObjectName("d->wInput");
    d->wInput->setWhatsThis( i18n("Set here the new image width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    d->hInput      = new RIntNumInput(firstPage);
    d->hInput->setSliderEnabled(true);
    d->hInput->setRange(1, qMax(d->orgHeight * 10, 9999), 1);
    d->hInput->setDefaultValue(d->orgHeight);
    d->hInput->setObjectName("d->hInput");
    d->hInput->setWhatsThis( i18n("New image height in pixels (px)."));

    QLabel *label3 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput     = new RDoubleNumInput(firstPage);
    d->wpInput->input()->setRange(1.0, 999.0, 1.0, true);
    d->wpInput->setDefaultValue(100.0);
    d->wpInput->setObjectName("d->wpInput");
    d->wpInput->setWhatsThis( i18n("New image width in percent (%)."));

    QLabel *label4 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput     = new RDoubleNumInput(firstPage);
    d->hpInput->input()->setRange(1.0, 999.0, 1.0, true);
    d->hpInput->setDefaultValue(100.0);
    d->hpInput->setObjectName("d->hpInput");
    d->hpInput->setWhatsThis( i18n("New image height in percent (%)."));

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    d->preserveRatioBox->setWhatsThis( i18n("Enable this option to maintain aspect "
                                            "ratio with new image sizes."));

    d->cimgLogoLabel = new KUrlLabel(firstPage);
    d->cimgLogoLabel->setText(QString());
    d->cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    d->cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    d->cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    d->useGreycstorationBox->setWhatsThis( i18n("Enable this option to scale-up an image to a huge size. "
                                                "<b>Warning</b>: This process can take some time."));

    d->restorationTips = new QLabel(i18n("<b>Note:</b> use Restoration Mode to scale-up an image to a huge size. "
                                         "This process can take some time."), firstPage);
    d->restorationTips->setWordWrap(true);

    d->progressBar = new QProgressBar(firstPage);
    d->progressBar->setValue(0);
    d->progressBar->setMaximum(100);
    d->progressBar->setWhatsThis( i18n("This shows the current progress when you use the Restoration mode."));

    grid->addWidget(d->preserveRatioBox,        0, 0, 1, 3);
    grid->addWidget(label1,                     1, 0, 1, 1);
    grid->addWidget(d->wInput,                  1, 1, 1, 2);
    grid->addWidget(label2,                     2, 0, 1, 1);
    grid->addWidget(d->hInput,                  2, 1, 1, 2);
    grid->addWidget(label3,                     3, 0, 1, 1);
    grid->addWidget(d->wpInput,                 3, 1, 1, 2);
    grid->addWidget(label4,                     4, 0, 1, 1);
    grid->addWidget(d->hpInput,                 4, 1, 1, 2);
    grid->addWidget(new KSeparator(firstPage),  5, 0, 1, 3);
    grid->addWidget(d->cimgLogoLabel,           6, 0, 3, 1);
    grid->addWidget(d->useGreycstorationBox,    6, 1, 1, 2);
    grid->addWidget(d->restorationTips,         7, 1, 1, 2);
    grid->addWidget(d->progressBar,             8, 1, 1, 2);
    grid->setRowStretch(8, 10);
    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationWidget(d->mainTab);
    vlay->addWidget(d->mainTab);

    // -------------------------------------------------------------

    adjustSize();
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    // -------------------------------------------------------------

    connect(d->cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->useGreycstorationBox, SIGNAL(toggled(bool)),
             this, SLOT(slotRestorationToggled(bool)) );

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancel()));

    connect(this, SIGNAL(defaultClicked()),
            this, SLOT(slotDefault()));

    connect(this, SIGNAL(user2Clicked()),
            this, SLOT(slotUser2()));

    connect(this, SIGNAL(user3Clicked()),
            this, SLOT(slotUser3()));

    connect(this, SIGNAL(helpClicked()),
            this, SLOT(slotHelp()));

    // -------------------------------------------------------------

    Digikam::GreycstorationSettings defaults;
    defaults.setResizeDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);
}

ImageResize::~ImageResize()
{
    if (d->greycstorationIface)
       delete d->greycstorationIface;

    delete d;
}

void ImageResize::slotHelp()
{
    KToolInvocation::invokeHelp("resize", "digikam");
}

void ImageResize::slotButtonClicked(int button)
{
    // KDialog calls QDialog::accept() for Ok.
    // We need to override this, we can only accept() when the thread has finished.

    if (button == Ok)
        slotOk();
    else
        KDialog::slotButtonClicked(button);
}

void ImageResize::slotRestorationToggled(bool b)
{
    d->settingsWidget->setEnabled(b);
    d->progressBar->setEnabled(b);
    d->cimgLogoLabel->setEnabled(b);
    enableButton(User2, b);
    enableButton(User3, b);
}

void ImageResize::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("resize Tool Dialog");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setResizeDefaultSettings();

    settings.fastApprox = group.readEntry("FastApprox",    defaults.fastApprox);
    settings.interp     = group.readEntry("Interpolation", defaults.interp);
    settings.amplitude  = group.readEntry("Amplitude",     (double)defaults.amplitude);
    settings.sharpness  = group.readEntry("Sharpness",     (double)defaults.sharpness);
    settings.anisotropy = group.readEntry("Anisotropy",    (double)defaults.anisotropy);
    settings.alpha      = group.readEntry("Alpha",         (double)defaults.alpha);
    settings.sigma      = group.readEntry("Sigma",         (double)defaults.sigma);
    settings.gaussPrec  = group.readEntry("GaussPrec",     (double)defaults.gaussPrec);
    settings.dl         = group.readEntry("Dl",            (double)defaults.dl);
    settings.da         = group.readEntry("Da",            (double)defaults.da);
    settings.nbIter     = group.readEntry("Iteration",     defaults.nbIter);
    settings.tile       = group.readEntry("Tile",          defaults.tile);
    settings.btile      = group.readEntry("BTile",         defaults.btile);
    d->settingsWidget->setSettings(settings);

    slotDefault();
}

void ImageResize::writeUserSettings()
{
    GreycstorationSettings settings = d->settingsWidget->getSettings();
    KConfigGroup group              = KGlobal::config()->group("resize Tool Dialog");

    group.writeEntry("FastApprox",        settings.fastApprox);
    group.writeEntry("Interpolation",     settings.interp);
    group.writeEntry("Amplitude",         (double)settings.amplitude);
    group.writeEntry("Sharpness",         (double)settings.sharpness);
    group.writeEntry("Anisotropy",        (double)settings.anisotropy);
    group.writeEntry("Alpha",             (double)settings.alpha);
    group.writeEntry("Sigma",             (double)settings.sigma);
    group.writeEntry("GaussPrec",         (double)settings.gaussPrec);
    group.writeEntry("Dl",                (double)settings.dl);
    group.writeEntry("Da",                (double)settings.da);
    group.writeEntry("Iteration",         settings.nbIter);
    group.writeEntry("Tile",              settings.tile);
    group.writeEntry("BTile",             settings.btile);
    group.writeEntry("RestorePhotograph", d->useGreycstorationBox->isChecked());
    group.sync();
}

void ImageResize::slotDefault()
{
    GreycstorationSettings settings;
    settings.setResizeDefaultSettings();

    d->settingsWidget->setSettings(settings);
    d->useGreycstorationBox->setChecked(false);
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    d->preserveRatioBox->blockSignals(true);
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);

    d->preserveRatioBox->setChecked(true);
    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();

    d->preserveRatioBox->blockSignals(false);
    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

void ImageResize::slotValuesChanged()
{
    enableButton(Ok, true);
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);

    QString s(sender()->objectName());

    if (s == "d->wInput")
    {
        double val = d->wInput->value();
        double wp  = val/(double)(d->orgWidth) * 100.0;
        d->wpInput->setValue(wp);

        if (d->preserveRatioBox->isChecked())
        {
            d->hpInput->setValue(wp);
            int h = (int)(wp*d->orgHeight/100);
            d->hInput->setValue(h);
        }
    }
    else if (s == "d->hInput")
    {
        double val = d->hInput->value();
        double hp  = val/(double)(d->orgHeight) * 100.0;
        d->hpInput->setValue(hp);

        if (d->preserveRatioBox->isChecked())
        {
            d->wpInput->setValue(hp);
            int w = (int)(hp*d->orgWidth/100);
            d->wInput->setValue(w);
        }
    }
    else if (s == "d->wpInput")
    {
        double val = d->wpInput->value();
        int w      = (int)(val*d->orgWidth/100);
        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            d->hpInput->setValue(val);
            int h = (int)(val*d->orgHeight/100);
            d->hInput->setValue(h);
        }
    }
    else if (s == "d->hpInput")
    {
        double val = d->hpInput->value();
        int h      = (int)(val*d->orgHeight/100);
        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            d->wpInput->setValue(val);
            int w = (int)(val*d->orgWidth/100);
            d->wInput->setValue(w);
        }
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

void ImageResize::slotCancel()
{
    if (d->currentRenderingMode != ImageResizePriv::NoneRendering)
    {
        d->greycstorationIface->cancelFilter();
        d->parent->unsetCursor();
    }

    done(Cancel);
}

void ImageResize::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void ImageResize::closeEvent(QCloseEvent *e)
{
    if (d->currentRenderingMode != ImageResizePriv::NoneRendering)
    {
        d->greycstorationIface->cancelFilter();
        d->parent->unsetCursor();
    }

    e->accept();
}

void ImageResize::slotOk()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
        slotValuesChanged();

    d->currentRenderingMode = ImageResizePriv::FinalRendering;
    d->mainTab->setCurrentIndex(0);
    d->settingsWidget->setEnabled(false);
    d->preserveRatioBox->setEnabled(false);
    d->useGreycstorationBox->setEnabled(false);
    d->wInput->setEnabled(false);
    d->hInput->setEnabled(false);
    d->wpInput->setEnabled(false);
    d->hpInput->setEnabled(false);
    enableButton(Ok, false);
    enableButton(Default, false);
    enableButton(User2, false);
    enableButton(User3, false);

    d->parent->setCursor( Qt::WaitCursor );
    writeUserSettings();
    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg image = DImg(iface.originalWidth(), iface.originalHeight(),
                      iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    if (d->useGreycstorationBox->isChecked())
    {
        d->progressBar->setValue(0);
        d->progressBar->setEnabled(true);

        if (d->greycstorationIface)
        {
            delete d->greycstorationIface;
            d->greycstorationIface = 0;
        }

        d->greycstorationIface = new GreycstorationIface(
                                     &image,
                                     d->settingsWidget->getSettings(),
                                     GreycstorationIface::Resize,
                                     d->wInput->value(),
                                     d->hInput->value(),
                                     QImage(),
                                     this);

        connect(d->greycstorationIface, SIGNAL(started()),
                this, SLOT(slotFilterStarted()));

        connect(d->greycstorationIface, SIGNAL(finished(bool)),
                this, SLOT(slotFilterFinished(bool)));

        connect(d->greycstorationIface, SIGNAL(progress(int)),
                this, SLOT(slotFilterProgress(int)));

        d->greycstorationIface->startFilter();
    }
    else
    {
        // See B.K.O #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.

        image.resize(d->wInput->value(), d->hInput->value());
        iface.putOriginalImage(i18n("Resize"), image.bits(),
                               image.width(), image.height());
        d->parent->unsetCursor();
        accept();
    }
}

void ImageResize::slotFilterStarted()
{
    d->progressBar->setValue(0);
}

void ImageResize::slotFilterFinished(bool success)
{
    if (success)        // Computation Completed !
    {
        switch (d->currentRenderingMode)
        {
            case ImageResizePriv::FinalRendering:
            {
                kDebug(50003) << "Final resizing completed..." << endl;

                ImageIface iface(0, 0);
                DImg resizedImage = d->greycstorationIface->getTargetImage();

                iface.putOriginalImage(i18n("Resize"), resizedImage.bits(),
                                       resizedImage.width(), resizedImage.height());
                d->parent->unsetCursor();
                accept();
                break;
            }
        }
    }
    else                   // Computation Failed !
    {
        switch (d->currentRenderingMode)
        {
            case ImageResizePriv::FinalRendering:
                break;
        }
    }
}

void ImageResize::slotFilterProgress(int progress)
{
    d->progressBar->setValue(progress);
}

void ImageResize::slotUser3()
{
    KUrl loadBlowupFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Load")) );
    if( loadBlowupFile.isEmpty() )
       return;

    QFile file(loadBlowupFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Resizing Configuration File")))
        {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a Photograph Resizing settings text file.",
                        loadBlowupFile.fileName()));
           file.close();
           return;
        }
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Resizing text file."));

    file.close();
}

void ImageResize::slotUser2()
{
    KUrl saveBlowupFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Save")) );
    if( saveBlowupFile.isEmpty() )
       return;

    QFile file(saveBlowupFile.path());

    if ( file.open(QIODevice::WriteOnly) )
        d->settingsWidget->saveSettings(file, QString("# Photograph Resizing Configuration File"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Resizing text file."));

    file.close();
}

}  // namespace Digikam
