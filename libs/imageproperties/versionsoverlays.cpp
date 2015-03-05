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

#include "versionsoverlays.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "imageinfo.h"
#include "imagehistorygraphmodel.h"
#include "imagelistmodel.h"
#include "itemviewhoverbutton.h"
#include "tagscache.h"
#include "versionmanagersettings.h"

namespace Digikam
{

class ShowHideVersionsOverlay::Button : public ItemViewHoverButton
{
public:

    explicit Button(QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();
};

ShowHideVersionsOverlay::Button::Button(QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView)
{
}

QSize ShowHideVersionsOverlay::Button::sizeHint() const
{
    return QSize(16, 16);
}

QIcon ShowHideVersionsOverlay::Button::icon()
{
    QString icon = isChecked() ? QLatin1String("edit-bomb") : QLatin1String("edit-clear-history");
    return QIcon::fromTheme(icon);
}

void ShowHideVersionsOverlay::Button::updateToolTip()
{
    setToolTip(isChecked() ?
               i18nc("@info:tooltip", "Hide item permanently") :
               i18nc("@info:tooltip", "Show item permanently"));
}

// ------------------------------------------------------------------------------------

ShowHideVersionsOverlay::ShowHideVersionsOverlay(QObject* const parent)
    : HoverButtonDelegateOverlay(parent)
{
}

void ShowHideVersionsOverlay::setSettings(const VersionManagerSettings& settings)
{
    m_filter = VersionImageFilterSettings(settings);
}

void ShowHideVersionsOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ItemViewHoverButton* ShowHideVersionsOverlay::createButton()
{
    return new Button(view());
}

void ShowHideVersionsOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();

    const int gap    = 5;
    const int x      = rect.right() - gap - size.width();
    const int y      = rect.bottom() - gap - size.height();
    button()->move(QPoint(x, y));

    ImageInfo info   = ImageModel::retrieveImageInfo(index);
    button()->setChecked(m_filter.isExemptedBySettings(info));
}

void ShowHideVersionsOverlay::slotClicked(bool checked)
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        ImageInfo info = ImageModel::retrieveImageInfo(index);
        int tagId      = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::versionAlwaysVisible());

        if (checked)
        {
            info.setTag(tagId);
        }
        else
        {
            info.removeTag(tagId);
        }
    }
}

bool ShowHideVersionsOverlay::checkIndex(const QModelIndex& index) const
{
    if (index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
    {
        ImageInfo info = ImageModel::retrieveImageInfo(index);
        return m_filter.isHiddenBySettings(info);
    }

    return false;
}

// --------------------------------------------------------------------

class ActionVersionsOverlay::Button : public ItemViewHoverButton
{
public:

    Button(QAbstractItemView* const parentView, const QIcon& icon, const QString& text, const QString& tip);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();

protected:

    QIcon   m_icon;
    QString m_text;
    QString m_tip;
};

ActionVersionsOverlay::Button::Button(QAbstractItemView* const parentView, const QIcon& icon, const QString& text, const QString& tip)
    : ItemViewHoverButton(parentView),
      m_icon(icon),
      m_text(text),
      m_tip(tip)
{
}

QSize ActionVersionsOverlay::Button::sizeHint() const
{
    return QSize(16, 16);
}

QIcon ActionVersionsOverlay::Button::icon()
{
    return m_icon;
}

void ActionVersionsOverlay::Button::updateToolTip()
{
    setToolTip(m_tip);
}

// ------------------------------------------------------------------------------------------

ActionVersionsOverlay::ActionVersionsOverlay(QObject* const parent, const QIcon& icon, const QString& text, const QString& tip)
    : HoverButtonDelegateOverlay(parent),
      m_icon(icon),
      m_text(text),
      m_tip(tip),
      m_referenceModel(0)
{
}

ActionVersionsOverlay::Button* ActionVersionsOverlay::button() const
{
    return static_cast<Button*>(HoverButtonDelegateOverlay::button());
}

void ActionVersionsOverlay::setReferenceModel(const ImageModel* model)
{
    m_referenceModel = model;
}

void ActionVersionsOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked(bool)));
    }
    else
    {
        // button is deleted
    }
}

ItemViewHoverButton* ActionVersionsOverlay::createButton()
{
    return new Button(view(), m_icon, m_text, m_tip);
}

void ActionVersionsOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const QSize size = button()->size();

    const int gap    = 5;
    const int x      = rect.right() - gap - size.width();
    const int y      = rect.top() + gap;
    button()->move(QPoint(x, y));
}

void ActionVersionsOverlay::slotClicked(bool checked)
{
    Q_UNUSED(checked);
    QModelIndex index = button()->index();

    if (index.isValid())
    {
        emit activated(ImageModel::retrieveImageInfo(index));
    }
}

bool ActionVersionsOverlay::checkIndex(const QModelIndex& index) const
{
    if (index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
    {
        if (m_referenceModel)
        {
            ImageInfo info = ImageModel::retrieveImageInfo(index);
            // show overlay if image is not contained in reference model
            return !m_referenceModel->hasImage(info);
        }

        return true;
    }

    return false;
}

} // namespace Digikam
