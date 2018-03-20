/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2001-2003 by Joseph Wenninger <jowenn at kde dot org>
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

#include "sidebar.h"

#include <cmath>

// Qt includes

#include <QDataStream>
#include <QDragEnterEvent>
#include <QEvent>
#include <QPixmap>
#include <QSplitter>
#include <QStackedWidget>
#include <QTimer>
#include <QHash>
#include <QScrollArea>
#include <QFrame>
#include <QActionEvent>
#include <QLayout>
#include <QPainter>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleOptionButton>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class DMultiTabBarFrame::Private
{
public:

    QBoxLayout*             mainLayout;
    QList<DMultiTabBarTab*> tabs;
    Qt::Edge                position;
    DMultiTabBar::TextStyle style;
};

DMultiTabBarFrame::DMultiTabBarFrame(QWidget* const parent, Qt::Edge pos)
    : QFrame(parent),
      d(new Private)
{
    d->position = pos;

    if (pos == Qt::LeftEdge || pos == Qt::RightEdge)
        d->mainLayout = new QVBoxLayout(this);
    else
        d->mainLayout = new QHBoxLayout(this);

    d->mainLayout->setContentsMargins(QMargins());
    d->mainLayout->setSpacing(0);
    d->mainLayout->addStretch();
    setFrameStyle(NoFrame);
    setBackgroundRole(QPalette::Background);
}

DMultiTabBarFrame::~DMultiTabBarFrame()
{
    qDeleteAll(d->tabs);
    d->tabs.clear();
    delete d;
}

void DMultiTabBarFrame::setStyle(DMultiTabBar::TextStyle style)
{
    d->style = style;

    for (int i = 0 ; i < d->tabs.count() ; i++)
        d->tabs.at(i)->setStyle(d->style);

    updateGeometry();
}

void DMultiTabBarFrame::contentsMousePressEvent(QMouseEvent* e)
{
    e->ignore();
}

void DMultiTabBarFrame::mousePressEvent(QMouseEvent* e)
{
    e->ignore();
}

DMultiTabBarTab* DMultiTabBarFrame::tab(int id) const
{
    QListIterator<DMultiTabBarTab*> it(d->tabs);

    while (it.hasNext())
    {
        DMultiTabBarTab* const tab = it.next();

        if (tab->id() == id)
            return tab;
    }

    return 0;
}

int DMultiTabBarFrame::appendTab(const QPixmap& pic, int id, const QString& text)
{
    DMultiTabBarTab* const tab = new DMultiTabBarTab(pic, text, id, this, d->position, d->style);
    d->tabs.append(tab);

    // Insert before the stretch.
    d->mainLayout->insertWidget(d->tabs.size()-1, tab);
    tab->show();
    return 0;
}

void DMultiTabBarFrame::removeTab(int id)
{
    for (int pos = 0 ; pos < d->tabs.count() ; pos++)
    {
        if (d->tabs.at(pos)->id() == id)
        {
            // remove & delete the tab
            delete d->tabs.takeAt(pos);
            break;
        }
    }
}

void DMultiTabBarFrame::setPosition(Qt::Edge pos)
{
    d->position = pos;

    for (int i = 0 ; i < d->tabs.count() ; i++)
        d->tabs.at(i)->setPosition(d->position);

    updateGeometry();
}

QList<DMultiTabBarTab*>* DMultiTabBarFrame::tabs()
{
    return &d->tabs;
}

// -------------------------------------------------------------------------------------

