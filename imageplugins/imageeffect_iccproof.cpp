/* ============================================================
 * Author: F.J. Cruz <fj.cruz@supercable.es>
 * Date  : 2005-12-21
 * Copyright 2005-2006 by F.J. Cruz
 * Description : digiKam image editor to correct an image using
 *               an ICC color profile
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

// Qt includes.
 
#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qradiobutton.h>

// KDE includes.

#include <knuminput.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ktabwidget.h>
#include <kconfig.h>
#include <kurlrequester.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kmessagebox.h>

// Digikam includes.

#include "imageiface.h"
#include "imagewidget.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "dimg.h"
#include "iccpreviewwidget.h"
#include "icctransform.h"
#include "iccprofileinfodlg.h"

// Local includes.

#include "imageeffect_iccproof.h"

namespace DigikamImagesPluginCore
{

ImageEffect_ICCProof::ImageEffect_ICCProof(QWidget* parent)
                    : Digikam::ImageDlgBase(parent,i18n("Color Management"), 
                                            "colormanagement", false, true)
{
    m_destinationPreviewData = 0L;

    cmEnabled   = true;
    hasICC      = false;
    inPath      = QString::null;
    spacePath   = QString::null;
    proofPath   = QString::null;
    displayPath = QString::null;

    setHelp("colormanagement.anchor", "digikam");

    m_previewWidget = new Digikam::ImageWidget(plainPage(),
                                               i18n("<p>Here you can see the image preview after "
                                                    "convert it with a color profile</p>"));
    setPreviewAreaWidget(m_previewWidget); 
    
    // -------------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout *gridSettings = new QGridLayout( gboxSettings, 6, 2, marginHint(), spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel: "), gboxSettings);
    label1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_channelCB = new QComboBox(false, gboxSettings);
    m_channelCB->insertItem(i18n("Luminosity"));
    m_channelCB->insertItem(i18n("Red"));
    m_channelCB->insertItem(i18n("Green"));
    m_channelCB->insertItem(i18n("Blue"));
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    m_scaleBG = new QHButtonGroup(gboxSettings);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    m_scaleBG->insert(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);
    
    QPushButton *logHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    m_scaleBG->insert(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);       

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(m_channelCB);
    l1->addWidget(m_scaleBG);
    l1->addStretch(10);
    
    gridSettings->addMultiCellLayout(l1, 0, 0, 0, 2);

    // -------------------------------------------------------------

    m_histogramWidget = new Digikam::HistogramWidget(256, 140, gboxSettings, false, true, true);
    QWhatsThis::add( m_histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing of the "
                                             "selected image channel. This one is re-computed at any "
                                             "settings changes."));
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    
    gridSettings->addMultiCellWidget(m_histogramWidget, 1, 1, 0, 2);
    gridSettings->addMultiCellWidget(m_hGradient, 2, 2, 0, 2);

    //-- Build rendering intents options group -----------------------

    QVButtonGroup *intentsBG = new QVButtonGroup(gboxSettings);
    intentsBG->setTitle(i18n("Select Rendering Intent"));

    m_renderingIntentsCB = new QComboBox(false, intentsBG);
    
    m_renderingIntentsCB->insertItem("Perceptual");
    m_renderingIntentsCB->insertItem("Absolute Colorimetric");
    m_renderingIntentsCB->insertItem("Relative Colorimetric");
    m_renderingIntentsCB->insertItem("Saturation");

    QWhatsThis::add( m_renderingIntentsCB, i18n("<ul><li>Perceptual intent causes the full gamut of the image to be compressed or expanded to fill the gamut of the destination device, so that gray balance is preserved but colorimetric accuracy may not be preserved.\n"
    "In other words, if certain colors in an image fall outside of the range of colors that the output device can render, the picture intent will cause all the colors in the image to be adjusted so that the every color in the image falls within the range that can be rendered and so that the relationship between colors is preserved as much as possible.\n"
    "This intent is most suitable for display of photographs and images, and is the default intent.</li>"
    "<li> Absolute Colorimetric intent causes any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged.\n"
    "This intent preserves the white point and is most suitable for spot colors (Pantone, TruMatch, logo colors, ...).</li>"
    "<li>Relative Colorimetric intent is defined such that any colors that fall outside the range that the output device can render are adjusted to the closest color that can be rendered, while all other colors are left unchanged. Proof intent does not preserve the white point.</li>"
    "<li>Saturarion intent preserves the saturation of colors in the image at the possible expense of hue and lightness.\n"
    "Implementation of this intent remains somewhat problematic, and the ICC is still working on methods to achieve the desired effects.\n"
    "This intent is most suitable for business graphics such as charts, where it is more important that the colors be vivid and contrast well with each other rather than a specific color.</li></ul>"));

    gridSettings->addMultiCellWidget(intentsBG, 3, 3, 0, 2);

    // -------------------------------------------------------------

    m_tabsWidgets = new KTabWidget(gboxSettings);
    QWidget *generalOptions = new QWidget(m_tabsWidgets);
    QWidget *inProfiles = new QWidget(m_tabsWidgets);
    QWidget *proofProfiles = new QWidget(m_tabsWidgets);
    QWidget *displayProfiles = new QWidget(m_tabsWidgets);
    QWidget *spaceProfiles = new QWidget(m_tabsWidgets);

    m_tabsWidgets->addTab(generalOptions, i18n("General"));
    QWhatsThis::add(generalOptions, i18n("<p>You can set here general parameters.</p>"));

    //---------- Zero Page Setup ----------------------------------

    QVBoxLayout *zeroPageLayout = new QVBoxLayout(generalOptions, 0, KDialog::spacingHint());

    QButtonGroup *m_optionsBG = new QButtonGroup(4, Qt::Vertical, generalOptions);
    m_optionsBG->setFrameStyle(QFrame::NoFrame);

    m_doSoftProofBox = new QCheckBox(m_optionsBG);
    m_doSoftProofBox->setText(i18n("Soft-proofing"));
    QWhatsThis::add(m_doSoftProofBox, i18n("<p>The obtained transform emulates the device described"
                                           " by the \"Proofing\" profile. Useful to preview final"
                                           " result whithout rendering to physical medium.</p>"));

    m_checkGamutBox = new QCheckBox(m_optionsBG);
    m_checkGamutBox->setText(i18n("Check gamut"));
    QWhatsThis::add(m_checkGamutBox, i18n("<p>You can use this option if you want to show"
                                          " the colors that are out of the printer gamut<p>"));

    QCheckBox *m_embeddProfileBox = new QCheckBox(m_optionsBG);
    m_embeddProfileBox->setText(i18n("Embed profile"));
    QWhatsThis::add(m_embeddProfileBox, i18n("<p>You can use this option if you want to embedd"
                                             " into the image the selected color profile.</p>"));

    m_BPCBox = new QCheckBox(m_optionsBG);
    m_BPCBox->setText(i18n("Use BPC"));
    QWhatsThis::add(m_BPCBox, i18n("<p>The Black Point Compensation (BPC) feature does work in conjunction "
                                   "with relative colorimetric intent. Perceptual intent should make no "
                                   "difference, although it affects some profiles.</p>"
                                   "<p>The mechanics are simple. BPC does scale full image across gray "
                                   "axis in order to accommodate the darkest tone origin media can "
                                   "render to darkest tone destination media can render. As a such, "
                                   "BPC is primarily targeting CMYK.</p>"));

    zeroPageLayout->addWidget(m_optionsBG);
    zeroPageLayout->addStretch();

    //---------- End Zero Page -----------------------------------

    m_tabsWidgets->addTab(inProfiles, i18n("Input"));
    QWhatsThis::add(inProfiles, i18n("<p>Set here all parameters relevant of Input Color Profiles.</p>"));

    //---------- First Page Setup ----------------------------------

    QVBoxLayout *firstPageLayout = new QVBoxLayout(inProfiles, 0, KDialog::spacingHint());

    QButtonGroup *inProfileBG = new QButtonGroup(6, Qt::Vertical, inProfiles);
    inProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useEmbeddedProfile = new QRadioButton(inProfileBG);
    m_useEmbeddedProfile->setText(i18n("Use embedded profile"));

    m_useSRGBDefaultProfile = new QRadioButton(inProfileBG);
    m_useSRGBDefaultProfile->setText(i18n("Use builtin sRGB profile"));
    m_useSRGBDefaultProfile->setChecked(true);

    m_useInDefaultProfile = new QRadioButton(inProfileBG);
    m_useInDefaultProfile->setText(i18n("Use default profile"));

    m_useInSelectedProfile = new QRadioButton(inProfileBG);
    m_useInSelectedProfile->setText(i18n("Use selected profile"));
    
    m_inProfilesCB = new KURLRequester(inProfileBG);
    m_inProfilesCB->setMode(KFile::File|KFile::ExistingOnly);
    m_inProfilesCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *inProfiles_dialog = m_inProfilesCB->fileDialog();
    m_iccInPreviewWidget = new Digikam::ICCPreviewWidget(inProfiles_dialog);
    inProfiles_dialog->setPreviewWidget(m_iccInPreviewWidget);
    
    QPushButton *inProfilesInfo = new QPushButton(i18n("Info..."), inProfileBG);

    firstPageLayout->addWidget(inProfileBG);
    firstPageLayout->addStretch();

    //---------- End First Page ------------------------------------

    m_tabsWidgets->addTab(spaceProfiles, i18n("Workspace"));
    QWhatsThis::add(spaceProfiles, i18n("<p>Set here all parameters relevant of Workspace Color Profiles.</p>"));

    //---------- Second Page Setup ---------------------------------

    QVBoxLayout *secondPageLayout = new QVBoxLayout(spaceProfiles, 0, KDialog::spacingHint());

    QButtonGroup *spaceProfileBG = new QButtonGroup(4, Qt::Vertical, spaceProfiles);
    spaceProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useSpaceDefaultProfile = new QRadioButton(spaceProfileBG);
    m_useSpaceDefaultProfile->setText(i18n("Use default workspace profile"));

    m_useSpaceSelectedProfile = new QRadioButton(spaceProfileBG);
    m_useSpaceSelectedProfile->setText(i18n("Use selected profile"));
    
    m_spaceProfileCB = new KURLRequester(spaceProfileBG);
    m_spaceProfileCB->setMode(KFile::File|KFile::ExistingOnly);
    m_spaceProfileCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *spaceProfiles_dialog = m_spaceProfileCB->fileDialog();
    m_iccSpacePreviewWidget = new Digikam::ICCPreviewWidget(spaceProfiles_dialog);
    spaceProfiles_dialog->setPreviewWidget(m_iccSpacePreviewWidget);

    QPushButton *spaceProfilesInfo = new QPushButton(i18n("Info..."), spaceProfileBG);

    secondPageLayout->addWidget(spaceProfileBG);
    secondPageLayout->addStretch();

    //---------- End Second Page -----------------------------------

    m_tabsWidgets->addTab(proofProfiles, i18n("Proofing"));
    QWhatsThis::add(proofProfiles, i18n("<p>Set here all parameters relevant of Proofing Color Profiles.</p>"));

    //---------- Third Page Setup ---------------------------------

    QVBoxLayout *thirdPageLayout = new QVBoxLayout(proofProfiles, 0, KDialog::spacingHint());

    QButtonGroup *proofProfileBG = new QButtonGroup(4, Qt::Vertical, proofProfiles);
    proofProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useProofDefaultProfile = new QRadioButton(proofProfileBG);
    m_useProofDefaultProfile->setText(i18n("Use default proof profile"));

    m_useProofSelectedProfile = new QRadioButton(proofProfileBG);
    m_useProofSelectedProfile->setText(i18n("Use selected profile"));
    
    m_proofProfileCB = new KURLRequester(proofProfileBG);
    m_proofProfileCB->setMode(KFile::File|KFile::ExistingOnly);
    m_proofProfileCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *proofProfiles_dialog = m_proofProfileCB->fileDialog();
    m_iccProofPreviewWidget = new Digikam::ICCPreviewWidget(proofProfiles_dialog);
    proofProfiles_dialog->setPreviewWidget(m_iccProofPreviewWidget);

    QPushButton *proofProfilesInfo = new QPushButton(i18n("Info..."), proofProfileBG);

    thirdPageLayout->addWidget(proofProfileBG);
    thirdPageLayout->addStretch();

    //---------- End Third Page -----------------------------------

    m_tabsWidgets->addTab(displayProfiles, i18n("Display"));
    QWhatsThis::add(displayProfiles, i18n("<p>Set here all parameters relevant of Display Color Profiles.</p>"));

    //---------- Fourth Page Setup ----------------------------------

    QVBoxLayout *fourthPageLayout = new QVBoxLayout(displayProfiles, 0, KDialog::spacingHint());

    QButtonGroup *displayProfileBG = new QButtonGroup(4, Qt::Vertical, displayProfiles);
    displayProfileBG->setFrameStyle(QFrame::NoFrame);

    m_useDisplayDefaultProfile = new QRadioButton(displayProfileBG);
    m_useDisplayDefaultProfile->setText(i18n("Use default display profile"));

    m_useDisplaySelectedProfile = new QRadioButton(displayProfileBG);
    m_useDisplaySelectedProfile->setText(i18n("Use selected profile"));

    m_displayProfileCB = new KURLRequester(displayProfileBG);
    m_displayProfileCB->setMode(KFile::File|KFile::ExistingOnly);
    m_displayProfileCB->setFilter("*.icc *.icm|"+i18n("ICC Files (*.icc; *.icm)"));
    KFileDialog *displayProfiles_dialog = m_displayProfileCB->fileDialog();
    m_iccDisplayPreviewWidget = new Digikam::ICCPreviewWidget(displayProfiles_dialog);
    displayProfiles_dialog->setPreviewWidget(m_iccDisplayPreviewWidget);

    QPushButton *displayProfilesInfo = new QPushButton(i18n("Info..."), displayProfileBG);

    fourthPageLayout->addWidget(displayProfileBG);
    fourthPageLayout->addStretch();
    
    //---------- End Fourth Page ------------------------------------

    gridSettings->addMultiCellWidget(m_tabsWidgets, 4, 4, 0, 2);
    gridSettings->setRowStretch(6, 10);    
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));    
            
    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(inProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotInICCInfo()));

    connect(spaceProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotSpaceICCInfo()));

    connect(proofProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotProofICCInfo()));

    connect(displayProfilesInfo, SIGNAL(clicked()),
            this, SLOT(slotDisplayICCInfo()));

    // -------------------------------------------------------------

    enableButtonOK( false );
    readSettings();
}

ImageEffect_ICCProof::~ImageEffect_ICCProof()
{
    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;
       
    delete m_histogramWidget;
    delete m_previewWidget;
}

void ImageEffect_ICCProof::slotColorSelectedFromTarget( const Digikam::DColor &color )
{
    m_histogramWidget->setHistogramGuideByColor(color);
}

void ImageEffect_ICCProof::slotChannelChanged( int channel )
{
    switch(channel)
    {
        case LuminosityChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            break;
    
        case RedChannel:
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            break;
    
        case GreenChannel:         
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            break;
    
        case BlueChannel:         
            m_histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
            m_hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            break;
    }

    m_histogramWidget->repaint(false);
}

void ImageEffect_ICCProof::slotScaleChanged( int scale )
{
    m_histogramWidget->m_scaleType = scale;
    m_histogramWidget->repaint(false);
}

void ImageEffect_ICCProof::slotDefault()
{
     kdDebug() << "Doing default" << endl;
    slotEffect();
}

void ImageEffect_ICCProof::slotEffect()
{
    kapp->setOverrideCursor( KCursor::waitCursor() );


    m_histogramWidget->stopHistogramComputation();

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    iface                      = m_previewWidget->imageIface();
    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);
    if (iface->getEmbeddedICCFromOriginalImage().isNull())
    {
        m_useEmbeddedProfile->setEnabled(false);
    }
    else
    {
        hasICC = true;
        m_embeddedICC = iface->getEmbeddedICCFromOriginalImage();
    }

    iface->putPreviewImage(preview.bits());

    m_previewWidget->updatePreview();

    // Update histogram.
   
    memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
    m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();    
}

void ImageEffect_ICCProof::finalRendering()
{
    /// @todo implement me
    if (!m_doSoftProofBox->isChecked())
    {
        kapp->setOverrideCursor( KCursor::waitCursor() );
        Digikam::ImageIface* iface = m_previewWidget->imageIface();
        
        uchar *data                = iface->getOriginalImage();
        int w                      = iface->originalWidth();
        int h                      = iface->originalHeight();
        bool a                     = iface->originalHasAlpha();
        bool sb                    = iface->originalSixteenBit();

        if (data)
        {
            Digikam::IccTransform transform;
            
            Digikam::DImg img(w, h, sb, a, data);

            QString tmpInPath;
            QString tmpProofPath;
            QString tmpSpacePath;
            bool proofCondition;
            
            ///////////////////////////////////////
            // Get parameters for transformation //
            ///////////////////////////////////////
            
            //---------Input profile ------------------
            
            if (useDefaultInProfile())
            {
                tmpInPath = inPath;
            }
            else if (useSelectedInProfile())
            {
                tmpInPath = m_inProfilesCB->url();
            }

            if (tmpInPath.isNull() && !m_useEmbeddedProfile->isChecked() && !m_useSRGBDefaultProfile->isChecked())
            {
                KMessageBox::information(this, "Profile error");
                kdDebug() << "here" << endl;
                return;
            }
                
            //--------Proof profile ------------------
        
            if (useDefaultProofProfile())
            {
                tmpProofPath = proofPath;
            }
            else
            {
                tmpProofPath = m_proofProfileCB->url();
            }

            if (tmpProofPath.isNull())
                proofCondition = false;
        
            //-------Workspace profile--------------
        
            if (useDefaultSpaceProfile())
            {
                tmpSpacePath = spacePath;
            }
            else
            {
                tmpSpacePath = m_spaceProfileCB->url();
            }
        
            //------------------------------------------
        
            transform.getTransformType(m_doSoftProofBox->isChecked());
        
            if (m_doSoftProofBox->isChecked())
            {
                if (m_useEmbeddedProfile->isChecked())
                {
                    transform.setProfiles( tmpSpacePath, tmpProofPath, true );
                }
                else
                {
                    transform.setProfiles( tmpInPath, tmpSpacePath, tmpProofPath);
                }
            }
            else
            {
                if (m_useEmbeddedProfile->isChecked())
                {
                    transform.setProfiles( tmpSpacePath );
                }
                else
                {
                    transform.setProfiles( tmpInPath, tmpSpacePath );
                }
            }
            
            if (m_useEmbeddedProfile->isChecked())
            {
                transform.apply(img, m_embeddedICC, m_renderingIntentsCB->currentItem(), useBPC(), m_checkGamutBox->isChecked(), useBuiltinProfile());
            }
            else
            {
                QByteArray fakeProfile = QByteArray();
                transform.apply(img, fakeProfile, m_renderingIntentsCB->currentItem(), useBPC(), m_checkGamutBox->isChecked(), useBuiltinProfile());
            }
            
            iface->putOriginalImage("Color Management", img.bits());
            delete [] data;
        }
        kapp->restoreOverrideCursor();
    }
    accept();
}

void ImageEffect_ICCProof::slotTry()
{
    /// @todo "embed profile" option is not implemented -- Paco Cruz
    /// @todo use of Display profile is not implemented -- Paco Cruz
    
    kapp->setOverrideCursor(KCursor::waitCursor());

    enableButtonOK(true);

    m_histogramWidget->stopHistogramComputation();

    Digikam::IccTransform transform;

    if (m_destinationPreviewData) 
       delete [] m_destinationPreviewData;

    m_destinationPreviewData   = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();
    bool a                     = iface->previewHasAlpha();
    bool sb                    = iface->previewSixteenBit();

    Digikam::DImg preview(w, h, sb, a, m_destinationPreviewData);

    QString tmpInPath = QString::null;
    QString tmpProofPath = QString::null;
    QString tmpSpacePath = QString::null;

    bool proofCondition = false;
    bool spaceCondition = false;
    
    ///////////////////////////////////////
    // Get parameters for transformation //
    ///////////////////////////////////////
    
    //---------Input profile ------------------
    
    if (useDefaultInProfile())
    {
        tmpInPath = inPath;
    }
    else if (useSelectedInProfile())
    {
        tmpInPath = m_inProfilesCB->url();
    }

    //--------Proof profile ------------------

    if (useDefaultProofProfile())
    {
        tmpProofPath = proofPath;
    }
    else
    {
        tmpProofPath = m_proofProfileCB->url();
    }

    if (m_doSoftProofBox->isChecked())
        proofCondition = tmpProofPath.isEmpty();

    //-------Workspace profile--------------

    if (useDefaultSpaceProfile())
    {
        tmpSpacePath = spacePath;
    }
    else
    {
        tmpSpacePath = m_spaceProfileCB->url();
    }

    spaceCondition = tmpSpacePath.isEmpty();

    //------------------------------------------

    transform.getTransformType(m_doSoftProofBox->isChecked());

    if (m_doSoftProofBox->isChecked())
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.setProfiles( tmpSpacePath, tmpProofPath, true );
        }
        else
        {
            transform.setProfiles( tmpInPath, tmpSpacePath, tmpProofPath);
        }
    }
    else
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.setProfiles( tmpSpacePath );
        }
        else
        {
            transform.setProfiles( tmpInPath, tmpSpacePath );
        }
    }
    if ( proofCondition || spaceCondition )
    {
        kapp->restoreOverrideCursor();
        QString error = i18n("<p>Your settings are not correct</p>\
                        <p>To apply a color transform, you need at least two ICC profiles:</p> \
                        <ul><li>An \"Input\" profile.</li>\
                        <li>A \"Workspace\" profile.</li></ul>\
                        <p>If you want to do a \"soft-proof\" transform, in adition to these profiles\
                        you need a \"Proof\" one.</p>");
        KMessageBox::information(this, error);
        enableButtonOK(false);
    }
    else
    {
        if (m_useEmbeddedProfile->isChecked())
        {
            transform.apply(preview, m_embeddedICC, m_renderingIntentsCB->currentItem(), useBPC(), m_checkGamutBox->isChecked(), useBuiltinProfile());
        }
        else
        {
            QByteArray fakeProfile = QByteArray();
            transform.apply(preview, fakeProfile, m_renderingIntentsCB->currentItem(), useBPC(), m_checkGamutBox->isChecked(), useBuiltinProfile());
        }
        
        iface->putPreviewImage(preview.bits());
    
        m_previewWidget->updatePreview();
    
        // Update histogram.
    
        memcpy(m_destinationPreviewData, preview.bits(), preview.numBytes());
        kdDebug() << "Doing updateData" << endl;
        m_histogramWidget->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);
        kapp->restoreOverrideCursor();
    }    
}

void ImageEffect_ICCProof::readSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Color Management");

    if (!config->readBoolEntry("EnableCM", false))
    {
        cmEnabled = false;
        slotToggledWidgets(false);
    }
    else
    {
        inPath = config->readPathEntry("InProfileFile");
        spacePath = config->readPathEntry("WorkProfileFile");
        displayPath = config->readPathEntry("MonitorProfileFile");
        proofPath = config->readPathEntry("ProofProfileFile");
    }
}

void ImageEffect_ICCProof::slotToggledWidgets( bool t)
{
    m_useInDefaultProfile->setEnabled(t);
    m_useProofDefaultProfile->setEnabled(t);
    m_useDisplayDefaultProfile->setEnabled(t);
    m_useSpaceDefaultProfile->setEnabled(t);
}

/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotInICCInfo()
 */
