//////////////////////////////////////////////////////////////////////////////
//
//    IMAGEVIEW.H
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

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

// Qt lib includes

#include <qwidget.h>

// KDE lib includes

#include <kurl.h>
#include <kio/job.h>

class QPopupMenu;
class QCloseEvent;

class CAction;
class ImageViewPrivate;

class ImageView : public QWidget {

    Q_OBJECT

public:

    // For a list of items
    ImageView(QWidget* parent, const KURL::List& urlList,
              const KURL& urlCurrent);

    // For a single item
    ImageView(QWidget* parent, const KURL& urlCurrent);
    
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

    ImageViewPrivate *d;
    
    KURL              newFile;

protected:

    void closeEvent(QCloseEvent *e);

private slots:

    void slotNextImage();
    void slotPrevImage();
    void slotShowRotateMenu();
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
};

#endif // IMAGEVIEW_H 