DMultiTabBarButton::DMultiTabBarButton(const QPixmap& pic, const QString& text,
                                       int id, QWidget* const parent)
    : QPushButton(QIcon(pic), text, parent),
      m_id(id)
{
    connect(this, SIGNAL(clicked()),
            this, SLOT(slotClicked()));

    // we can't see the focus, so don't take focus. #45557
    // If keyboard navigation is wanted, then only the bar should take focus,
    // and arrows could change the focused button; but generally, tabbars don't take focus anyway.
    setFocusPolicy(Qt::NoFocus);
    // See RB #128005
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

DMultiTabBarButton::~DMultiTabBarButton()
{
}

void DMultiTabBarButton::setText(const QString& text)
{
    QPushButton::setText(text);
}

void DMultiTabBarButton::slotClicked()
{
    updateGeometry();
    emit clicked(m_id);
}

int DMultiTabBarButton::id() const
{
    return m_id;
}

void DMultiTabBarButton::hideEvent(QHideEvent* e)
{
    QPushButton::hideEvent(e);
    DMultiTabBar* const tb = dynamic_cast<DMultiTabBar*>(parentWidget());

    if (tb)
        tb->updateSeparator();
}

void DMultiTabBarButton::showEvent(QShowEvent* e)
{
    QPushButton::showEvent(e);
    DMultiTabBar* const tb = dynamic_cast<DMultiTabBar*>(parentWidget());

    if (tb)
        tb->updateSeparator();
}

void DMultiTabBarButton::paintEvent(QPaintEvent*)
{
    QStyleOptionButton opt;
    opt.initFrom(this);
    opt.icon = icon();
    opt.iconSize = iconSize();
    // removes the QStyleOptionButton::HasMenu ButtonFeature
    opt.features = QStyleOptionButton::Flat;
    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);
}

// -------------------------------------------------------------------------------------

class DMultiTabBarTab::Private
{
public:

    Qt::Edge                position;
    DMultiTabBar::TextStyle style;
};

DMultiTabBarTab::DMultiTabBarTab(const QPixmap& pic, const QString& text,
                                       int id, QWidget* const parent,
                                       Qt::Edge pos,
                                       DMultiTabBar::TextStyle style)
    : DMultiTabBarButton(pic, text, id, parent),
      d(new Private)
{
    d->style    = style;
    d->position = pos;
    setToolTip(text);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
     // shrink down to icon only, but prefer to show text if it's there
}

DMultiTabBarTab::~DMultiTabBarTab()
{
    delete d;
}

void DMultiTabBarTab::setPosition(Qt::Edge pos)
{
    d->position = pos;
    updateGeometry();
}

void DMultiTabBarTab::setStyle(DMultiTabBar::TextStyle style)
{
    d->style = style;
    updateGeometry();
}

QPixmap DMultiTabBarTab::iconPixmap() const
{
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    return icon().pixmap(iconSize);
}

void DMultiTabBarTab::initStyleOption(QStyleOptionToolButton* opt) const
{
    opt->initFrom(this);

    // Setup icon..
    if (!icon().isNull())
    {
        opt->iconSize = iconPixmap().size();
        opt->icon     = icon();
    }

    // Should we draw text?
    if (shouldDrawText())
        opt->text = text();

    if (underMouse())
        opt->state |= QStyle::State_AutoRaise | QStyle::State_MouseOver | QStyle::State_Raised;

    if (isChecked())
        opt->state |= QStyle::State_Sunken | QStyle::State_On;

    opt->font            = font();
    opt->toolButtonStyle = shouldDrawText() ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    opt->subControls     = QStyle::SC_ToolButton;
}

QSize DMultiTabBarTab::sizeHint() const
{
    return computeSizeHint(shouldDrawText());
}

QSize DMultiTabBarTab::minimumSizeHint() const
{
    return computeSizeHint(false);
}

void DMultiTabBarTab::computeMargins(int* hMargin, int* vMargin) const
{
    // Unfortunately, QStyle does not give us enough information to figure out
    // where to place things, so we try to reverse-engineer it
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPixmap iconPix  = iconPixmap();
    QSize trialSize  = iconPix.size();
    QSize expandSize = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, trialSize, this);

    *hMargin = (expandSize.width()  - trialSize.width())/2;
    *vMargin = (expandSize.height() - trialSize.height())/2;
}

