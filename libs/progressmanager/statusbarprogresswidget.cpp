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
 * Copyright (C) 2004 David Faure <faure at kde dot org>
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

#include "statusbarprogresswidget.moc"

// Qt includes

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>

// KDE includes

#include <KLocale>
#include <KIconLoader>
#include <KDebug>

// Local includes

#include "progressdialog.h"
#include "progressmanager.h"

namespace Digikam
{
    
StatusbarProgressWidget::StatusbarProgressWidget( ProgressDialog* progressDialog, QWidget* parent, bool button )
    : QFrame( parent ), mCurrentItem( 0 ), mProgressDialog( progressDialog ),
      mDelayTimer( 0 ), mBusyTimer( 0 ), mCleanTimer( 0 )
{
    m_bShowButton = button;
    int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
    box = new QHBoxLayout( this );
    box->setMargin(0);
    box->setSpacing(0);

    m_pButton = new QPushButton( this );
    m_pButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                            QSizePolicy::Minimum ) );
    QPixmap smallIcon = SmallIcon( "go-up" );
    m_pButton->setIcon( smallIcon );
    box->addWidget( m_pButton  );
    stack = new QStackedWidget( this );
    int maximumHeight = qMax( smallIcon.height(), fontMetrics().height() );
    stack->setMaximumHeight( maximumHeight );
    box->addWidget( stack );

    m_pButton->setToolTip( i18n("Open detailed progress dialog") );

    m_pProgressBar = new QProgressBar( this );
    m_pProgressBar->installEventFilter( this );
    m_pProgressBar->setMinimumWidth( w );
    stack->insertWidget( 1,m_pProgressBar );

    m_pLabel = new QLabel( QString(), this );
    m_pLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    m_pLabel->installEventFilter( this );
    m_pLabel->setMinimumWidth( w );
    stack->insertWidget( 2, m_pLabel );
    m_pButton->setMaximumHeight( maximumHeight );
    setMinimumWidth( minimumSizeHint().width() );

    mode = None;
    setMode();

    connect( m_pButton, SIGNAL( clicked() ),
            progressDialog, SLOT( slotToggleVisibility() ) );

    connect ( ProgressManager::instance(), SIGNAL( progressItemAdded( Digikam::ProgressItem * ) ),
                this, SLOT( slotProgressItemAdded( Digikam::ProgressItem * ) ) );
    
    connect ( ProgressManager::instance(), SIGNAL( progressItemCompleted( Digikam::ProgressItem * ) ),
                this, SLOT( slotProgressItemCompleted( Digikam::ProgressItem * ) ) );
    
    connect ( ProgressManager::instance(), SIGNAL(progressItemUsesBusyIndicator(Digikam::ProgressItem*,bool)),
                this, SLOT( updateBusyMode() ) );

    connect ( progressDialog, SIGNAL( visibilityChanged( bool )),
                this, SLOT( slotProgressDialogVisible( bool ) ) );

    mDelayTimer = new QTimer( this );
    mDelayTimer->setSingleShot( true );
    connect ( mDelayTimer, SIGNAL( timeout() ),
              this, SLOT( slotShowItemDelayed() ) );

    mCleanTimer = new QTimer( this );
    mCleanTimer->setSingleShot( true );
    connect ( mCleanTimer, SIGNAL(timeout()),
              this, SLOT(slotClean()) );
}

// There are three cases: no progressitem, one progressitem (connect to it directly),
// or many progressitems (display busy indicator). Let's call them 0,1,N.
// In slot..Added we can only end up in 1 or N.
// In slot..Removed we can end up in 0, 1, or we can stay in N if we were already.

void StatusbarProgressWidget::updateBusyMode()
{
    connectSingleItem(); // if going to 1 item
    if ( mCurrentItem )
    {
        // Exactly one item
        delete mBusyTimer;
        mBusyTimer = 0;
        mDelayTimer->start( 1000 );
    }
    else
    {
        // N items
        if ( !mBusyTimer )
        {
            mBusyTimer = new QTimer( this );
            connect( mBusyTimer, SIGNAL( timeout() ),
                    this, SLOT( slotBusyIndicator() ) );
            mDelayTimer->start( 1000 );
        }
    }
}

void StatusbarProgressWidget::slotProgressItemAdded( ProgressItem *item )
{
    if ( item->parent() )
        return; // we are only interested in top level items

    updateBusyMode();
}

