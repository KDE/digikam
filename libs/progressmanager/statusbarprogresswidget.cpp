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

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes

#include "progressview.h"
#include "progressmanager.h"

namespace Digikam
{

StatusbarProgressWidget::StatusbarProgressWidget( ProgressView* progressView, QWidget* parent, bool button )
    : QFrame( parent ), m_currentItem( 0 ), m_progressView( progressView ),
      m_delayTimer( 0 ), m_busyTimer( 0 ), m_cleanTimer( 0 )
{
    m_bShowButton = button;
    int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
    m_box = new QHBoxLayout( this );
    m_box->setMargin(0);
    m_box->setSpacing(0);

    m_pButton = new QPushButton( this );
    m_pButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                            QSizePolicy::Minimum ) );
    QPixmap smallIcon = SmallIcon( "go-up" );
    m_pButton->setIcon( smallIcon );
    m_box->addWidget( m_pButton  );
    m_stack = new QStackedWidget( this );
    int maximumHeight = qMax( smallIcon.height(), fontMetrics().height() );
    m_stack->setMaximumHeight( maximumHeight );
    m_box->addWidget( m_stack );

    m_pButton->setToolTip( i18n("Open detailed progress dialog") );

    m_pProgressBar = new QProgressBar( this );
    m_pProgressBar->installEventFilter( this );
    m_pProgressBar->setMinimumWidth( w );
    m_stack->insertWidget( 1,m_pProgressBar );

    m_pLabel = new QLabel( QString(), this );
    m_pLabel->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    m_pLabel->installEventFilter( this );
    m_pLabel->setMinimumWidth( w );
    m_stack->insertWidget( 2, m_pLabel );
    m_pButton->setMaximumHeight( maximumHeight );
    setMinimumWidth( minimumSizeHint().width() );

    m_mode = None;
    setMode();

    connect(m_pButton, SIGNAL(clicked()),
            progressView, SLOT(slotToggleVisibility()));

    connect(ProgressManager::instance(), SIGNAL(progressItemAdded(ProgressItem*)),
            this, SLOT( slotProgressItemAdded(ProgressItem*)));

    connect(ProgressManager::instance(), SIGNAL(progressItemCompleted(ProgressItem*)),
            this, SLOT(slotProgressItemCompleted(ProgressItem*)));

    connect(ProgressManager::instance(), SIGNAL(progressItemUsesBusyIndicator(ProgressItem*,bool)),
            this, SLOT(updateBusyMode()));

    connect(progressView, SIGNAL(visibilityChanged(bool)),
            this, SLOT(slotProgressViewVisible(bool)));

    m_delayTimer = new QTimer( this );
    m_delayTimer->setSingleShot( true );
    connect(m_delayTimer, SIGNAL( timeout() ),
            this, SLOT( slotShowItemDelayed() ) );

    m_cleanTimer = new QTimer( this );
    m_cleanTimer->setSingleShot( true );
    connect(m_cleanTimer, SIGNAL(timeout()),
            this, SLOT(slotClean()) );
}

// There are three cases: no progressitem, one progressitem (connect to it directly),
// or many progressitems (display busy indicator). Let's call them 0,1,N.
// In slot..Added we can only end up in 1 or N.
// In slot..Removed we can end up in 0, 1, or we can stay in N if we were already.

void StatusbarProgressWidget::updateBusyMode()
{
    connectSingleItem(); // if going to 1 item
    if ( m_currentItem )
    {
        // Exactly one item
        delete m_busyTimer;
        m_busyTimer = 0;
        m_delayTimer->start( 1000 );
    }
    else
    {
        // N items
        if ( !m_busyTimer )
        {
            m_busyTimer = new QTimer( this );
            connect( m_busyTimer, SIGNAL( timeout() ),
                    this, SLOT( slotBusyIndicator() ) );
            m_delayTimer->start( 1000 );
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
    if ( item && item->parent() ) return; // we are only interested in top level items

    connectSingleItem(); // if going back to 1 item

    if ( ProgressManager::instance()->isEmpty() )
    {
        // No item
        // Done. In 5s the progress-widget will close, then we can clean up the statusbar
        m_cleanTimer->start(5000);
    }
    else if ( m_currentItem )
    {
        // Exactly one item
        delete m_busyTimer;
        m_busyTimer = 0;
        activateSingleItemMode();
    }
}

void StatusbarProgressWidget::connectSingleItem()
{
    if ( m_currentItem )
    {
        disconnect(m_currentItem, SIGNAL( progressItemProgress( ProgressItem *, unsigned int ) ),
                   this, SLOT( slotProgressItemProgress( ProgressItem *, unsigned int ) ) );
        m_currentItem = 0;
    }
    m_currentItem = ProgressManager::instance()->singleItem();
    if ( m_currentItem )
    {
        connect(m_currentItem, SIGNAL( progressItemProgress( ProgressItem *, unsigned int ) ),
                this, SLOT( slotProgressItemProgress( ProgressItem *, unsigned int ) ) );
    }
}

void StatusbarProgressWidget::activateSingleItemMode()
{
    m_pProgressBar->setMaximum( 100 );
    m_pProgressBar->setValue( m_currentItem->progress() );
    m_pProgressBar->setTextVisible( true );
}

void StatusbarProgressWidget::slotShowItemDelayed()
{
    bool noItems = ProgressManager::instance()->isEmpty();
    if ( m_currentItem )
    {
        activateSingleItemMode();
    }
    else if ( !noItems )
    {
        // N items
        m_pProgressBar->setMaximum( 0 );
        m_pProgressBar->setTextVisible( false );
        Q_ASSERT( m_busyTimer );
        if ( m_busyTimer )
        m_busyTimer->start( 100 );
    }

    if ( !noItems && m_mode == None )
    {
        m_mode = Progress;
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
    Q_ASSERT( item == m_currentItem); // the only one we should be connected to
    m_pProgressBar->setValue( value );
}

void StatusbarProgressWidget::setMode()
{
    switch ( m_mode )
    {
        case None:
            if ( m_bShowButton )
            {
                m_pButton->hide();
            }
            // show the empty label in order to make the status bar look better
            m_stack->show();
            m_stack->setCurrentWidget( m_pLabel );
            break;

        case Progress:
            m_stack->show();
            m_stack->setCurrentWidget( m_pProgressBar );
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
        m_mode = None;
        setMode();
    }
}

bool StatusbarProgressWidget::eventFilter(QObject*, QEvent* ev)
{
    if ( ev->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent* e = (QMouseEvent*)ev;

        if ( e->button() == Qt::LeftButton && m_mode != None )
        {
            // toggle view on left mouse button
            // Consensus seems to be that we should show/hide the fancy dialog when the user
            // clicks anywhere in the small one.
            m_progressView->slotToggleVisibility();
            return true;
        }
    }
    return false;
}

void StatusbarProgressWidget::slotProgressViewVisible(bool b)
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