QSize DMultiTabBarTab::computeSizeHint(bool withText) const
{
    // Compute as horizontal first, then flip around if need be.
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    int hMargin, vMargin;
    computeMargins(&hMargin, &vMargin);

    // Compute interior size, starting from pixmap..
    QPixmap iconPix = iconPixmap();
    QSize size = iconPix.size();

    // Always include text height in computation, to avoid resizing the minor direction
    // when expanding text..
    QSize textSize = fontMetrics().size(0, text());
    size.setHeight(qMax(size.height(), textSize.height()));

    // Pick margins for major/minor direction, depending on orientation
    int majorMargin = isVertical() ? vMargin : hMargin;
    int minorMargin = isVertical() ? hMargin : vMargin;

    size.setWidth (size.width()  + 2*majorMargin);
    size.setHeight(size.height() + 2*minorMargin);

    if (withText)
    {
        // Add enough room for the text, and an extra major margin.
        size.setWidth(size.width() + textSize.width() + majorMargin);
    }

    if (isVertical())
    {
        return QSize(size.height(), size.width());
    }

    return size;
}

void DMultiTabBarTab::setState(bool newState)
{
    setChecked(newState);
    updateGeometry();
}

void DMultiTabBarTab::setIcon(const QString& icon)
{
    const QIcon i      = QIcon::fromTheme(icon);
    const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, 0, this);
    setIcon(i.pixmap(iconSize));
}

void DMultiTabBarTab::setIcon(const QPixmap& icon)
{
    QPushButton::setIcon(icon);
}

bool DMultiTabBarTab::shouldDrawText() const
{
    return (d->style == DMultiTabBar::AllIconsText) || isChecked();
}

bool DMultiTabBarTab::isVertical() const
{
    return (d->position == Qt::RightEdge || d->position == Qt::LeftEdge);
}

void DMultiTabBarTab::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    // Paint bevel..
    if (underMouse() || isChecked())
    {
        opt.text.clear();
        opt.icon = QIcon();
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);
    }

    int hMargin, vMargin;
    computeMargins(&hMargin, &vMargin);

    // We first figure out how much room we have for the text, based on
    // icon size and margin, try to fit in by eliding, and perhaps
    // give up on drawing the text entirely if we're too short on room
    QPixmap icon = iconPixmap();
    int textRoom = 0;
    int iconRoom = 0;

    QString t;

    if (shouldDrawText())
    {
        if (isVertical())
        {
            iconRoom = icon.height() + 2*vMargin;
            textRoom = height() - iconRoom - vMargin;
        }
        else
        {
            iconRoom = icon.width() + 2*hMargin;
            textRoom = width() - iconRoom - hMargin;
        }

        t = painter.fontMetrics().elidedText(text(), Qt::ElideRight, textRoom);

        // See whether anything is left. Qt will return either
        // ... or the ellipsis unicode character, 0x2026
        if (t == QLatin1String("...") || t == QChar(0x2026))
        {
            t.clear();
        }
    }

    // Label time.... Simple case: no text, so just plop down the icon right in the center
    // We only do this when the button never draws the text, to avoid jumps in icon position
    // when resizing
    if (!shouldDrawText())
    {
        style()->drawItemPixmap(&painter, rect(), Qt::AlignCenter | Qt::AlignVCenter, icon);
        return;
    }

    // Now where the icon/text goes depends on text direction and tab position
    QRect iconArea;
    QRect labelArea;

    bool bottomIcon = false;
    bool rtl        = layoutDirection() == Qt::RightToLeft;

    if (isVertical())
    {
        if (d->position == Qt::LeftEdge && !rtl)
            bottomIcon = true;

        if (d->position == Qt::RightEdge && rtl)
            bottomIcon = true;
    }

    if (isVertical())
    {
        if (bottomIcon)
        {
            labelArea = QRect(0, vMargin, width(), textRoom);
            iconArea  = QRect(0, vMargin + textRoom, width(), iconRoom);
        }
        else
        {
            labelArea = QRect(0, iconRoom, width(), textRoom);
            iconArea  = QRect(0, 0, width(), iconRoom);
        }
    }
    else
    {
        // Pretty simple --- depends only on RTL/LTR
        if (rtl)
        {
            labelArea = QRect(hMargin, 0, textRoom, height());
            iconArea  = QRect(hMargin + textRoom, 0, iconRoom, height());
        }
        else
        {
            labelArea = QRect(iconRoom, 0, textRoom, height());
            iconArea  = QRect(0, 0, iconRoom, height());
        }
    }

    style()->drawItemPixmap(&painter, iconArea, Qt::AlignCenter | Qt::AlignVCenter, icon);

    if (t.isEmpty())
    {
        return;
    }

    QRect labelPaintArea = labelArea;

    if (isVertical())
    {
        // If we're vertical, we paint to a simple 0,0 origin rect,
        // and get the transformations to get us in the right place
        labelPaintArea = QRect(0, 0, labelArea.height(), labelArea.width());

        QTransform tr;

        if (bottomIcon)
        {
            tr.translate(labelArea.x(), labelPaintArea.width() + labelArea.y());
            tr.rotate(-90);
        }
        else
        {
            tr.translate(labelPaintArea.height() + labelArea.x(), labelArea.y());
            tr.rotate(90);
        }

        painter.setTransform(tr);
    }

    style()->drawItemText(&painter, labelPaintArea, Qt::AlignLeading | Qt::AlignVCenter,
                          palette(), true, t, QPalette::ButtonText);
}

