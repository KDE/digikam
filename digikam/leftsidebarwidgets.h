/*
 * leftsidebarwidgets.h
 *
 *  Created on: 14.11.2009
 *      Author: languitar
 */

#ifndef LEFTSIDEBARWIDGETS_H
#define LEFTSIDEBARWIDGETS_H

// KDE includes
#include <kconfiggroup.h>

// Local includes
#include "sidebarwidget.h"
#include "albummodel.h"

namespace Digikam
{

class AlbumFolderViewSideBarWidgetPriv;
/**
 * SideBarWidget for the folder view.
 *
 * @author jwienke
 */
class AlbumFolderViewSideBarWidget : public SideBarWidget
{
    Q_OBJECT
public:
    AlbumFolderViewSideBarWidget(QWidget *parent, AlbumModel *model);
    virtual ~AlbumFolderViewSideBarWidget();

    void setActive(bool active);
    void loadViewState(KConfigGroup &group);
    void saveViewState(KConfigGroup &group);
    void changeAlbumFromHistory(Album *album);
    void gotoAlbumAndItem(const ImageInfo &info);
    QPixmap getIcon();
    QString getCaption();

Q_SIGNALS:
    void signalFindDuplicatesInAlbum(Album*);

private:
    AlbumFolderViewSideBarWidgetPriv *d;

};

}

#endif /* LEFTSIDEBARWIDGETS_H */
