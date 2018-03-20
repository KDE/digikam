/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2004      by Till Adam <adam at kde dot org>
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

#include "progressview.h"

// Qt includes

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <QToolButton>
#include <QMap>
#include <QPixmap>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "progressmanager.h"

namespace Digikam
{

class TransactionItem;

TransactionItemView::TransactionItemView(QWidget* const parent, const QString& name)
    : QScrollArea(parent)
{
    setObjectName(name);
    setFrameStyle(NoFrame);
    m_bigBox = new DVBox(this);
    setWidget(m_bigBox);
    setWidgetResizable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

TransactionItem* TransactionItemView::addTransactionItem(ProgressItem* const item, bool first)
{
    TransactionItem* const ti = new TransactionItem(m_bigBox, item, first);
    m_bigBox->layout()->addWidget(ti);
    resize(m_bigBox->width(), m_bigBox->height());

    return ti;
}

void TransactionItemView::resizeEvent(QResizeEvent* event)
{
    // Tell the layout in the parent (progressview) that our size changed
    updateGeometry();

    QSize sz         = parentWidget()->sizeHint();
    int currentWidth = parentWidget()->width();

    // Don't resize to sz.width() every time when it only reduces a little bit
    if ( currentWidth < sz.width() || currentWidth > sz.width() + 100 )
    {
        currentWidth = sz.width();
    }

    parentWidget()->resize( currentWidth, sz.height() );

    QScrollArea::resizeEvent( event );
}

QSize TransactionItemView::sizeHint() const
{
    return minimumSizeHint();
}

QSize TransactionItemView::minimumSizeHint() const
{
    int f      = 2 * frameWidth();
    // Make room for a vertical scrollbar in all cases, to avoid a horizontal one
    int vsbExt = verticalScrollBar()->sizeHint().width();
    int minw   = topLevelWidget()->width() / 3;
    int maxh   = topLevelWidget()->height() / 2;
    QSize sz( m_bigBox->minimumSizeHint() );
    sz.setWidth( qMax( sz.width(), minw ) + f + vsbExt );
    sz.setHeight( qMin( sz.height(), maxh ) + f );

    return sz;
}

void TransactionItemView::slotLayoutFirstItem()
{
    // This slot is called whenever a TransactionItem is deleted, so this is a
    // good place to call updateGeometry(), so our parent takes the new size
    // into account and resizes.
    updateGeometry();

    /*
        The below relies on some details in Qt's behaviour regarding deleting
        objects. This slot is called from the destroyed signal of an item just
        going away. That item is at that point still in the  list of chilren, but
        since the vtable is already gone, it will have type QObject. The first
        one with both the right name and the right class therefor is what will
        be the first item very shortly. That's the one we want to remove the
        hline for.
    */
    TransactionItem* const ti = m_bigBox->findChild<TransactionItem*>(QLatin1String("TransactionItem"));

    if (ti)
    {
        ti->hideHLine();
    }
    else
    {
        emit signalTransactionViewIsEmpty();
    }
}

// ----------------------------------------------------------------------------

class TransactionItem::Private
{
public:

    Private() :
        maxLabelWidth(650),
        progress(0),
        cancelButton(0),
        itemLabel(0),
        itemStatus(0),
        itemThumb(0),
        frame(0),
        item(0)
    {
    }

    const int     maxLabelWidth;

    QProgressBar* progress;
    QPushButton*  cancelButton;
    QLabel*       itemLabel;
    QLabel*       itemStatus;
    QLabel*       itemThumb;
    QFrame*       frame;

    ProgressItem* item;
};

TransactionItem::TransactionItem(QWidget* const parent, ProgressItem* const item, bool first)
    : DVBox(parent),
      d(new Private)
{
    d->item  = item;
    setSpacing(2);
    setContentsMargins(2, 2, 2, 2);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    d->frame = new QFrame(this);
    d->frame->setFrameShape(QFrame::HLine);
    d->frame->setFrameShadow(QFrame::Raised);
    d->frame->show();
    setStretchFactor(d->frame, 3);
    layout()->addWidget(d->frame);

    DHBox* h = new DHBox(this);
    h->setSpacing(5);
    layout()->addWidget(h);

    if (item->hasThumbnail())
    {
        d->itemThumb = new QLabel(h);
        d->itemThumb->setFixedSize(QSize(22, 22));
        h->layout()->addWidget(d->itemThumb);
        h->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    }

    d->itemLabel = new QLabel(fontMetrics().elidedText(item->label(), Qt::ElideRight, d->maxLabelWidth), h);
    h->layout()->addWidget(d->itemLabel);
    h->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    d->progress = new QProgressBar(h);
    d->progress->setMaximum(100);
    d->progress->setValue(item->progress());
    h->layout()->addWidget(d->progress);

    if (item->canBeCanceled())
    {
        d->cancelButton = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-cancel")), QString(), h);
        d->cancelButton->setToolTip( i18n("Cancel this operation."));

        connect(d->cancelButton, SIGNAL(clicked()),
                this, SLOT(slotItemCanceled()));

        h->layout()->addWidget(d->cancelButton);
    }

    h = new DHBox(this);
    h->setSpacing(5);
    h->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    layout()->addWidget(h);

    d->itemStatus = new QLabel(h);
    d->itemStatus->setTextFormat(Qt::RichText);
    d->itemStatus->setText(fontMetrics().elidedText(item->status(), Qt::ElideRight, d->maxLabelWidth));
    h->layout()->addWidget(d->itemStatus);

    if (first)
    {
        hideHLine();
    }
}

TransactionItem::~TransactionItem()
{
    delete d;
}

ProgressItem* TransactionItem::item() const
{
    return d->item;
}

void TransactionItem::hideHLine()
{
    d->frame->hide();
}

void TransactionItem::setProgress(int progress)
{
    d->progress->setValue(progress);
}

void TransactionItem::setItemComplete()
{
    d->item = 0;
}

void TransactionItem::setLabel(const QString& label)
{
    d->itemLabel->setText(fontMetrics().elidedText(label, Qt::ElideRight, d->maxLabelWidth));
}

void TransactionItem::setThumbnail(const QPixmap& thumb)
{
    d->itemThumb->setPixmap(thumb);
}

void TransactionItem::setStatus(const QString& status)
{
    d->itemStatus->setText(fontMetrics().elidedText(status, Qt::ElideRight, d->maxLabelWidth));
}

void TransactionItem::setTotalSteps(int totalSteps)
{
    d->progress->setMaximum(totalSteps);
}

void TransactionItem::slotItemCanceled()
{
    if ( d->item )
    {
        d->item->cancel();
    }
}

void TransactionItem::addSubTransaction(ProgressItem* const item)
{
    Q_UNUSED(item);
}

// ---------------------------------------------------------------------------

class ProgressView::Private
{
public:

    Private() :
        wasLastShown(false),
        scrollView(0),
        previousItem(0)
    {
    }

    bool                                        wasLastShown;
    TransactionItemView*                        scrollView;
    TransactionItem*                            previousItem;
    QMap<const ProgressItem*, TransactionItem*> transactionsToListviewItems;
};

ProgressView::ProgressView(QWidget* const alignWidget, QWidget* const parent, const QString& name)
    : OverlayWidget(alignWidget, parent, name),
      d(new Private)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setAutoFillBackground(true);

    d->scrollView = new TransactionItemView(this, QLatin1String("ProgressScrollView"));
    layout()->addWidget( d->scrollView );

    // No more close button for now, since there is no more autoshow
/*
    QVBox* const rightBox      = new QVBox( this );
    QToolButton* const pbClose = new QToolButton( rightBox );
    pbClose->setAutoRaise(true);
    pbClose->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    pbClose->setFixedSize( 16, 16 );
    pbClose->setIcon( KIconLoader::global()->loadIconSet( "window-close", KIconLoader::Small, 14 ) );
    pbClose->setToolTip( i18n( "Hide detailed progress window" ) );
    connect(pbClose, SIGNAL(clicked()), this, SLOT(slotClose()));
    QWidget* const spacer = new QWidget( rightBox ); // don't let the close button take up all the height
    rightBox->setStretchFactor( spacer, 100 );
*/

    /*
     * Get the singleton ProgressManager item which will inform us of
     * appearing and vanishing items.
     */
    ProgressManager* const pm = ProgressManager::instance();

    connect(pm, SIGNAL(progressItemAdded(ProgressItem*)),
            this, SLOT(slotTransactionAdded(ProgressItem*)));

    connect(pm, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotTransactionCompleted(ProgressItem*)));

