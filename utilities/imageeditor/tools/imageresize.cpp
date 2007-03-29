/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-07
 * Description : a tool to resize a picture
 *
 * Copyright 2005-2007 by Gilles Caulier 
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

// C++ include.

#include <cmath>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qfile.h>
#include <qimage.h>

// KDE includes.

#include <kseparator.h>
#include <kcursor.h>
#include <kurllabel.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kglobalsettings.h>

// Digikam includes.

#include "dimg.h"
#include "ddebug.h"
#include "imageiface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationiface.h"
#include "greycstorationwidget.h"
#include "greycstorationsettings.h"

// Local includes.

#include "imageresize.h"
#include "imageresize.moc"

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
    }

    int                   currentRenderingMode;
    int                   orgWidth;    
    int                   orgHeight;   
    int                   prevW; 
    int                   prevH; 

    double                prevWP;    
    double                prevHP;    

    QWidget              *parent;
    
    QCheckBox            *preserveRatioBox;
    QCheckBox            *useGreycstorationBox;
    
    QTabWidget           *mainTab;

    KIntNumInput         *wInput;
    KIntNumInput         *hInput;

    KDoubleNumInput      *wpInput;
    KDoubleNumInput      *hpInput;
    
    KProgress            *progressBar;
    
    GreycstorationIface  *greycstorationIface;
    GreycstorationWidget *settingsWidget;
};

