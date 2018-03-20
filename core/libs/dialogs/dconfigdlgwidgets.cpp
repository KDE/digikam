/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006      by Tobias Koenig <tokoe at kde dot org>
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

#include "dconfigdlgwidgets.h"
#include "dconfigdlgwidgets_p.h"

// Qt include

#include <QApplication>
#include <QTimer>
#include <QMouseEvent>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QTextDocument>
#include <QIcon>
#include <QStyle>

namespace Digikam
{

DConfigDlgWdgPrivate::DConfigDlgWdgPrivate(DConfigDlgWdg* const q)
    : DConfigDlgViewPrivate(q)
{
}

void DConfigDlgWdgPrivate::_k_slotCurrentPageChanged(const QModelIndex& current, const QModelIndex& before)
{
    DConfigDlgWdgItem* currentItem = 0;

    if (current.isValid())
    {
        currentItem = model()->item(current);
    }

    DConfigDlgWdgItem* beforeItem = 0;

    if (before.isValid())
    {
        beforeItem = model()->item(before);
    }

    Q_Q(DConfigDlgWdg);
    emit q->currentPageChanged(currentItem, beforeItem);
}

DConfigDlgWdg::DConfigDlgWdg(DConfigDlgWdgPrivate& dd, QWidget* const parent)
    : DConfigDlgView(dd, parent)
{
    Q_D(DConfigDlgWdg);

    connect(this, SIGNAL(currentPageChanged(QModelIndex,QModelIndex)),
            this, SLOT(_k_slotCurrentPageChanged(QModelIndex,QModelIndex)));

    if (!d->DConfigDlgViewPrivate::model)
    {
        setModel(new DConfigDlgWdgModel(this));
    }
    else
    {
        Q_ASSERT(qobject_cast<DConfigDlgWdgModel*>(d->DConfigDlgViewPrivate::model));
    }

    connect(d->model(), &DConfigDlgWdgModel::toggled,
            this, &DConfigDlgWdg::pageToggled);
}

DConfigDlgWdg::DConfigDlgWdg(QWidget* const parent)
    : DConfigDlgView(*new DConfigDlgWdgPrivate(this), parent)
{
    Q_D(DConfigDlgWdg);

    connect(this, SIGNAL(currentPageChanged(QModelIndex,QModelIndex)),
            this, SLOT(_k_slotCurrentPageChanged(QModelIndex,QModelIndex)));

    setModel(new DConfigDlgWdgModel(this));

    connect(d->model(), &DConfigDlgWdgModel::toggled,
            this, &DConfigDlgWdg::pageToggled);
}

DConfigDlgWdg::~DConfigDlgWdg()
{
}

DConfigDlgWdgItem* DConfigDlgWdg::addPage(QWidget* widget, const QString& name)
{
    // force layout margin to zero so that it aligns well with title widget
    if (widget->layout())
    {
        widget->layout()->setMargin(0);
    }

    return d_func()->model()->addPage(widget, name);
}

void DConfigDlgWdg::addPage(DConfigDlgWdgItem* item)
{
    d_func()->model()->addPage(item);
}

DConfigDlgWdgItem* DConfigDlgWdg::insertPage(DConfigDlgWdgItem* before, QWidget* widget, const QString& name)
{
    return d_func()->model()->insertPage(before, widget, name);
}

void DConfigDlgWdg::insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item)
{
    d_func()->model()->insertPage(before, item);
}

DConfigDlgWdgItem* DConfigDlgWdg::addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name)
{
    return d_func()->model()->addSubPage(parent, widget, name);
}

void DConfigDlgWdg::addSubPage(DConfigDlgWdgItem* parent, DConfigDlgWdgItem* item)
{
    d_func()->model()->addSubPage(parent, item);
}

void DConfigDlgWdg::removePage(DConfigDlgWdgItem* item)
{
    emit pageRemoved(item); // Q_EMIT signal before we remove it, because the item will be deleted in the model
    d_func()->model()->removePage(item);
}

void DConfigDlgWdg::setCurrentPage(DConfigDlgWdgItem* item)
{
    const QModelIndex index = d_func()->model()->index(item);

    if (!index.isValid())
    {
        return;
    }

    DConfigDlgView::setCurrentPage(index);
}

DConfigDlgWdgItem* DConfigDlgWdg::currentPage() const
{
    const QModelIndex index = DConfigDlgView::currentPage();

    if (!index.isValid())
    {
        return 0;
    }

    return d_func()->model()->item(index);
}

// -----------------------------------------------------------------------------------------

DConfigDlgTitle::DConfigDlgTitle(QWidget* const parent)
    : QWidget(parent),
      d(new Private(this))
{
    QFrame* const titleFrame = new QFrame(this);
    titleFrame->setAutoFillBackground(true);
    titleFrame->setFrameShape(QFrame::StyledPanel);
    titleFrame->setFrameShadow(QFrame::Plain);
    titleFrame->setBackgroundRole(QPalette::Base);

    // default image / text part start
    d->headerLayout = new QGridLayout(titleFrame);
    d->headerLayout->setColumnStretch(0, 1);
    d->headerLayout->setMargin(6);

    d->textLabel = new QLabel(titleFrame);
    d->textLabel->setVisible(false);
    d->textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);

    d->imageLabel = new QLabel(titleFrame);
    d->imageLabel->setVisible(false);

    d->headerLayout->addWidget(d->textLabel,  0, 0);
    d->headerLayout->addWidget(d->imageLabel, 0, 1, 1, 2);

    d->commentLabel = new QLabel(titleFrame);
    d->commentLabel->setVisible(false);
    d->commentLabel->setOpenExternalLinks(true);
    d->commentLabel->setWordWrap(true);
    d->commentLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    d->headerLayout->addWidget(d->commentLabel, 1, 0);

    // default image / text part end

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleFrame);
    mainLayout->setMargin(0);
    setLayout(mainLayout);
}

