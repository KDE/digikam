/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2004 Till Adam <adam at kde dot org>
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

#include "progressview.moc"

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

// KDE includes

#include <KDebug>
#include <KDialog>
#include <KHBox>
#include <KIconLoader>
#include <KLocale>
#include <KStandardGuiItem>
#include <KVBox>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

static const int MAX_LABEL_WIDTH = 650;

class TransactionItem;

TransactionItemView::TransactionItemView(QWidget* parent, const char* name)
    : QScrollArea( parent )
{
    setObjectName( name );
    setFrameStyle( NoFrame );
    mBigBox = new KVBox( this );
    setWidget( mBigBox );
    setWidgetResizable( true );
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
}

TransactionItem* TransactionItemView::addTransactionItem(ProgressItem* item, bool first)
{
    TransactionItem* ti = new TransactionItem( mBigBox, item, first );
    mBigBox->layout()->addWidget( ti );

    resize( mBigBox->width(), mBigBox->height() );

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
    QSize sz( mBigBox->minimumSizeHint() );
    sz.setWidth( qMax( sz.width(), minw ) + f + vsbExt );
    sz.setHeight( qMin( sz.height(), maxh ) + f );
    return sz;
}

void TransactionItemView::slotLayoutFirstItem()
{
    //This slot is called whenever a TransactionItem is deleted, so this is a
    //good place to call updateGeometry(), so our parent takes the new size
    //into account and resizes.
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
    TransactionItem* ti = mBigBox->findChild<TransactionItem*>("TransactionItem");
    if ( ti )
    {
        ti->hideHLine();
    }
}

// ----------------------------------------------------------------------------

TransactionItem::TransactionItem(QWidget* parent, ProgressItem* item, bool first)
    : KVBox(parent), mCancelButton(0), mItem(item)
{
    setSpacing(2);
    setMargin(2);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    mFrame = new QFrame(this);
    mFrame->setFrameShape(QFrame::HLine);
    mFrame->setFrameShadow(QFrame::Raised);
    mFrame->show();
    setStretchFactor(mFrame, 3);
    layout()->addWidget(mFrame);

    KHBox* h = new KHBox(this);
    h->setSpacing(5);
    layout()->addWidget(h);

    if (item->hasThumbnail())
    {
        mItemThumb = new QLabel(h);
        mItemThumb->setFixedSize(QSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium));
        h->layout()->addWidget(mItemThumb);
        h->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    }

    mItemLabel = new QLabel(fontMetrics().elidedText(item->label(), Qt::ElideRight, MAX_LABEL_WIDTH), h);
    h->layout()->addWidget(mItemLabel);
    h->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    mProgress = new QProgressBar(h);
    mProgress->setMaximum(100);
    mProgress->setValue(item->progress());
    h->layout()->addWidget(mProgress);

    if (item->canBeCanceled())
    {
        mCancelButton = new QPushButton(SmallIcon("dialog-cancel"), QString(), h);
        mCancelButton->setToolTip( i18n("Cancel this operation."));
        connect(mCancelButton, SIGNAL( clicked()),
                this, SLOT(slotItemCanceled()));
        h->layout()->addWidget(mCancelButton);
    }

    h = new KHBox(this);
    h->setSpacing(5);
    h->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    layout()->addWidget(h);

    mItemStatus = new QLabel(h);
    mItemStatus->setTextFormat(Qt::RichText);
    mItemStatus->setText(fontMetrics().elidedText(item->status(), Qt::ElideRight, MAX_LABEL_WIDTH));
    h->layout()->addWidget(mItemStatus);
    if (first)
    {
        hideHLine();
    }
}

TransactionItem::~TransactionItem()
{
}

void TransactionItem::hideHLine()
{
    mFrame->hide();
}

void TransactionItem::setProgress(int progress)
{
    mProgress->setValue(progress);
}

void TransactionItem::setLabel(const QString& label)
{
    mItemLabel->setText(fontMetrics().elidedText(label, Qt::ElideRight, MAX_LABEL_WIDTH));
}

void TransactionItem::setThumbnail(const QPixmap& thumb)
{
    mItemThumb->setPixmap(thumb);
}

void TransactionItem::setStatus(const QString& status)
{
    mItemStatus->setText(fontMetrics().elidedText(status, Qt::ElideRight, MAX_LABEL_WIDTH));
}

void TransactionItem::setTotalSteps(int totalSteps)
{
    mProgress->setMaximum(totalSteps);
}

void TransactionItem::slotItemCanceled()
{
    if ( mItem )
    {
        mItem->cancel();
    }
}

void TransactionItem::addSubTransaction(ProgressItem* item)
{
    Q_UNUSED(item);
}

// ---------------------------------------------------------------------------

