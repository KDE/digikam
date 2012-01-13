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

#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

// Qt includes

#include <QScrollArea>
#include <QMap>

// KDE includes

#include <KVBox>

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

    explicit TransactionItemView( QWidget * parent = 0, const char * name = 0 );
    virtual ~TransactionItemView() {}

    TransactionItem *addTransactionItem( ProgressItem *item, bool first );

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public Q_SLOTS:

    void slotLayoutFirstItem();

protected:

    virtual void resizeEvent ( QResizeEvent *event );

private:

    KVBox* mBigBox;
};

// --------------------------------------------------------------------------------

class TransactionItem : public KVBox
{
    Q_OBJECT

public:

    TransactionItem( QWidget *parent, ProgressItem *item, bool first );
    ~TransactionItem();

    void hideHLine();

    void setProgress( int progress );
    void setLabel( const QString & );

    // the given text is interpreted as RichText, so you might need to
    // Qt::escape() it before passing
    void setStatus( const QString & );

    void setTotalSteps( int totalSteps );

    ProgressItem *item() const { return mItem; }

    void addSubTransaction( ProgressItem *item );

    // The progressitem is deleted immediately, we take 5s to go out,
    // so better not use mItem during this time.
    void setItemComplete() { mItem = 0; }

public Q_SLOTS:

    void slotItemCanceled();

protected:

    QProgressBar* mProgress;
    QPushButton*  mCancelButton;
    QLabel*       mItemLabel;
    QLabel*       mItemStatus;
    QFrame*       mFrame;
    ProgressItem* mItem;
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT ProgressDialog : public OverlayWidget
{
    Q_OBJECT

public:

    ProgressDialog( QWidget *alignWidget, QWidget *parent, const char *name = 0 );
    ~ProgressDialog();
    void setVisible( bool b );

public Q_SLOTS:

    void slotToggleVisibility();

Q_SIGNALS:

    void visibilityChanged( bool );

protected Q_SLOTS:

    void slotTransactionAdded( Digikam::ProgressItem *item );
    void slotTransactionCompleted( Digikam::ProgressItem *item );
    void slotTransactionCanceled( Digikam::ProgressItem *item );
    void slotTransactionProgress( Digikam::ProgressItem *item, unsigned int progress );
    void slotTransactionStatus( Digikam::ProgressItem *item, const QString & );
    void slotTransactionLabel( Digikam::ProgressItem *item, const QString & );
    void slotTransactionUsesBusyIndicator( Digikam::ProgressItem *, bool );

    void slotClose();
    void slotShow();
    void slotHide();

protected:

    virtual void closeEvent( QCloseEvent * );

protected:

    bool                                        mWasLastShown;
    TransactionItemView*                        mScrollView;
    TransactionItem*                            mPreviousItem;
    QMap<const ProgressItem*, TransactionItem*> mTransactionsToListviewItems;
};

} // namespace Digikam

#endif // PROGRESS_DIALOG_H
