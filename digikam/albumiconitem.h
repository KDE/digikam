#ifndef ALBUMICONITEM_H
#define ALBUMICONITEM_H

#include <qstring.h>
#include <qrect.h>
#include <thumbitem.h>


class QPainter;
class QPixmap;
class QImage;
class QColorGroup;
class KFileItem;

class AlbumIconView;


class AlbumIconItem : public ThumbItem {

    friend class AlbumIconView;

public:

    AlbumIconItem(AlbumIconView* parent, const QString& text,
                  const QPixmap& pix, int size, 
                  const KFileItem* fileItem,
                  int imageWidth=0, int imageHeight=0);
    ~AlbumIconItem();


    const KFileItem* fileItem() {
        return fileItem_;
    }

    void setPixmap(const QImage& thumb);
    void setPixmap(const QPixmap& thumb);
    void setImageDimensions(int width, int height);

protected:


    virtual void calcRect();
    virtual void paintItem(QPainter* p, const QColorGroup& cg);

    void updateExtraText();

private:

    int size_;
    int imageWidth_;
    int imageHeight_;

    QString extraText_;
    QRect   itemExtraRect_;

    const KFileItem *fileItem_;
    AlbumIconView *view_;

    
};


#endif
