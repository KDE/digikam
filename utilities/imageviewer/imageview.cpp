//////////////////////////////////////////////////////////////////////////////
//
//    IMAGEVIEW.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt lib includes

#include <qobject.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qkeysequence.h>
#include <qstring.h>
#include <qdict.h>
#include <qsize.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qsignal.h>
#include <qevent.h>

// KDE lib includes

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapp.h>
#include <kconfig.h>

// Local includes

#include "exifrestorer.h"
#include "imeventfilter.h"
#include "canvas.h"
#include "imagebcgedit.h"
#include "imageview.h"
#include "imagedescedit.h"
#include "albummanager.h"
#include "albuminfo.h"
#include "kexif.h"


/////////////////////////////////////// CLASS ////////////////////////////////////////////

class CAction {

public:

    CAction(const QString& text_, const QObject* receiver_,
            const char* slot_, const QKeySequence& key_) {

        text   = text_;
        receiver = receiver_;
        slot     = slot_;
        menuID   = -1;
        button   =  0;
        key      = key_;

        signal.connect(receiver, slot);
    }

    void activate() {
        signal.activate();
    }

    QString text;
    const QObject* receiver;
    const char* slot;
    int  menuID;
    void *button;
    QKeySequence key;

    QSignal signal;
};

class CButton : public QToolButton {

public:

    CButton(QWidget *parent,
            CAction *action,
            const QPixmap& pix,
            bool enabled=true,
            bool isToggle=false) : QToolButton(parent) {

        setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                  QSizePolicy::Fixed));
        setTextLabel(action->text);
        setPixmap(pix);
        setEnabled(enabled);
        setToggleButton(isToggle);

        connect(this, SIGNAL(clicked()),
                action->receiver,
                action->slot);
        action->button = this;
    }

protected:

    // override this to paint with no border
    void drawButton( QPainter * p ) {

        QStyle::SCFlags controls = QStyle::SC_ToolButton;
        QStyle::SCFlags active = QStyle::SC_None;

        if (isDown())
            active |= QStyle::SC_ToolButton;

        QStyle::SFlags flags = QStyle::Style_Default;
        if (isEnabled())
            flags |= QStyle::Style_Enabled;
        if (hasFocus())
            flags |= QStyle::Style_HasFocus;
        if (isDown())
            flags |= QStyle::Style_Down;
        if (isOn())
            flags |= QStyle::Style_On;
        if (autoRaise()) {
            flags |= QStyle::Style_AutoRaise;
            if (uses3D()) {
                flags |= QStyle::Style_MouseOver;
                if (! isOn() && ! isDown())
                    flags |= QStyle::Style_Raised;
            }
        } else if (! isOn() && ! isDown())
            flags |= QStyle::Style_Raised;

        if (isDown() || isOn())
            style().drawComplexControl(QStyle::CC_ToolButton, p,
                                   this, rect(), colorGroup(),
                                   flags, controls, active,
                                   QStyleOption());
        drawButtonLabel(p);
    }
};

class ImageViewPrivate {

public:

    bool   singleItemMode;
    KURL::List urlList;
    KURL       urlCurrent;

    Canvas *canvas;
    QWidget *buttonBar;
    QHBoxLayout *buttonLayout;
    QLabel *nameLabel;
    QLabel *zoomLabel;

    QDict<CAction> actions;
    QDict<CAction> actionKeys;

    bool fullScreen;
    bool preloadNext;

    CButton *bNext;
    CButton *bPrev;
    CButton *bZoomIn;
    CButton *bZoomOut;
    CButton *bZoomAuto;
    CButton *bZoom1;
    CButton *bFullScreen;
    CButton *bCrop;
    CButton *bRotate;
    CButton *bBCGEdit;
    CButton *bCommentsEdit;
    CButton *bExifInfo;
    CButton *bSave;
    CButton *bRestore;
    CButton *bRemoveCurrent;
    CButton *bClose;

