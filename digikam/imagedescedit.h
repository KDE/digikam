#ifndef IMAGEDESCEDIT_H
#define IMAGEDESCEDIT_H

#include <kdialogbase.h>
#include <qguardedptr.h>
#include <qpixmap.h>

#include "thumbnailjob.h"

class QLabel;
class QListView;
class QPixmap;
class QCheckListItem;
class QCheckBox;
class KFileMetaInfo;
class KTextEdit;
class AlbumIconView;
class AlbumIconItem;
class AlbumLister;
class TAlbum;

class ImageDescEdit : public KDialogBase
{
    Q_OBJECT

public:

    ImageDescEdit(AlbumIconView* view, AlbumIconItem* currItem);
    ~ImageDescEdit();

    static bool editDescription(AlbumIconView* view, AlbumIconItem* currItem);

protected:

    bool eventFilter(QObject *o, QEvent *e);

private:

    AlbumIconView *m_view;
    AlbumIconItem *m_currItem;
    AlbumLister   *m_lister;
    QLabel        *m_thumbLabel;
    QLabel        *m_nameLabel;
    KTextEdit     *m_commentsEdit;
    QListView     *m_tagsView;
    QCheckBox     *m_autoSaveBox;
    bool           m_modified;

    void tagNew(TAlbum* parAlbum, QCheckListItem *item);
    void tagEdit(TAlbum* album);
    void tagDelete(TAlbum *album, QCheckListItem *item);
    
    QGuardedPtr<Digikam::ThumbnailJob> m_thumbJob;

    void populateTags();
    void populateTags(QCheckListItem* parItem, TAlbum* parAlbum);
    
private slots:

    void slotItemChanged();
    void slotModified();
    void slotUser1();
    void slotUser2();
    void slotApply();
    void slotOk();
    void slotGotThumbnail(const KURL&, const QPixmap& pix,
                          const KFileMetaInfo*);    
    void slotRightButtonClicked(QListViewItem *, const QPoint &, int);
};

#endif
