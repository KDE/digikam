/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGELOADINGTHREAD_H
#define IMAGELOADINGTHREAD_H

// Qt includes

#include <QThread>
#include <QImage>

// KDE includes

#include <QUrl>

namespace PhotoLayoutsEditor
{

class ImageLoadingThread : public QThread
{
    Q_OBJECT

public:

    explicit ImageLoadingThread(QObject* const parent = 0);
    ~ImageLoadingThread();

    virtual void run();

Q_SIGNALS:

    void imageLoaded(const QUrl& url, const QImage& image);

public Q_SLOTS:

    void setMaximumProgress(double limit);
    void setImageUrl(const QUrl& url);
    void setImagesUrls(const QList<QUrl>& urls);

private:

    void loadRaw(const QUrl& url);
    void loadImage(const QUrl& url);

private:

    class ImageLoadingThreadPrivate;
    ImageLoadingThreadPrivate* d;

    friend class ImageLoadingThreadPrivate;
};

} // namespace PhotoLayoutsEditor

#endif // IMAGELOADINGTHREAD_H
