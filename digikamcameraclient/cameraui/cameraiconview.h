/* ============================================================
 * File  : cameraiconview.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-23
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

#ifndef CAMERAICONVIEW_H
#define CAMERAICONVIEW_H

#include "thumbview.h"

class QString;
class QPainter;
class QImage;
class QPixmap;
class QPoint;

class GPFileItemInfo;
class ThumbItem;
class ThumbnailSize;
class CameraIconItem;
class CameraIconViewPrivate;


class CameraIconView : public ThumbView
{
    Q_OBJECT

public:

    CameraIconView(QWidget *parent);
    ~CameraIconView();

    void setThumbnailSize(const ThumbnailSize& thumbSize);
    
    CameraIconItem* addItem(const GPFileItemInfo* fileInfo);
    void setThumbnail(CameraIconItem* iconItem,
                      const QImage& thumbnail);
    void markDownloaded(CameraIconItem* iconItem);

    virtual void clear();

protected:

    void calcBanner();
    void paintBanner(QPainter*);
    void startDrag();

private:

    void createPixmap(QPixmap& pix, const QString& icon,
                      double scale);
    
private:

    CameraIconViewPrivate *d;

signals:

    void signalCleared();

private slots:

    void slotRightButtonClicked(ThumbItem *item, const QPoint &pos);
    void slotDoubleClicked(ThumbItem *item);
    void slotItemInformation(CameraIconItem *item);

signals:

    void signalOpenItem(const QString& folder,
                        const QString& name);
    void signalOpenItem(const QString& folder,
                        const QString& name,
                        const QString& serviceName);
    void signalDownloadSelectedItems();
    void signalDeleteSelectedItems();
    void signalExifInformation(const QString& folder,
                               const QString& name);
};

#endif /* CAMERAICONVIEW_H */