// -------------------------------------------------------------------------------------

class DMultiTabBar::Private
{
public:

    DMultiTabBarFrame*         internal;
    QBoxLayout*                layout;
    QFrame*                    btnTabSep;
    QList<DMultiTabBarButton*> buttons;
    Qt::Edge                   position;
};

DMultiTabBar::DMultiTabBar(Qt::Edge pos, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    if (pos == Qt::LeftEdge || pos == Qt::RightEdge)
    {
        d->layout = new QVBoxLayout(this);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding/*, true*/);
    }
    else
    {
        d->layout = new QHBoxLayout(this);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed/*, true*/);
    }

    d->layout->setContentsMargins(QMargins());
    d->layout->setSpacing(0);

    d->internal = new DMultiTabBarFrame(this, pos);
    setPosition(pos);
    setStyle(ActiveIconText);
    d->layout->insertWidget(0, d->internal);
    d->layout->insertWidget(0, d->btnTabSep = new QFrame(this));
    d->btnTabSep->setFixedHeight(4);
    d->btnTabSep->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    d->btnTabSep->setLineWidth(2);
    d->btnTabSep->hide();

    updateGeometry();
}

DMultiTabBar::~DMultiTabBar()
{
    qDeleteAll(d->buttons);
    d->buttons.clear();
    delete d;
}

int DMultiTabBar::appendButton(const QPixmap &pic, int id, QMenu *popup, const QString&)
{
    DMultiTabBarButton* const btn = new DMultiTabBarButton(pic, QString(), id, this);
    // a button with a QMenu can have another size. Make sure the button has always the same size.
    btn->setFixedWidth(btn->height());
    btn->setMenu(popup);
    d->buttons.append(btn);
    d->layout->insertWidget(0,btn);
    btn->show();
    d->btnTabSep->show();
    return 0;
}

void DMultiTabBar::updateSeparator()
{
    bool hideSep = true;
    QListIterator<DMultiTabBarButton*> it(d->buttons);

    while (it.hasNext())
    {
        if (it.next()->isVisibleTo(this))
        {
            hideSep = false;
            break;
        }
    }

    if (hideSep)
        d->btnTabSep->hide();
    else
        d->btnTabSep->show();
}

int DMultiTabBar::appendTab(const QPixmap& pic, int id, const QString& text)
{
    d->internal->appendTab(pic,id,text);
    return 0;
}

DMultiTabBarButton* DMultiTabBar::button(int id) const
{
    QListIterator<DMultiTabBarButton*> it(d->buttons);

    while (it.hasNext())
    {
        DMultiTabBarButton* const button = it.next();

        if (button->id() == id)
            return button;
    }

    return 0;
}

DMultiTabBarTab* DMultiTabBar::tab(int id) const
{
    return d->internal->tab(id);
}