DConfigDlgTitle::~DConfigDlgTitle()
{
    delete d;
}

bool DConfigDlgTitle::eventFilter(QObject* object, QEvent* event)
{
    // Hide message label on click
    if (d->autoHideTimeout > 0 &&
        event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent && mouseEvent->button() == Qt::LeftButton)
        {
            setVisible(false);
            return true;
        }
    }

    return QWidget::eventFilter(object, event);
}

void DConfigDlgTitle::setWidget(QWidget* const widget)
{
    d->headerLayout->addWidget(widget, 2, 0, 1, 2);
}

QString DConfigDlgTitle::text() const
{
    return d->textLabel->text();
}

QString DConfigDlgTitle::comment() const
{
    return d->commentLabel->text();
}

const QPixmap *DConfigDlgTitle::pixmap() const
{
    return d->imageLabel->pixmap();
}

void DConfigDlgTitle::setBuddy(QWidget* const buddy)
{
    d->textLabel->setBuddy(buddy);
}

void DConfigDlgTitle::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);

    if (e->type() == QEvent::PaletteChange || e->type() == QEvent::FontChange
                                           || e->type() == QEvent::ApplicationFontChange)
    {
        d->textLabel->setStyleSheet(d->textStyleSheet());
        d->commentLabel->setStyleSheet(d->commentStyleSheet());
    }
}

void DConfigDlgTitle::setText(const QString& text, Qt::Alignment alignment)
{
    d->textLabel->setVisible(!text.isNull());

    if (!Qt::mightBeRichText(text))
    {
        d->textLabel->setStyleSheet(d->textStyleSheet());
    }

    d->textLabel->setText(text);
    d->textLabel->setAlignment(alignment);
    show();
}

void DConfigDlgTitle::setText(const QString& text, MessageType type)
{
    setPixmap(type);
    setText(text);
}

void DConfigDlgTitle::setComment(const QString& comment, MessageType type)
{
    d->commentLabel->setVisible(!comment.isNull());

    //TODO: should we override the current icon with the corresponding MessageType icon?
    d->messageType = type;
    d->commentLabel->setStyleSheet(d->commentStyleSheet());
    d->commentLabel->setText(comment);
    show();
}

void DConfigDlgTitle::setPixmap(const QPixmap& pixmap, ImageAlignment alignment)
{
    d->imageLabel->setVisible(!pixmap.isNull());

    d->headerLayout->removeWidget(d->textLabel);
    d->headerLayout->removeWidget(d->commentLabel);
    d->headerLayout->removeWidget(d->imageLabel);

    if (alignment == ImageLeft)
    {
        // swap the text and image labels around
        d->headerLayout->addWidget(d->imageLabel,   0, 0, 2, 1);
        d->headerLayout->addWidget(d->textLabel,    0, 1);
        d->headerLayout->addWidget(d->commentLabel, 1, 1);
        d->headerLayout->setColumnStretch(0, 0);
        d->headerLayout->setColumnStretch(1, 1);
    }
    else
    {
        d->headerLayout->addWidget(d->textLabel,    0, 0);
        d->headerLayout->addWidget(d->commentLabel, 1, 0);
        d->headerLayout->addWidget(d->imageLabel,   0, 1, 2, 1);
        d->headerLayout->setColumnStretch(1, 0);
        d->headerLayout->setColumnStretch(0, 1);
    }

    d->imageLabel->setPixmap(pixmap);
}

void DConfigDlgTitle::setPixmap(const QString& icon, ImageAlignment alignment)
{
    setPixmap(QIcon::fromTheme(icon), alignment);
}

void DConfigDlgTitle::setPixmap(const QIcon& icon, ImageAlignment alignment)
{
    setPixmap(icon.pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize)), alignment);
}

void DConfigDlgTitle::setPixmap(MessageType type, ImageAlignment alignment)
{
    setPixmap(QIcon::fromTheme(d->iconTypeToIconName(type)), alignment);
}

int DConfigDlgTitle::autoHideTimeout() const
{
    return d->autoHideTimeout;
}

void DConfigDlgTitle::setAutoHideTimeout(int msecs)
{
    d->autoHideTimeout = msecs;

    if (msecs > 0)
    {
        installEventFilter(this);
    }
    else
    {
        removeEventFilter(this);
    }
}

void DConfigDlgTitle::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)

    if (d->autoHideTimeout > 0)
    {
        QTimer::singleShot(d->autoHideTimeout, this, SLOT(_k_timeoutFinished()));
    }
}

}  // namespace Digikam

#include "moc_dconfigdlgwidgets.cpp"