void StatusbarProgressWidget::slotProgressItemCompleted( ProgressItem *item )
{
    if ( item->parent() ) return; // we are only interested in top level items

    connectSingleItem(); // if going back to 1 item
    
    if ( ProgressManager::instance()->isEmpty() )
    {
        // No item
        // Done. In 5s the progress-widget will close, then we can clean up the statusbar
        mCleanTimer->start( 5000 );
    }
    else if ( mCurrentItem )
    { // Exactly one item
        delete mBusyTimer;
        mBusyTimer = 0;
        activateSingleItemMode();
    }
}

void StatusbarProgressWidget::connectSingleItem()
{
    if ( mCurrentItem )
    {
        disconnect ( mCurrentItem, SIGNAL( progressItemProgress( Digikam::ProgressItem *, unsigned int ) ),
                    this, SLOT( slotProgressItemProgress( Digikam::ProgressItem *, unsigned int ) ) );
        mCurrentItem = 0;
    }
    mCurrentItem = ProgressManager::instance()->singleItem();
    if ( mCurrentItem )
    {
        connect ( mCurrentItem, SIGNAL( progressItemProgress( Digikam::ProgressItem *, unsigned int ) ),
                this, SLOT( slotProgressItemProgress( Digikam::ProgressItem *, unsigned int ) ) );
    }
}

void StatusbarProgressWidget::activateSingleItemMode()
{
    m_pProgressBar->setMaximum( 100 );
    m_pProgressBar->setValue( mCurrentItem->progress() );
    m_pProgressBar->setTextVisible( true );
}

void StatusbarProgressWidget::slotShowItemDelayed()
{
    bool noItems = ProgressManager::instance()->isEmpty();
    if ( mCurrentItem )
    {
        activateSingleItemMode();
    }
    else if ( !noItems )
    {
        // N items
        m_pProgressBar->setMaximum( 0 );
        m_pProgressBar->setTextVisible( false );
        Q_ASSERT( mBusyTimer );
        if ( mBusyTimer )
        mBusyTimer->start( 100 );
    }

    if ( !noItems && mode == None )
    {
        mode = Progress;
        setMode();
    }
}

void StatusbarProgressWidget::slotBusyIndicator()
{
    int p = m_pProgressBar->value();
    m_pProgressBar->setValue( p + 10 );
}

void StatusbarProgressWidget::slotProgressItemProgress( ProgressItem *item, unsigned int value )
{
    Q_ASSERT( item == mCurrentItem); // the only one we should be connected to
    m_pProgressBar->setValue( value );
}

void StatusbarProgressWidget::setMode()
{
    switch ( mode )
    {
        case None:
            if ( m_bShowButton )
            {
                m_pButton->hide();
            }
            // show the empty label in order to make the status bar look better
            stack->show();
            stack->setCurrentWidget( m_pLabel );
            break;

        case Progress:
            stack->show();
            stack->setCurrentWidget( m_pProgressBar );
            if ( m_bShowButton )
            {
                m_pButton->show();
            }
            break;
    }
}

void StatusbarProgressWidget::slotClean()
{
    // check if a new item showed up since we started the timer. If not, clear
    if ( ProgressManager::instance()->isEmpty() )
    {
        m_pProgressBar->setValue( 0 );
        //m_pLabel->clear();
        mode = None;
        setMode();
    }
}

bool StatusbarProgressWidget::eventFilter( QObject *, QEvent *ev )
{
    if ( ev->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent *e = (QMouseEvent*)ev;

        if ( e->button() == Qt::LeftButton && mode != None )
        {
            // toggle view on left mouse button
            // Consensus seems to be that we should show/hide the fancy dialog when the user
            // clicks anywhere in the small one.
            mProgressDialog->slotToggleVisibility();
            return true;
        }
    }
    return false;
}

void StatusbarProgressWidget::slotProgressDialogVisible( bool b )
{
    // Update the hide/show button when the detailed one is shown/hidden
    if ( b )
    {
        m_pButton->setIcon( SmallIcon( "go-down" ) );
        m_pButton->setToolTip( i18n("Hide detailed progress window") );
        setMode();
    }
    else
    {
        m_pButton->setIcon( SmallIcon( "go-up" ) );
        m_pButton->setToolTip( i18n("Show detailed progress window") );
    }
}

} // namespace Digikam
