/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : widget displaying all image versions in a list
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

#include "versionswidget.moc"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QToolButton>
#include <QTreeView>

// KDE includes

#include <KConfigGroup>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KDebug>

// Local includes

#include "albumsettings.h"
#include "dimagehistory.h"
#include "imagehistorygraphmodel.h"
#include "imagepropertiesversionsdelegate.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "imagelistmodel.h"
#include "itemviewtooltip.h"
#include "thumbnailloadthread.h"
#include "tooltipfiller.h"

namespace Digikam
{

class VersionsWidgetToolTip : public ItemViewToolTip
{
public:

    VersionsWidgetToolTip(QAbstractItemView* view)
        : ItemViewToolTip(view),
          m_mode(InvalidMode)
    {
    }

    enum Mode
    {
        InvalidMode,
        ImageMode,
        FilterActionMode
    };

    void show(QHelpEvent* event, const QStyleOptionViewItem& option, const QModelIndex& index, Mode mode)
    {
        m_mode = mode;
        ItemViewToolTip::show(event, option, index);
        m_mode = InvalidMode;
    }

protected:

    virtual QString tipContents()
    {
        switch (m_mode)
        {
            default:
            case InvalidMode:
                return QString();
            case ImageMode:
            {
                ImageInfo info = ImageModel::retrieveImageInfo(currentIndex());
                return ToolTipFiller::imageInfoTipContents(info);
            }
            case FilterActionMode:
            {
                // TODO
                return QString();
            }
        }
    }

protected:

    Mode m_mode;
};

// ----

class VersionsWidgetTreeView : public QTreeView
{
public:

    VersionsWidgetTreeView(QWidget *parent = 0);

    void setToolTipEnabled(bool on);

    virtual void paintEvent(QPaintEvent *e);
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    virtual bool viewportEvent(QEvent* event);

public:

    bool m_showToolTip;
    VersionsWidgetToolTip* m_toolTip;
};

VersionsWidgetTreeView::VersionsWidgetTreeView(QWidget *parent)
    : QTreeView(parent),
      m_showToolTip(false),
      m_toolTip(0)
{
}

void VersionsWidgetTreeView::setToolTipEnabled(bool on)
{
    if (on == m_showToolTip)
        return;

    m_showToolTip = on;

    if (m_showToolTip && !m_toolTip)
    {
        m_toolTip = new VersionsWidgetToolTip(this);
    }
}

void VersionsWidgetTreeView::paintEvent(QPaintEvent *e)
{
    static_cast<ImagePropertiesVersionsDelegate*>(itemDelegate())->beginPainting();
    QTreeView::paintEvent(e);
    static_cast<ImagePropertiesVersionsDelegate*>(itemDelegate())->finishPainting();
}

QModelIndex VersionsWidgetTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // TODO: Need to find a solution to skip non-vertex items in CombinedTreeMode. Not easy.
    return QTreeView::moveCursor(cursorAction, modifiers);
}

bool VersionsWidgetTreeView::viewportEvent(QEvent* event)
{
    switch (event->type())
    {
        case QEvent::ToolTip:
        {
            if (!m_showToolTip)
            {
                break;
            }

            QHelpEvent* he = static_cast<QHelpEvent*>(event);
            const QModelIndex index = indexAt(he->pos());

            if (!index.isValid())
            {
                break;
            }

            VersionsWidgetToolTip::Mode mode;
            if (index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
            {
                mode = VersionsWidgetToolTip::ImageMode;
            }
            else if (index.data(ImageHistoryGraphModel::IsImageItemRole).toBool())
            {
                mode = VersionsWidgetToolTip::FilterActionMode;
            }
            else
            {
                break;
            }

            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(index);
            option.state |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);

            m_toolTip->show(he, option, index, mode);

            return true;
        }
        default:
            break;
    }

    return QTreeView::viewportEvent(event);
}

// ----

class VersionsWidget::VersionsWidgetPriv
{
public:

    VersionsWidgetPriv()
        : view(0),
          model(0),
          delegate(0),
          viewButtonGroup(0),
          listModeButton(0),
          treeModeButton(0),
          combinedModeButton(0)
    {
    }

    VersionsWidgetTreeView*          view;
    ImageHistoryGraphModel*          model;
    ImagePropertiesVersionsDelegate* delegate;

    QButtonGroup*                    viewButtonGroup;
    QToolButton*                     listModeButton;
    QToolButton*                     treeModeButton;
    QToolButton*                     combinedModeButton;