void ImageEffect_ICCProof::slotInICCInfo()
{
    if (useEmbeddedProfile())
    {
        getICCInfo(m_embeddedICC);
    }
    else if(useBuiltinProfile())
    {
        QString message = QString(i18n("<p>You have selected the \"Default builtin sRGB profile\"</p>"));
        message.append(QString(i18n("<p>This profile is built on the fly, so there is not relevant information about it.</p>")));
        KMessageBox::information(this, message);
    }
    else if (useDefaultInProfile())
    {
        getICCInfo(inPath);
    }
    else if (useSelectedInProfile())
    {
        getICCInfo(m_inProfilesCB->url());
    }
}

/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotProofICCInfo()
 */
void ImageEffect_ICCProof::slotProofICCInfo()
{
    if (useDefaultProofProfile())
    {
        getICCInfo(proofPath);
    }
    else
    {
        getICCInfo(m_proofProfileCB->url());
    }
}

/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotSpaceICCInfo()
 */
void ImageEffect_ICCProof::slotSpaceICCInfo()
{
    if (useDefaultSpaceProfile())
    {
        getICCInfo(spacePath);
    }
    else
    {
        getICCInfo(m_spaceProfileCB->url());
    }
}

/*!
    \fn DigikamImagesPluginCore::ImageEffect_ICCProof::slotDisplayICCInfo()
 */
