/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : EXIF caption settings page.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef EXIF_CAPTION_H
#define EXIF_CAPTION_H

// Qt includes

#include <QWidget>
#include <QByteArray>

namespace Digikam
{

class EXIFCaption : public QWidget
{
    Q_OBJECT

public:

    explicit EXIFCaption(QWidget* const parent);
    ~EXIFCaption();

    void applyMetadata(QByteArray& exifData, QByteArray& iptcData, QByteArray& xmpData);
    void readMetadata(QByteArray& exifData);

    bool syncJFIFCommentIsChecked() const;
    bool syncXMPCaptionIsChecked() const;
    bool syncIPTCCaptionIsChecked() const;

    void setCheckedSyncJFIFComment(bool c);
    void setCheckedSyncXMPCaption(bool c);
    void setCheckedSyncIPTCCaption(bool c);

    QString getEXIFUserComments() const;

Q_SIGNALS:

    void signalModified();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // EXIF_CAPTION_H
