/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Tom Albers <tomalbers@kde.nl>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2002-16-10
 * Description : main interface implementation
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright      2006 by Tom Albers
 * Copyright      2006 by Gilles Caulier
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
#include <kio/global.h>

// Local includes.

#include "digikam_export.h"
#include "dcopiface.h"

class KAction;
class KAccel;
class KActionMenu;
class KToolBarPopupAction;
class KSelectAction;
class KConfig;

namespace KIPI
{
class PluginLoader;
}

namespace Digikam
{
class ImagePluginLoader;
class AlbumManager;
class PluginLoader;               //   For KIPI pluggins support.
class DigikamKipiInterface;
class CameraList;
class CameraType;
class DigikamView;
class AlbumSettings;
class SplashScreen;

class DIGIKAM_EXPORT DigikamApp : public KMainWindow
{
    Q_OBJECT

public:

    DigikamApp();
    ~DigikamApp();

    virtual void show();

    static DigikamApp* getinstance();

    // KIPI Actions collections access.
    const QPtrList<KAction>& menuImageActions();
    const QPtrList<KAction>& menuBatchActions();
    const QPtrList<KAction>& menuAlbumActions();
    
    const QPtrList<KAction> menuImportActions();

    void autoDetect();
    void downloadFrom(const QString &cameraGuiPath);
    void enableThumbSizePlusAction(bool val);
    void enableThumbSizeMinusAction(bool val);
    void enableAlbumBackwardHistory(bool enable);
    void enableAlbumForwardHistory(bool enable);
    
signals:

    void signalEscapePressed();
    void signalNextItem();
    void signalPrevItem();
    void signalFirstItem();
    void signalLastItem();

protected:

    bool queryClose();

protected slots:

    void slotCameraMediaMenuEntries( KIO::Job *, const KIO::UDSEntryList & );

private:

    void setupView();
    void setupActions();
    void setupAccelerators();
    void loadPlugins();
    void loadCameras();
    void populateThemes();

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
    
    QString convertToLocalUrl( const QString& folder );
    void slotDownloadImages( const QString& folder );
    void slotDownloadImages();
    void slotCameraConnect();
    void slotCameraMediaMenu();
    void slotDownloadImagesFromMedia( int id );
    void slotCameraAdded(CameraType *ctype);
    void slotCameraRemoved(CameraType *ctype);
    void slotCameraAutoDetect();
    void slotDcopDownloadImages( const QString& folder );
    void slotDcopCameraAutoDetect();
    void slotEditKeys();
    void slotConfToolbars();
    void slotToggleFullScreen();
    void slotDatabaseRescan();
    void slotRebuildAllThumbs();
    void slotRebuildAllThumbsDone();
    
    void slotChangeTheme(const QString& theme);

private:

    bool                   mFullScreen;

    bool                   mValidIccPath;
    
    // KIPI plugins support
    QPtrList<KAction>      m_kipiFileActionsExport;
    QPtrList<KAction>      m_kipiFileActionsImport;
    QPtrList<KAction>      m_kipiImageActions;
    QPtrList<KAction>      m_kipiToolsActions;
    QPtrList<KAction>      m_kipiBatchActions;
    QPtrList<KAction>      m_kipiAlbumActions;

    QPopupMenu            *mCameraMediaList;
    QMap<int, QString>     mMediaItems;

    QString                mCameraGuiPath;

    KAccel                *m_accelerators;

    KConfig               *m_config;    
    
    // Camera Actions
    KActionMenu           *mCameraMenuAction;

    // Theme Actions
    KSelectAction         *mThemeMenuAction;

    // Album Actions
    KAction               *mNewAction;
    KAction               *mDeleteAction;
    KAction               *mImageDeletePermanentlyAction;
    KAction               *mImageDeletePermanentlyDirectlyAction;
    KAction               *mImageTrashDirectlyAction;
    KSelectAction         *mAlbumSortAction;
    KToolBarPopupAction   *mBackwardActionMenu;
    KToolBarPopupAction   *mForwardActionMenu;

    KAction               *mAddImagesAction;
    KAction               *mPropsEditAction;
    KAction               *mAlbumImportAction;
    KAction               *mOpenInKonquiAction;
    KAction               *mRefreshAlbumAction;
    
    // Tag Actions
    KAction               *mNewTagAction;
    KAction               *mDeleteTagAction;
    KAction               *mEditTagAction;
    
    // Image Actions
    KAction               *mImagePreviewAction;
    KAction               *mImageViewAction;
    KAction               *mImageSetExifOrientation1Action;
    KAction               *mImageSetExifOrientation2Action;
    KAction               *mImageSetExifOrientation3Action;
    KAction               *mImageSetExifOrientation4Action;
    KAction               *mImageSetExifOrientation5Action;
    KAction               *mImageSetExifOrientation6Action;
    KAction               *mImageSetExifOrientation7Action;
    KAction               *mImageSetExifOrientation8Action;
    KAction               *mImageRenameAction;
    KAction               *mImageDeleteAction;
    KSelectAction         *mImageSortAction;
    KActionMenu           *mImageExifOrientationActionMenu;

    // Selection Actions
    KAction               *mSelectAllAction;
    KAction               *mSelectNoneAction;
    KAction               *mSelectInvertAction;

    // View Actions
    KAction               *mThumbSizePlusAction;
    KAction               *mThumbSizeMinusAction;
    KAction               *mFullScreenAction;

    KAction               *m_0Star;
    KAction               *m_1Star;
    KAction               *m_2Star;
    KAction               *m_3Star;
    KAction               *m_4Star;
    KAction               *m_5Star;

    // Application Actions
    KAction               *mQuitAction;
    KAction               *mTipAction;
    KAction               *mKipiHelpAction;
    KAction               *mGammaAdjustmentAction;
    
    AlbumSettings         *mAlbumSettings;
    SplashScreen          *mSplash;
    DCOPIface             *mDcopIface;
    AlbumManager          *mAlbumManager;
    ImagePluginLoader     *m_ImagePluginsLoader;
    DigikamKipiInterface  *KipiInterface_;
    DigikamView           *mView;
    CameraList            *mCameraList;
    
    static DigikamApp     *m_instance;

    KIPI::PluginLoader    *KipiPluginLoader_;
};

}  // namespace Digikam

#endif  // DIGIKAMAPP_H