    QPopupMenu *rotateMenu;
    QPopupMenu *contextMenu;

};


//////////////////////////////////// CONSTRUCTORS ///////////////////////////////////////////

ImageView::ImageView(QWidget* parent,
                     const KURL::List& urlList,
                     const KURL& urlCurrent)
    : QWidget(parent, 0, Qt::WDestructiveClose)
{

    d = new ImageViewPrivate;
    
    d->fullScreen = false;
    d->preloadNext = true;
 
    d->urlList    = KURL::List(urlList);
    d->urlCurrent = urlCurrent;
    d->singleItemMode = false;

    init();    
}


ImageView::ImageView(QWidget* parent, const KURL& urlCurrent)
    : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageViewPrivate;

    d->fullScreen = false;
    d->preloadNext = true;
     
    d->urlList.append(urlCurrent);
    d->urlCurrent = urlCurrent;
    d->singleItemMode = true;

    init();
}


//////////////////////////////////// DESTRUCTOR /////////////////////////////////////////////

ImageView::~ImageView()
{
    saveSettings();
    d->actions.clear();
    delete d;
}


//////////////////////////////////////// FONCTIONS //////////////////////////////////////////

void ImageView::init()
{
    initGui();

    setupActions();
    setupPopupMenu();
    setupButtons();

    setupConnections();

    IMEventFilter *efilter = new IMEventFilter(this);
    installEventFilter(efilter);
    d->canvas->installEventFilter(efilter);

    connect(efilter, SIGNAL(signalKeyPress(int)),
            this,    SLOT(slotKeyPress(int)));
    
    readSettings();

    loadCurrentItem();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::readSettings()
{
    KConfig* config = kapp->config();
    
    int width, height;
    bool autoZoom;
    bool fullScreen;
    
    
    config->setGroup("ImageViewer Settings");

    width = config->readNumEntry("Width", 500);
    height = config->readNumEntry("Height", 500);
    autoZoom = config->readBoolEntry("AutoZoom", true);
    fullScreen = config->readBoolEntry("FullScreen", false);

    resize(width, height);
    
    if (autoZoom)
        d->bZoomAuto->animateClick();
    if (fullScreen)
        d->bFullScreen->animateClick();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::saveSettings()
{

    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");

    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->writeEntry("AutoZoom", d->bZoomAuto->isOn());
    config->writeEntry("FullScreen", d->bFullScreen->isOn());
    config->sync();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::loadCurrentItem()
{
    KURL::List::iterator it = d->urlList.find(d->urlCurrent);
    int index = d->urlList.findIndex(d->urlCurrent);

    if (it != d->urlList.end()) {

        d->canvas->load(d->urlCurrent.path());

        QString text = i18n("%1 (%2 of %3)")
                       .arg(d->urlCurrent.filename())
                       .arg(QString::number(index+1))
                       .arg(QString::number(d->urlList.count()));

        d->nameLabel->setText(text);
            
        // Going up, Mr. Tyler?
        if (d->preloadNext && (d->urlCurrent != d->urlList.last())) {

            // preload the next item
            KURL urlNext = *(++it);
            d->canvas->preload(urlNext.path());
        }
        else if (d->urlCurrent != d->urlList.first()){
            // No, going down
            // preload the prev item

            KURL urlPrev = *(--it);
            d->canvas->preload(urlPrev.path());
        }

    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::initGui()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);

    d->buttonBar = new QWidget(this);
    d->buttonLayout = new QHBoxLayout(d->buttonBar, 2);
    vlayout->addWidget(d->buttonBar);

    d->canvas = new Canvas(this);
    vlayout->addWidget(d->canvas);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupConnections()
{
    connect(d->canvas, SIGNAL(signalChanged(bool)),
            this, SLOT(slotChanged(bool)));
    connect(d->canvas, SIGNAL(signalCropSelected(bool)),
            this, SLOT(slotCropSelected(bool)));
    connect(d->canvas, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotShowContextMenu()));
    connect(d->canvas, SIGNAL(signalShowNextImage()),
            this, SLOT(slotNextImage()));
    connect(d->canvas, SIGNAL(signalShowPrevImage()),
            this, SLOT(slotPrevImage()));
    connect(d->canvas, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupActions()
{

    d->actions.setAutoDelete(true);

    d->actions.insert("prev",
                      new CAction(i18n("Previous Image"),
                                  this, SLOT(slotPrevImage()),
                                  QKeySequence(Key_PageUp)));

    d->actions.insert("next",
                      new CAction(i18n("Next Image"),
                                  this, SLOT(slotNextImage()),
                                  QKeySequence(Key_PageDown)));

    d->actions.insert("zoomIn",
                      new CAction(i18n("Zoom In"),
                                  d->canvas, SLOT(slotIncreaseZoom()),
                                  QKeySequence(Key_Plus)));

    d->actions.insert("zoomOut",
                      new CAction(i18n("Zoom Out"),
                                  d->canvas,
                                  SLOT(slotDecreaseZoom()),
                                  QKeySequence(Key_Minus)));

    d->actions.insert("zoom1",
                      new CAction(i18n("Zoom 1:1"),
                                  d->canvas,
                                  SLOT(slotSetZoom1()),
                                  QKeySequence(Key_1)));

    d->actions.insert("toggleAutoZoom",
                      new CAction(i18n("Toggle Auto Zoom"),
                                  this,
                                  SLOT(slotToggleAutoZoom()),
                                  QKeySequence(Key_A)));

    d->actions.insert("toggleFullScreen",
                      new CAction(i18n("Toggle Full Screen"),
                                  this,
                                  SLOT(slotToggleFullScreen()),
                                  QKeySequence(Key_F)));

    d->actions.insert("rotate",
                      new CAction(i18n("Rotate Image"),
                                  this,
                                  SLOT(slotShowRotateMenu()),
                                  QKeySequence(Key_R)));


    d->actions.insert("rotate90",
                      new CAction(i18n("Rotate 90 degrees"),
                                  d->canvas,
                                  SLOT(slotRotate90()),
                                  QKeySequence(Key_1)));

    d->actions.insert("rotate180",
                      new CAction(i18n("Rotate 180 degrees"),
                                  d->canvas,
                                  SLOT(slotRotate180()),
                                  QKeySequence(Key_2)));
    
    d->actions.insert("rotate270",
                      new CAction(i18n("Rotate 270 degrees"),
                                  d->canvas,
                                  SLOT(slotRotate270()),
                                  QKeySequence(Key_3)));

    d->actions.insert("gamma+",
                      new CAction(i18n("Increase Gamma"),
                                  d->canvas,
                                  SLOT(slotGammaPlus()),
                                  QKeySequence(Key_G)));

    d->actions.insert("gamma-",
                      new CAction(i18n("Decrease Gamma"),
                                  d->canvas,
                                  SLOT(slotGammaMinus()),
                                  QKeySequence(SHIFT+Key_G)));
    
    d->actions.insert("brightness+",
                      new CAction(i18n("Increase Brightness"),
                                  d->canvas,
                                  SLOT(slotBrightnessPlus()),
                                  QKeySequence(Key_B)));
    
    d->actions.insert("brightness-",
                      new CAction(i18n("Decrease Brightness"),
                                  d->canvas,
                                  SLOT(slotBrightnessMinus()),
                                  QKeySequence(SHIFT+Key_B)));

    d->actions.insert("contrast+",
                      new CAction(i18n("Increase Contrast"),
                                  d->canvas,
                                  SLOT(slotContrastPlus()),
                                  QKeySequence(Key_C)));
    
    d->actions.insert("contrast-",
                      new CAction(i18n("Decrease Contrast"),
                                  d->canvas,
                                  SLOT(slotContrastMinus()),
                                  QKeySequence(SHIFT+Key_C)));

    d->actions.insert("bcgEdit",
                      new CAction(i18n("Adjust Brightness/Contrast/Gamma"),
                                  this,
                                  SLOT(slotBCGEdit()),
                                  QKeySequence(CTRL+Key_E)));
    
    d->actions.insert("commentsEdit",
                      new CAction(i18n("Edit Image Comments"),
                                  this,
                                  SLOT(slotCommentsEdit()),
                                  QKeySequence(Key_F3)));
    
    d->actions.insert("ExifInfo",
                      new CAction(i18n("View Exif Information"),
                                  this,
                                  SLOT(slotExifInfo()),
                                  QKeySequence(Key_F6)));
                                  
    d->actions.insert("crop",
                      new CAction(i18n("Crop"),
                                  d->canvas,
                                  SLOT(slotCrop()),
                                  QKeySequence(CTRL+Key_C)));

    d->actions.insert("save",
                      new CAction(i18n("Save"),
                                  this,
                                  SLOT(slotSave()),
                                  QKeySequence(CTRL+Key_S)));


    d->actions.insert("restore",
                      new CAction(i18n("Restore"),
                                  d->canvas,
                                  SLOT(slotRestore()),
                                  QKeySequence(CTRL+Key_R)));

    d->actions.insert("remove",
                      new CAction(i18n("Remove from Album"),
                                  this,
                                  SLOT(slotRemoveCurrentItemfromAlbum()),
                                  QKeySequence(SHIFT+Key_Delete)));

    d->actions.insert("close",
                      new CAction(i18n("Close"),
                                  this,
                                  SLOT(slotClose()),
                                  QKeySequence(CTRL+Key_Q)));


    // Now insert these keys into the keydict

    d->actionKeys.setAutoDelete(false);
    d->actionKeys.clear();

    addKeyInDict("prev");
    addKeyInDict("next");
    addKeyInDict("zoomIn");
    addKeyInDict("zoomOut");
    addKeyInDict("toggleAutoZoom");
    addKeyInDict("toggleFullScreen");
    addKeyInDict("rotate");
    addKeyInDict("rotate90");
    addKeyInDict("rotate180");
    addKeyInDict("rotate270");
    addKeyInDict("gamma+");
    addKeyInDict("gamma-");
    addKeyInDict("brightness+");
    addKeyInDict("brightness-");
    addKeyInDict("contrast+");
    addKeyInDict("contrast-");
    addKeyInDict("bcgEdit");
    addKeyInDict("commentsEdit");
    addKeyInDict("ExifInfo");
    addKeyInDict("crop");
    addKeyInDict("save");
    addKeyInDict("restore");
    addKeyInDict("remove");
    addKeyInDict("close");

    d->actionKeys.insert(QKeySequence(Key_Escape),
                         d->actions.find("close"));
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupPopupMenu()
{

    // Setup the rotate menu

    d->rotateMenu = new QPopupMenu(this);

    addMenuItem(d->rotateMenu, d->actions.find("rotate90"));
    addMenuItem(d->rotateMenu, d->actions.find("rotate180"));
    addMenuItem(d->rotateMenu, d->actions.find("rotate270"));


    // Setup the context menu

    d->contextMenu = new QPopupMenu(this);

    addMenuItem(d->contextMenu, d->actions.find("next"));
    addMenuItem(d->contextMenu, d->actions.find("prev"));

    d->contextMenu->insertSeparator();

    addMenuItem(d->contextMenu, d->actions.find("zoomIn"));
    addMenuItem(d->contextMenu, d->actions.find("zoomOut"));

    d->contextMenu->insertSeparator();

    d->contextMenu->insertItem(i18n("Rotate Image"), d->rotateMenu);

    addMenuItem(d->contextMenu, d->actions.find("crop"));

    addMenuItem(d->contextMenu, d->actions.find("bcgEdit"));
    addMenuItem(d->contextMenu, d->actions.find("commentsEdit"));
    addMenuItem(d->contextMenu, d->actions.find("ExifInfo"));

    QPopupMenu *brightnessMenu = new QPopupMenu(d->contextMenu);
    addMenuItem(brightnessMenu, d->actions.find("brightness+"));
    addMenuItem(brightnessMenu, d->actions.find("brightness-"));
    d->contextMenu->insertItem(i18n("Brightness"), brightnessMenu);

    QPopupMenu *contrastMenu = new QPopupMenu(d->contextMenu);
    addMenuItem(contrastMenu, d->actions.find("contrast+"));
    addMenuItem(contrastMenu, d->actions.find("contrast-"));
    d->contextMenu->insertItem(i18n("Contrast"), contrastMenu);

    QPopupMenu *gammaMenu = new QPopupMenu(d->contextMenu);
    addMenuItem(gammaMenu, d->actions.find("gamma+"));
    addMenuItem(gammaMenu, d->actions.find("gamma-"));
    d->contextMenu->insertItem(i18n("Gamma"), gammaMenu);

    d->contextMenu->insertSeparator();

    addMenuItem(d->contextMenu, d->actions.find("save"));
    addMenuItem(d->contextMenu, d->actions.find("restore"));
    addMenuItem(d->contextMenu, d->actions.find("remove"));

    d->contextMenu->insertSeparator();

    addMenuItem(d->contextMenu, d->actions.find("close"));


    // Disable save, crop, and restore actions in popupmenu.
    
    CAction *action = 0;

    action = d->actions.find("crop");
    d->contextMenu->setItemEnabled(action->menuID, false);

    action = d->actions.find("save");
    d->contextMenu->setItemEnabled(action->menuID, false);

    action = d->actions.find("restore");
    d->contextMenu->setItemEnabled(action->menuID, false);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::addMenuItem(QPopupMenu *menu, CAction *action)
{
    if (action) {
        action->menuID = menu->insertItem(action->text, action->receiver,
                                          action->slot, action->key);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::addKeyInDict(const QString& key)
{
    CAction *action = d->actions.find(key);
    if (action)
        d->actionKeys.insert(action->key, action);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupButtons()
{
    d->bPrev = new CButton(d->buttonBar,
                           d->actions.find("prev"),
                           BarIcon("back"));
    d->buttonLayout->addWidget(d->bPrev);

    d->bNext = new CButton(d->buttonBar,
                           d->actions.find("next"),
                           BarIcon("forward"));
    d->buttonLayout->addWidget(d->bNext);

    QHBox *labelBox = new QHBox(d->buttonBar);
    labelBox->setPaletteBackgroundColor(Qt::white);
    labelBox->setMargin(0);
    labelBox->setSpacing(2);
    d->buttonLayout->addSpacing(5);
    d->buttonLayout->addWidget(labelBox);
    d->buttonLayout->addSpacing(5);

    d->nameLabel = new QLabel(labelBox);
    d->nameLabel->setPaletteBackgroundColor(Qt::white);
    d->nameLabel->setFrameShape(QFrame::Box);
    d->zoomLabel = new QLabel(labelBox);
    d->zoomLabel->setPaletteBackgroundColor(Qt::white);
    d->zoomLabel->setFrameShape(QFrame::Box);

    d->bZoomIn = new CButton(d->buttonBar,
                             d->actions.find("zoomIn"),
                             BarIcon("viewmag+"));
                             
    d->buttonLayout->addWidget(d->bZoomIn);

    d->bZoomOut = new CButton(d->buttonBar,
                             d->actions.find("zoomOut"),
                             BarIcon("viewmag-"));
    d->buttonLayout->addWidget(d->bZoomOut);

    d->bZoomAuto = new CButton(d->buttonBar,
                             d->actions.find("toggleAutoZoom"),
                             BarIcon("viewmagfit"),
                             true, true);
    d->buttonLayout->addWidget(d->bZoomAuto);

    d->bZoom1 = new CButton(d->buttonBar,
                             d->actions.find("zoom1"),
                             BarIcon("viewmag1"));
    d->buttonLayout->addWidget(d->bZoom1);

    d->bFullScreen = new CButton(d->buttonBar,
                             d->actions.find("toggleFullScreen"),
                             BarIcon("window_fullscreen"),
                             true, true);
    d->buttonLayout->addWidget(d->bFullScreen);

    d->bBCGEdit = new CButton(d->buttonBar,
                             d->actions.find("bcgEdit"),
                             BarIcon("blend"));
    d->buttonLayout->addWidget(d->bBCGEdit);

    d->bCommentsEdit = new CButton(d->buttonBar,
                             d->actions.find("commentsEdit"),
                             BarIcon("imagecomment"));
    d->buttonLayout->addWidget(d->bCommentsEdit);

    d->bExifInfo = new CButton(d->buttonBar,
                             d->actions.find("ExifInfo"),
                             BarIcon("exifinfo"));
    d->buttonLayout->addWidget(d->bExifInfo);

    
    d->bRotate = new CButton(d->buttonBar,
                             d->actions.find("rotate"),
                             BarIcon("rotate_cw"));
    d->buttonLayout->addWidget(d->bRotate);

    d->bCrop  = new CButton(d->buttonBar,
                             d->actions.find("crop"),
                             BarIcon("crop"),
                             false);
    d->buttonLayout->addWidget(d->bCrop);

    d->bSave  = new CButton(d->buttonBar,
                             d->actions.find("save"),
                             BarIcon("filesave"),
                             false);
    d->buttonLayout->addWidget(d->bSave);

    d->bRestore  = new CButton(d->buttonBar,
                             d->actions.find("restore"),
                             BarIcon("undo"),
                             false);
    d->buttonLayout->addWidget(d->bRestore);

    d->bRemoveCurrent  = new CButton(d->buttonBar,
                             d->actions.find("remove"),
                             BarIcon("editdelete"));
    d->buttonLayout->addWidget(d->bRemoveCurrent);

    d->bClose = new CButton(d->buttonBar,
                             d->actions.find("close"),
                             BarIcon("exit"));
    d->buttonLayout->addWidget(d->bClose);


    d->buttonLayout->insertStretch(-1);
}


//////////////////////////////////////// SLOTS //////////////////////////////////////////////

void ImageView::slotNextImage()
{
    promptUserSave();

    KURL::List::iterator it = d->urlList.find(d->urlCurrent);

    if (it != d->urlList.end()) {

         d->preloadNext = true;

         if (d->urlCurrent != d->urlList.last()) {

             KURL urlNext = *(++it);
             d->urlCurrent = urlNext;
             loadCurrentItem();
             
             if (d->urlList.count() == 1)
                d->bPrev->setEnabled(false);
             else       
                d->bPrev->setEnabled(true);
                          
             if (d->urlCurrent == d->urlList.last()) 
                 d->bNext->setEnabled(false);   
         }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotPrevImage()
{
    promptUserSave();

    KURL::List::iterator it = d->urlList.find(d->urlCurrent);

    if (it != d->urlList.begin()) {

         d->preloadNext = true;

         if (d->urlCurrent != d->urlList.first()) {

             KURL urlPrev = *(--it);
             d->urlCurrent = urlPrev;
             loadCurrentItem();
             
             if (d->urlList.count() == 1)
                d->bNext->setEnabled(false);
             else       
                d->bNext->setEnabled(true);
             
             if (d->urlCurrent == d->urlList.first()) 
                 d->bPrev->setEnabled(false);
         }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotRemoveCurrentItemfromAlbum()
{
    KURL currentImage = d->urlCurrent;

    QString warnMsg(i18n("About to delete this Image from current Album\nAre you sure?"));

    if (KMessageBox::warningContinueCancel(this,
                                           warnMsg,
                                           i18n("Warning"),
                                           i18n("Delete"))
        ==  KMessageBox::Continue) {

       KIO::DeleteJob* job = KIO::del(currentImage, false, true);

       connect(job, SIGNAL(result(KIO::Job*)),
               this, SLOT(slot_onDeleteCurrentItemFinished(KIO::Job*)));
       }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slot_onDeleteCurrentItemFinished(KIO::Job* job)
{
    if (job->error())
        {
        job->showErrorDialog(this);
        return;
        }

    promptUserSave();
    KURL CurrentToRemove = d->urlCurrent;
    KURL::List::iterator it = d->urlList.find(d->urlCurrent);

    if (it != d->urlList.end())
       {
       d->preloadNext = true;

       // Try to get the next image in the current Album...

       if (d->urlCurrent != d->urlList.last())
          {
          KURL urlNext = *(++it);
          d->urlCurrent = urlNext;
          d->urlList.remove(CurrentToRemove);
          loadCurrentItem();
          
          if (d->urlList.count() == 1)
             d->bPrev->setEnabled(false);
          else       
             d->bPrev->setEnabled(true);
             
          if (d->urlCurrent == d->urlList.last()) 
              d->bNext->setEnabled(false);   
              
          return;
          }

       // Try to get the preview image in the current Album...

       else if (d->urlCurrent != d->urlList.first())
          {
          KURL urlPrev = *(--it);
          d->urlCurrent = urlPrev;
          d->urlList.remove(CurrentToRemove);
          loadCurrentItem();
          
          if (d->urlList.count() == 1)
             d->bNext->setEnabled(false);
          else       
             d->bNext->setEnabled(true);

          if (d->urlCurrent == d->urlList.first()) 
              d->bPrev->setEnabled(false);
              
          return;
          }

      // No image in the current Album -> Quit ImageViever...

      else
         {
         d->bPrev->setEnabled(false);    
         d->bNext->setEnabled(false);         
         KMessageBox::information(this,
                                  i18n("There is no image to show in the current Album!\n"
                                       "The Digikam ImageViewer will be closed..."),
                                  i18n("No image in the current Album")
                                  );

         slotClose();
         }
       }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotShowRotateMenu()
{
    d->rotateMenu->exec(QCursor::pos());
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotShowContextMenu()
{
    d->contextMenu->exec(QCursor::pos());
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotSave()
{
    if (!d->urlCurrent.isValid()) return;
    
    QString tmpFile = locateLocal("tmp",
                                  d->urlCurrent.filename());
    
    int result = d->canvas->save(tmpFile);

    if (result != 1) {
        KMessageBox::error(this, i18n("Failed to save file:")
                           + d->urlCurrent.filename());
         return;
    }

    ExifRestorer exifHolder;
    exifHolder.readFile(d->urlCurrent.path(),
                        ExifRestorer::ExifOnly);

    if (exifHolder.hasExif()) {
        ExifRestorer restorer;
        restorer.readFile(tmpFile, ExifRestorer::EntireImage);
        restorer.insertExifData(exifHolder.exifData());
        restorer.writeFile(tmpFile);
    }
    else {
        qWarning("No Exif Data Found");
    }

    KIO::FileCopyJob* job = KIO::file_move(KURL(tmpFile), d->urlCurrent,
                                           -1, true, false, false);

    connect(job, SIGNAL(result(KIO::Job *) ),
            this, SLOT(slotSaveResult(KIO::Job *)));
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotToggleAutoZoom()
{
    bool val;
    
    if (d->canvas->autoZoomOn()) {
        d->canvas->slotSetAutoZoom(false);
        val = true;
    }
    else {
        d->canvas->slotSetAutoZoom(true);
        val = false;
    }

    d->bZoomIn->setEnabled(val);
    d->bZoomOut->setEnabled(val);
    d->bZoom1->setEnabled(val);
    CAction *action = d->actions.find("zoomIn");
    d->contextMenu->setItemEnabled(action->menuID, val);
    action = d->actions.find("zoomOut");
    d->contextMenu->setItemEnabled(action->menuID, val);
    action = d->actions.find("zoom1");
    d->contextMenu->setItemEnabled(action->menuID, val);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotToggleFullScreen()
{
    if (d->fullScreen) {
        showNormal();
        d->fullScreen = false;
        move(0, 0);
    }
    else {
        showFullScreen();
        d->fullScreen = true;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotZoomChanged(double zoom)
{
    d->zoomLabel->setText(i18n("Zoom: %1 %").arg(QString::number(zoom*100, 'f', 1)));
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotCropSelected(bool val)
{
    d->bCrop->setEnabled(val);
    CAction *action = d->actions.find("crop");
    d->contextMenu->setItemEnabled(action->menuID, val);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotChanged(bool val)
{
    d->bSave->setEnabled(val);
    d->bRestore->setEnabled(val);

    CAction *action = 0;

    action = d->actions.find("save");
    d->contextMenu->setItemEnabled(action->menuID, val);

    action = d->actions.find("restore");
    d->contextMenu->setItemEnabled(action->menuID, val);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotClose()
{
    close();    
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotBCGEdit()
{
    ImageBCGEdit *bcgEdit = new ImageBCGEdit(this);
    connect(bcgEdit, SIGNAL(signalGammaIncrease()),
            d->canvas, SLOT(slotGammaPlus()));
    connect(bcgEdit, SIGNAL(signalGammaDecrease()),
            d->canvas, SLOT(slotGammaMinus()));
    connect(bcgEdit, SIGNAL(signalBrightnessIncrease()),
            d->canvas, SLOT(slotBrightnessPlus()));
    connect(bcgEdit, SIGNAL(signalBrightnessDecrease()),
            d->canvas, SLOT(slotBrightnessMinus()));
    connect(bcgEdit, SIGNAL(signalContrastIncrease()),
            d->canvas, SLOT(slotContrastPlus()));
    connect(bcgEdit, SIGNAL(signalContrastDecrease()),
            d->canvas, SLOT(slotContrastMinus()));
    bcgEdit->adjustSize();
    bcgEdit->show();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotCommentsEdit()
{
    Digikam::AlbumInfo *currentAlbum = Digikam::AlbumManager::instance()->currentAlbum();
    currentAlbum->openDB();
    QString comments(currentAlbum->getItemComments(d->urlCurrent.filename()));
    
    if (ImageDescEdit::editComments(d->urlCurrent.filename(), comments, this)) 
       {
       currentAlbum->setItemComments(d->urlCurrent.filename(), comments);
       Digikam::AlbumManager::instance()->refreshItemHandler(d->urlCurrent.filename());
       }
       
    currentAlbum->closeDB();   
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotExifInfo()
{
    KExif *exif = new KExif(0);
    
    if (exif->loadFile(d->urlCurrent.path()) == 0)
        exif->show();
    else {
        delete exif;
        KMessageBox::sorry(this,
                           i18n("This item has no Exif Information"));
    }

}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotSaveResult(KIO::Job *job)
{
    if (job->error()) {
        job->showErrorDialog(this);
        return;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotKeyPress(int key)
{
    QKeySequence keyPressed(key);

    CAction *action = d->actionKeys.find(QString(keyPressed));
    if (!action) return;

    if (action->button) {
        CButton *button = (CButton*) action->button;
        button->animateClick();
    }
    else
        action->activate();
    
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::closeEvent(QCloseEvent *e)
{
    if (!e) return;

    promptUserSave();
    
    e->accept();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::promptUserSave()
{
    if (d->bSave->isEnabled()) {

        int result =
            KMessageBox::warningYesNo(this,
                                      d->urlCurrent.filename() +
                                      i18n(" has been modified.\n"
                                           "Do you wish to want to save it?"));
        if (result == KMessageBox::Yes)
            slotSave();
    }
}
