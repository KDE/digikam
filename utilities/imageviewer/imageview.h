//////////////////////////////////////////////////////////////////////////////
//
//    IMAGEVIEW.H
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

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

// Qt lib includes

#include <qwidget.h>
#include <qfontmetrics.h>
#include <qstring.h>
#include <qguardedptr.h>

// KDE lib includes

#include <kurl.h>
#include <kio/job.h>
#include <kdeprint/kprintdialogpage.h>

// local includes.

#include "thumbnailjob.h"

class QPopupMenu;
class QCloseEvent;
class QCheckBox;
class QRadioButton;

class KComboBox;
class KPrinter;
class KIntNumInput;

class CAction;
class ImageViewPrivate;

class ImageView : public QWidget 
{
    Q_OBJECT

public:

    // For a list of items
    ImageView(QWidget* parent, const KURL::List& urlList,
              const KURL& urlCurrent, bool fromCameraUI=false);

    // For a single item
    ImageView(QWidget* parent, const KURL& urlCurrent,
              bool fromCameraUI=false);
    
    ~ImageView();

private:

    void init();
    void initGui();
    void readSettings();
    void saveSettings();
    void setupConnections();
    void setupActions();
    void setupButtons();
    void setupPopupMenu();
    void addMenuItem(QPopupMenu *menu, CAction *action);
    void addKeyInDict(const QString& key);
    void promptUserSave();
    void loadCurrentItem();
    void setPrevAction(bool val);
    void setNextAction(bool val);
    
    // For printing. 
    
    bool printImageWithQt( const QString& filename, KPrinter& printer,
                           const QString& originalFileName );
    void addConfigPages();
    QString minimizeString( QString text, const QFontMetrics& metrics,
                            int maxWidth );                           

    ImageViewPrivate *d;
    
    KURL              newFile;
    bool              fromCameraUIFlag;     // Flag used for to limit ImageViewer options 
                                            // when the images are opened from the cameraUI 
                                            // interface (like 'Image Comments Editor'
                                            // and 'Remove From Album').
                                            
    QGuardedPtr<Digikam::ThumbnailJob> m_thumbJob;                                            

protected:

    void closeEvent(QCloseEvent *e);

private slots:

    void slotNextImage();
    void slotPrevImage();
    void slotShowRotateMenu();
    void slotShowFlipMenu();
    void slotShowContextMenu();
    void slotSave();
    void slotSaveAs();
    void slotSaveResult(KIO::Job *job);
    void slotSaveAsResult(KIO::Job *job);
    void slotToggleAutoZoom();
    void slotToggleFullScreen();
    void slotZoomChanged(double zoom);
    void slotCropSelected(bool val);
    void slotChanged(bool val);
    void slotClose();
    void slotBCGEdit();
    void slotCommentsEdit();
    void slotExifInfo();
    void slotRemoveCurrentItemfromAlbum();
    void slot_onDeleteCurrentItemFinished(KIO::Job *job);
    void slotKeyPress(int key);
    void slotPrintImage();
    void slotImageProperties();
    void slotImageNameActived(const QString & filename);
    void slotHelp(void);
    void slotAbout(void);
    void slotGotPreview(const KURL &url, const QPixmap &pixmap);
};


///////////////////////////////////////////////////////////////////////////////////

class ImageViewPrintDialogPage : public KPrintDialogPage
{
    Q_OBJECT

public:

    ImageViewPrintDialogPage( QWidget *parent = 0L, const char *name = 0 );
    ~ImageViewPrintDialogPage();

    virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
    virtual void setOptions(const QMap<QString,QString>& opts);

private slots:

    void toggleScaling( bool enable );

private:

    // return values in pixels!
    int scaleWidth() const;
    int scaleHeight() const;

    void setScaleWidth( int pixels );
    void setScaleHeight( int pixels );

    int fromUnitToPixels( float val ) const;
    float pixelsToUnit( int pixels ) const;

    QCheckBox *m_shrinkToFit;
    QRadioButton *m_scale;
    KIntNumInput *m_width;
    KIntNumInput *m_height;
    KComboBox *m_units;
    QCheckBox *m_addFileName;
    QCheckBox *m_blackwhite;
};

#endif // IMAGEVIEW_H 
