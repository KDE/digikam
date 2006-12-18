/* ============================================================
 * File  : imageeffect_blowup.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-04-07
 * Description : a digiKam image editor plugin to blowup
 *               a photograph
 *
 * Copyright 2005 by Gilles Caulier
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

#include <cstdio>
#include <cmath>
#include <cstring>

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
#include <qpainter.h>
#include <qbrush.h>
#include <qfile.h>
#include <qimage.h>

// KDE includes.

#include <kcursor.h>
#include <kurllabel.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kglobalsettings.h>
#include <kpassivepopup.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "cimgiface.h"
#include "bannerwidget.h"
#include "imageeffect_blowup.h"

namespace DigikamBlowUpImagesPlugin
{

ImageEffect_BlowUp::ImageEffect_BlowUp(QWidget* parent)
                  : KDialogBase(Plain, i18n("Blowup Photograph"),
                                Help|Default|User2|User3|Ok|Cancel, Ok,
                                parent, 0, true, true,
                                QString::null,
                                i18n("&Save As..."),
                                i18n("&Load...")),
                    m_parent(parent)
{
    QString whatsThis;
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );

    m_currentRenderingMode = NoneRendering;
    m_cimgInterface        = 0L;

    // About data and help button.

    m_about = new KAboutData("digikamimageplugins",
                             I18N_NOOP("Blowup Photograph"),
                             digikamimageplugins_version,
                             I18N_NOOP("A digiKam image plugin to blowup a photograph."),
                             KAboutData::License_GPL,
                             "(c) 2005, Gilles Caulier",
                             0,
                             "http://extragear.kde.org/apps/digikamimageplugins");

    m_about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                       "caulier dot gilles at kdemail dot net");

    m_about->addAuthor("David Tschumperle", I18N_NOOP("CImg library"), 0,
                       "http://cimg.sourceforge.net");

    m_about->addAuthor("Gerhard Kulzer", I18N_NOOP("Feedback and plugin polishing"),
                       "gerhard at kulzer.net");

    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, m_about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );

    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(plainPage(), i18n("Blowup Photograph"));
    topLayout->addWidget(headerFrame);

    // -------------------------------------------------------------

    QVBoxLayout *vlay = new QVBoxLayout(topLayout);
    m_mainTab = new QTabWidget( plainPage() );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 3, 2, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("New Size") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString::null);
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("cimg-logo", KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    QString directory = KGlobal::dirs()->findResourceDir("cimg-logo", "cimg-logo.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "cimg-logo.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_newWidth = new KIntNumInput(firstPage);
    m_newWidth->setValue(1024);
    QWhatsThis::add( m_newWidth, i18n("<p>Set here the new imager width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_newHeight = new KIntNumInput(firstPage);
    m_newHeight->setValue(768);
    QWhatsThis::add( m_newHeight, i18n("<p>Set here the new image height in pixels."));

    m_preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    QWhatsThis::add( m_preserveRatioBox, i18n("<p>Enable this option to maintain aspect ratio with new image sizes."));

    grid->addMultiCellWidget(cimgLogoLabel, 0, 2, 0, 0);
    grid->addMultiCellWidget(m_preserveRatioBox, 0, 0, 2, 2);
    grid->addMultiCellWidget(label1, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_newWidth, 1, 1, 2, 2);
    grid->addMultiCellWidget(label2, 2, 2, 1, 1);
    grid->addMultiCellWidget(m_newHeight, 2, 2, 2, 2);

    m_progressBar = new KProgress(100, firstPage);
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    grid->addMultiCellWidget(m_progressBar, 3, 3, 0, 2);

    // -------------------------------------------------------------

    QWidget* secondPage = new QWidget( m_mainTab );
    QGridLayout* grid2 = new QGridLayout( secondPage, 2, 4, marginHint(), spacingHint());
    m_mainTab->addTab( secondPage, i18n("Smoothing") );

    m_detailLabel = new QLabel(i18n("Detail preservation:"), secondPage);
    m_detailLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_detailInput = new KDoubleNumInput(secondPage);
    m_detailInput->setPrecision(2);
    m_detailInput->setRange(0.0, 100.0, 0.01, true);
    QWhatsThis::add( m_detailInput, i18n("<p>Preservation of details to set the sharpening level "
                                         "of the small features in the target image. "
                                         "Higher values leave details sharp."));
    grid2->addMultiCellWidget(m_detailLabel, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_detailInput, 0, 0, 1, 1);

    m_gradientLabel = new QLabel(i18n("Anisotropy:"), secondPage);
    m_gradientLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_gradientInput = new KDoubleNumInput(secondPage);
    m_gradientInput->setPrecision(2);
    m_gradientInput->setRange(0.0, 100.0, 0.01, true);
    QWhatsThis::add( m_gradientInput, i18n("<p>Anisotropic (directional) modifier of the details. Keep it small for Gaussian noise."));
    grid2->addMultiCellWidget(m_gradientLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_gradientInput, 1, 1, 1, 1);

    m_timeStepLabel = new QLabel(i18n("Smoothing:"), secondPage);
    m_timeStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_timeStepInput = new KDoubleNumInput(secondPage);
    m_timeStepInput->setPrecision(2);
    m_timeStepInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_timeStepInput, i18n("<p>Total smoothing power: if Detail Factor sets the relative smoothing and Gradient Factor the "
                                           "direction, Time Step sets the overall effect."));
    grid2->addMultiCellWidget(m_timeStepLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_timeStepInput, 2, 2, 1, 1);

    m_blurLabel = new QLabel(i18n("Regularity:"), secondPage);
    m_blurLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_blurInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(2);
    m_blurInput->setRange(0.0, 10.0, 0.01, true);
    QWhatsThis::add( m_blurInput, i18n("<p>This value controls the smoothing regularity of the target image. "
                                       "Do not use an high value here, else the "
                                       "target image will be completely blurred."));
    grid2->addMultiCellWidget(m_blurLabel, 0, 0, 3, 3);
    grid2->addMultiCellWidget(m_blurInput, 0, 0, 4, 4);

    m_blurItLabel = new QLabel(i18n("Iterations:"), secondPage);
    m_blurItLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_blurItInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(1);
    m_blurItInput->setRange(1.0, 100.0, 1.0, true);
    QWhatsThis::add( m_blurItInput, i18n("<p>Sets the number of times the filter is applied on the target image."));
    grid2->addMultiCellWidget(m_blurItLabel, 1, 1, 3, 3);
    grid2->addMultiCellWidget(m_blurItInput, 1, 1, 4, 4);

    // -------------------------------------------------------------

    QWidget* thirdPage = new QWidget( m_mainTab );
    QGridLayout* grid3 = new QGridLayout( thirdPage, 2, 3, marginHint(), spacingHint());
    m_mainTab->addTab( thirdPage, i18n("Advanced Settings") );

    m_angularStepLabel = new QLabel(i18n("Angular step:"), thirdPage);
    m_angularStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_angularStepInput = new KDoubleNumInput(thirdPage);
    m_angularStepInput->setPrecision(2);
    m_angularStepInput->setRange(5.0, 90.0, 0.01, true);
    QWhatsThis::add( m_angularStepInput, i18n("<p>Set here the angular integration step in degrees in analogy to anisotropy."));
    grid3->addMultiCellWidget(m_angularStepLabel, 0, 0, 0, 0);
    grid3->addMultiCellWidget(m_angularStepInput, 0, 0, 1, 1);

    m_integralStepLabel = new QLabel(i18n("Integral step:"), thirdPage);
    m_integralStepLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_integralStepInput = new KDoubleNumInput(thirdPage);
    m_integralStepInput->setPrecision(2);
    m_integralStepInput->setRange(0.1, 10.0, 0.01, true);
    QWhatsThis::add( m_integralStepInput, i18n("<p>Set here the spatial integral step. Stay below 1."));
    grid3->addMultiCellWidget(m_integralStepLabel, 1, 1, 0, 0);
    grid3->addMultiCellWidget(m_integralStepInput, 1, 1, 1, 1);

    m_gaussianLabel = new QLabel(i18n("Gaussian:"), thirdPage);
    m_gaussianLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_gaussianInput = new KDoubleNumInput(thirdPage);
    m_gaussianInput->setPrecision(2);
    m_gaussianInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_gaussianInput, i18n("<p>Set here the precision of the Gaussian function."));
    grid3->addMultiCellWidget(m_gaussianLabel, 2, 2, 0, 0);
    grid3->addMultiCellWidget(m_gaussianInput, 2, 2, 1, 1);

    m_linearInterpolationBox = new QCheckBox(i18n("Use linear interpolation"), thirdPage);
    QWhatsThis::add( m_linearInterpolationBox, i18n("<p>Enable this option to quench the last bit of quality (slow)."));
    grid3->addMultiCellWidget(m_linearInterpolationBox, 0, 0, 3, 3);

    m_normalizeBox = new QCheckBox(i18n("Normalize photograph"), thirdPage);
    QWhatsThis::add( m_normalizeBox, i18n("<p>Enable this option to process an output image normalization."));
    grid3->addMultiCellWidget(m_normalizeBox, 1, 1, 3, 3);

    vlay->addWidget(m_mainTab);

    // -------------------------------------------------------------

    adjustSize();
    disableResize();
    QTimer::singleShot(0, this, SLOT(slotDefault())); // Reset all parameters to the default values.

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_newWidth, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustRatioFromWidth(int)));

    connect(m_newHeight, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustRatioFromHeight(int)));

    // details must be < gradient !
    connect(m_detailInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotCheckSettings()));

    connect(m_gradientInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotCheckSettings()));
}

ImageEffect_BlowUp::~ImageEffect_BlowUp()
{
    delete m_about;
    
    if (m_cimgInterface)
       delete m_cimgInterface;
}

void ImageEffect_BlowUp::slotCheckSettings(void)
{
    m_gradientInput->setMinValue(m_detailInput->value());
    m_detailInput->setMaxValue(m_gradientInput->value());
}

void ImageEffect_BlowUp::slotDefault()
{
    Digikam::ImageIface iface(0, 0);
    m_aspectRatio = (double)iface.originalWidth() / (double)iface.originalHeight();

    m_detailInput->setValue(0.1);
    m_gradientInput->setValue(5.0);
    m_timeStepInput->setValue(15.0);
    m_blurInput->setValue(2.0);
    m_blurItInput->setValue(1.0);
    m_angularStepInput->setValue(45.0);
    m_integralStepInput->setValue(0.8);
    m_gaussianInput->setValue(3.0);
    m_linearInterpolationBox->setChecked(true);
    m_normalizeBox->setChecked(false);

    m_preserveRatioBox->setChecked(true);
    m_newWidth->blockSignals(true);
    m_newHeight->blockSignals(true);
    m_newWidth->setValue(iface.originalWidth());
    m_newHeight->setValue(iface.originalHeight());
    m_newWidth->blockSignals(false);
    m_newHeight->blockSignals(false);
}

void ImageEffect_BlowUp::slotAdjustRatioFromWidth(int w)
{
    if ( m_preserveRatioBox->isChecked() )
    {
       m_newHeight->blockSignals(true);
       m_newHeight->setValue( (int) (w / m_aspectRatio) );
       m_newHeight->blockSignals(false);
    }
}

void ImageEffect_BlowUp::slotAdjustRatioFromHeight(int h)
{
    if ( m_preserveRatioBox->isChecked() )
    {
       m_newWidth->blockSignals(true);
       m_newWidth->setValue( (int)(m_aspectRatio * h) );
       m_newWidth->blockSignals(false);
    }
}

void ImageEffect_BlowUp::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_cimgInterface->stopComputation();
       m_parent->unsetCursor();
    }

    done(Cancel);
}

void ImageEffect_BlowUp::slotHelp()
{
    KApplication::kApplication()->invokeHelp("blowup", "digikamimageplugins");
}

void ImageEffect_BlowUp::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_BlowUp::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_cimgInterface->stopComputation();
       m_parent->unsetCursor();
    }

    e->accept();
}

void ImageEffect_BlowUp::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    m_detailInput->setEnabled(false);
    m_gradientInput->setEnabled(false);
    m_timeStepInput->setEnabled(false);
    m_blurInput->setEnabled(false);
    m_blurItInput->setEnabled(false);
    m_angularStepInput->setEnabled(false);
    m_integralStepInput->setEnabled(false);
    m_gaussianInput->setEnabled(false);
    m_linearInterpolationBox->setEnabled(false);
    m_normalizeBox->setEnabled(false);
    m_newWidth->setEnabled(false);
    m_newHeight->setEnabled(false);
    m_preserveRatioBox->setEnabled(false);
    enableButton(Ok, false);
    enableButton(Default, false);
    enableButton(User2, false);
    enableButton(User3, false);
    m_mainTab->setCurrentPage(0);

    m_parent->setCursor( KCursor::waitCursor() );
    m_progressBar->setValue(0);

    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    Digikam::DImg originalImage = Digikam::DImg(iface.originalWidth(), iface.originalHeight(),
                                  iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    if (m_cimgInterface)
       delete m_cimgInterface;

    m_cimgInterface = new DigikamImagePlugins::CimgIface(&originalImage,
                                    (uint)m_blurItInput->value(),
                                    m_timeStepInput->value(),
                                    m_integralStepInput->value(),
                                    m_angularStepInput->value(),
                                    m_blurInput->value(),
                                    m_detailInput->value(),
                                    m_gradientInput->value(),
                                    m_gaussianInput->value(),
                                    m_normalizeBox->isChecked(),
                                    m_linearInterpolationBox->isChecked(),
                                    false, false, true, NULL,
                                    m_newWidth->value(),
                                    m_newHeight->value(), 0, this);
}

void ImageEffect_BlowUp::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DigikamImagePlugins::CimgIface::EventData *d = (DigikamImagePlugins::CimgIface::EventData*) event->data();

    if (!d) return;

    if (d->starting)           // Computation in progress !
    {
        m_progressBar->setValue(d->progress);
    }
    else
    {
        if (d->success)        // Computation Completed !
        {
            switch (m_currentRenderingMode)
            {
                case FinalRendering:
                {
                    DDebug() << "Final BlowUp completed..." << endl;
                    
                    Digikam::ImageIface iface(0, 0);
                    Digikam::DImg resizedImage = m_cimgInterface->getTargetImage();

                    iface.putOriginalImage(i18n("BlowUp"), resizedImage.bits(),
                                           resizedImage.width(), resizedImage.height());
                    m_parent->unsetCursor();
                    accept();
                    break;
                }
            }
        }
        else                   // Computation Failed !
        {
            switch (m_currentRenderingMode)
            {
                case FinalRendering:
                    break;
            }
        }
    }

    delete d;
}

void ImageEffect_BlowUp::slotUser3()
{
    KURL loadInpaintingFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Blowup Settings File to Load")) );
    if( loadInpaintingFile.isEmpty() )
       return;

    QFile file(loadInpaintingFile.path());

    if ( file.open(IO_ReadOnly) )
        {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Blowup Configuration File" )
           {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a Photograph Blowup settings text file.")
                        .arg(loadInpaintingFile.fileName()));
           file.close();
           return;
           }

        blockSignals(true);
        m_normalizeBox->setChecked( stream.readLine().toInt() );
        m_linearInterpolationBox->setChecked( stream.readLine().toInt() );

        m_detailInput->setValue( stream.readLine().toDouble() );
        m_gradientInput->setValue( stream.readLine().toDouble() );
        m_timeStepInput->setValue( stream.readLine().toDouble() );
        m_blurInput->setValue( stream.readLine().toDouble() );
        m_blurItInput->setValue( stream.readLine().toDouble() );
        m_angularStepInput->setValue( stream.readLine().toDouble() );
        m_integralStepInput->setValue( stream.readLine().toDouble() );
        m_gaussianInput->setValue( stream.readLine().toDouble() );
        blockSignals(false);
        }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Blowup text file."));

    file.close();
}

void ImageEffect_BlowUp::slotUser2()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Blowup Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(IO_WriteOnly) )
        {
        QTextStream stream( &file );
        stream << "# Photograph Blowup Configuration File\n";
        stream << m_normalizeBox->isChecked() << "\n";
        stream << m_linearInterpolationBox->isChecked() << "\n";
        stream << m_detailInput->value() << "\n";
        stream << m_gradientInput->value() << "\n";
        stream << m_timeStepInput->value() << "\n";
        stream << m_blurInput->value() << "\n";
        stream << m_blurItInput->value() << "\n";
        stream << m_angularStepInput->value() << "\n";
        stream << m_integralStepInput->value() << "\n";
        stream << m_gaussianInput->value() << "\n";
        }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Blowup text file."));

    file.close();
}

}  // NameSpace DigikamBlowUpImagesPlugin

#include "imageeffect_blowup.moc"