void ImageEffect_ICCProof::slotDisplayICCInfo()
{
    if (useDefaultDisplayProfile())
    {
        getICCInfo(displayPath);
    }
    else
    {
        getICCInfo(m_displayProfileCB->url());
    }
}

void ImageEffect_ICCProof::getICCInfo(const QString& profile)
{
    if (profile.isEmpty())
    {
        KMessageBox::error(this, i18n("Sorry, there is not any selected profile"), i18n("Profile Error"));
        return;
    }
    
    Digikam::ICCProfileInfoDlg infoDlg(this, profile);
    infoDlg.exec();
}

void ImageEffect_ICCProof::getICCInfo(QByteArray& profile)
{
    if (profile.isNull())
    {
        KMessageBox::error(this, i18n("Sorry, it seems there is no embedded profile"), i18n("Profile Error"));
        return;
    }
    QString intent;
    cmsHPROFILE selectedProfile;
    selectedProfile = cmsOpenProfileFromMem(profile.data(), (DWORD)profile.size());

    QString  profileName = QString((cmsTakeProductName(selectedProfile)));
    QString profileDescription = QString((cmsTakeProductDesc(selectedProfile)));
    QString profileManufacturer = QString(cmsTakeCopyright(selectedProfile));
    int profileIntent = cmsTakeRenderingIntent(selectedProfile);
    
    //"Decode" profile rendering intent
    switch (profileIntent)
    {
        case 0:
            intent = i18n("Perceptual");
            break;
        case 1:
            intent = i18n("Relative Colorimetric");
            break;
        case 2:
            intent = i18n("Saturation");
            break;
        case 3:
            intent = i18n("Absolute Colorimetric");
            break;
    }

    QString message = QString(i18n("<p><b>Name:</b> "));
    message.append(profileName);
    message.append(QString(i18n("</p><p><b>Description:</b>  ")));
    message.append(profileDescription);
    message.append(QString(i18n("</p><p><b>Copyright:</b>  ")));
    message.append(profileManufacturer);
    message.append(QString(i18n("</p><p><b>Rendering Intent:</b>  ")));
    message.append(intent);
    message.append(QString(i18n("</p><p><b>Path:</b> Embedded profile</p>")));
    KMessageBox::information(this, message);
}