ImageResize::ImageResize(QWidget* parent)
           : KDialogBase(Plain, i18n("Resize Image"),
                         Help|Default|User2|User3|Ok|Cancel, Ok,
                         parent, 0, true, false,
                         QString(),
                         i18n("&Save As..."),
                         i18n("&Load..."))             
{
    d = new ImageResizePriv;
    d->parent = parent;
    setHelp("resizetool.anchor", "digikam");
    QString whatsThis;
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    enableButton(Ok, false);

    ImageIface iface(0, 0);
    d->orgWidth    = iface.originalWidth();
    d->orgHeight   = iface.originalHeight();
    d->prevW       = d->orgWidth;
    d->prevH       = d->orgHeight;
    d->prevWP      = 100.0;
    d->prevHP      = 100.0;

    // -------------------------------------------------------------

    QVBoxLayout *vlay  = new QVBoxLayout(plainPage(), 0, spacingHint());
    d->mainTab          = new QTabWidget( plainPage() );

    QWidget* firstPage = new QWidget( d->mainTab );
    QGridLayout* grid  = new QGridLayout( firstPage, 8, 2, spacingHint());
    d->mainTab->addTab( firstPage, i18n("New Size") );

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    d->wInput = new KIntNumInput(firstPage);
    d->wInput->setRange(1, QMAX(d->orgWidth * 10, 9999), 1, true);
    d->wInput->setName("d->wInput");
    QWhatsThis::add( d->wInput, i18n("<p>Set here the new image width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    d->hInput = new KIntNumInput(firstPage);
    d->hInput->setRange(1, QMAX(d->orgHeight * 10, 9999), 1, true);
    d->hInput->setName("d->hInput");
    QWhatsThis::add( d->hInput, i18n("<p>Set here the new image height in pixels."));

    QLabel *label3 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput = new KDoubleNumInput(firstPage);
    d->wpInput->setRange(1.0, 999.0, 1.0, true);
    d->wpInput->setName("d->wpInput");
    QWhatsThis::add( d->wpInput, i18n("<p>Set here the new image width in percents."));

    QLabel *label4 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput = new KDoubleNumInput(firstPage);
    d->hpInput->setRange(1.0, 999.0, 1.0, true);
    d->hpInput->setName("d->hpInput");
    QWhatsThis::add( d->hpInput, i18n("<p>Set here the new image height in percents."));

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    QWhatsThis::add( d->preserveRatioBox, i18n("<p>Enable this option to maintain aspect "
                                              "ratio with new image sizes."));

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("logo-cimg", KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-cimg", "logo-cimg.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "logo-cimg.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    QWhatsThis::add( d->useGreycstorationBox, i18n("<p>Enable this option to restore photograph content. "
                                                  "Warning: this process can take a while."));

    d->progressBar = new KProgress(100, firstPage);
    d->progressBar->setValue(0);
    QWhatsThis::add(d->progressBar, i18n("<p>This is the current progress when you use Restoration mode."));

    grid->addMultiCellWidget(d->preserveRatioBox, 0, 0, 0, 2);
    grid->addMultiCellWidget(label1, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->wInput, 1, 1, 1, 2);
    grid->addMultiCellWidget(label2, 2, 2, 0, 0);
    grid->addMultiCellWidget(d->hInput, 2, 2, 1, 2);
    grid->addMultiCellWidget(label3, 3, 3, 0, 0);
    grid->addMultiCellWidget(d->wpInput, 3, 3, 1, 2);
    grid->addMultiCellWidget(label4, 4, 4, 0, 0);
    grid->addMultiCellWidget(d->hpInput, 4, 4, 1, 2);
    grid->addMultiCellWidget(new KSeparator(firstPage), 5, 5, 0, 2);
    grid->addMultiCellWidget(cimgLogoLabel, 6, 7, 0, 0);
    grid->addMultiCellWidget(d->useGreycstorationBox, 6, 6, 1, 2);
    grid->addMultiCellWidget(d->progressBar, 7, 7, 1, 2);
    grid->setRowStretch(8, 10);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationWidget(d->mainTab);
    vlay->addWidget(d->mainTab);

    // -------------------------------------------------------------

    adjustSize();
    disableResize();
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

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
}

ImageResize::~ImageResize()
{
    if (d->greycstorationIface)
       delete d->greycstorationIface;

    delete d;
}

void ImageResize::slotRestorationToggled(bool b)
{
    d->settingsWidget->setEnabled(b);
    d->progressBar->setEnabled(b);
    enableButton(User2, b);
    enableButton(User3, b);
}    

void ImageResize::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("resize Tool Dialog");

    GreycstorationSettings settings;
    settings.fastApprox = config->readBoolEntry("FastApprox", true);
    settings.interp     = config->readNumEntry("Interpolation",
                          GreycstorationSettings::NearestNeighbor);
    settings.amplitude  = config->readDoubleNumEntry("Amplitude", 20.0);
    settings.sharpness  = config->readDoubleNumEntry("Sharpness", 0.2);
    settings.anisotropy = config->readDoubleNumEntry("Anisotropy", 0.9);
    settings.alpha      = config->readDoubleNumEntry("Alpha", 0.1);
    settings.sigma      = config->readDoubleNumEntry("Sigma", 1.5);
    settings.gaussPrec  = config->readDoubleNumEntry("GaussPrec", 2.0);
    settings.dl         = config->readDoubleNumEntry("Dl", 0.8);
    settings.da         = config->readDoubleNumEntry("Da", 30.0);
    settings.nbIter     = config->readNumEntry("Iteration", 3);
    settings.tile       = config->readNumEntry("Tile", 512);
    settings.btile      = config->readNumEntry("BTile", 4);
    d->settingsWidget->setSettings(settings);
    d->useGreycstorationBox->setChecked(config->readBoolEntry("RestorePhotograph", false));
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    d->preserveRatioBox->blockSignals(true);
    d->wInput->blockSignals(true);
    d->hInput->blockSignals(true);
    d->wpInput->blockSignals(true);
    d->hpInput->blockSignals(true);
    d->preserveRatioBox->setChecked(true);
    d->wInput->setValue(d->orgWidth);
    d->hInput->setValue(d->orgHeight);
    d->wpInput->setValue(100);
    d->hpInput->setValue(100);
    d->preserveRatioBox->blockSignals(false);
    d->wInput->blockSignals(false);
    d->hInput->blockSignals(false);
    d->wpInput->blockSignals(false);
    d->hpInput->blockSignals(false);
}

