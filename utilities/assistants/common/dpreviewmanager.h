/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : a widget to manage preview.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#ifndef DPREVIEW_MANAGER_H
#define DPREVIEW_MANAGER_H

// Qt includes

#include <QStackedWidget>
#include <QString>
#include <QColor>
#include <QImage>
#include <QUrl>

namespace Digikam
{

class DPreviewManager : public QStackedWidget
{
    Q_OBJECT

public:

    enum DisplayMode
    {
        MessageMode = 0,
        PreviewMode
    };

public:

    DPreviewManager(QWidget* const parent);
    ~DPreviewManager();

    bool load(const QUrl& file, bool fit = true);
    void setImage(const QImage& img, bool fit = true);
    void setText(const QString& text, const QColor& color=Qt::white);
    void setBusy(bool b, const QString& text=QString());
    void setThumbnail(const QPixmap& preview=QPixmap());
    void setButtonText(const QString& text);
    void setButtonVisible(bool b);
    void setSelectionAreaPossible(bool b);

    /**
     * Manage a selection area and show it
     *
     * @param rectangle This rectangle should have height and width of 1.0
     */
    void   setSelectionArea(const QRectF& rectangle);
    QRectF getSelectionArea() const;

Q_SIGNALS:

    void signalButtonClicked();

public Q_SLOTS:

    void slotLoad(const QUrl& url);

private Q_SLOTS:

    void slotProgressTimerDone();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DPREVIEW_MANAGER_H