void DMultiTabBar::removeButton(int id)
{
    for (int pos = 0 ; pos < d->buttons.count() ; pos++)
    {
        if (d->buttons.at(pos)->id() == id)
        {
            d->buttons.takeAt(pos)->deleteLater();
            break;
        }
    }

    if (d->buttons.count() == 0)
        d->btnTabSep->hide();
}

void DMultiTabBar::removeTab(int id)
{
    d->internal->removeTab(id);
}

void DMultiTabBar::setTab(int id,bool state)
{
    DMultiTabBarTab* const ttab = tab(id);

    if (ttab)
        ttab->setState(state);
}

bool DMultiTabBar::isTabRaised(int id) const
{
    DMultiTabBarTab* const ttab = tab(id);

    if (ttab)
        return ttab->isChecked();

    return false;
}

void DMultiTabBar::setStyle(TextStyle style)
{
    d->internal->setStyle(style);
}

DMultiTabBar::TextStyle DMultiTabBar::tabStyle() const
{
    return d->internal->d->style;
}

void DMultiTabBar::setPosition(Qt::Edge pos)
{
    d->position = pos;
    d->internal->setPosition(pos);
}

Qt::Edge DMultiTabBar::position() const
{
    return d->position;
}

void DMultiTabBar::fontChange(const QFont&)
{
    updateGeometry();
}

// -------------------------------------------------------------------------------------

class SidebarState
{

public:

    SidebarState()
      : activeWidget(0),
        size(0)
    {
    }

    SidebarState(QWidget* const w, int size)
      : activeWidget(w),
        size(size)
    {
    }

    QWidget* activeWidget;
    int      size;
};

// -------------------------------------------------------------------------------------

class Sidebar::Private
{

public:

    Private() :
        minimizedDefault(false),
        minimized(false),
        isMinimized(false),
        tabs(0),
        activeTab(-1),
        dragSwitchId(-1),
        restoreSize(0),
        stack(0),
        splitter(0),
        dragSwitchTimer(0),
        appendedTabsStateCache(),
        optionActiveTabEntry(QLatin1String("ActiveTab")),
        optionMinimizedEntry(QLatin1String("Minimized")),
        optionRestoreSizeEntry(QLatin1String("RestoreSize"))
    {
    }

    bool                          minimizedDefault;
    bool                          minimized;
    bool                          isMinimized;      // Backup of shrinked status before backup(), restored by restore()
                                                    // NOTE: when sidebar is hidden, only icon bar is affected. If sidebar view is
                                                    // visible, this one must be shrink and restored accordingly.

    int                           tabs;
    int                           activeTab;
    int                           dragSwitchId;
    int                           restoreSize;

    QStackedWidget*               stack;
    SidebarSplitter*              splitter;
    QTimer*                       dragSwitchTimer;

    QHash<QWidget*, SidebarState> appendedTabsStateCache;

    const QString                 optionActiveTabEntry;
    const QString                 optionMinimizedEntry;
    const QString                 optionRestoreSizeEntry;
};

class SidebarSplitter::Private
{
public:

    QList<Sidebar*> sidebars;
};

// -------------------------------------------------------------------------------------

Sidebar::Sidebar(QWidget* const parent, SidebarSplitter* const sp, Qt::Edge side, bool minimizedDefault)
    : DMultiTabBar(side, parent),
      StateSavingObject(this),
      d(new Private)
{
    d->splitter         = sp;
    d->minimizedDefault = minimizedDefault;
    d->stack            = new QStackedWidget(d->splitter);
    d->dragSwitchTimer  = new QTimer(this);

    connect(d->dragSwitchTimer, SIGNAL(timeout()),
            this, SLOT(slotDragSwitchTimer()));

    d->splitter->d->sidebars << this;

    setStyle(DMultiTabBar::ActiveIconText);
}

Sidebar::~Sidebar()
{
    saveState();

    if (d->splitter)
    {
        d->splitter->d->sidebars.removeAll(this);
    }

    delete d;
}

SidebarSplitter* Sidebar::splitter() const
{
    return d->splitter;
}

