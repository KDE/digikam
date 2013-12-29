#ifndef SHOWFOTO_P_H
#define SHOWFOTO_P_H

#include "showfoto.h"
#include "QDir"
#include "showfotoiteminfo.h"
#include "showfotothumbnailbar.h"
#include "splashscreen.h"
#include "imagepropertiessidebar.h"
#include "showfotodelegate.h"
#include "showfotosettings.h"

namespace ShowFoto {

class ShowFoto::Private
{
public:

    Private() :
        deleteItem2Trash(true),
        validIccPath(true),
        itemsNb(0),
        vSplitter(0),
        fileOpenAction(0),
        openFilesInFolderAction(0),
        first(0),
        model(0),
        filterModel(0),
        thumbLoadThread(0),
        thumbBar(0),
        thumbBarDock(0),
        normalDelegate(0),
        rightSideBar(0),
        splash(0),
        settings(0)
    {
    }

    bool                             deleteItem2Trash;
    bool                             validIccPath;

    int                              itemsNb;

    QSplitter*                       vSplitter;

    QAction*                         fileOpenAction;

    KUrl                             lastOpenedDirectory;

    KAction*                         openFilesInFolderAction;
    KAction*                         first;
    QDir                             dir;
    ShowfotoItemInfoList             infoList;
    ShowfotoThumbnailModel*          model;
    ShowfotoFilterModel*             filterModel;
    Digikam::ThumbnailLoadThread*    thumbLoadThread;
    ShowfotoThumbnailBar*            thumbBar;
    Digikam::ThumbBarDock*           thumbBarDock;
    ShowfotoNormalDelegate*          normalDelegate;
    Digikam::ImagePropertiesSideBar* rightSideBar;
    Digikam::SplashScreen*           splash;
    ShowfotoSettings*                settings;
};
} // namespace Showfoto
#endif // SHOWFOTO_P_H
