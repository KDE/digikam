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

#ifndef PROGRESS_MANAGER_H
#define PROGRESS_MANAGER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QMap>
#include <QHash>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ProgressItem : public QObject
{
    Q_OBJECT

public:

    ProgressItem(ProgressItem* const parent, const QString& id, const QString& label,
                 const QString& status, bool canBeCanceled, bool hasThumb);
    virtual ~ProgressItem();

    /**
     * Set the property to pop-up item when it's added in progress manager.
     * Use this method if you consider that item is important to be notified to end-user.
     */
    void setShowAtStart(bool showAtStart);

    /**
     * @return true if item must be pop-up when it's added in progress manager.
     */
    bool showAtStart() const;

    /**
     * @return The id string which uniquely identifies the operation
     *         represented by this item.
     */
    const QString& id() const;

    /**
     * @return The parent item of this one, if there is one.
     */
    ProgressItem* parent() const;

    /**
     * @return The user visible string to be used to represent this item.
     */
    const QString& label() const;

    /**
     * @param v Set the user visible string identifying this item.
     */
    void setLabel(const QString& v);

    /**
     * @return The string to be used for showing this item's current status.
     */
    const QString& status() const;

    /**
     * Set the string to be used for showing this item's current status.
     * @param v The status string.
     */
    void setStatus(const QString& v);

    /**
     * @return Whether this item can be canceled.
     */
    bool canBeCanceled() const;

    /**
     * @return whether this item uses a busy indicator instead of real progress display
     */
    bool usesBusyIndicator() const;

    /**
     * Sets whether this item uses a busy indicator instead of real progress for its progress bar.
     * If it uses a busy indicator, you are still responsible for calling setProgress() from time to
     * time to update the busy indicator.
     */
    void setUsesBusyIndicator(bool useBusyIndicator);

    /**
     * @return whether this item has a thumbnail.
     */
    bool hasThumbnail() const;

    /**
     * Sets whether this item has a thumbnail.
     */
    void setThumbnail(const QIcon &icon);

    /**
     * @return The current progress value of this item in percent.
     */
    unsigned int progress() const;

    /**
     * Set the progress (percentage of completion) value of this item.
     * @param v The percentage value.
     */
    void setProgress(unsigned int v);

    /**
     * Tell the item it has finished. This will emit progressItemCompleted()
     * result in the destruction of the item after all slots connected to this
     * signal have executed. This is the only way to get rid of an item and
     * needs to be called even if the item is canceled. Don't use the item
     * after this has been called on it.
     */
    void setComplete();

    /**
     * Reset the progress value of this item to 0 and the status string to
     * the empty string.
     */
    void reset();

    void cancel();
    bool canceled() const;

    // Often needed values for calculating progress.
    void         setTotalItems(unsigned int v);
    void         incTotalItems(unsigned int v = 1);
    unsigned int totalItems() const;
    bool         setCompletedItems(unsigned int v);
    bool         incCompletedItems(unsigned int v = 1);
    unsigned int completedItems() const;
    bool         totalCompleted() const;

    /**
     * Recalculate progress according to total/completed items and update.
     */
    void updateProgress();

    /**
     * Advance total items processed by n values and update percentage in progressbar.
     * @param v The value to advance.
     * @return true if totalCompleted()
     */
    bool advance(unsigned int v);

    void addChild(ProgressItem* const kiddo);
    void removeChild(ProgressItem* const kiddo);

Q_SIGNALS:

    /**
     * Emitted when a new ProgressItem is added.
     * @param The ProgressItem that was added.
     */
    void progressItemAdded(ProgressItem*);

    /**
     * Emitted when the progress value of an item changes.
     * @param  The item which got a new value.
     * @param  The value, for convenience.
     */
    void progressItemProgress(ProgressItem*, unsigned int);

    /**
     * Emitted when a progress item was completed. The item will be
     * deleted afterwards, so slots connected to this are the last
     * chance to work with this item.
     * @param The completed item.
     */
    void progressItemCompleted(ProgressItem*);

    /**
     * Emitted when an item was canceled. It will _not_ go away immediately,
     * only when the owner sets it complete, which will usually happen. Can be
     * used to visually indicate the canceled status of an item. Should be used
     * by the owner of the item to make sure it is set completed even if it is
     * canceled. There is a ProgressManager::slotStandardCancelHandler which
     * simply sets the item completed and can be used if no other work needs to
     * be done on cancel.
     * @param The canceled item;
     */
    void progressItemCanceled(ProgressItem*);
    void progressItemCanceled(const QString& id);

    /**
     * Emitted when the status message of an item changed. Should be used by
     * progress dialogs to update the status message for an item.
     * @param  The updated item.
     * @param  The new message.
     */
    void progressItemStatus(ProgressItem*, const QString&);

    /**
     * Emitted when the label of an item changed. Should be used by
     * progress dialogs to update the label of an item.
     * @param  The updated item.
     * @param  The new label.
     */
    void progressItemLabel(ProgressItem*, const QString&);

    /**
     * Emitted when the busy indicator state of an item changes. Should be used
     * by progress dialogs so that they can adjust the display of the progress bar
     * to the new mode.
     * @param item The updated item
     * @param value True if the item uses a busy indicator now, false otherwise
     */
    void progressItemUsesBusyIndicator(ProgressItem* item, bool value);

    /**
     * Emitted when the thumbnail data must be set in item.
     * @param item The updated item
     * @param thumb thumbnail data
     */
    void progressItemThumbnail(ProgressItem* item, const QPixmap& thumb);

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------------------------------------------

/**
 * The ProgressManager singleton keeps track of all ongoing transactions
 * and notifies observers (progress dialogs) when their progress percent value
 * changes, when they are completed (by their owner), and when they are canceled.
 * Each ProgressItem emits those signals individually and the singleton
 * broadcasts them. Use the ::createProgressItem() statics to acquire an item
 * and then call ->setProgress( int percent ) on it every time you want to
 * update the item and ->setComplete() when the operation is done. This will
 * delete the item. Connect to the item's progressItemCanceled() signal to be
 * notified when the user cancels the transaction using one of the observing
 * progress dialogs or by calling item->cancel() in some other way. The owner
 * is responsible for calling setComplete() on the item, even if it is canceled.
 * Use the standardCancelHandler() slot if that is all you want to do on cancel.
 *
 * Note that if you request an item with a certain id and there is already
 * one with that id, there will not be a new one created but the existing
 * one will be returned. This is convenient for accessing items that are
 * needed regularly without the to store a pointer to them or to add child
 * items to parents by id.
 */
class DIGIKAM_EXPORT ProgressManager : public QObject
{
    Q_OBJECT

public:

    /**
     * @return true when there are no more progress items.
     */
    bool isEmpty() const;

    /** @return the progressitem for this id if it exist, else null.
     */
    ProgressItem* findItembyId(const QString& id) const;

    /**
     * @return the only top level progressitem when there's only one.
     * Returns 0 if there is no item, or more than one top level item.
     * Since this is used to calculate the overall progress, it will also return
     * 0 if there is an item which uses a busy indicator, since that will invalidate
     * the overall progress.
     */
    ProgressItem* singleItem() const;

    /**
     * @return The singleton instance of this class.
     */
    static ProgressManager* instance();

    /**
     * Use this to acquire a unique id number which can be used to discern
     * an operation from all others going on at the same time. Use that
     * number as the id string for your progressItem to ensure it is unique.
     * @return
     */
    QString getUniqueID();

     /**
      * Creates a ProgressItem with a unique id and the given label.
      * This is the simplest way to acquire a progress item. It will not
      * have a parent.
      * @param label The text to be displayed by progress handlers
      * @param status Additional text to be displayed for the item.
      * @param canBeCanceled can the user cancel this operation?
      * Cancelling the parent will cancel the children as well (if they can be
      * canceled) and ongoing children prevent parents from finishing.
      * @return The ProgressItem representing the operation.
      */
    static ProgressItem* createProgressItem(const QString& label,
                                            const QString& status = QString(),
                                            bool  canBeCanceled = true,
                                            bool  hasThumb = false);

    /**
     * Creates a new progressItem with the given parent, id, label and initial
     * status.
     *
     * @param parent Specify an already existing item as the parent of this one.
     * @param id Used to identify this operation for cancel and progress info.
     * @param label The text to be displayed by progress handlers
     * @param status Additional text to be displayed for the item.
     * @param canBeCanceled can the user cancel this operation?
     * Cancelling the parent will cancel the children as well (if they can be
     * canceled) and ongoing children prevent parents from finishing.
     * @return The ProgressItem representing the operation.
     */
    static ProgressItem* createProgressItem(ProgressItem* const parent,
                                            const QString& id,
                                            const QString& label,
                                            const QString& status = QString(),
                                            bool  canBeCanceled = true,
                                            bool  hasThumb = false);

    /**
     * Use this version if you have the id string of the parent and want to
     * add a subjob to it.
     */
    static ProgressItem* createProgressItem(const QString& parent,
                                            const QString& id,
                                            const QString& label,
                                            const QString& status = QString(),
                                            bool  canBeCanceled = true,
                                            bool  hasThumb = false);

    /**
     * Version without a parent.
     */
    static ProgressItem* createProgressItem(const QString& id,
                                            const QString& label,
                                            const QString& status = QString(),
                                            bool  canBeCanceled = true,
                                            bool  hasThumb = false);

    /**
     * Add a created progressItem outside manager with the given parent.
     *
     * @param t The process to add on manager.
     * @param parent Specify an already existing item as the parent of this one (can be null).
     * @return true if ProgressItem have been added to manager, else false.
     */
    static bool addProgressItem(ProgressItem* const t, ProgressItem* const parent=0);

    /**
     * Ask all listeners to show the progress dialog, because there is
     * something that wants to be shown.
     */
    static void emitShowProgressView();

Q_SIGNALS:

    /** @see ProgressItem::progressItemAdded() */
    void progressItemAdded(ProgressItem*);

    /** @see ProgressItem::progressItemProgress() */
    void progressItemProgress(ProgressItem*, unsigned int);

    /** @see ProgressItem::progressItemCompleted() */
    void progressItemCompleted(ProgressItem*);

    /** @see ProgressItem::progressItemCanceled() */
    void progressItemCanceled(ProgressItem*);

    /** @see ProgressItem::progressItemStatus() */
    void progressItemStatus(ProgressItem*, const QString&);

    /** @see ProgressItem::progressItemLabel() */
    void progressItemLabel(ProgressItem*, const QString&);

    /** @see ProgressItem::progressItemUsesBusyIndicator */
    void progressItemUsesBusyIndicator(ProgressItem*, bool);

    /** @see ProgressItem::progressItemThumbnail */
    void progressItemThumbnail(ProgressItem*, const QPixmap&);

    /**
     * Emitted when an operation requests the listeners to be shown.
     * Use emitShowProgressView() to trigger it.
     */
    void showProgressView();

    void completeTransactionDeferred(ProgressItem* item);

public Q_SLOTS:

    /**
     * Calls setCompleted() on the item, to make sure it goes away.
     * Provided for convenience.
     * @param item the canceled item.
     */
    void slotStandardCancelHandler(ProgressItem* item);

    /**
     * Aborts all running jobs. Bound to "Esc"
     */
    void slotAbortAll();

private Q_SLOTS:

    void slotTransactionCompleted(ProgressItem* item);
    void slotTransactionCompletedDeferred(ProgressItem* item);
    void slotTransactionViewIsEmpty();

private:

    ProgressManager();
     // prevent unsolicited copies
    ProgressManager(const ProgressManager&);
    ~ProgressManager();

    void emitShowProgressViewImpl();

    virtual ProgressItem* createProgressItemImpl(ProgressItem* const parent,
                                                 const QString& id,
                                                 const QString& label,
                                                 const QString& status,
                                                 bool  cancellable,
                                                 bool  hasThumb);

    virtual ProgressItem* createProgressItemImpl(const QString& parent,
                                                 const QString& id,
                                                 const QString& label,
                                                 const QString& status,
                                                 bool  cancellable,
                                                 bool  hasThumb);

    virtual void addProgressItemImpl(ProgressItem* const t, ProgressItem* const parent);

private:

    class Private;
    Private* const d;

    friend class ProgressManagerCreator;
};

} // namespace Digikam

#endif // PROGRESS_MANAGER_H