void ImageResize::writeUserSettings()
{
    GreycstorationSettings settings = d->settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("resize Tool Dialog");
    config->writeEntry("FastApprox", settings.fastApprox);
    config->writeEntry("Interpolation", settings.interp);
    config->writeEntry("Amplitude", settings.amplitude);
    config->writeEntry("Sharpness", settings.sharpness);
    config->writeEntry("Anisotropy", settings.anisotropy);
    config->writeEntry("Alpha", settings.alpha);
    config->writeEntry("Sigma", settings.sigma);
    config->writeEntry("GaussPrec", settings.gaussPrec);
    config->writeEntry("Dl", settings.dl);
    config->writeEntry("Da", settings.da);
    config->writeEntry("Iteration", settings.nbIter);
    config->writeEntry("Tile", settings.tile);
    config->writeEntry("BTile", settings.btile);
    config->writeEntry("RestorePhotograph", d->useGreycstorationBox->isChecked());
    config->sync();
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
    d->wInput->setValue(d->orgWidth);
    d->hInput->setValue(d->orgHeight);
    d->wpInput->setValue(100.0);
    d->hpInput->setValue(100.0);
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
    
    QString s(sender()->name());
    
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
        d->greycstorationIface->stopComputation();
        d->parent->unsetCursor();
    }

    done(Cancel);
}

void ImageResize::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageResize::closeEvent(QCloseEvent *e)
{
    if (d->currentRenderingMode != ImageResizePriv::NoneRendering)
    {
        d->greycstorationIface->stopComputation();
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
    d->mainTab->setCurrentPage(0);
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

    d->parent->setCursor( KCursor::waitCursor() );
    if (d->useGreycstorationBox->isChecked())
    {
        d->progressBar->setValue(0);
        d->progressBar->setEnabled(true);
    }
    writeUserSettings();

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage = DImg(iface.originalWidth(), iface.originalHeight(),
                              iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    if (d->greycstorationIface)
    {
        delete d->greycstorationIface;
        d->greycstorationIface = 0;
    }

    int mode = d->useGreycstorationBox->isChecked() ? GreycstorationIface::Resize
                                                   : GreycstorationIface::SimpleResize;

    d->greycstorationIface = new GreycstorationIface(
                          &originalImage, d->settingsWidget->getSettings(),
                          mode, 
                          d->wInput->value(),
                          d->hInput->value(),
                          0, this);
}

void ImageResize::customEvent(QCustomEvent *event)
{
    if (!event) return;

    GreycstorationIface::EventData *data = (GreycstorationIface::EventData*) event->data();

    if (!data) return;

    if (data->starting)           // Computation in progress !
    {
        d->progressBar->setValue(data->progress);
    }
    else
    {
        if (data->success)        // Computation Completed !
        {
            switch (d->currentRenderingMode)
            {
                case ImageResizePriv::FinalRendering:
                {
                    DDebug() << "Final resizing completed..." << endl;
                    
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

    delete data;
}

void ImageResize::slotUser3()
{
    KURL loadBlowupFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Load")) );
    if( loadBlowupFile.isEmpty() )
       return;

    QFile file(loadBlowupFile.path());

    if ( file.open(IO_ReadOnly) )   
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Resizing Configuration File")))
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Resizing settings text file.")
                        .arg(loadBlowupFile.fileName()));
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
    KURL saveBlowupFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Save")) );
    if( saveBlowupFile.isEmpty() )
       return;

    QFile file(saveBlowupFile.path());

    if ( file.open(IO_WriteOnly) )   
        d->settingsWidget->saveSettings(file, QString("# Photograph Resizing Configuration File"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Resizing text file."));

    file.close();
}

}  // NameSpace Digikam