    static const QString             configCurrentMode;
};
const QString VersionsWidget::VersionsWidgetPriv::configCurrentMode("Version Properties View Mode");


VersionsWidget::VersionsWidget(QWidget* parent)
    : QWidget(parent), d(new VersionsWidgetPriv)
{
    QGridLayout* layout      = new QGridLayout;

    d->viewButtonGroup    = new QButtonGroup(this);
    d->listModeButton     = new QToolButton;
    d->listModeButton->setIcon(SmallIcon("view-list-icons"));
    d->listModeButton->setCheckable(true);
    d->listModeButton->setToolTip(i18n("Show available versions in a list"));
    d->viewButtonGroup->addButton(d->listModeButton, ImageHistoryGraphModel::ImagesListMode);

    d->treeModeButton     = new QToolButton;
    d->treeModeButton->setIcon(SmallIcon("view-list-tree"));
    d->treeModeButton->setCheckable(true);
    d->treeModeButton->setToolTip(i18n("Show available versions as a tree"));
    d->viewButtonGroup->addButton(d->treeModeButton, ImageHistoryGraphModel::ImagesTreeMode);

    d->combinedModeButton = new QToolButton;
    d->combinedModeButton->setIcon(SmallIcon("view-list-details"));
    d->combinedModeButton->setCheckable(true);
    d->combinedModeButton->setToolTip(i18n("Show available version and the applied filters in a combined list"));
    d->viewButtonGroup->addButton(d->combinedModeButton, ImageHistoryGraphModel::CombinedTreeMode);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(d->listModeButton);
    buttonLayout->addWidget(d->treeModeButton);
    buttonLayout->addWidget(d->combinedModeButton);

    d->model                = new ImageHistoryGraphModel(this);
    d->model->imageModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    d->delegate             = new ImagePropertiesVersionsDelegate(this);

    d->view                 = new VersionsWidgetTreeView;
    d->view->setItemDelegate(d->delegate);
    d->view->setModel(d->model);
    d->view->setWordWrap(true);
    d->view->setRootIsDecorated(false);
    d->view->setHeaderHidden(true);
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    //d->view->setFrameShape(QFrame::NoFrame);
    d->view->setFrameShadow(QFrame::Plain);

    layout->addLayout(buttonLayout,   0, 1);
    layout->addWidget(d->view,        1, 0, 1, 2);
    layout->setColumnStretch(0, 1);
    layout->setRowStretch(1, 1);
    setLayout(layout);

    connect(d->delegate, SIGNAL(animationStateChanged()),
            d->view->viewport(), SLOT(update()));

    connect(d->view, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(slotViewItemSelected(const QModelIndex&)));

    connect(d->viewButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotViewModeChanged(int)));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

VersionsWidget::~VersionsWidget()
{
    delete d->delegate;
    delete d->model;
    delete d;
}

void VersionsWidget::readSettings(const KConfigGroup& group)
{
    int mode = group.readEntry(d->configCurrentMode, (int)ImageHistoryGraphModel::CombinedTreeMode);
    switch (mode)
    {
        case ImageHistoryGraphModel::ImagesListMode:
            d->listModeButton->setChecked(true);
            break;
        case ImageHistoryGraphModel::ImagesTreeMode:
            d->treeModeButton->setChecked(true);
            break;
        default:
        case ImageHistoryGraphModel::CombinedTreeMode:
            d->combinedModeButton->setChecked(true);
            break;
    }
    slotViewModeChanged(mode);
}

void VersionsWidget::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configCurrentMode, d->viewButtonGroup->checkedId());
}

void VersionsWidget::setCurrentItem(const ImageInfo& info)
{
    d->model->setHistory(info);
}

void VersionsWidget::slotViewItemSelected(const QModelIndex& index)
{
    ImageInfo info = d->model->imageInfo(index);
    if (!info.isNull())
    {
        emit imageSelected(info);
    }
}

void VersionsWidget::slotViewModeChanged(int mode)
{
    d->model->setMode((ImageHistoryGraphModel::Mode)mode);

    if (mode == ImageHistoryGraphModel::ImagesTreeMode)
        d->view->expandAll();

    QModelIndex subjectIndex = d->model->indexForInfo(d->model->subject());
    d->view->scrollTo(subjectIndex, QAbstractItemView::PositionAtCenter);
    d->view->setCurrentIndex(subjectIndex);
}

void VersionsWidget::slotSetupChanged()
{
    d->view->setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());
}

} // namespace Digikam