void ImageEffect_ICCProof::slotCMDisabledWarning()
{
    if (!cmEnabled)
    {
        QString message = QString(i18n("<p>You don't have enabled Color Management in Digikam preferences.</p>"));
        message.append( i18n("<p>\"Use default profile\" options will be disabled now.</p>"));
        KMessageBox::information(this, message);
        slotToggledWidgets(false);
    }
}

//-- General Tab ---------------------------

bool ImageEffect_ICCProof::useBPC()
{
    return m_BPCBox->isChecked();
}

bool ImageEffect_ICCProof::doProof()
{
    return m_doSoftProofBox->isChecked();
}

bool ImageEffect_ICCProof::checkGamut()
{
    return m_checkGamutBox->isChecked();
}

bool ImageEffect_ICCProof::embedProfile()
{
    return m_embeddProfileBox->isChecked();
}

//-- Input Tab ---------------------------

bool ImageEffect_ICCProof::useEmbeddedProfile()
{
    return m_useEmbeddedProfile->isChecked();
}

bool ImageEffect_ICCProof::useBuiltinProfile()
{
    return m_useSRGBDefaultProfile->isChecked();
}

bool ImageEffect_ICCProof::useDefaultInProfile()
{
    return m_useInDefaultProfile->isChecked();
}

bool ImageEffect_ICCProof::useSelectedInProfile()
{
    return m_useInSelectedProfile->isChecked();
}

//-- Workspace Tab ---------------------------

bool ImageEffect_ICCProof::useDefaultSpaceProfile()
{
    return m_useSpaceDefaultProfile->isChecked();
}

//-- Proofing Tab ---------------------------

bool ImageEffect_ICCProof::useDefaultProofProfile()
{
    return m_useProofDefaultProfile->isChecked();
}

//-- Display Tab ---------------------------

bool ImageEffect_ICCProof::useDefaultDisplayProfile()
{
    return m_useDisplayDefaultProfile->isChecked();
}

}// NameSpace DigikamImagesPluginCore

#include "imageeffect_iccproof.moc"