    connect(pm, SIGNAL(progressItemProgress(ProgressItem*,uint)),
            this, SLOT(slotTransactionProgress(ProgressItem*,uint)));

    connect(pm, SIGNAL(progressItemStatus(ProgressItem*,QString)),
            this, SLOT(slotTransactionStatus(ProgressItem*,QString)));

    connect(pm, SIGNAL(progressItemLabel(ProgressItem*,QString)),
            this, SLOT(slotTransactionLabel(ProgressItem*,QString)));

    connect(pm, SIGNAL(progressItemUsesBusyIndicator(ProgressItem*,bool)),
            this, SLOT(slotTransactionUsesBusyIndicator(ProgressItem*,bool)));

    connect(pm, SIGNAL(progressItemThumbnail(ProgressItem*,QPixmap)),
            this, SLOT(slotTransactionThumbnail(ProgressItem*,QPixmap)));

    connect(pm, SIGNAL(showProgressView()),
            this, SLOT(slotShow()));

    connect(d->scrollView, SIGNAL(signalTransactionViewIsEmpty()),
            pm, SLOT(slotTransactionViewIsEmpty()));
}

ProgressView::~ProgressView()
{
    // NOTE: no need to delete child widgets.
    delete d;
}

void ProgressView::closeEvent(QCloseEvent* e)
{
    e->accept();
    hide();
}

