/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "progressmanager.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

namespace Digikam
{

unsigned int ProgressManager::s_uID = 1000;

ProgressItem::ProgressItem( ProgressItem* parent, const QString& id,
                            const QString& label, const QString& status,
                            bool canBeCanceled, bool hasThumb)
    : mId(id),
      mLabel(label),
      mStatus(status),
      mParent(parent),
      mCanBeCanceled(canBeCanceled),
      mHasThumb(hasThumb),
      mProgress(0),
      mTotal(0),
      mCompleted(0),
      mWaitingForKids(false),
      mCanceled(false),
      mUsesBusyIndicator(false)
{
}

ProgressItem::~ProgressItem()
{
}

void ProgressItem::setComplete()
{
    if ( mChildren.isEmpty() )
    {
        if ( !mCanceled )
        {
            setProgress( 100 );
        }
        emit progressItemCompleted( this );
        if ( parent() )
        {
            parent()->removeChild( this );
        }
        deleteLater();
    }
    else
    {
        mWaitingForKids = true;
    }
}

void ProgressItem::addChild( ProgressItem *kiddo )
{
    mChildren.insert( kiddo, true );
}

void ProgressItem::removeChild( ProgressItem *kiddo )
{
    mChildren.remove( kiddo );
    // in case we were waiting for the last kid to go away, now is the time
    if ( mChildren.count() == 0 && mWaitingForKids )
    {
        emit progressItemCompleted( this );
        deleteLater();
    }
}

void ProgressItem::cancel()
{
    if ( mCanceled || !mCanBeCanceled )
    {
        return;
    }

    kDebug() << label();
    mCanceled = true;
    // Cancel all children.
    QList<ProgressItem*> kids = mChildren.keys();
    QList<ProgressItem*>::Iterator it( kids.begin() );
    QList<ProgressItem*>::Iterator end( kids.end() );

    for ( ; it != end; it++ )
    {
        ProgressItem *kid = *it;
        if ( kid->canBeCanceled() )
        {
            kid->cancel();
        }
    }
    setStatus( i18n( "Aborting..." ) );
    emit progressItemCanceled( this );
}

void ProgressItem::setProgress( unsigned int v )
{
    mProgress = v;
    // kDebug() << label() << " :" << v;
    emit progressItemProgress( this, mProgress );
}

void ProgressItem::setLabel( const QString &v )
{
    mLabel = v;
    emit progressItemLabel( this, mLabel );
}

void ProgressItem::setStatus(const QString& v)
{
    mStatus = v;
    emit progressItemStatus(this, mStatus);
}

void ProgressItem::setUsesBusyIndicator(bool useBusyIndicator)
{
    mUsesBusyIndicator = useBusyIndicator;
    emit progressItemUsesBusyIndicator(this, useBusyIndicator);
}

void ProgressItem::setThumbnail(const QPixmap& thumb)
{
    if (!hasThumbnail()) return;
    
    QPixmap pix = thumb;

    if (pix.isNull())
    {
        pix = DesktopIcon("image-missing", KIconLoader::SizeMedium);    // 32x32 px
    }
    else
    {
        pix = pix.scaled(22, 22, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    emit progressItemThumbnail(this, pix);
}

// --------------------------------------------------------------------------

struct ProgressManagerPrivate
{
    ProgressManager instance;
};

K_GLOBAL_STATIC( ProgressManagerPrivate, progressManagerPrivate )

ProgressManager::ProgressManager()
    : QObject()
{
}

ProgressManager::~ProgressManager()
{
}

ProgressManager* ProgressManager::instance()
{
    return progressManagerPrivate.isDestroyed() ? 0 : &progressManagerPrivate->instance ;
}

ProgressItem* ProgressManager::createProgressItemImpl(ProgressItem* parent,
                                                      const QString& id,
                                                      const QString& label,
                                                      const QString& status,
                                                      bool  cancellable,
                                                      bool  hasThumb
                                                     )
{
    ProgressItem* t = 0;
    
    if ( !mTransactions.value( id ) )
    {
        t = new ProgressItem(parent, id, label, status, cancellable, hasThumb);
        addProgressItemImpl(t, parent);
    }
    else
    {
        // Hm, is this what makes the most sense?
        t = mTransactions.value( id );
    }
    return t;
}

ProgressItem* ProgressManager::createProgressItemImpl(const QString& parent,
                                                      const QString& id,
                                                      const QString& label,
                                                      const QString& status,
                                                      bool  canBeCanceled,
                                                      bool  hasThumb
                                                     )
{
    ProgressItem* p = mTransactions.value(parent);
    return createProgressItemImpl(p, id, label, status, canBeCanceled, hasThumb);
}

void ProgressManager::addProgressItemImpl(ProgressItem* t, ProgressItem* parent)
{
    mTransactions.insert(t->id(), t);
    if (parent)
    {
        ProgressItem* p = mTransactions.value( parent->id() );
        if ( p )
        {
            p->addChild( t );
        }
    }

    connect(t, SIGNAL(progressItemCompleted(Digikam::ProgressItem*)),
            this, SLOT(slotTransactionCompleted(Digikam::ProgressItem*)));

    connect(t, SIGNAL(progressItemProgress(Digikam::ProgressItem*, unsigned int)),
            this, SIGNAL(progressItemProgress(Digikam::ProgressItem*, unsigned int)));

    connect(t, SIGNAL(progressItemAdded(Digikam::ProgressItem*)),
            this, SIGNAL(progressItemAdded(Digikam::ProgressItem*)));

    connect(t, SIGNAL(progressItemCanceled(Digikam::ProgressItem*)),
            this, SIGNAL(progressItemCanceled(Digikam::ProgressItem*)));

    connect(t, SIGNAL(progressItemStatus(Digikam::ProgressItem*, const QString&)),
            this, SIGNAL(progressItemStatus(Digikam::ProgressItem*, const QString&)));

    connect(t, SIGNAL(progressItemLabel(Digikam::ProgressItem*, const QString&)),
            this, SIGNAL(progressItemLabel(Digikam::ProgressItem*, const QString&)));

    connect(t, SIGNAL(progressItemUsesBusyIndicator(Digikam::ProgressItem*, bool)),
            this, SIGNAL(progressItemUsesBusyIndicator(Digikam::ProgressItem*, bool)));

    connect(t, SIGNAL(progressItemThumbnail(Digikam::ProgressItem*, const QPixmap&)),
            this, SIGNAL(progressItemThumbnail(Digikam::ProgressItem*, const QPixmap&)));

    emit progressItemAdded( t );
}

void ProgressManager::emitShowProgressViewImpl()
{
    emit showProgressView();
}

void ProgressManager::slotTransactionCompleted(ProgressItem* item)
{
    mTransactions.remove( item->id() );
    emit progressItemCompleted( item );
}

void ProgressManager::slotStandardCancelHandler(ProgressItem* item)
{
    item->setComplete();
}

ProgressItem* ProgressManager::singleItem() const
{
    ProgressItem* item                                 = 0;
    QHash< QString, ProgressItem* >::const_iterator it = mTransactions.constBegin();
    while ( it != mTransactions.constEnd() )
    {
        // No single item for progress possible, as one of them is a busy indicator one.
        if ( (*it)->usesBusyIndicator() )
            return 0;

        if ( !(*it)->parent() )
        {             // if it's a top level one, only those count
            if ( item )
            {
                return 0; // we found more than one
            }
            else
            {
                item = (*it);
            }
        }
        ++it;
    }
    return item;
}

void ProgressManager::slotAbortAll()
{
    QHashIterator<QString, ProgressItem*> it(mTransactions);
    while (it.hasNext())
    {
        it.next();
        it.value()->cancel();
    }
}

} // namespace Digikam
