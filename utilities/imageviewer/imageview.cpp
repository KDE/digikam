//////////////////////////////////////////////////////////////////////////////
//
//    IMAGEVIEW.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    Original printing code from Kuickshow program.
//    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer at kde.org>
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
#include <qcheckbox.h>
#include <qfont.h>
#include <qgrid.h>
#include <qlayout.h>
#include <qimage.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qcolor.h>
#include <qcombobox.h>

// KDE lib includes

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kapp.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kimageeffect.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <knuminput.h>
#include <kprinter.h>
#include <ktempfile.h>
#include <kpropertiesdialog.h>
#include <kapplication.h> 

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
#include "kexifdata.h"


/////////////////////////////////////// CLASS ////////////////////////////////////////////

class CAction 
{
public:

    CAction(const QString& text_, const QObject* receiver_,
            const char* slot_, const QKeySequence& key_) 
       {
       text   = text_;
       receiver = receiver_;
       slot     = slot_;
       menuID   = -1;
       button   =  0;
       key      = key_;

       signal.connect(receiver, slot);
       }

    void activate() 
       {
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

class CButton : public QToolButton 
{
public:

    CButton(QWidget *parent,
            CAction *action,
            const QPixmap& pix,
            bool enabled=true,
            bool isToggle=false) : QToolButton(parent) 
        {
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

    // Override this to paint with no border.
    
    void drawButton( QPainter * p ) 
        {
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
        if (autoRaise()) 
            {
            flags |= QStyle::Style_AutoRaise;
            
            if (uses3D()) 
                {
                flags |= QStyle::Style_MouseOver;
                
                if (! isOn() && ! isDown())
                    flags |= QStyle::Style_Raised;
                }
            }
        else if (! isOn() && ! isDown())
            flags |= QStyle::Style_Raised;

        if (isDown() || isOn())
            style().drawComplexControl(QStyle::CC_ToolButton, p,
                                   this, rect(), colorGroup(),
                                   flags, controls, active,
                                   QStyleOption());
        drawButtonLabel(p);
        }
};

class ImageViewPrivate 
{
public:

    bool         singleItemMode;
    KURL::List   urlList;
    KURL         urlCurrent;

    Canvas      *canvas;
    QWidget     *buttonBar;
    QHBoxLayout *buttonLayout;
    QWidget     *buttonBar2;
    QHBoxLayout *buttonLayout2;
    QComboBox   *nameComboBox;
    QLabel      *countLabel;
    QLabel      *zoomLabel;

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
    CButton *bFlip;
    CButton *bBCGEdit;
    CButton *bCommentsEdit;
    CButton *bExifInfo;
    CButton *bImageProperties;
    CButton *bPrint;
    CButton *bSave;
    CButton *bSaveAs;
    CButton *bRestore;
    CButton *bRemoveCurrent;
    CButton *bClose;

    QPopupMenu *rotateMenu;
    QPopupMenu *flipMenu;
    QPopupMenu *contextMenu;
};


//////////////////////////////////// CONSTRUCTORS ///////////////////////////////////////////

ImageView::ImageView(QWidget* parent,
                     const KURL::List& urlList,
                     const KURL& urlCurrent,
                     bool  fromCameraUI)
         : QWidget(parent, 0, Qt::WDestructiveClose)
{
    fromCameraUIFlag  = fromCameraUI;
    
    if (!fromCameraUIFlag)
        setCaption(i18n("Digikam ImageViewer - Images from Album \"%1\"")
                   .arg(urlCurrent.path().section('/', -2, -2)));

    d = new ImageViewPrivate;
    
    d->fullScreen     = false;
    d->preloadNext    = true;
 
    d->urlList        = KURL::List(urlList);
    d->urlCurrent     = urlCurrent;
    d->singleItemMode = false;

    init();    
}


ImageView::ImageView(QWidget* parent,
                     const KURL& urlCurrent,
                     bool  fromCameraUI)
         : QWidget(parent, 0, Qt::WDestructiveClose)
{
    fromCameraUIFlag  = fromCameraUI;
    
    if (!fromCameraUIFlag)
       setCaption(i18n("Digikam ImageViewer - Image from Album \"%1\"")
                  .arg(urlCurrent.path().section('/', -2, -2)));
    
    d = new ImageViewPrivate;

    d->fullScreen     = false;
    d->preloadNext    = true;
     
    d->urlList.append(urlCurrent);
    d->urlCurrent     = urlCurrent;
    d->singleItemMode = true;

    init();
}


ImageViewPrintDialogPage::ImageViewPrintDialogPage( QWidget *parent, const char *name )
                        : KPrintDialogPage( parent, name )
{
    setTitle( i18n("Image Settings") );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( KDialog::marginHint() );
    layout->setSpacing( KDialog::spacingHint() );

    m_addFileName = new QCheckBox( i18n("Print fi&lename below image"), this);
    m_addFileName->setChecked( true );
    layout->addWidget( m_addFileName );

    m_blackwhite = new QCheckBox ( i18n("Print image in &black and white"), this);
    m_blackwhite->setChecked( false );
    layout->addWidget (m_blackwhite );

    QVButtonGroup *group = new QVButtonGroup( i18n("Scaling"), this );
    group->setRadioButtonExclusive( true );
    layout->addWidget( group );
    m_shrinkToFit = new QCheckBox( i18n("Shrink image to &fit, if necessary"), group );
    m_shrinkToFit->setChecked( true );

    QWidget *widget = new QWidget( group );
    QGridLayout *grid = new QGridLayout( widget, 3, 3 );
    grid->addColSpacing( 0, 30 );
    grid->setColStretch( 0, 0 );
    grid->setColStretch( 1, 1 );
    grid->setColStretch( 2, 10 );

    m_scale = new QRadioButton( i18n("Print e&xact size: "), widget );
    m_scale->setEnabled( false ); // ###
    grid->addMultiCellWidget( m_scale, 0, 0, 0, 1 );
    group->insert( m_scale );
    
    connect( m_scale, SIGNAL( toggled( bool )),
             SLOT( toggleScaling( bool )));

    m_units = new KComboBox( false, widget, "unit combobox" );
    grid->addWidget( m_units, 0, 2, AlignLeft );
    m_units->insertItem( i18n("Millimeters") );
    m_units->insertItem( i18n("Centimeters") );
    m_units->insertItem( i18n("Inches") );

    m_width = new KIntNumInput( widget, "exact width" );
    grid->addWidget( m_width, 1, 1 );
    m_width->setLabel( i18n("&Width:" ) );
    m_width->setMinValue( 1 );

    m_height = new KIntNumInput( widget, "exact height" );
    grid->addWidget( m_height, 2, 1 );
    m_height->setLabel( i18n("&Height:" ) );
    m_height->setMinValue( 1 );
}


//////////////////////////////////// DESTRUCTOR /////////////////////////////////////////////

ImageView::~ImageView()
{
    saveSettings();
    d->actions.clear();
    
    if (!m_thumbJob.isNull())
        delete m_thumbJob;    
    
    delete d;
}


ImageViewPrintDialogPage::~ImageViewPrintDialogPage()
{
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
    config->setGroup("EXIF Settings");
    setExifOrientation = config->readBoolEntry("EXIF Set Orientation", true);

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

    if (it != d->urlList.end()) 
        {
        d->canvas->load(d->urlCurrent.path());

        QString text = i18n("%2 of %3")
                       .arg(QString::number(index+1))
                       .arg(QString::number(d->urlList.count()));

        d->countLabel->setText(text);
        
        d->nameComboBox->setCurrentText(d->urlCurrent.filename());
            
        // Going up, Mr. Tyler?
        if (d->preloadNext && (d->urlCurrent != d->urlList.last())) 
            {
            // preload the next item
            KURL urlNext = *(++it);
            d->canvas->preload(urlNext.path());
            
            if (d->urlList.count() == 1)
               d->bPrev->setEnabled(false);
            else       
               d->bPrev->setEnabled(true);
             
            if (d->urlCurrent == d->urlList.last()) 
                d->bNext->setEnabled(false);   
            }
        else if (d->urlCurrent != d->urlList.first())
            {
            // No, going down
            // preload the prev item

            KURL urlPrev = *(--it);
            d->canvas->preload(urlPrev.path());
            
            if (d->urlList.count() == 1)
                d->bNext->setEnabled(false);
            else       
                d->bNext->setEnabled(true);

            if (d->urlCurrent == d->urlList.first()) 
                d->bPrev->setEnabled(false);
            }
        }

    if (d->urlList.count() == 1)
        {
        setPrevAction(false);
        setNextAction(false);
        }
    else       
        {
        setNextAction(true);
        setPrevAction(true);
        }
                          
    if (d->urlCurrent == d->urlList.last()) 
        setNextAction(false);
        
    if (d->urlCurrent == d->urlList.first()) 
        setPrevAction(false);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::initGui()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);

    d->buttonBar2 = new QWidget(this);
    d->buttonLayout2 = new QHBoxLayout(d->buttonBar2, 2, -1, "buttonbar2");
    vlayout->addWidget(d->buttonBar2);

    d->buttonBar = new QWidget(this);
    d->buttonLayout = new QHBoxLayout(d->buttonBar, 2, -1, "buttonbar1");
    vlayout->addWidget(d->buttonBar);
        
    d->canvas = new Canvas(this);
    vlayout->addWidget(d->canvas);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupConnections()
{
    connect(d->canvas, SIGNAL(signalChanged(bool)),
            this, SLOT(slotChanged(bool)));
            
    connect(d->canvas, SIGNAL(signalRotatedOrFlipped()),
            this, SLOT(slotRotatedOrFlipped()));
            
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
            
    connect(d->nameComboBox, SIGNAL( activated( const QString & ) ),
            this, SLOT( slotImageNameActived( const QString & ) ) );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupActions()
{
    d->actions.setAutoDelete(true);

    d->actions.insert("prev",
                      new CAction(i18n("Previous Image"),
                                  this,
                                  SLOT(slotPrevImage()),
                                  QKeySequence(Key_PageUp)));

    d->actions.insert("next",
                      new CAction(i18n("Next Image"),
                                  this,
                                  SLOT(slotNextImage()),
                                  QKeySequence(Key_PageDown)));

    d->actions.insert("zoomIn",
                      new CAction(i18n("Zoom In"),
                                  d->canvas,
                                  SLOT(slotIncreaseZoom()),
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
                                  QKeySequence(SHIFT+Key_1)));

    d->actions.insert("toggleAutoZoom",
                      new CAction(i18n("Toggle Auto Zoom"),
                                  this,
                                  SLOT(slotToggleAutoZoom()),
                                  QKeySequence(Key_A)));

    d->actions.insert("toggleFullScreen",
                      new CAction(i18n("Toggle Full Screen"),
                                  this,
                                  SLOT(slotToggleFullScreen()),
                                  QKeySequence(CTRL+SHIFT+Key_F)));

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

    d->actions.insert("flip",
                      new CAction(i18n("Flip Image"),
                                  this,
                                  SLOT(slotShowFlipMenu()),
                                  QKeySequence(Key_F)));


    d->actions.insert("fliphorizontal",
                      new CAction(i18n("Flip Horizontal"),
                                  d->canvas,
                                  SLOT(slotFlipHorizontal()),
                                  QKeySequence(Key_H)));

    d->actions.insert("flipvertical",
                      new CAction(i18n("Flip Vertical"),
                                  d->canvas,
                                  SLOT(slotFlipVertical()),
                                  QKeySequence(Key_V)));
                                  
    d->actions.insert("bcgEdit",
                      new CAction(i18n("Adjust Image..."),
                                  this,
                                  SLOT(slotBCGEdit()),
                                  QKeySequence(CTRL+Key_E)));
    
    if ( fromCameraUIFlag == false )
       {
       d->actions.insert("commentsEdit",
                      new CAction(i18n("Edit Image Comments..."),
                                  this,
                                  SLOT(slotCommentsEdit()),
                                  QKeySequence(Key_F3)));
       }
    
    d->actions.insert("ExifInfo",
                      new CAction(i18n("View Exif Information..."),
                                  this,
                                  SLOT(slotExifInfo()),
                                  QKeySequence(Key_F6)));

    d->actions.insert("ImageProperties",
                      new CAction(i18n("View Image properties..."),
                                  this,
                                  SLOT(slotImageProperties()),
                                  QKeySequence(ALT+Key_Return)));
                                                                    
    d->actions.insert("crop",
                      new CAction(i18n("Crop"),
                                  d->canvas,
                                  SLOT(slotCrop()),
                                  QKeySequence(CTRL+Key_C)));

    d->actions.insert("print",
                      new CAction(i18n("Print Image..."),
                                  this,
                                  SLOT(slotPrintImage()),
                                  QKeySequence(CTRL+Key_P)));
    
    d->actions.insert("save",
                      new CAction(i18n("Save"),
                                  this,
                                  SLOT(slotSave()),
                                  QKeySequence(CTRL+Key_S)));
                                   
    d->actions.insert("saveas",
                      new CAction(i18n("Save As..."),
                                  this,
                                  SLOT(slotSaveAs()),
                                  QKeySequence(CTRL+SHIFT+Key_S)));                                  

    d->actions.insert("restore",
                      new CAction(i18n("Restore"),
                                  d->canvas,
                                  SLOT(slotRestore()),
                                  QKeySequence(CTRL+Key_R)));

    if ( fromCameraUIFlag == false )
       {
       d->actions.insert("remove",
                      new CAction(i18n("Remove from Album"),
                                  this,
                                  SLOT(slotRemoveCurrentItemfromAlbum()),
                                  QKeySequence(SHIFT+Key_Delete)));
       }

    d->actions.insert("close",
                      new CAction(i18n("Close"),
                                  this,
                                  SLOT(slotClose()),
                                  QKeySequence(CTRL+Key_Q)));

    d->actions.insert("help",
                      new CAction(i18n("Help"),
                                  this,
                                  SLOT(slotHelp()),
                                  QKeySequence(Key_F1)));
    
    d->actions.insert("about",
                      new CAction(i18n("About"),
                                  this,
                                  SLOT(slotAbout()),
                                  QKeySequence(CTRL+SHIFT+Key_A)));
                                  
    // Now insert these keys into the keydict

    d->actionKeys.setAutoDelete(false);
    d->actionKeys.clear();

    addKeyInDict("prev");
    addKeyInDict("next");
    addKeyInDict("zoomIn");
    addKeyInDict("zoomOut");
    addKeyInDict("zoom1");
    addKeyInDict("toggleAutoZoom");
    addKeyInDict("toggleFullScreen");
    addKeyInDict("rotate");
    addKeyInDict("rotate90");
    addKeyInDict("rotate180");
    addKeyInDict("rotate270");
    addKeyInDict("flip");
    addKeyInDict("fliphorizontal");
    addKeyInDict("flipvertical");
    addKeyInDict("bcgEdit");
    
    if ( fromCameraUIFlag == false )
       addKeyInDict("commentsEdit");
       
    addKeyInDict("ExifInfo");
    addKeyInDict("ImageProperties");
    addKeyInDict("crop");
    addKeyInDict("print");
    addKeyInDict("save");
    addKeyInDict("saveas");
    addKeyInDict("restore");
    
    if ( fromCameraUIFlag == false )
       addKeyInDict("remove");
    
    addKeyInDict("close");

    d->actionKeys.insert(QKeySequence(Key_Escape),
                         d->actions.find("close"));
    
    addKeyInDict("help");
    addKeyInDict("about");
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setupPopupMenu()
{
    // Setup the rotate menu

    d->rotateMenu = new QPopupMenu(this);

    addMenuItem(d->rotateMenu, d->actions.find("rotate90"));
    addMenuItem(d->rotateMenu, d->actions.find("rotate180"));
    addMenuItem(d->rotateMenu, d->actions.find("rotate270"));

    // Setup the flip menu

    d->flipMenu = new QPopupMenu(this);

    addMenuItem(d->flipMenu, d->actions.find("fliphorizontal"));
    addMenuItem(d->flipMenu, d->actions.find("flipvertical"));
    
    // Setup the context menu

    d->contextMenu = new QPopupMenu(this);

    addMenuItem(d->contextMenu, d->actions.find("next"));
    addMenuItem(d->contextMenu, d->actions.find("prev"));

    d->contextMenu->insertSeparator();

    addMenuItem(d->contextMenu, d->actions.find("zoomIn"));
    addMenuItem(d->contextMenu, d->actions.find("zoomOut"));
    addMenuItem(d->contextMenu, d->actions.find("zoom1"));
    addMenuItem(d->contextMenu, d->actions.find("toggleAutoZoom"));
    addMenuItem(d->contextMenu, d->actions.find("toggleFullScreen"));
    
    d->contextMenu->insertSeparator();

    d->contextMenu->insertItem(i18n("Rotate Image"), d->rotateMenu);
    d->contextMenu->insertItem(i18n("Flip Image"), d->flipMenu);

    addMenuItem(d->contextMenu, d->actions.find("crop"));

    addMenuItem(d->contextMenu, d->actions.find("bcgEdit"));

    d->contextMenu->insertSeparator();
    
    if ( fromCameraUIFlag == false )
       addMenuItem(d->contextMenu, d->actions.find("commentsEdit"));
       
    addMenuItem(d->contextMenu, d->actions.find("ExifInfo"));
    addMenuItem(d->contextMenu, d->actions.find("ImageProperties"));

    d->contextMenu->insertSeparator();
    
    addMenuItem(d->contextMenu, d->actions.find("print"));
    addMenuItem(d->contextMenu, d->actions.find("save"));
    addMenuItem(d->contextMenu, d->actions.find("saveas"));
    addMenuItem(d->contextMenu, d->actions.find("restore"));
    
    if ( fromCameraUIFlag == false )
       addMenuItem(d->contextMenu, d->actions.find("remove"));

    d->contextMenu->insertSeparator();

    addMenuItem(d->contextMenu, d->actions.find("close"));
    addMenuItem(d->contextMenu, d->actions.find("help"));
    addMenuItem(d->contextMenu, d->actions.find("about"));

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
    if (action) 
       {
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
    d->bPrev = new CButton(d->buttonBar2,
                             d->actions.find("prev"),
                             BarIcon("back"));
    d->buttonLayout2->addWidget(d->bPrev);

    d->bNext = new CButton(d->buttonBar2,
                             d->actions.find("next"),
                             BarIcon("forward"));
    d->buttonLayout2->addWidget(d->bNext);

    QHBox *labelBox = new QHBox(d->buttonBar2);
    labelBox->setMargin(0);
    labelBox->setSpacing(2);
    
    d->buttonLayout2->addSpacing(5);
    d->buttonLayout2->addWidget(labelBox);
    d->buttonLayout2->addSpacing(5);
    
    d->nameComboBox = new QComboBox(labelBox);
  
    m_thumbJob = new Digikam::ThumbnailJob(d->urlList, 32, false);
    
    connect(m_thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
            SLOT(slotGotPreview(const KURL&, const QPixmap&)));
    
    connect(m_thumbJob, SIGNAL(signalCompleted()),
            SLOT(slotPreviewCompleted()));            
            
    labelBox->setStretchFactor(d->nameComboBox, 5);
    
    d->countLabel = new QLabel(labelBox);
    d->countLabel->setPaletteBackgroundColor(Qt::white);
    d->countLabel->setFrameShape(QFrame::Box);
    labelBox->setStretchFactor(d->countLabel, 1);

    d->zoomLabel = new QLabel(labelBox);
    d->zoomLabel->setPaletteBackgroundColor(Qt::white);
    d->zoomLabel->setFrameShape(QFrame::Box);
    labelBox->setStretchFactor(d->zoomLabel, 1);
    
    d->bZoomIn = new CButton(d->buttonBar2,
                             d->actions.find("zoomIn"),
                             BarIcon("viewmag+"));
    d->buttonLayout2->addWidget(d->bZoomIn);

    d->bZoomOut = new CButton(d->buttonBar2,
                             d->actions.find("zoomOut"),
                             BarIcon("viewmag-"));
    d->buttonLayout2->addWidget(d->bZoomOut);

    d->bZoomAuto = new CButton(d->buttonBar2,
                             d->actions.find("toggleAutoZoom"),
                             BarIcon("viewmagfit"),
                             true, true);
    d->buttonLayout2->addWidget(d->bZoomAuto);

    d->bZoom1 = new CButton(d->buttonBar2,
                             d->actions.find("zoom1"),
                             BarIcon("viewmag1"));
    d->buttonLayout2->addWidget(d->bZoom1);

    //-----------------------------------------------------------------
    
    d->bFullScreen = new CButton(d->buttonBar,
                             d->actions.find("toggleFullScreen"),
                             BarIcon("window_fullscreen"),
                             true, true);
    d->buttonLayout->addWidget(d->bFullScreen);

    d->bBCGEdit = new CButton(d->buttonBar,
                             d->actions.find("bcgEdit"),
                             BarIcon("blend"));
    d->buttonLayout->addWidget(d->bBCGEdit);
   
    d->bRotate = new CButton(d->buttonBar,
                             d->actions.find("rotate"),
                             BarIcon("rotate_cw"));
    d->buttonLayout->addWidget(d->bRotate);

    d->bFlip = new CButton(d->buttonBar,
                             d->actions.find("flip"),
                             BarIcon("flip_image"));
    d->buttonLayout->addWidget(d->bFlip);
    
    d->bCrop  = new CButton(d->buttonBar,
                             d->actions.find("crop"),
                             BarIcon("crop"),
                             false);
    d->buttonLayout->addWidget(d->bCrop);
        
    if ( fromCameraUIFlag == false )
       {
       d->bCommentsEdit = new CButton(d->buttonBar,
                             d->actions.find("commentsEdit"),
                             BarIcon("imagecomment"));
       d->buttonLayout->addWidget(d->bCommentsEdit);
       }

    d->bExifInfo = new CButton(d->buttonBar,
                             d->actions.find("ExifInfo"),
                             BarIcon("exifinfo"));
    d->buttonLayout->addWidget(d->bExifInfo);

    d->bImageProperties = new CButton(d->buttonBar,
                             d->actions.find("ImageProperties"),
                             BarIcon("image"));
    d->buttonLayout->addWidget(d->bImageProperties);
    
    d->bPrint = new CButton(d->buttonBar,
                             d->actions.find("print"),
                             BarIcon("fileprint"));
    d->buttonLayout->addWidget(d->bPrint);
    
    d->bSave  = new CButton(d->buttonBar,
                             d->actions.find("save"),
                             BarIcon("filesave"),
                             false);
    d->buttonLayout->addWidget(d->bSave);

    d->bSaveAs  = new CButton(d->buttonBar,
                             d->actions.find("saveas"),
                             BarIcon("filesaveas"),
                             true);
    d->buttonLayout->addWidget(d->bSaveAs);

    d->bRestore  = new CButton(d->buttonBar,
                             d->actions.find("restore"),
                             BarIcon("undo"),
                             false);
    d->buttonLayout->addWidget(d->bRestore);

    if ( fromCameraUIFlag == false )
       {
       d->bRemoveCurrent  = new CButton(d->buttonBar,
                             d->actions.find("remove"),
                             BarIcon("editdelete"));
       d->buttonLayout->addWidget(d->bRemoveCurrent);
       }

    d->bClose = new CButton(d->buttonBar,
                             d->actions.find("close"),
                             BarIcon("exit"));
    d->buttonLayout->addWidget(d->bClose);

    d->buttonLayout->insertStretch(-1);
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
    if (d->bSave->isEnabled()) 
        {
        int result =
            KMessageBox::warningYesNo(this,                                      
                                      i18n("The image \"%1\" has been modified.\n"
                                           "Do you want to save it?")
                                           .arg(d->urlCurrent.filename()));
        if (result == KMessageBox::Yes)
            slotSave();
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////

bool ImageView::printImageWithQt( const QString& filename, KPrinter& printer,
                                  const QString& originalFileName )
{
    QImage image( filename );
    
    if ( image.isNull() ) 
        {
        kdWarning() << "Can't load image: " << filename << " for printing.\n";
        return false;
        }

    QPainter p;
    p.begin( &printer );

    QPaintDeviceMetrics metrics( &printer );
    p.setFont( KGlobalSettings::generalFont() );
    QFontMetrics fm = p.fontMetrics();

    int w = metrics.width();
    int h = metrics.height();

    QString t = "true";
    QString f = "false";

    // Black & white print ?
    
    if ( printer.option( "app-imageviewer-blackwhite" ) != f) 
        {
        image = image.convertDepth( 1, Qt::MonoOnly | Qt::ThresholdDither | Qt::AvoidDither );
        }

    int filenameOffset = 0;
    bool printFilename = printer.option( "app-imageviewer-printFilename" ) != f;
    
    if ( printFilename ) 
        {
        filenameOffset = fm.lineSpacing() + 14;
        h -= filenameOffset; // filename goes into one line!
        }

    //
    // shrink image to pagesize, if necessary
    //
    
    bool shrinkToFit = (printer.option( "app-imageviewer-shrinkToFit" ) != f);
    
    if ( shrinkToFit && image.width() > w || image.height() > h )
        {
        image = image.smoothScale( w, h, QImage::ScaleMin );
        }

    //
    // align image
    //
    
    bool ok = false;
    int alignment = printer.option("app-imageviewer-alignment").toInt( &ok );
    
    if ( !ok )
        alignment = Qt::AlignCenter; // default

    int x = 0;
    int y = 0;

    // ### need a GUI for this in KuickPrintDialogPage!
    // x - alignment
    
    if ( alignment & Qt::AlignHCenter )
        x = (w - image.width())/2;
    else if ( alignment & Qt::AlignLeft )
        x = 0;
    else if ( alignment & Qt::AlignRight )
        x = w - image.width();

    // y - alignment
    if ( alignment & Qt::AlignVCenter )
        y = (h - image.height())/2;
    else if ( alignment & Qt::AlignTop )
        y = 0;
    else if ( alignment & Qt::AlignBottom )
        y = h - image.height();

    //
    // perform the actual drawing
    //
    
    p.drawImage( x, y, image );

    if ( printFilename )
        {
        QString fname = minimizeString( originalFileName, fm, w );
        
        if ( !fname.isEmpty() )
            {
            int fw = fm.width( fname );
            int x = (w - fw)/2;
            int y = metrics.height() - filenameOffset/2;
            p.drawText( x, y, fname );
            }
         }

    p.end();

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////

QString ImageView::minimizeString( QString text, const QFontMetrics& metrics,
                                   int maxWidth )
{
    if ( text.length() <= 5 )
        return QString::null; // no sense to cut that tiny little string

    bool changed = false;
    
    while ( metrics.width( text ) > maxWidth )
        {
        int mid = text.length() / 2;
        text.remove( mid, 2 ); // remove 2 characters in the middle
        changed = true;
        }

    if ( changed ) // add "..." in the middle
        {
        int mid = text.length() / 2;
        
        if ( mid <= 5 ) // sanity check
            return QString::null;

        text.replace( mid - 1, 3, "..." );
        }

    return text;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageViewPrintDialogPage::getOptions( QMap<QString,QString>& opts,
                                           bool /*incldef*/ )
{
    QString t = "true";
    QString f = "false";

//    ### opts["app-imageviewer-alignment"] = ;
    opts["app-imageviewer-printFilename"] = m_addFileName->isChecked() ? t : f;
    opts["app-imageviewer-blackwhite"] = m_blackwhite->isChecked() ? t : f;
    opts["app-imageviewer-shrinkToFit"] = m_shrinkToFit->isChecked() ? t : f;
    opts["app-imageviewer-scale"] = m_scale->isChecked() ? t : f;
    opts["app-imageviewer-scale-unit"] = m_units->currentText();
    opts["app-imageviewer-scale-width-pixels"] = QString::number( scaleWidth() );
    opts["app-imageviewer-scale-height-pixels"] = QString::number( scaleHeight() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageViewPrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
    QString t = "true";
    QString f = "false";

    m_addFileName->setChecked( opts["app-imageviewer-printFilename"] != f );
    // This sound strange, but if I copy the code on the line above, the checkbox
    // was always checked. And this isn't the wanted behavior. So, with this works.
    // KPrint magic ;-)
    m_blackwhite->setChecked ( false );
    m_shrinkToFit->setChecked( opts["app-imageviewer-shrinkToFit"] != f );
    m_scale->setChecked( opts["app-imageviewer-scale"] == t );

    m_units->setCurrentItem( opts["app-imageviewer-scale-unit"] );

    bool ok;
    int val = opts["app-imageviewer-scale-width-pixels"].toInt( &ok );
    
    if ( ok )
        setScaleWidth( val );
    
    val = opts["app-imageviewer-scale-height-pixels"].toInt( &ok );
    
    if ( ok )
        setScaleHeight( val );

    if ( m_scale->isChecked() == m_shrinkToFit->isChecked() )
        m_shrinkToFit->setChecked( !m_scale->isChecked() );

    // ### re-enable when implementednn
    toggleScaling( false && m_scale->isChecked() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageViewPrintDialogPage::toggleScaling( bool enable )
{
    m_width->setEnabled( enable );
    m_height->setEnabled( enable );
    m_units->setEnabled( enable );
}


/////////////////////////////////////////////////////////////////////////////////////////////

int ImageViewPrintDialogPage::scaleWidth() const
{
    return fromUnitToPixels( m_width->value() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

int ImageViewPrintDialogPage::scaleHeight() const
{
    return fromUnitToPixels( m_height->value() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageViewPrintDialogPage::setScaleWidth( int pixels )
{
    m_width->setValue( (int) pixelsToUnit( pixels ) );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageViewPrintDialogPage::setScaleHeight( int pixels )
{
    m_width->setValue( (int) pixelsToUnit( pixels ) );
}


/////////////////////////////////////////////////////////////////////////////////////////////

int ImageViewPrintDialogPage::fromUnitToPixels( float /*value*/ ) const
{
    return 1; // ###
}


/////////////////////////////////////////////////////////////////////////////////////////////

float ImageViewPrintDialogPage::pixelsToUnit( int /*pixels*/ ) const
{
    return 1.0; // ###
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setNextAction(bool val)
{
    d->bNext->setEnabled(val);
    CAction *action = 0;
    action = d->actions.find("next");
    d->contextMenu->setItemEnabled(action->menuID, val);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::setPrevAction(bool val)
{
    d->bPrev->setEnabled(val);
    CAction *action = 0;
    action = d->actions.find("prev");
    d->contextMenu->setItemEnabled(action->menuID, val);
}


//////////////////////////////////////// SLOTS //////////////////////////////////////////////

void ImageView::slotNextImage()
{
    promptUserSave();

    KURL::List::iterator it = d->urlList.find(d->urlCurrent);

    if (it != d->urlList.end()) 
        {
        d->preloadNext = true;

        if (d->urlCurrent != d->urlList.last()) 
           {
           KURL urlNext = *(++it);
           d->urlCurrent = urlNext;
           loadCurrentItem();
           }
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotPrevImage()
{
    promptUserSave();

    KURL::List::iterator it = d->urlList.find(d->urlCurrent);

    if (it != d->urlList.begin()) 
         {
         d->preloadNext = true;

         if (d->urlCurrent != d->urlList.first()) 
             {
             KURL urlPrev = *(--it);
             d->urlCurrent = urlPrev;
             loadCurrentItem();
             }
         }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotRemoveCurrentItemfromAlbum()
{
    KURL currentImage = d->urlCurrent;

    QString warnMsg(i18n("About to delete \"%1\"\nfrom Album\n\"%2\"\nAre you sure?")
                    .arg(d->urlCurrent.filename())
                    .arg(d->urlCurrent.path().section('/', -2, -2)));

    if (KMessageBox::warningContinueCancel(this,
                                           warnMsg,
                                           i18n("Warning"),
                                           i18n("Delete"))
        ==  KMessageBox::Continue) 
           {
           loadCurrentItem();
           
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
          d->nameComboBox->removeItem ( d->nameComboBox->currentItem() );
          loadCurrentItem();
          return;
          }

       // Try to get the preview image in the current Album...

       else if (d->urlCurrent != d->urlList.first())
          {
          KURL urlPrev = *(--it);
          d->urlCurrent = urlPrev;
          d->urlList.remove(CurrentToRemove);
          d->nameComboBox->removeItem ( d->nameComboBox->currentItem() );
          loadCurrentItem();
          return;
          }

      // No image in the current Album -> Quit ImageViever...

      else
         {
         d->bPrev->setEnabled(false);    
         d->bNext->setEnabled(false);         
         KMessageBox::information(this,
                                  i18n("There is no image to show in the current Album!\n"
                                       "The ImageViewer will be closed..."),
                                  i18n("No image in the current Album"));

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

void ImageView::slotShowFlipMenu()
{
    d->flipMenu->exec(QCursor::pos());
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
    
    QString tmpFile = locateLocal("tmp", d->urlCurrent.filename());
    
    int result = d->canvas->save(tmpFile);

    if (result != 1) 
        {
        KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\"")
                                 .arg(d->urlCurrent.filename())
                                 .arg(d->urlCurrent.path().section('/', -2, -2)));
        loadCurrentItem();
        return;
        }

    ExifRestorer exifHolder;
    exifHolder.readFile(d->urlCurrent.path(), ExifRestorer::ExifOnly);

    if (exifHolder.hasExif()) 
        {
        ExifRestorer restorer;
        restorer.readFile(tmpFile, ExifRestorer::EntireImage);
        restorer.insertExifData(exifHolder.exifData());
        restorer.writeFile(tmpFile);
        }
    else 
        {
        qWarning("No Exif Data Found");
        }
    
    if( rotatedOrFlipped )
    {
       KExifData *exifData = new KExifData;
       exifData->writeOrientation(tmpFile, KExifData::NORMAL);
       delete exifData;
    }

    KIO::FileCopyJob* job = KIO::file_move(KURL(tmpFile), d->urlCurrent,
                                           -1, true, false, false);

    connect(job, SIGNAL(result(KIO::Job *) ),
            this, SLOT(slotSaveResult(KIO::Job *)));

}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotSaveAs()
 {
     if (!d->urlCurrent.isValid()) return;
     
     // Get the new filename. 
     
     QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Writing );
     
     newFile = KURL(KFileDialog::getSaveFileName(d->urlCurrent.directory(),
                                                 mimetypes.join(" "),
                                                 this,
                                                 i18n("New image filename")));
 
     // Check for cancel.
     
     if (!newFile.isValid()) return;
 
     QString tmpFile = locateLocal("tmp", d->urlCurrent.filename());
     
     int result = d->canvas->saveAs(tmpFile);
 
     if (result != 1) 
         {
         KMessageBox::error(this, i18n("Failed to save file\n\"%1\" to Album\n\"%2\"")
                                 .arg(newFile.filename())
                                 .arg(newFile.path().section('/', -2, -2)));
         loadCurrentItem();                                 
         return;
         }
     
     ExifRestorer exifHolder;
     exifHolder.readFile(d->urlCurrent.path(), ExifRestorer::ExifOnly);
 
     if (exifHolder.hasExif()) 
         {
         ExifRestorer restorer;
         restorer.readFile(tmpFile, ExifRestorer::EntireImage);
         restorer.insertExifData(exifHolder.exifData());
         restorer.writeFile(tmpFile);
         }
     else 
         {
         qWarning("No Exif Data Found");
         }
    
    if( rotatedOrFlipped )
    {
       KExifData *exifData = new KExifData;
       exifData->writeOrientation(tmpFile, KExifData::NORMAL);
       delete exifData;
    }
 
     KIO::FileCopyJob* job = KIO::file_move(KURL(tmpFile), newFile,
                                            -1, true, false, false);
  
     connect(job, SIGNAL(result(KIO::Job *) ),
             this, SLOT(slotSaveAsResult(KIO::Job *)));
}

 
/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotSaveResult(KIO::Job *job)
{
    if (job->error()) 
       {
       job->showErrorDialog(this);
       return;
       }
    
    loadCurrentItem();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotSaveAsResult(KIO::Job *job)
{
    if (job->error()) 
       {
       job->showErrorDialog(this);
       return;
       }

    // Added new file URL into list if the new file have been added in the current Album
    // and added the comments if exists.

    Digikam::AlbumInfo *sourceAlbum = Digikam::AlbumManager::instance()
                                      ->findAlbum( d->urlCurrent.path().section('/', -2, -2) );
    Digikam::AlbumInfo *targetAlbum = Digikam::AlbumManager::instance()
                                      ->findAlbum( newFile.path().section('/', -2, -2) );

    if (targetAlbum && sourceAlbum)     // The target Album is in the database ?
       {
       // Copy the comments from the original image to the target image.
       
       sourceAlbum->openDB();
       QString comments = sourceAlbum->getItemComments(d->urlCurrent.filename());
       sourceAlbum->closeDB();

       targetAlbum->openDB();
       targetAlbum->setItemComments(newFile.filename(), comments);
       targetAlbum->closeDB();
       
       if ( d->urlCurrent.directory() == newFile.directory() &&  // Target Album = current Album ?
            d->urlList.find(newFile) == d->urlList.end() )       // The image file not already exist
                                                                 // in the list.            
          {
          d->canvas->slotRestore();
          d->canvas->load(newFile.path());
          KURL::List::iterator it = d->urlList.find(d->urlCurrent);
          d->urlList.insert(it, newFile);
          d->urlCurrent = newFile;
          m_thumbJob = new Digikam::ThumbnailJob(d->urlCurrent, 32, false, false);
    
          connect(m_thumbJob, SIGNAL(signalThumbnail(const KURL&, const QPixmap&)),
                  SLOT(slotGotPreview(const KURL&, const QPixmap&)));
          }
       }

    loadCurrentItem(); // Load the new target images.
}
 

/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotToggleAutoZoom()
{
    bool val;
    
    if (d->canvas->autoZoomOn()) 
        {
        d->canvas->slotSetAutoZoom(false);
        val = true;
        }
    else 
        {
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
    if (d->fullScreen) 
        {
        showNormal();
        d->fullScreen = false;
        move(0, 0);
        }
    else 
        {
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

void ImageView::slotRotatedOrFlipped()
{
   rotatedOrFlipped = true;
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
    
    d->canvas->ajustInit();
    
    connect(bcgEdit, SIGNAL(signalGammaValueChanged(int)),
            d->canvas, SLOT(slotGammaChanged(int)));
            
    connect(bcgEdit, SIGNAL(signalBrightnessValueChanged(int)),
            d->canvas, SLOT(slotBrightnessChanged(int)));
            
    connect(bcgEdit, SIGNAL(signalContrastValueChanged(int)),
            d->canvas, SLOT(slotContrastChanged(int)));
    
    if ( bcgEdit->exec() == KMessageBox::Ok )
       d->canvas->ajustAccepted();
    else
       d->canvas->ajustRejected();           
       
    delete bcgEdit;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotCommentsEdit()
{
    Digikam::AlbumInfo *currentAlbum = Digikam::AlbumManager::instance()
                                      ->findAlbum( d->urlCurrent.path().section('/', -2, -2) );

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
    else 
        {
        delete exif;
        KMessageBox::sorry(this,
                           i18n("This item has no Exif Information"));
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotKeyPress(int key)
{
    QKeySequence keyPressed(key);
    CAction *action = d->actionKeys.find(QString(keyPressed));
    
    if (!action) return;

    if (action->button) 
        {
        CButton *button = (CButton*) action->button;
        button->animateClick();
        }
    else
        action->activate();
    
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotPrintImage()
{
    KPrinter printer;
    printer.setDocName( d->urlCurrent.filename() );
    printer.setCreator( "Digikam-ImageViewer");

    KPrinter::addDialogPage( new ImageViewPrintDialogPage( this, "imageviewer page"));

    if ( printer.setup( this, i18n("Print %1").arg(printer.docName().section('/', -1)) ) )
        {
        KTempFile tmpFile( "imageviewer", ".png" );
        
        if ( tmpFile.status() == 0 )
            {
            tmpFile.setAutoDelete( true );
            
            if ( d->canvas->save(tmpFile.name()) )
                printImageWithQt( tmpFile.name(), printer, d->urlCurrent.filename() );
            }
        }
    
    loadCurrentItem();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotImageProperties()
{
    KURL url;
    url.setPath( d->urlCurrent.path() );
    (void) new KPropertiesDialog( url, this, "props dialog", true );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotImageNameActived( const QString & filename )
{
    promptUserSave();
    
    for ( KURL::List::iterator it = d->urlList.begin() ; it != d->urlList.end() ; ++it )
        {
        KURL name = *it;

        if ( name.filename() == filename )
           d->urlCurrent = *it;
        }

    loadCurrentItem();
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotHelp( void )
{
    KApplication::kApplication()->invokeHelp( "imageviewer.anchor", "digikam" );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotAbout( void )
{
    KMessageBox::about(this, i18n("An image viewer/editor for Digikam\n\n"
                                  "Authors: Renchi Raju and Gilles Caulier\n\n"
                                  "Emails:\n"
                                  "renchi at pooh.tam.uiuc.edu\n"
                                  "caulier dot gilles at free.fr\n\n"
                                  "This program use 'imlib2' API for all images rendering.\n"),
                                  i18n("About Digikam image viewer"));
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotGotPreview(const KURL &url, const QPixmap &pixmap)
{
    d->nameComboBox->insertItem( pixmap, url.filename() );
}


/////////////////////////////////////////////////////////////////////////////////////////////

void ImageView::slotPreviewCompleted(void)
{
    loadCurrentItem();
}

#include "imageview.moc"
