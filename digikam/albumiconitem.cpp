#include <qapplication.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qpen.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include <kfileitem.h>
#include <kmimetype.h>
#include <kfilemetainfo.h>
#include <kurl.h>
#include <kio/global.h>
#include <klocale.h>
#include <kexifdata.h>

#include "albumsettings.h"
#include "albumiconview.h"
#include "albumiconitem.h"


AlbumIconItem::AlbumIconItem(AlbumIconView* parent,
                             const QString& text,
                             const QPixmap& pix,
                             int size,
                             const KFileItem* fileItem,
                             int imageWidth, int imageHeight)
    : ThumbItem(parent, text, pix)
{
    view_ = parent;
    size_ = size;
    fileItem_ = fileItem;
    imageWidth_ = imageWidth;
    imageHeight_ = imageHeight;
    updateExtraText();
    calcRect();
}


AlbumIconItem::~AlbumIconItem()
{
}


void AlbumIconItem::setPixmap(const QImage& thumbnail)
{
    QPainter painter;
    painter.begin(pixmap());
    painter.fillRect(0, 0, size_, size_,
                     QBrush(iconView()->colorGroup().base()));
    painter.drawImage((size_-thumbnail.width())/2,
                      (size_-thumbnail.height())/2,
                      thumbnail);
    painter.end();

    QRect pRect(iconView()->contentsRectToViewport(pixmapRect(false)));
    if (pRect.intersects(iconView()->visibleRect())) {
        pRect.moveBy(-1,-1);
        pRect.setWidth(pRect.width()+2);
        pRect.setHeight(pRect.height()+2);
        painter.begin(iconView()->viewport());
        painter.drawPixmap(pRect.x()+1, pRect.y()+1, *pixmap());
        painter.end();
    }

}

void AlbumIconItem::setPixmap(const QPixmap& thumbnail)
{
    QPainter painter;
    painter.begin(pixmap());
    painter.fillRect(0, 0, size_, size_,
                     QBrush(iconView()->colorGroup().base()));
    painter.drawPixmap((size_-thumbnail.width())/2,
                      (size_-thumbnail.height())/2,
                      thumbnail);
    painter.end();

    QRect pRect(iconView()->contentsRectToViewport(pixmapRect(false)));
    if (pRect.intersects(iconView()->visibleRect())) {
        pRect.moveBy(-1,-1);
        pRect.setWidth(pRect.width()+2);
        pRect.setHeight(pRect.height()+2);
        bitBlt(iconView()->viewport(), pRect.x()+1, pRect.y()+1,
               pixmap(), 0, 0, pRect.width(), pRect.height());
        //painter.begin(iconView()->viewport());
        //painter.drawPixmap(pRect.x()+1, pRect.y()+1, *pixmap());
        //painter.end();
    }
}

void AlbumIconItem::calcRect()
{
    QRect itemIconRect = QRect(0,0,0,0);
    QRect itemTextRect = QRect(0,0,0,0);
    itemExtraRect_ = QRect(0,0,0,0);
    QRect itemRect = rect();

    itemRect.setWidth(100);
    itemRect.setHeight(0xFFFFFFFF);

    // set initial pixrect
    int pw = pixmap()->width();
    int ph = pixmap()->height();

    itemIconRect.setWidth(pw);
    itemIconRect.setHeight(ph);

    // word wrap main text
    QFontMetrics fm(view_->font());
    QRect r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                    0xFFFFFFFF, Qt::AlignHCenter |
                                    Qt::WordBreak | Qt::BreakAnywhere
                                    | Qt::AlignTop,
                                    text()));
    r.setWidth(r.width() + 2);

    itemTextRect.setWidth(r.width());
    itemTextRect.setHeight(r.height());

    // word wrap extra Text
    if (!extraText_.isEmpty()) {

        QFont font(view_->font());
        int fontSize = font.pointSize();
        if (fontSize > 0) {
            font.setPointSize(fontSize-2);
        }
        else {
            fontSize = font.pixelSize();
            font.setPixelSize(fontSize-2);
        }
        
        fm = QFontMetrics(font);

        r = QRect(fm.boundingRect(0, 0, itemIconRect.width(),
                                  0xFFFFFFFF, Qt::AlignHCenter |
                                  Qt::WordBreak | Qt::BreakAnywhere
                                  | Qt::AlignTop,
                                  extraText_));
        r.setWidth(r.width() + 2);

        itemExtraRect_.setWidth(r.width());
        itemExtraRect_.setHeight(r.height());

        itemTextRect.setWidth(QMAX(itemTextRect.width(), itemExtraRect_.width()));
        itemTextRect.setHeight(itemTextRect.height() + itemExtraRect_.height());
    }


    // Now start updating the rects
    int w = QMAX(itemTextRect.width(), itemIconRect.width() );
    int h = itemTextRect.height() + itemIconRect.height() + 1;

    itemRect.setWidth(w);
    itemRect.setHeight(h);

    // Center the pix and text rect

    itemIconRect = QRect((itemRect.width() - itemIconRect.width())/2,
                         0,
                         itemIconRect.width(), itemIconRect.height());
    itemTextRect = QRect((itemRect.width() - itemTextRect.width())/2,
                         itemRect.height() - itemTextRect.height(),
                         itemTextRect.width(), itemTextRect.height());
    if (!itemExtraRect_.isEmpty()) {
        itemExtraRect_ = QRect((itemRect.width() - itemExtraRect_.width())/2,
                               itemRect.height() - itemExtraRect_.height(),
                               itemExtraRect_.width(), itemExtraRect_.height());
    }

    // Update rects
    if ( itemIconRect != pixmapRect() )
        setPixmapRect( itemIconRect );
    if ( itemTextRect != textRect() )
        setTextRect( itemTextRect );
    if ( itemRect != rect() )
        setRect( itemRect );

}