void Sidebar::doLoadState()
{
    KConfigGroup group        = getConfigGroup();
    int tab                   = group.readEntry(entryName(d->optionActiveTabEntry),   0);
    bool minimized            = group.readEntry(entryName(d->optionMinimizedEntry),   d->minimizedDefault);
    d->restoreSize            = group.readEntry(entryName(d->optionRestoreSizeEntry), -1);

    // validate
    if (tab >= d->tabs || tab < 0)
    {
        tab = 0;
    }

    if (minimized)
    {
        d->activeTab = tab;
        setTab(d->activeTab, false);
        d->stack->setCurrentIndex(d->activeTab);
        shrink();
        emit signalChangedTab(d->stack->currentWidget());
        return;
    }

    d->activeTab = -1;
    clicked(tab);
}

void Sidebar::doSaveState()
{
    KConfigGroup group = getConfigGroup();
    group.writeEntry(entryName(d->optionActiveTabEntry),   d->activeTab);
    group.writeEntry(entryName(d->optionMinimizedEntry),   d->minimized);
    group.writeEntry(entryName(d->optionRestoreSizeEntry), d->minimized ? d->restoreSize : -1);
}

void Sidebar::backup()
{
    // backup preview state of sidebar view (shrink or not)
    d->isMinimized = d->minimized;

    // In all case, shrink sidebar view
    shrink();

    DMultiTabBar::hide();
}

void Sidebar::backup(const QList<QWidget*> thirdWidgetsToBackup, QList<int>* const sizes)
{
    sizes->clear();

    foreach(QWidget* const widget, thirdWidgetsToBackup)
    {
        *sizes << d->splitter->size(widget);
    }

    backup();
}

void Sidebar::restore()
{
    DMultiTabBar::show();

    // restore preview state of sidebar view, stored in backup()
    if (!d->isMinimized)
    {
        expand();
    }
}

void Sidebar::restore(const QList<QWidget*> thirdWidgetsToRestore, const QList<int>& sizes)
{
    restore();

    if (thirdWidgetsToRestore.size() == sizes.size())
    {
        for (int i=0; i<thirdWidgetsToRestore.size(); ++i)
        {
            d->splitter->setSize(thirdWidgetsToRestore.at(i), sizes.at(i));
        }
    }
}

void Sidebar::appendTab(QWidget* const w, const QIcon& pic, const QString& title)
{
    // Store state (but not on initialization)
    if (isVisible())
    {
        d->appendedTabsStateCache[w] = SidebarState(d->stack->currentWidget(), d->splitter->size(this));
    }

    // Add tab
    w->setParent(d->stack);
    DMultiTabBar::appendTab(pic.pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)), d->tabs, title);
    d->stack->insertWidget(d->tabs, w);

    tab(d->tabs)->setAcceptDrops(true);
    tab(d->tabs)->installEventFilter(this);

    connect(tab(d->tabs), SIGNAL(clicked(int)),
            this, SLOT(clicked(int)));

    d->tabs++;
}

void Sidebar::deleteTab(QWidget* const w)
{
    int tab = d->stack->indexOf(w);

    if (tab < 0)
    {
        return;
    }

    bool removingActiveTab = (tab == d->activeTab);

    if (removingActiveTab)
    {
        d->activeTab = -1;
    }

    d->stack->removeWidget(d->stack->widget(tab));
    // delete widget
    removeTab(tab);
    d->tabs--;

    // restore or reset active tab and width
    if (!d->minimized)
    {
        // restore to state before adding tab
        // using a hash is simple, but does not handle well multiple add/removal operations at a time
        SidebarState state = d->appendedTabsStateCache.take(w);

        if (state.activeWidget)
        {
            int tab = d->stack->indexOf(state.activeWidget);

            if (tab != -1)
            {
                switchTabAndStackToTab(tab);
                emit signalChangedTab(d->stack->currentWidget());

                if (state.size == 0)
                {
                    d->minimized = true;
                    setTab(d->activeTab, false);
                }

                d->splitter->setSize(this, state.size);
            }
        }
        else
        {
            if (removingActiveTab)
            {
                clicked(d->tabs - 1);
            }

            d->splitter->setSize(this, -1);
        }
    }
    else
    {
        d->restoreSize = -1;
    }
}

