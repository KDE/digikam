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

#ifndef PROGRESS_VIEW_H
#define PROGRESS_VIEW_H

// Qt includes

#include <QPixmap>
#include <QScrollArea>
#include <QMap>

// KDE includes

#include <kvbox.h>

// Local includes

#include "overlaywidget.h"
#include "digikam_export.h"

class QProgressBar;
class QFrame;
class QLabel;
class QPushButton;

namespace Digikam
{
class ProgressItem;
class TransactionItem;

class TransactionItemView : public QScrollArea
{
    Q_OBJECT

public:

    explicit TransactionItemView(QWidget* parent=0, const char* name=0);
    virtual ~TransactionItemView() {}

    TransactionItem* addTransactionItem(ProgressItem* item, bool first);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public Q_SLOTS:

    void slotLayoutFirstItem();

protected:

    virtual void resizeEvent(QResizeEvent* event);

private:

    KVBox* mBigBox;
};

// --------------------------------------------------------------------------------

class TransactionItem : public KVBox
{
    Q_OBJECT

public:

    TransactionItem(QWidget* parent, ProgressItem* item, bool first);
    ~TransactionItem();

    void hideHLine();

    void setProgress(int progress);
    void setLabel(const QString&);
    void setThumbnail(const QPixmap&);

    // the given text is interpreted as RichText, so you might need to
    // Qt::escape() it before passing
    void setStatus(const QString&);

    void setTotalSteps( int totalSteps );

    ProgressItem* item() const { return mItem; }

    void addSubTransaction(ProgressItem* item);

    // The progressitem is deleted immediately, we take 5s to go out,
    // so better not use mItem during this time.
    void setItemComplete() { mItem = 0; }

public Q_SLOTS:

    void slotItemCanceled();

private:

    QProgressBar* mProgress;
    QPushButton*  mCancelButton;
    QLabel*       mItemLabel;
    QLabel*       mItemStatus;
    QLabel*       mItemThumb;
    QFrame*       mFrame;
    ProgressItem* mItem;
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT ProgressView : public OverlayWidget
{
    Q_OBJECT

public:

    ProgressView(QWidget* alignWidget, QWidget* parent, const char* name = 0);
    ~ProgressView();

    void setVisible(bool b);

public Q_SLOTS:

    void slotToggleVisibility();

Q_SIGNALS:

    void visibilityChanged(bool);

protected Q_SLOTS:

    void slotTransactionAdded(ProgressItem*);
    void slotTransactionCompleted(ProgressItem*);
    void slotTransactionCanceled(ProgressItem*);
    void slotTransactionProgress(ProgressItem*, unsigned int progress);
    void slotTransactionStatus(ProgressItem*, const QString&);
    void slotTransactionLabel(ProgressItem*, const QString&);
    void slotTransactionUsesBusyIndicator(ProgressItem*, bool);
    void slotTransactionThumbnail(ProgressItem*, const QPixmap&);
    void slotClose();
    void slotShow();
    void slotHide();

protected:

    virtual void closeEvent(QCloseEvent*);

private:

    bool                                        mWasLastShown;
    TransactionItemView*                        mScrollView;
    TransactionItem*                            mPreviousItem;
    QMap<const ProgressItem*, TransactionItem*> mTransactionsToListviewItems;
};

} // namespace Digikam

#endif // PROGRESS_VIEW_H
