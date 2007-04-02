/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
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
#include <kpassivepopup.h>

// Local includes.

#include "version.h"
#include "ddebug.h"
#include "imageiface.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "imageeffect_inpainting.h"
#include "imageeffect_inpainting.moc"

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
                             : Digikam::ImageGuideDlg(parent, i18n("Photograph Inpainting"), 
                                                      "inpainting", true, true, false, 
                                                      Digikam::ImageGuideWidget::HVGuideMode, 
                                                      0, true, true, true)
{
    m_isComputed = false;
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam",
                                       I18N_NOOP("Photograph Inpainting"),
                                       digikam_version,
                                       I18N_NOOP("A digiKam image plugin to inpaint a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005-2007, Gilles Caulier",
                                       0,
                                       "http://www.digikam.org");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor("David Tschumperle", I18N_NOOP("CImg library"), 0,
                     "http://cimg.sourceforge.net");

    about->addAuthor("Gerhard Kulzer", I18N_NOOP("Feedback and plugin polishing"),
                     "gerhard at kulzer.net");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(plainPage());
    QGridLayout* gridSettings = new QGridLayout(gboxSettings, 2, 1, spacingHint());
    m_mainTab = new QTabWidget( gboxSettings );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid  = new QGridLayout( firstPage, 2, 2, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("logo-cimg", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-cimg", "logo-cimg.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "logo-cimg.png" ) );
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

    grid->addMultiCellWidget(cimgLogoLabel, 0, 0, 1, 1);
    grid->addMultiCellWidget(typeLabel, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_inpaintingTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);

    // -------------------------------------------------------------

    m_settingsWidget = new Digikam::GreycstorationWidget(m_mainTab);

    gridSettings->addMultiCellWidget(m_mainTab, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(new QLabel(gboxSettings), 1, 1, 1, 1);
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_inpaintingTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));
}

ImageEffect_InPainting_Dialog::~ImageEffect_InPainting_Dialog()
{
}

void ImageEffect_InPainting_Dialog::renderingFinished()
{
    m_mainTab->setEnabled(true);
}

void ImageEffect_InPainting_Dialog::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("inpainting Tool Dialog");

    Digikam::GreycstorationSettings settings;
    settings.fastApprox = config->readBoolEntry("FastApprox", true);
    settings.interp     = config->readNumEntry("Interpolation",
                          Digikam::GreycstorationSettings::NearestNeighbor);
    settings.amplitude  = config->readDoubleNumEntry("Amplitude", 20.0);
    settings.sharpness  = config->readDoubleNumEntry("Sharpness", 0.3);
    settings.anisotropy = config->readDoubleNumEntry("Anisotropy", 1.0);
    settings.alpha      = config->readDoubleNumEntry("Alpha", 0.8);
    settings.sigma      = config->readDoubleNumEntry("Sigma", 2.0);
    settings.gaussPrec  = config->readDoubleNumEntry("GaussPrec", 2.0);
    settings.dl         = config->readDoubleNumEntry("Dl", 0.8);
    settings.da         = config->readDoubleNumEntry("Da", 30.0);
    settings.nbIter     = config->readNumEntry("Iteration", 30);
    settings.tile       = config->readNumEntry("Tile", 512);
    settings.btile      = config->readNumEntry("BTile", 4);
    m_settingsWidget->setSettings(settings);

    int p = config->readNumEntry("Preset", NoPreset);
    m_inpaintingTypeCB->setCurrentItem(p);
    if (p == NoPreset)
        m_settingsWidget->setEnabled(true);
    else        
        m_settingsWidget->setEnabled(false);
}

void ImageEffect_InPainting_Dialog::writeUserSettings()
{
    Digikam::GreycstorationSettings settings = m_settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("inpainting Tool Dialog");
    config->writeEntry("Preset", m_inpaintingTypeCB->currentItem());
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
    config->sync();
}

void ImageEffect_InPainting_Dialog::slotResetValues(int i)
{
    if (i == NoPreset)
        m_settingsWidget->setEnabled(true);
    else        
        m_settingsWidget->setEnabled(false);

    resetValues();
}

void ImageEffect_InPainting_Dialog::resetValues()
{
    Digikam::GreycstorationSettings settings;
    settings.setInpaintingDefaultSettings();    

    switch(m_inpaintingTypeCB->currentItem())
    {
        case RemoveSmallArtefact:
            // We use default settings here.
            break;
        
        case RemoveMediumArtefact:
        {
            settings.amplitude = 50.0;
            settings.nbIter    = 50;
            break;
        }

        case RemoveLargeArtefact:
        {
            settings.amplitude = 100.0;
            settings.nbIter    = 100;
            break;
        }
    }

    m_settingsWidget->setSettings(settings);
}