void AlbumIconItem::paintItem(QPainter *, const QColorGroup& cg)
{

    QRect pRect=pixmapRect(true);
    QRect tRect=textRect(true);

    QPixmap pix(rect().width(), rect().height());
    pix.fill(cg.base());
    QPainter painter(&pix);
    painter.drawPixmap(pRect.x(), pRect.y(), *pixmap() );

    if (isSelected()) {
        QPen pen;
        pen.setColor(cg.highlight());
        painter.setPen(pen);
        painter.drawRect(0, 0, pix.width(), pix.height());
        painter.fillRect(0, tRect.y(), pix.width(),
                     tRect.height(), cg.highlight() );
        painter.setPen( QPen( cg.highlightedText() ) );
    }
    else
        painter.setPen( cg.text() );

    painter.drawText(tRect, Qt::WordBreak|Qt::BreakAnywhere|
                     Qt::AlignHCenter|Qt::AlignTop,text());

    if (!extraText_.isEmpty()) {
        QFont font(view_->font());
        int fontSize = font.pointSize();
        if (fontSize > 0) {
            font.setPointSize(fontSize-2);
        }
        else {
            fontSize = font.pixelSize();
            font.setPixelSize(fontSize-2);
        }
        painter.setFont(font);
        if (!isSelected())
            painter.setPen(QPen("steelblue"));
        painter.drawText(itemExtraRect_, Qt::WordBreak|
                         Qt::BreakAnywhere|Qt::AlignHCenter|
                         Qt::AlignTop,extraText_);
    }

    painter.end();

    QRect r(rect());
    r = QRect(view_->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));

    bitBlt(view_->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}


void AlbumIconItem::updateExtraText()
{
    QString extraText;
    bool firstLine = true;

    const AlbumSettings *settings = view_->settings();
    if (!settings) return;

    if (settings->getIconShowMime()) {
        firstLine = false;
        KMimeType::Ptr mimePtr =
            KMimeType::findByURL(fileItem_->url());
        extraText += mimePtr->name();
    }

    if (settings->getIconShowSize()) {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        extraText += KIO::convertSize(fileItem_->size());
    }

    if (settings->getIconShowDate()) {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        QDateTime date;
        date.setTime_t(fileItem_->time(KIO::UDS_MODIFICATION_TIME));
        extraText += date.toString();
    }

    if (settings->getIconShowComments()) {
        QString comments;
        view_->getItemComments(text(), comments);
        if (!comments.isEmpty()) {
            if (!firstLine)
                extraText += "\n";
            else
                firstLine = false;
            extraText += comments;
        }
    }

    if(settings->getIconShowFileComments())
    {
       // read and display JPEG COM comment and JPEG EXIF UserComment
       KFileMetaInfo metaInfo(fileItem_->url(), "image/jpeg", KFileMetaInfo::Fastest);

       if(metaInfo.isValid() && metaInfo.mimeType() == "image/jpeg" && metaInfo.containsGroup("Jpeg EXIF Data"))
       {
          KExifData *exifData = new KExifData;
          exifData->readFromFile(fileItem_->url().path());

          extraText += "\n";
          QString jpegComments = metaInfo["Jpeg EXIF Data"].item("Comment").value().toString();
          QString exifComments = exifData->getUserComment();

          QString comments;
          view_->getItemComments(text(), comments);

          // Only append comments if they are different from the regular comment
          if( !settings->getIconShowComments() || jpegComments != comments ) {
             extraText += jpegComments;
          }

          if( !settings->getIconShowComments() || exifComments != comments ) {
             if( jpegComments != exifComments ) extraText += exifComments;
          }

          delete exifData;

       }  

    }

    if (imageWidth_ != 0 && imageHeight_ != 0) {
        if (!firstLine)
            extraText += "\n";
        else
            firstLine = false;
        extraText += QString::number(imageWidth_)
                     + "x"
                     + QString::number(imageHeight_)
                     + i18n("Pixels");
    }

    extraText_ = extraText;

}

void AlbumIconItem::setImageDimensions(int width, int height)
{
    imageWidth_  = width;
    imageHeight_ = height;
}