void Sidebar::clicked(int tab)
{
    if (tab >= d->tabs || tab < 0)
    {
        return;
    }

    if (tab == d->activeTab)
    {
        d->stack->isHidden() ? expand() : shrink();
    }
    else
    {
        switchTabAndStackToTab(tab);

        if (d->minimized)
        {
            expand();
        }

        emit signalChangedTab(d->stack->currentWidget());
    }
}

void Sidebar::setActiveTab(QWidget* const w)
{
    int tab = d->stack->indexOf(w);

    if (tab < 0)
    {
        return;
    }

    switchTabAndStackToTab(tab);

    if (d->minimized)
    {
        expand();
    }

    emit signalChangedTab(d->stack->currentWidget());
}

void Sidebar::activePreviousTab()
{
    int tab = d->stack->indexOf(d->stack->currentWidget());

    if (tab == 0)
        tab = d->tabs-1;
    else
        tab--;

    setActiveTab(d->stack->widget(tab));
}

void Sidebar::activeNextTab()
{
    int tab = d->stack->indexOf(d->stack->currentWidget());

    if (tab== d->tabs-1)
        tab = 0;
    else
        tab++;

    setActiveTab(d->stack->widget(tab));
}

void Sidebar::switchTabAndStackToTab(int tab)
{
    if (d->activeTab >= 0)
    {
        setTab(d->activeTab, false);
    }

    d->activeTab = tab;
    setTab(d->activeTab, true);
    d->stack->setCurrentIndex(d->activeTab);
}

QWidget* Sidebar::getActiveTab() const
{
    if (d->splitter)
    {
        return d->stack->currentWidget();
    }
    else
    {
        return 0;
    }
}

void Sidebar::shrink()
{
    d->minimized = true;

    // store the size that we had. We may later need it when we restore to visible.
    int currentSize = d->splitter->size(this);

    if (currentSize)
    {
        d->restoreSize = currentSize;
    }

    d->stack->hide();
    emit signalViewChanged();
}

void Sidebar::expand()
{
    d->minimized = false;
    d->stack->show();

    // Do not expand to size 0 (only splitter handle visible)
    // but either to previous size, or the minimum size hint
    if (d->splitter->size(this) == 0)
    {
        setTab(d->activeTab, true);
        d->splitter->setSize(this, d->restoreSize ? d->restoreSize : -1);
    }

    emit signalViewChanged();
}

bool Sidebar::isExpanded() const
{
    return !d->minimized;
}

bool Sidebar::eventFilter(QObject* obj, QEvent* ev)
{
    for (int i = 0 ; i < d->tabs; ++i)
    {
        if ( obj == tab(i) )
        {
            if ( ev->type() == QEvent::DragEnter)
            {
                QDragEnterEvent* const e = static_cast<QDragEnterEvent*>(ev);
                enterEvent(e);
                e->accept();
                return false;
            }
            else if (ev->type() == QEvent::DragMove)
            {
                if (!d->dragSwitchTimer->isActive())
                {
                    d->dragSwitchTimer->setSingleShot(true);
                    d->dragSwitchTimer->start(800);
                    d->dragSwitchId = i;
                }

                return false;
            }
            else if (ev->type() == QEvent::DragLeave)
            {
                d->dragSwitchTimer->stop();
                QDragLeaveEvent* const e = static_cast<QDragLeaveEvent*>(ev);
                leaveEvent(e);
                return false;
            }
            else if (ev->type() == QEvent::Drop)
            {
                d->dragSwitchTimer->stop();
                QDropEvent* const e = static_cast<QDropEvent*>(ev);
                leaveEvent(e);
                return false;
            }
            else
            {
                return false;
            }
        }
    }

    // Else, pass the event on to the parent class
    return DMultiTabBar::eventFilter(obj, ev);
}

void Sidebar::slotDragSwitchTimer()
{
    clicked(d->dragSwitchId);
}