void ImageEffect_InPainting_Dialog::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_InPainting_Dialog::prepareEffect()
{
    m_mainTab->setEnabled(false);

    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    m_originalImage = Digikam::DImg(iface.originalWidth(), iface.originalHeight(),
                                    iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    // Selected area from the image and mask creation:
    //
    // We optimize the computation time to use the current selected area in image editor
    // and to create an inpainting mask with it. Because inpainting is done by interpolation
    // neighboor pixels which can be located far from the selected area, we need to ajust the
    // mask size in according with the parameter algorithms, especially 'amplitude'.
    // Mask size is computed like this :
    //
    // (image_size_x + 2*amplitude , image_size_y + 2*amplitude)
    

    QRect selectionRect = QRect(iface.selectedXOrg(), iface.selectedYOrg(),
                                iface.selectedWidth(), iface.selectedHeight());

    QPixmap inPaintingMask(iface.originalWidth(), iface.originalHeight());
    inPaintingMask.fill(Qt::black);
    QPainter p(&inPaintingMask);
    p.fillRect( selectionRect, QBrush(Qt::white) );
    p.end();

    Digikam::GreycstorationSettings settings = m_settingsWidget->getSettings();

    int x1 = (int)(selectionRect.left()   - 2*settings.amplitude);
    int y1 = (int)(selectionRect.top()    - 2*settings.amplitude);
    int x2 = (int)(selectionRect.right()  + 2*settings.amplitude);
    int y2 = (int)(selectionRect.bottom() + 2*settings.amplitude);
    m_maskRect = QRect(x1, y1, x2-x1, y2-y1);

    // Mask area normalization.
    // We need to check if mask area is out of image size else inpainting give strange results.

    if (m_maskRect.left()   < 0) m_maskRect.setLeft(0);
    if (m_maskRect.top()    < 0) m_maskRect.setTop(0);
    if (m_maskRect.right()  > iface.originalWidth())  m_maskRect.setRight(iface.originalWidth());
    if (m_maskRect.bottom() > iface.originalHeight()) m_maskRect.setBottom(iface.originalHeight());

    m_maskImage = inPaintingMask.convertToImage().copy(m_maskRect);
    m_cropImage = m_originalImage.copy(m_maskRect);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Digikam::GreycstorationIface(
                                    &m_cropImage,
                                    settings,
                                    Digikam::GreycstorationIface::InPainting, 
                                    0, 0,
                                    m_maskImage, this));
}

void ImageEffect_InPainting_Dialog::prepareFinal()
{
    if (!m_isComputed)
    {
        setProgressVisible(true);
        prepareEffect();
    }
    else
    {
        putFinalData();
        kapp->restoreOverrideCursor();
        accept();
    }
}

void ImageEffect_InPainting_Dialog::putPreviewData()
{
    Digikam::ImageIface* iface               = m_imagePreviewWidget->imageIface();
    Digikam::GreycstorationSettings settings = m_settingsWidget->getSettings();

    m_cropImage = m_threadedFilter->getTargetImage();
    QRect cropSel((int)(2*settings.amplitude), (int)(2*settings.amplitude), 
                  iface->selectedWidth(), iface->selectedHeight());
    Digikam::DImg imDest = m_cropImage.copy(cropSel);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                                               iface->previewHeight())).bits());
    m_imagePreviewWidget->updatePreview();
    m_isComputed = true;
}

void ImageEffect_InPainting_Dialog::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    
    if (!m_isComputed)
        m_cropImage = m_threadedFilter->getTargetImage();

    m_originalImage.bitBltImage(&m_cropImage, m_maskRect.left(), m_maskRect.top());

    iface.putOriginalImage(i18n("InPainting"), m_originalImage.bits());
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
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Inpainting Configuration File V2")))
        {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a Photograph Inpainting settings text file.")
                        .arg(loadInpaintingFile.fileName()));
           file.close();
           return;
        }
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Inpainting text file."));

    file.close();
    m_inpaintingTypeCB->blockSignals(true);
    m_inpaintingTypeCB->setCurrentItem(NoPreset);
    m_inpaintingTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);             
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
        m_settingsWidget->saveSettings(file, QString("# Photograph Inpainting Configuration File V2"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Inpainting text file."));

    file.close();
}

}  // NameSpace DigikamInPaintingImagesPlugin

