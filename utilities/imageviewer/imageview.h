/* ============================================================
 * File  : imageview.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-11
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <qwidget.h>
#include <kurl.h>
#include <kio/job.h>

class QPopupMenu;
class QCloseEvent;
class CAction;
class ImageViewPrivate;

class ImageView : public QWidget {

    Q_OBJECT

public:

    // for a list of items
    ImageView(QWidget* parent, const KURL::List& urlList,
              const KURL& urlCurrent);

    // for a single item
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

protected:

    void closeEvent(QCloseEvent *e);

private slots:

    void slotNextImage();
    void slotPrevImage();
    void slotShowRotateMenu();
    void slotShowContextMenu();
    void slotSave();
    void slotToggleAutoZoom();
    void slotToggleFullScreen();
    void slotZoomChanged(double zoom);
    void slotCropSelected(bool val);
    void slotChanged(bool val);
    void slotClose();
    void slotSaveResult(KIO::Job *job);
    void slotBCGEdit();
    void slotRemoveCurrentItemfromAlbum();
    void slot_onDeleteCurrentItemFinished(KIO::Job *job);
    void slotKeyPress(int key);
};

#endif /* IMAGEVIEW_H */
