/***************************************************************************
                          digikamapp.h  -  description
                             -------------------
    begin                : Sat Nov 16 10:11:43 CST 2002
    copyright            : (C) 2002 by Renchi Raju
    email                : renchi@pooh.tam.uiuc.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIGIKAMAPP_H
#define DIGIKAMAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <kmainwindow.h>
#include <qstring.h>
#include <qmap.h>


class KAction;
class KActionMenu;
class KSelectAction;

class CameraList;
class CameraType;
class DigikamView;
class AlbumSettings;
class DigikamPluginManager;

namespace Digikam
{
class AlbumManager;
}

class DigikamApp : public KMainWindow
{
    Q_OBJECT

public:

    DigikamApp();
    ~DigikamApp();

    void enableThumbSizePlusAction(bool val);
    void enableThumbSizeMinusAction(bool val);

private:

    void setupView();
    void setupActions();


protected:

    bool queryClose();

private:

    Digikam::AlbumManager *mAlbumManager;
    DigikamPluginManager  *pluginManager_;
    
    DigikamView*  mView;
    CameraList*   mCameraList;
    bool          mFullScreen;

    // Album Settings
    AlbumSettings* mAlbumSettings;

    // Camera Actions
    KActionMenu *mCameraMenuAction;
 
    // Album Actions
    KAction *mNewAction;
    KAction *mDeleteAction;
    KSelectAction *mAlbumSortAction;

    KAction *mAddImagesAction;
    KAction *mPropsEditAction;

    // Image Actions
    KAction *mImageViewAction;
    KAction *mImageCommentsAction;
    KAction *mImageExifAction;
    KAction *mImageRenameAction;
    KAction *mImageDeleteAction;
    KAction *mImagePropsAction;

    // Selection Actions
    KAction *mSelectAllAction;
    KAction *mSelectNoneAction;
    KAction *mSelectInvertAction;

    // View Actions
    KAction *mThumbSizePlusAction;
    KAction *mThumbSizeMinusAction;
    KAction *mFullScreenAction;

    // Application Actions
    KAction* mQuitAction;
    KAction* mTipAction;

private slots:

    void slot_albumSelected(bool val);
    void slot_imageSelected(bool val);
    void slot_exit();
    void slotShowTip();
    
    void slotSetup();
    void slotSetupChanged();
    
    void slotCameraConnect();
    void slotCameraAdded(CameraType *ctype);
    void slotCameraRemoved(CameraType *ctype);
    void slotEditKeys();
    void slotConfToolbars();
    void slotToggleFullScreen();
};

#endif
