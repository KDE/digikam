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

#include "progressmanager.moc"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmessagebox.h>

namespace Digikam
{

unsigned int ProgressManager::s_uID = 1000;

class ProgressItem::ProgressItemPriv
{
public:

    typedef QMap<ProgressItem*, bool> ProgressItemMap;

public:
    
    ProgressItemPriv() :
      waitingForKids(false),
      canceled(false),
      usesBusyIndicator(false),
      canBeCanceled(false),
      hasThumb(false),
      progress(0),
      total(0),
      completed(0),
      parent(0)
    {
    }

    bool            waitingForKids;
    bool            canceled;
    bool            usesBusyIndicator;
    bool            canBeCanceled;
    bool            hasThumb;

    unsigned int    progress;
    unsigned int    total;
    unsigned int    completed;

    QString         id;
    QString         label;
    QString         status;

    ProgressItem*   parent;
    ProgressItemMap children;
};

ProgressItem::ProgressItem(ProgressItem* parent, const QString& id,
                           const QString& label, const QString& status,
                           bool canBeCanceled, bool hasThumb)
    : d(new ProgressItemPriv)
{
      d->canBeCanceled = canBeCanceled;
      d->hasThumb      = hasThumb;
      d->id            = id;
      d->label         = label;
      d->status        = status;
      d->parent        = parent;
}

ProgressItem::~ProgressItem()
{
    delete d;
}

void ProgressItem::setComplete()
{
    if ( d->children.isEmpty() )
    {
        if ( !d->canceled )
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
        d->waitingForKids = true;
    }
}

void ProgressItem::addChild(ProgressItem* kiddo)
{
    d->children.insert(kiddo, true);
}

void ProgressItem::removeChild(ProgressItem* kiddo)
{
    d->children.remove(kiddo);

    // in case we were waiting for the last kid to go away, now is the time
    if (d->children.count() == 0 && d->waitingForKids)
    {
        emit progressItemCompleted(this);
        deleteLater();
    }
}

void ProgressItem::cancel()
{
    if ( d->canceled || !d->canBeCanceled )
    {
        return;
    }

    d->canceled = true;

    // Cancel all children.
    QList<ProgressItem*> kids = d->children.keys();
    QList<ProgressItem*>::Iterator it( kids.begin() );
    QList<ProgressItem*>::Iterator end( kids.end() );

    for ( ; it != end; it++ )
    {
        ProgressItem* kid = *it;
        if ( kid->canBeCanceled() )
        {
            kid->cancel();
        }
    }

    setStatus( i18n( "Aborting..." ) );

    emit progressItemCanceled( this );
    emit progressItemCanceled( this->id() );
}

void ProgressItem::setProgress(unsigned int v)
{
    d->progress = v;
    emit progressItemProgress(this, d->progress);
}

void ProgressItem::setLabel(const QString& v)
{
    d->label = v;
    emit progressItemLabel( this, d->label );
}

void ProgressItem::setStatus(const QString& v)
{
    d->status = v;
    emit progressItemStatus(this, d->status);
}

void ProgressItem::setUsesBusyIndicator(bool useBusyIndicator)
{
    d->usesBusyIndicator = useBusyIndicator;
    emit progressItemUsesBusyIndicator(this, useBusyIndicator);
}

void ProgressItem::setThumbnail(const QPixmap& thumb)
{
    if (!hasThumbnail()) return;

    QPixmap pix = thumb;

    if (pix.isNull())
    {
        pix = DesktopIcon("image-missing", KIconLoader::SizeSmallMedium);
    }
    else
    {
        pix = pix.scaled(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium, 
                         Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    emit progressItemThumbnail(this, pix);
}

void ProgressItem::reset()
{
    setProgress(0);
    setStatus(QString());
    d->completed = 0;
}

void ProgressItem::updateProgress()
{
    setProgress(d->total? d->completed * 100 / d->total : 0);
}

void ProgressItem::advance(unsigned int v)
{
    setCompletedItems(completedItems() + v);
    updateProgress();
}

void ProgressItem::setTotalItems(unsigned int v)
{
    d->total = v;
}

unsigned int ProgressItem::totalItems() const
{
    return d->total;
}

void ProgressItem::setCompletedItems(unsigned int v)
{
    d->completed = v;
}

unsigned int ProgressItem::completedItems() const
{
    return d->completed;
}

void ProgressItem::incCompletedItems(unsigned int v)
{
    d->completed += v;
}

bool ProgressItem::canceled() const
{
    return d->canceled;
}

const QString& ProgressItem::id() const
{
    return d->id;
}

ProgressItem* ProgressItem::parent() const
{
    return d->parent;
}

const QString& ProgressItem::label() const
{
    return d->label;
}

const QString& ProgressItem::status() const
{
    return d->status;
}

bool ProgressItem::canBeCanceled() const
{
    return d->canBeCanceled;
}

bool ProgressItem::usesBusyIndicator() const
{
    return d->usesBusyIndicator;
}

bool ProgressItem::hasThumbnail() const
{
    return d->hasThumb;
}

unsigned int ProgressItem::progress() const
{
    return d->progress;
}

// --------------------------------------------------------------------------

class ProgressManagerPrivate
{
public:

    ProgressManager instance;
};

K_GLOBAL_STATIC(ProgressManagerPrivate, progressManagerPrivate)

// --------------------------------------------------------------------------

ProgressManager::ProgressManager()
    : QObject()
{
}

ProgressManager::~ProgressManager()
{
}

bool ProgressManager::isEmpty() const
{
    return mTransactions.isEmpty();
}

ProgressItem* ProgressManager::findItembyId(const QString& id) const
{
    if (!id.isEmpty())
        return mTransactions.value(id, 0);

    return 0;
}

QString ProgressManager::getUniqueID()
{
    return QString::number( ++s_uID );
}

ProgressManager* ProgressManager::instance()
{
    return progressManagerPrivate.isDestroyed() ? 0 : &progressManagerPrivate->instance;
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

    if ( !findItembyId(id))
    {
        t = new ProgressItem(parent, id, label, status, cancellable, hasThumb);
        addProgressItemImpl(t, parent);
    }
    else
    {
        // Hm, is this what makes the most sense?
        t = findItembyId(id);
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
    ProgressItem* p = findItembyId(parent);
    return createProgressItemImpl(p, id, label, status, canBeCanceled, hasThumb);
}

bool ProgressManager::addProgressItem(ProgressItem* t, ProgressItem* parent)
{
    if (!instance()->findItembyId(t->id()))
    {
        instance()->addProgressItemImpl(t, parent);
        return true;
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("A tool named \"%1\" is already running....", t->label()));
        t->setComplete();
        return false;
    }
}

void ProgressManager::addProgressItemImpl(ProgressItem* t, ProgressItem* parent)
{
    mTransactions.insert(t->id(), t);
    if (parent)
    {
        ProgressItem* p = findItembyId(parent->id());
        if (p)
        {
            kDebug() << "Add child progress " << t->id() << " to " << p->id();
            p->addChild(t);
        }
    }

    connect(t, SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotTransactionCompleted(ProgressItem*)));

    connect(t, SIGNAL(progressItemProgress(ProgressItem*, unsigned int)),
            this, SIGNAL(progressItemProgress(ProgressItem*, unsigned int)));

    connect(t, SIGNAL(progressItemAdded(ProgressItem*)),
            this, SIGNAL(progressItemAdded(ProgressItem*)));

    connect(t, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SIGNAL(progressItemCanceled(ProgressItem*)));

    connect(t, SIGNAL(progressItemStatus(ProgressItem*, const QString&)),
            this, SIGNAL(progressItemStatus(ProgressItem*, const QString&)));

    connect(t, SIGNAL(progressItemLabel(ProgressItem*, const QString&)),
            this, SIGNAL(progressItemLabel(ProgressItem*, const QString&)));

    connect(t, SIGNAL(progressItemUsesBusyIndicator(ProgressItem*, bool)),
            this, SIGNAL(progressItemUsesBusyIndicator(ProgressItem*, bool)));

    connect(t, SIGNAL(progressItemThumbnail(ProgressItem*, const QPixmap&)),
            this, SIGNAL(progressItemThumbnail(ProgressItem*, const QPixmap&)));

    emit progressItemAdded(t);
}

void ProgressManager::emitShowProgressViewImpl()
{
    emit showProgressView();
}

void ProgressManager::slotTransactionCompleted(ProgressItem* item)
{
    if (item)
    {
        mTransactions.remove(item->id());
        emit progressItemCompleted(item);
    }
}

void ProgressManager::slotStandardCancelHandler(ProgressItem* item)
{
    item->setComplete();
}

ProgressItem* ProgressManager::singleItem() const
{
    ProgressItem* item                               = 0;
    QHash<QString, ProgressItem*>::const_iterator it = mTransactions.constBegin();
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

void ProgressManager::emitShowProgressView()
{
    instance()->emitShowProgressViewImpl();
}

ProgressItem* ProgressManager::createProgressItem(const QString& label, const QString& status, bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(0, getUniqueID(), label, status, canBeCanceled, hasThumb);
}

ProgressItem* ProgressManager::createProgressItem(ProgressItem* parent, const QString& id, const QString& label,
                                                  const QString& status, bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(parent, id, label, status, canBeCanceled, hasThumb);
}

ProgressItem* ProgressManager::createProgressItem(const QString& parent, const QString& id, const QString& label,
                                                  const QString& status, bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(parent, id, label, status, canBeCanceled, hasThumb);
}

ProgressItem* ProgressManager::createProgressItem(const QString& id, const QString& label, const QString& status,
                                                  bool canBeCanceled, bool hasThumb)
{
    return instance()->createProgressItemImpl(0, id, label, status, canBeCanceled, hasThumb);
}

} // namespace Digikam
