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
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "bannerwidget.h"
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
                             : KDialogBase(Plain, i18n("Photograph Inpainting"),
                                           Help|Default|User2|User3|Ok|Cancel, Ok,
                                           parent, 0, true, true,
                                           QString(),
                                           i18n("&Save As..."),
                                           i18n("&Load...")),
                               m_parent(parent)
{
    QString whatsThis;
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );

    m_currentRenderingMode = NoneRendering;
    m_greycstorationIface  = 0L;

    // About data and help button.

    m_about = new KAboutData("digikamimageplugins",
                             I18N_NOOP("Photograph Inpainting"),
                             digikamimageplugins_version,
                             I18N_NOOP("A digiKam image plugin to inpaint a photograph."),
                             KAboutData::License_GPL,
                             "(c) 2005-2007, Gilles Caulier",
                             0,
                             "http://extragear.kde.org/apps/digikamimageplugins");

    m_about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                       "caulier dot gilles at gmail dot com");

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
    cimgLogoLabel->setText(QString());
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

    m_settingsWidget = new DigikamImagePlugins::GreycstorationWidget( m_mainTab );
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
}

ImageEffect_InPainting_Dialog::~ImageEffect_InPainting_Dialog()
{
    delete m_about;

    if (m_greycstorationIface)
       delete m_greycstorationIface;
}

void ImageEffect_InPainting_Dialog::slotDefault()
{
    DigikamImagePlugins::GreycstorationSettings settings;
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

void ImageEffect_InPainting_Dialog::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
       m_greycstorationIface->stopComputation();
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
       m_greycstorationIface->stopComputation();
       m_parent->unsetCursor();
    }

    e->accept();
}

void ImageEffect_InPainting_Dialog::slotOk()
{
    m_currentRenderingMode = FinalRendering;

    enableButton(Ok, false);
    enableButton(Default, false);
    enableButton(User2, false);
    enableButton(User3, false);
    m_mainTab->setCurrentPage(0);
    m_mainTab->setEnabled(false);

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

    DigikamImagePlugins::GreycstorationSettings settings = m_settingsWidget->getSettings();

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

    if (m_greycstorationIface)
       delete m_greycstorationIface;

    m_greycstorationIface = new DigikamImagePlugins::GreycstorationIface(
                                    &m_cropImage,
                                    settings,
                                    DigikamImagePlugins::GreycstorationIface::InPainting, 
                                    0, 0,
                                    m_maskImage, this);
}

void ImageEffect_InPainting_Dialog::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DigikamImagePlugins::GreycstorationIface::EventData *d = (DigikamImagePlugins::GreycstorationIface::EventData*) event->data();

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
                    DDebug() << "Final InPainting completed..." << endl;
                    Digikam::ImageIface iface(0, 0);
                    Digikam::DImg target = m_greycstorationIface->getTargetImage();
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

