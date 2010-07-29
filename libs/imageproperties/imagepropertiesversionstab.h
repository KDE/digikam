/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying image versions
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGEPROPERTIESVERSIONSTAB_H
#define IMAGEPROPERTIESVERSIONSTAB_H

// Qt includes

#include <QWidget>
#include <QModelIndex>

// KDE includes

#include <KUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam 
{

class ImageVersionsModel;
class DImageHistory;
class ImageInfoList;

class DIGIKAM_EXPORT ImagePropertiesVersionsTab : public QWidget
{
    Q_OBJECT

public:

    ImagePropertiesVersionsTab(QWidget* parent, ImageVersionsModel* model);
    ~ImagePropertiesVersionsTab();

//    void setCurrentURL(const KUrl& url = KUrl());
//     void setImageHistory(const DImageHistory& history);

public Q_SLOTS:

    void slotDigikamViewNoCurrentItem();
    void slotDigikamViewImageSelected(const ImageInfoList& selectedImage, bool hasPrevious, bool hasNext, const ImageInfoList &allImages);
    void slotViewItemSelected(QModelIndex index);

Q_SIGNALS:

    void setCurrentUrlSignal(const KUrl& url);

private:

    class ImagePropertiesVersionsTabPriv;
    ImagePropertiesVersionsTabPriv* const d;
};

} // namespace Digikam

#endif // IMAGEPROPERTIESVERSIONSTAB_H