void ProgressView::slotTransactionAdded(ProgressItem* item)
{
    TransactionItem* parent = 0;

    if ( item->parent() )
    {
        if ( d->transactionsToListviewItems.contains( item->parent() ) )
        {
            parent = d->transactionsToListviewItems[ item->parent() ];
            parent->addSubTransaction( item );
        }
    }
    else
    {
        const bool first          = d->transactionsToListviewItems.empty();
        TransactionItem* const ti = d->scrollView->addTransactionItem( item, first );

        if ( ti )
        {
            d->transactionsToListviewItems.insert( item, ti );
        }

        if (item->showAtStart())
        {
            // Force to show progress view for 5 seconds to inform user about new process add in queue.
            QTimer::singleShot( 1000, this, SLOT(slotShow()) );
            QTimer::singleShot( 6000, this, SLOT(slotClose()) );
            return;
        }

        if (first && d->wasLastShown)
        {
            QTimer::singleShot( 1000, this, SLOT(slotShow()) );
        }
    }
}

void ProgressView::slotTransactionCompleted(ProgressItem* item)
{
    if ( d->transactionsToListviewItems.contains( item ) )
    {
        TransactionItem* const ti = d->transactionsToListviewItems[item];
        d->transactionsToListviewItems.remove( item );
        ti->setItemComplete();
        QTimer::singleShot( 3000, ti, SLOT(deleteLater()) );

        // see the slot for comments as to why that works
        connect ( ti, SIGNAL(destroyed()),
                d->scrollView, SLOT(slotLayoutFirstItem()) );
    }

    // This was the last item, hide.
    if ( d->transactionsToListviewItems.empty() )
    {
        QTimer::singleShot( 3000, this, SLOT(slotHide()) );
    }
}

void ProgressView::slotTransactionCanceled(ProgressItem*)
{
}

void ProgressView::slotTransactionProgress(ProgressItem* item, unsigned int progress)
{
    if (d->transactionsToListviewItems.contains(item))
    {
        TransactionItem* const ti = d->transactionsToListviewItems[item];
        ti->setProgress(progress);
    }
}

void ProgressView::slotTransactionStatus(ProgressItem* item, const QString& status)
{
    if (d->transactionsToListviewItems.contains(item))
    {
        TransactionItem* const ti = d->transactionsToListviewItems[item];
        ti->setStatus(status);
    }
}

void ProgressView::slotTransactionLabel(ProgressItem* item, const QString& label )
{
    if ( d->transactionsToListviewItems.contains(item))
    {
        TransactionItem* const ti = d->transactionsToListviewItems[item];
        ti->setLabel(label);
    }
}

void ProgressView::slotTransactionUsesBusyIndicator(ProgressItem* item, bool value)
{
    if (d->transactionsToListviewItems.contains(item))
    {
        TransactionItem* const ti = d->transactionsToListviewItems[item];

        if (value)
        {
            ti->setTotalSteps(0);
        }
        else
        {
            ti->setTotalSteps(100);
        }
    }
}

void ProgressView::slotTransactionThumbnail(ProgressItem* item, const QPixmap& thumb)
{
    if (d->transactionsToListviewItems.contains(item))
    {
        TransactionItem* const ti = d->transactionsToListviewItems[item];
        ti->setThumbnail(thumb);
    }
}

void ProgressView::slotShow()
{
    setVisible(true);
}

void ProgressView::slotHide()
{
    // check if a new item showed up since we started the timer. If not, hide
    if ( d->transactionsToListviewItems.isEmpty() )
    {
        setVisible(false);
    }
}

void ProgressView::slotClose()
{
    d->wasLastShown = false;
    setVisible(false);
}

void ProgressView::setVisible(bool b)
{
    OverlayWidget::setVisible(b);
    emit visibilityChanged(b);
}

void ProgressView::slotToggleVisibility()
{
    /* Since we are only hiding with a timeout, there is a short period of
     * time where the last item is still visible, but clicking on it in
     * the statusbarwidget should not display the dialog, because there
     * are no items to be shown anymore. Guard against that.
     */
    d->wasLastShown = isHidden();

    if ( !isHidden() || !d->transactionsToListviewItems.isEmpty() )
    {
        setVisible( isHidden() );
    }
}

} // namespace Digikam
