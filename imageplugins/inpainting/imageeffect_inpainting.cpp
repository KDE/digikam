/* ============================================================
 * File  : imageeffect_inpainting.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
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
#include <kdebug.h>
#include <kpassivepopup.h>

// Local includes.

#include "version.h"
#include "cimgiface.h"
#include "bannerwidget.h"
#include "imageeffect_inpainting.h"

namespace DigikamInPaintingImagesPlugin
{

class InPaintingPassivePopup : public KPassivePopup
{
public:

    InPaintingPassivePopup(QWidget* parent) : KPassivePopup(parent), m_parent(parent) {}

protected:

    virtual void positionSelf() { move(m_parent->x() + 30, m_parent->y() + 30); }

private:

    QWidget* m_parent;
};

//------------------------------------------------------------------------------------------

void ImageEffect_InPainting::inPainting(QWidget* parent)
{
    // -- check if we actually have a selection --------------------

    Digikam::ImageIface iface(0, 0);

    int w = iface.selectedWidth();
    int h = iface.selectedHeight();

    if (!w || !h)
    {
        InPaintingPassivePopup* popup = new InPaintingPassivePopup(parent);
        popup->setView(i18n("Inpainting Photograph Tool"),
                       i18n("You need to select a region to inpaint to use "
                            "this tool"));
        popup->setAutoDelete(true);
        popup->setTimeout(2500);
        popup->show();
        return;
    }

    // -- run the dlg ----------------------------------------------

    ImageEffect_InPainting_Dialog dlg(parent);
    dlg.exec();
}

//------------------------------------------------------------------------------------------

ImageEffect_InPainting_Dialog::ImageEffect_InPainting_Dialog(QWidget* parent)
                             : KDialogBase(Plain, i18n("Photograph Inpainting"),
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
                             I18N_NOOP("Photograph Inpainting"),
                             digikamimageplugins_version,
                             I18N_NOOP("A digiKam image plugin to inpaint a photograph."),
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

    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(plainPage(), i18n("Photograph Inpainting"));
    topLayout->addWidget(headerFrame);

    // -------------------------------------------------------------

    QVBoxLayout *vlay = new QVBoxLayout(topLayout);
    m_mainTab = new QTabWidget( plainPage() );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 2, 1, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString::null);
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("cimg-logo", KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    QString directory = KGlobal::dirs()->findResourceDir("cimg-logo", "cimg-logo.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "cimg-logo.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    QLabel *typeLabel = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_inpaintingTypeCB = new QComboBox( false, firstPage );
    m_inpaintingTypeCB->insertItem( i18n("None") );
    m_inpaintingTypeCB->insertItem( i18n("Remove Small Artefact") );
    m_inpaintingTypeCB->insertItem( i18n("Remove Medium Artefact") );
    m_inpaintingTypeCB->insertItem( i18n("Remove Large Artefact") );
    QWhatsThis::add( m_inpaintingTypeCB, i18n("<p>Select here the filter preset to use for photograph restoration:<p>"
                                               "<b>None</b>: Most common values. Puts settings to default.<p>"
                                               "<b>Remove Small Artefact</b>: inpaint small image artefact like image glitch.<p>"
                                               "<b>Remove Medium Artefact</b>: inpaint medium image artefact.<p>"
                                               "<b>Remove Large Artefact</b>: inpaint image artefact like unwanted object.<p>"));

    grid->addMultiCellWidget(cimgLogoLabel, 0, 0, 0, 0);
    grid->addMultiCellWidget(typeLabel, 0, 0, 1, 1);
    grid->addMultiCellWidget(m_inpaintingTypeCB, 0, 0, 2, 2);

    m_progressBar = new KProgress(100, firstPage);
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    grid->addMultiCellWidget(m_progressBar, 1, 1, 0, 2);

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
    // Reset all parameters to the default values.
    QTimer::singleShot(0, this, SLOT(slotDefault()));

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_inpaintingTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotDefault()));

    // details must be < gradient !
    connect(m_detailInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotCheckSettings()));

    connect(m_gradientInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotCheckSettings()));
}

ImageEffect_InPainting_Dialog::~ImageEffect_InPainting_Dialog()
{
    delete m_about;

    if (m_cimgInterface)
       delete m_cimgInterface;
}

void ImageEffect_InPainting_Dialog::slotCheckSettings(void)
{
    m_gradientInput->setMinValue(m_detailInput->value());
    m_detailInput->setMaxValue(m_gradientInput->value());
}

void ImageEffect_InPainting_Dialog::slotDefault()
{
    m_detailInput->blockSignals(true);
    m_gradientInput->blockSignals(true);
    m_timeStepInput->blockSignals(true);
    m_blurInput->blockSignals(true);
    m_blurItInput->blockSignals(true);
    m_angularStepInput->blockSignals(true);
    m_integralStepInput->blockSignals(true);
    m_gaussianInput->blockSignals(true);
    m_linearInterpolationBox->blockSignals(true);
    m_normalizeBox->blockSignals(true);

    m_detailInput->setValue(0.1);
    m_gradientInput->setValue(100.0);
    m_timeStepInput->setValue(10.0);
    m_blurInput->setValue(2.0);
    m_blurItInput->setValue(20.0);
    m_angularStepInput->setValue(45.0);
    m_integralStepInput->setValue(0.8);
    m_gaussianInput->setValue(3.0);
    m_linearInterpolationBox->setChecked(true);
    m_normalizeBox->setChecked(false);

    switch(m_inpaintingTypeCB->currentItem())
    {
        case RemoveSmallArtefact:
        {
            m_timeStepInput->setValue(20.0);
            m_blurItInput->setValue(30.0);
            break;
        }

        case RemoveMediumArtefact:
        {
            m_timeStepInput->setValue(50.0);
            m_blurItInput->setValue(50.0);
            break;
        }

        case RemoveLargeArtefact:
        {
            m_timeStepInput->setValue(100.0);
            m_blurItInput->setValue(100.0);
            break;
        }
    }

    m_detailInput->blockSignals(false);
    m_gradientInput->blockSignals(false);
    m_timeStepInput->blockSignals(false);
    m_blurInput->blockSignals(false);
    m_blurItInput->blockSignals(false);
    m_angularStepInput->blockSignals(false);
    m_integralStepInput->blockSignals(false);
    m_gaussianInput->blockSignals(false);
    m_linearInterpolationBox->blockSignals(false);
    m_normalizeBox->blockSignals(false);
}

void ImageEffect_InPainting_Dialog::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_cimgInterface->stopComputation();
       m_parent->unsetCursor();
    }

    done(Cancel);
}

void ImageEffect_InPainting_Dialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp("inpainting", "digikamimageplugins");
}

void ImageEffect_InPainting_Dialog::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_InPainting_Dialog::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_cimgInterface->stopComputation();
       m_parent->unsetCursor();
    }

    e->accept();
}

void ImageEffect_InPainting_Dialog::slotOk()
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
    enableButton(Ok, false);
    enableButton(Default, false);
    enableButton(User2, false);
    enableButton(User3, false);
    m_mainTab->setCurrentPage(0);

    m_parent->setCursor( KCursor::waitCursor() );
    m_progressBar->setValue(0);

    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    m_originalImage = Digikam::DImg(iface.originalWidth(), iface.originalHeight(),
                                    iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    // Selected area from the image and mask creation:
    //
    // We optimize the computation time to use the current selected area in image editor
    // and to create an inpainting mask with it. Because inpainting is done by interpolation
    // neighboor pixels which can be located far from the selected area, we need to ajust the
    // mask size in according with the parameter algorithms, especially 'dt' (m_timeStepInput).
    // Mask size is computed like this :
    //
    // (mask_radius_x + 2*dt , mask_radius_y + 2*dt)
    //
    // Where mask_radius_x is the 'width' of the mask, and mask_radius_y is the 'height' of the mask.


    QRect selectionRect = QRect(iface.selectedXOrg(), iface.selectedYOrg(),
                                iface.selectedWidth(), iface.selectedHeight());

    QPixmap inPaintingMask(iface.originalWidth(), iface.originalHeight());
    inPaintingMask.fill(Qt::black);
    QPainter p(&inPaintingMask);
    p.fillRect( selectionRect, QBrush(Qt::white) );
    p.end();

    int x1 = (int)(selectionRect.left()   - 2*m_timeStepInput->value());
    int y1 = (int)(selectionRect.top()    - 2*m_timeStepInput->value());
    int x2 = (int)(selectionRect.right()  + 2*m_timeStepInput->value());
    int y2 = (int)(selectionRect.bottom() + 2*m_timeStepInput->value());
    m_maskRect = QRect(x1, y1, x2-x1, y2-y1);

    // Mask area normalization.
    // We need to check if mask area is out of image size else inpainting give strange results.

    if (m_maskRect.left()   < 0) m_maskRect.setLeft(0);
    if (m_maskRect.top()    < 0) m_maskRect.setTop(0);
    if (m_maskRect.right()  > iface.originalWidth())  m_maskRect.setRight(iface.originalWidth());
    if (m_maskRect.bottom() > iface.originalHeight()) m_maskRect.setBottom(iface.originalHeight());

    m_maskImage = inPaintingMask.convertToImage().copy(m_maskRect);
    m_cropImage = m_originalImage.copy(m_maskRect);

    if (m_cimgInterface)
       delete m_cimgInterface;

    m_cimgInterface = new DigikamImagePlugins::CimgIface(&m_cropImage,
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
                                    false, true, false, NULL, 0, 0,
                                    &m_maskImage, this);
}

void ImageEffect_InPainting_Dialog::customEvent(QCustomEvent *event)
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
                    kdDebug() << "Final InPainting completed..." << endl;
                    Digikam::ImageIface iface(0, 0);
                    Digikam::DImg target = m_cimgInterface->getTargetImage();
                    m_originalImage.bitBltImage(&target, m_maskRect.left(), m_maskRect.top());
    
                    iface.putOriginalImage(i18n("InPainting"), m_originalImage.bits());
    
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

void ImageEffect_InPainting_Dialog::slotUser3()
{
    KURL loadInpaintingFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Inpainting Settings File to Load")) );
    if( loadInpaintingFile.isEmpty() )
       return;

    QFile file(loadInpaintingFile.path());

    if ( file.open(IO_ReadOnly) )
    {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Inpainting Configuration File" )
        {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a Photograph Inpainting settings text file.")
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
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Inpainting text file."));

    file.close();
}

void ImageEffect_InPainting_Dialog::slotUser2()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Inpainting Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(IO_WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Photograph Inpainting Configuration File\n";
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
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Inpainting text file."));

    file.close();
}

}  // NameSpace DigikamInPaintingImagesPlugin

#include "imageeffect_inpainting.moc"
