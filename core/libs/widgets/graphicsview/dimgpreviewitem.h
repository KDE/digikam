/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View items for DImg
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIMGPREVIEWITEM_H
#define DIMGPREVIEWITEM_H

// Qt includes

#include <QGraphicsItem>
#include <QObject>

// Local includes

#include "digikam_export.h"
#include "graphicsdimgitem.h"

namespace Digikam
{

class DImg;
class ICCSettingsContainer;
class LoadingDescription;
class PreviewSettings;

class DIGIKAM_EXPORT DImgPreviewItem : public GraphicsDImgItem
{
    Q_OBJECT

public:

    enum State
    {
        NoImage,
        Loading,
        ImageLoaded,
        ImageLoadingFailed
    };

public:

    explicit DImgPreviewItem(QGraphicsItem* const parent = 0);
    virtual ~DImgPreviewItem();

    void setDisplayingWidget(QWidget* const widget);
    void setPreviewSettings(const PreviewSettings& settings);

    QString path() const;
    void    setPath(const QString& path, bool rePreview = false);

    State state() const;
    bool  isLoaded() const;
    void  reload();

    void setPreloadPaths(const QStringList& pathsToPreload);

    QString userLoadingHint() const;

Q_SIGNALS:

    void stateChanged(int state);
    void loaded();
    void loadingFailed();


private Q_SLOTS:

    void slotGotImagePreview(const LoadingDescription& loadingDescription, const DImg& image);
    void preloadNext();
    void slotFileChanged(const QString& path);
    void iccSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous);

private:

    class DImgPreviewItemPrivate;
    Q_DECLARE_PRIVATE(DImgPreviewItem)

protected:

    DImgPreviewItem(DImgPreviewItemPrivate& dd, QGraphicsItem* const parent = 0);
};

} // namespace Digikam

#endif // DIMGPREVIEWITEM_H
