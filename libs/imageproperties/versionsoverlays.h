/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-26
 * Description : images versions tree view overlays
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef VERSIONS_OVERLAYS_H
#define VERSIONS_OVERLAYS_H

#include <QString>
#include <QIcon>

// Local includes

#include "digikam_export.h"
#include "imagedelegateoverlay.h"
#include "imagefiltersettings.h"

namespace Digikam
{

class ImageInfo;
class ImageModel;
class VersionManagerSettings;

class ShowHideVersionsOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ShowHideVersionsOverlay(QObject* const parent);
    virtual void setActive(bool active);

    void setSettings(const VersionManagerSettings& settings);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

protected Q_SLOTS:

    void slotClicked(bool checked);

protected:

    VersionImageFilterSettings m_filter;
    class Button;
};

// -------------------------------------------------------------------

class ActionVersionsOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ActionVersionsOverlay(QObject* const parent,
                                   const QIcon& icon,
                                   const QString& text,
                                   const QString& tip = QString());
    virtual void setActive(bool active);

    void setReferenceModel(const ImageModel* model);

Q_SIGNALS:

    void activated(const ImageInfo& info);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

protected Q_SLOTS:

    void slotClicked(bool checked);

protected:

    class Button;
    Button* button() const;

protected:

    QIcon             m_icon;
    QString           m_text;
    QString           m_tip;
    const ImageModel* m_referenceModel;
};

} // namespace Digikam

#endif // VERSIONS_OVERLAYS_H
