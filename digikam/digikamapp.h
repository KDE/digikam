//////////////////////////////////////////////////////////////////////////////
//
//    DIGIKAMAPP.H
//
//    Copyright (C) 2002-2005 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
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

#ifndef DIGIKAMAPP_H
#define DIGIKAMAPP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes.

#include <qstring.h>
#include <qmap.h>

// KDE includes.

#include <kapplication.h>
#include <kmainwindow.h>
#include "digikam_export.h"

namespace KIPI
{
class PluginLoader;
}

class AlbumManager;
class PluginLoader;               //   For KIPI pluggins support.
class DigikamKipiInterface;
class ImagePluginLoader;

class KAction;
class KActionMenu;
class KToolBarPopupAction;
class KSelectAction;
class KConfig;

class CameraList;
class CameraType;
class DigikamView;
class AlbumSettings;
class SplashScreen;

class DIGIKAM_EXPORT DigikamApp : public KMainWindow
{
    Q_OBJECT

public:

    DigikamApp(bool detectCamera=false);
    ~DigikamApp();

    virtual void show();
    
    static DigikamApp* getinstance();

    // KIPI Actions collections access.
    const QPtrList<KAction>& menuImageActions();
    const QPtrList<KAction>& menuBatchActions();
    const QPtrList<KAction>& menuAlbumActions();
    
    const QPtrList<KAction> menuImportActions();

    void enableThumbSizePlusAction(bool val);
    void enableThumbSizeMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);
    
private:

    void setupView();
    void setupActions();
    void loadPlugins();
    void loadCameras();
    void populateThemes();
    void updateDeleteTrashMenu();

protected:

    bool queryClose();

private:

    static DigikamApp     *m_instance;
    AlbumManager          *mAlbumManager;
    
    ImagePluginLoader     *m_ImagePluginsLoader;
    
    // For KIPI plugins support 
    KIPI::PluginLoader    *KipiPluginLoader_;
    DigikamKipiInterface  *KipiInterface_;
    QPtrList<KAction>      m_kipiFileActionsExport;
    QPtrList<KAction>      m_kipiFileActionsImport;
    QPtrList<KAction>      m_kipiImageActions;
    QPtrList<KAction>      m_kipiToolsActions;
    QPtrList<KAction>      m_kipiBatchActions;
    QPtrList<KAction>      m_kipiAlbumActions;

    KConfig               *m_config;    
    
    DigikamView           *mView;
    CameraList            *mCameraList;
    bool                   mFullScreen;

    SplashScreen          *mSplash;
    
    // Album Settings
    AlbumSettings *mAlbumSettings;

    // Camera Actions
    KActionMenu   *mCameraMenuAction;

    // Theme Actions
    KSelectAction *mThemeMenuAction;

    // Album Actions
    KAction       *mNewAction;
    KAction       *mDeleteAction;
    KSelectAction *mAlbumSortAction;
    KToolBarPopupAction   *mBackwardActionMenu;
    KToolBarPopupAction   *mForwardActionMenu;

    KAction       *mAddImagesAction;
    KAction       *mPropsEditAction;
    KAction       *mAlbumImportAction;
    KAction       *mOpenInKonquiAction;
    
    // Tag Actions
    KAction       *mNewTagAction;
    KAction       *mDeleteTagAction;
    KAction       *mEditTagAction;
    
    // Image Actions
    KAction       *mImageViewAction;
    KAction       *mImageCommentsAction;
    KAction       *mImageViewExifAction;
    KAction       *mImageSetExifOrientation1Action;
    KAction       *mImageSetExifOrientation2Action;
    KAction       *mImageSetExifOrientation3Action;
    KAction       *mImageSetExifOrientation4Action;
    KAction       *mImageSetExifOrientation5Action;
    KAction       *mImageSetExifOrientation6Action;
    KAction       *mImageSetExifOrientation7Action;
    KAction       *mImageSetExifOrientation8Action;
    KAction       *mImageRenameAction;
    KAction       *mImageDeleteAction;
    KAction       *mImagePropsAction;
    KSelectAction *mImageSortAction;
    KActionMenu   *mImageExifOrientationActionMenu;

    // Selection Actions
    KAction       *mSelectAllAction;
    KAction       *mSelectNoneAction;
    KAction       *mSelectInvertAction;

    // View Actions
    KAction       *mThumbSizePlusAction;
    KAction       *mThumbSizeMinusAction;
    KAction       *mFullScreenAction;

    // Application Actions
    KAction       *mQuitAction;
    KAction       *mTipAction;
    KAction       *mKipiHelpAction;
    KAction       *mGammaAdjustmentAction;

private slots:

    void slot_albumSelected(bool val);
    void slot_tagSelected(bool val);
    void slot_imageSelected(bool val);
    void slot_exit();
    void slotShowTip();
    void slotShowKipiHelp();
    void slot_gammaAdjustment();

    void slotAboutToShowForwardMenu();
    void slotAboutToShowBackwardMenu();
            
    void slotSetup();
    void slotSetupCamera();
    void slotSetupChanged();

    void slotKipiPluginPlug();
    
    void slotCameraConnect();
    void slotCameraAdded(CameraType *ctype);
    void slotCameraRemoved(CameraType *ctype);
    void slotCameraAutoDetect();
    void slotEditKeys();
    void slotConfToolbars();
    void slotToggleFullScreen();

    void slotChangeTheme(const QString& theme);
};

#endif  // DIGIKAMAPP_H