void Sidebar::slotSplitterBtnClicked()
{
    clicked(d->activeTab);
}

// -----------------------------------------------------------------------------

const QString SidebarSplitter::DEFAULT_CONFIG_KEY = QLatin1String("SplitterState");

SidebarSplitter::SidebarSplitter(QWidget* const parent)
    : QSplitter(parent),
      d(new Private)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved(int,int)));
}

SidebarSplitter::SidebarSplitter(Qt::Orientation orientation, QWidget* const parent)
    : QSplitter(orientation, parent),
      d(new Private)
{
    connect(this, SIGNAL(splitterMoved(int,int)),
            this, SLOT(slotSplitterMoved(int,int)));
}

SidebarSplitter::~SidebarSplitter()
{
    // retreat cautiously from sidebars that live longer
    foreach(Sidebar* const sidebar, d->sidebars)
    {
        sidebar->d->splitter = 0;
    }

    delete d;
}

void SidebarSplitter::restoreState(KConfigGroup& group)
{
    restoreState(group, DEFAULT_CONFIG_KEY);
}

void SidebarSplitter::restoreState(KConfigGroup& group, const QString& key)
{
    if (group.hasKey(key))
    {
        QByteArray state;
        state = group.readEntry(key, state);
        QSplitter::restoreState(QByteArray::fromBase64(state));
    }
}

void SidebarSplitter::saveState(KConfigGroup& group)
{
    saveState(group, DEFAULT_CONFIG_KEY);
}

void SidebarSplitter::saveState(KConfigGroup& group, const QString& key)
{
    group.writeEntry(key, QSplitter::saveState().toBase64());
}

int SidebarSplitter::size(Sidebar* const bar) const
{
    return size(bar->d->stack);
}

int SidebarSplitter::size(QWidget* const widget) const
{
    int index = indexOf(widget);

    if (index == -1)
    {
        return -1;
    }

    return sizes().at(index);
}

void SidebarSplitter::setSize(Sidebar* const bar, int size)
{
    setSize(bar->d->stack, size);
}

void SidebarSplitter::setSize(QWidget* const widget, int size)
{
    int index = indexOf(widget);

    if (index == -1)
    {
        return;
    }

    // special case: Use minimum size hint
    if (size == -1)
    {
        if (orientation() == Qt::Horizontal)
        {
            size = widget->minimumSizeHint().width();
        }

        if (orientation() == Qt::Vertical)
        {
            size = widget->minimumSizeHint().height();
        }
    }

    QList<int> sizeList = sizes();
    sizeList[index] = size;
    setSizes(sizeList);
}

void SidebarSplitter::slotSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);

    // When the user moves the splitter so that size is 0 (collapsed),
    // it may be difficult to restore the sidebar as clicking the buttons
    // has no effect (only hides/shows the splitter handle)
    // So we want to transform this size-0-sidebar
    // to a sidebar that is shrunk (d->stack hidden)
    // and can be restored by clicking a tab bar button

    // We need to look at the widget between index-1 and index
    // and the one between index and index+1

    QList<int> sizeList = sizes();

    // Is there a sidebar with size 0 before index ?

    if (index > 0 && sizeList.at(index-1) == 0)
    {
        QWidget* const w = widget(index-1);

        foreach(Sidebar* const sidebar, d->sidebars)
        {
            if (w == sidebar->d->stack)
            {
                if (!sidebar->d->minimized)
                {
                    sidebar->setTab(sidebar->d->activeTab, false);
                    sidebar->shrink();
                }

                break;
            }
        }
    }

    // Is there a sidebar with size 0 after index ?

    if (sizeList.at(index) == 0)
    {
        QWidget* const w = widget(index);

        foreach(Sidebar* sidebar, d->sidebars)
        {
            if (w == sidebar->d->stack)
            {
                if (!sidebar->d->minimized)
                {
                    sidebar->setTab(sidebar->d->activeTab, false);
                    sidebar->shrink();
                }

                break;
            }
        }
    }
}

}  // namespace Digikam