ProgressView::ProgressView(QWidget* alignWidget, QWidget* parent, const char* name)
    : OverlayWidget(alignWidget, parent, name), mWasLastShown(false)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);

    setAutoFillBackground(true);

    mScrollView = new TransactionItemView( this, "ProgressScrollView" );
    layout()->addWidget( mScrollView );

    // No more close button for now, since there is no more autoshow
    /*
        QVBox* rightBox = new QVBox( this );
        QToolButton* pbClose = new QToolButton( rightBox );
        pbClose->setAutoRaise(true);
        pbClose->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
        pbClose->setFixedSize( 16, 16 );
        pbClose->setIcon( KIconLoader::global()->loadIconSet( "window-close", KIconLoader::Small, 14 ) );
        pbClose->setToolTip( i18n( "Hide detailed progress window" ) );
        connect(pbClose, SIGNAL(clicked()), this, SLOT(slotClose()));
        QWidget* spacer = new QWidget( rightBox ); // don't let the close button take up all the height
        rightBox->setStretchFactor( spacer, 100 );
    */

    /*
    * Get the singleton ProgressManager item which will inform us of
    * appearing and vanishing items.
    */
    ProgressManager* pm = ProgressManager::instance();
    
    connect(pm, SIGNAL(progressItemAdded(ProgressItem*)),
            this, SLOT(slotTransactionAdded(ProgressItem*)));
    
    connect(pm, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotTransactionCompleted(ProgressItem*)));
    
    connect(pm, SIGNAL(progressItemProgress(ProgressItem*, unsigned int)),
            this, SLOT(slotTransactionProgress(ProgressItem*, unsigned int)));
    
    connect(pm, SIGNAL(progressItemStatus(ProgressItem*, const QString&)),
            this, SLOT(slotTransactionStatus(ProgressItem*, const QString&)));
    
    connect(pm, SIGNAL(progressItemLabel(ProgressItem*, const QString&)),
            this, SLOT(slotTransactionLabel(ProgressItem*, const QString&)));
    
    connect(pm, SIGNAL(progressItemUsesBusyIndicator(ProgressItem*, bool)),
            this, SLOT(slotTransactionUsesBusyIndicator(ProgressItem*, bool)));

    connect(pm, SIGNAL(progressItemThumbnail(ProgressItem*, const QPixmap&)),
            this, SLOT(slotTransactionThumbnail(ProgressItem*, const QPixmap&)));
    
    connect(pm, SIGNAL(showProgressView()),
            this, SLOT(slotShow()));
}

void ProgressView::closeEvent(QCloseEvent* e)
{
    e->accept();
    hide();
}

ProgressView::~ProgressView()
{
    // no need to delete child widgets.
}

void ProgressView::slotTransactionAdded(ProgressItem* item)
{
    TransactionItem* parent = 0;
    if ( item->parent() )
    {
        if ( mTransactionsToListviewItems.contains( item->parent() ) )
        {
            parent = mTransactionsToListviewItems[ item->parent() ];
            parent->addSubTransaction( item );
        }
    }
    else
    {
        const bool first    = mTransactionsToListviewItems.empty();
        TransactionItem* ti = mScrollView->addTransactionItem( item, first );
        if ( ti )
        {
            mTransactionsToListviewItems.insert( item, ti );
        }
        if ( first && mWasLastShown )
        {
            QTimer::singleShot( 1000, this, SLOT( slotShow() ) );
        }
    }
}

void ProgressView::slotTransactionCompleted(ProgressItem* item)
{
    if ( mTransactionsToListviewItems.contains( item ) )
    {
        TransactionItem* ti = mTransactionsToListviewItems[item];
        mTransactionsToListviewItems.remove( item );
        ti->setItemComplete();
        QTimer::singleShot( 3000, ti, SLOT( deleteLater() ) );

        // see the slot for comments as to why that works
        connect ( ti, SIGNAL( destroyed() ),
                mScrollView, SLOT( slotLayoutFirstItem() ) );
    }
    // This was the last item, hide.
    if ( mTransactionsToListviewItems.empty() )
    {
        QTimer::singleShot( 3000, this, SLOT( slotHide() ) );
    }
}

void ProgressView::slotTransactionCanceled(ProgressItem*)
{
}

void ProgressView::slotTransactionProgress(ProgressItem* item, unsigned int progress)
{
    if (mTransactionsToListviewItems.contains(item))
    {
        TransactionItem* ti = mTransactionsToListviewItems[item];
        ti->setProgress(progress);
    }
}

void ProgressView::slotTransactionStatus(ProgressItem* item, const QString& status)
{
    if (mTransactionsToListviewItems.contains(item))
    {
        TransactionItem* ti = mTransactionsToListviewItems[item];
        ti->setStatus(status);
    }
}

void ProgressView::slotTransactionLabel(ProgressItem* item, const QString& label )
{
    if ( mTransactionsToListviewItems.contains(item))
    {
        TransactionItem* ti = mTransactionsToListviewItems[item];
        ti->setLabel(label);
    }
}

void ProgressView::slotTransactionUsesBusyIndicator(ProgressItem* item, bool value)
{
    if (mTransactionsToListviewItems.contains(item))
    {
        TransactionItem* ti = mTransactionsToListviewItems[item];
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
    if (mTransactionsToListviewItems.contains(item))
    {
        TransactionItem* ti = mTransactionsToListviewItems[item];
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
    if ( mTransactionsToListviewItems.isEmpty() )
    {
        setVisible(false);
    }
}

void ProgressView::slotClose()
{
    mWasLastShown = false;
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
    mWasLastShown = isHidden();
    if ( !isHidden() || !mTransactionsToListviewItems.isEmpty() )
    {
        setVisible( isHidden() );
    }
}

} // namespace Digikam
