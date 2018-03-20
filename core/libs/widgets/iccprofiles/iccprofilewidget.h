/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-06-23
 * Description : a tab widget to display ICC profile infos
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ICCPROFILEWIDGET_H
#define ICCPROFILEWIDGET_H

// Qt includes

#include <QWidget>
#include <QString>

// Local includes

#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class IccProfile;

class DIGIKAM_EXPORT ICCProfileWidget : public MetadataWidget
{
    Q_OBJECT

public:

    explicit ICCProfileWidget(QWidget* const parent, int w=256, int h=256);
    ~ICCProfileWidget();

    bool    loadFromURL(const QUrl& url);
    bool    loadFromProfileData(const QString& fileName, const QByteArray& data);
    bool    loadProfile(const QString& fileName, const IccProfile& data);

    QString getTagDescription(const QString& key);
    QString getTagTitle(const QString& key);

    QString getMetadataTitle();

    void    setLoadingFailed();
    void    setDataLoading();
    void    setUncalibratedColor();

    bool  setProfile(const IccProfile& profile);
    IccProfile getProfile() const;

protected Q_SLOTS:

    virtual void slotSaveMetadataToFile();

private:

    bool decodeMetadata();
    void buildView();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ICCPROFILEWIDGET_H
