/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "searchview.h"

// Qt includes

#include <QGradient>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QTimeLine>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDialogButtonBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "searchgroup.h"
#include "searchutilities.h"
#include "searchwindow.h"
#include "coredbsearchxml.h"
#include "thememanager.h"

namespace Digikam
{

AbstractSearchGroupContainer::AbstractSearchGroupContainer(QWidget* const parent)
    : QWidget(parent),
      m_groupIndex(0)
{
}

SearchGroup* AbstractSearchGroupContainer::addSearchGroup()
{
    SearchGroup* const group = createSearchGroup();
    m_groups << group;
    addGroupToLayout(group);

    connect(group, SIGNAL(removeRequested()),
            this, SLOT(removeSendingSearchGroup()));

    return group;
}

void AbstractSearchGroupContainer::removeSearchGroup(SearchGroup* group)
{
    if (group->groupType() == SearchGroup::FirstGroup)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Attempt to delete the primary search group";
        return;
    }

    m_groups.removeAll(group);
    // This method call may arise from an event handler of a widget within group. Defer deletion.
    group->deleteLater();
}

void AbstractSearchGroupContainer::startReadingGroups(SearchXmlCachingReader&)
{
    m_groupIndex = 0;
}

void AbstractSearchGroupContainer::readGroup(SearchXmlCachingReader& reader)
{
    SearchGroup* group = 0;

    if (m_groupIndex >= m_groups.size())
    {
        group = addSearchGroup();
    }
    else
    {
        group = m_groups.at(m_groupIndex);
    }

    group->read(reader);

    ++m_groupIndex;
}

void AbstractSearchGroupContainer::finishReadingGroups()
{
    // remove superfluous groups
    while (m_groups.size() > (m_groupIndex + 1))
    {
        delete m_groups.takeLast();
    }

    // for empty searches, and we have an initial search group, reset the remaining search group
    if (!m_groupIndex && !m_groups.isEmpty())
    {
        m_groups.first()->reset();
    }
}

void AbstractSearchGroupContainer::writeGroups(SearchXmlWriter& writer) const
{
    foreach(SearchGroup* const group, m_groups)
    {
        group->write(writer);
    }
}

void AbstractSearchGroupContainer::removeSendingSearchGroup()
{
    removeSearchGroup(static_cast<SearchGroup*>(sender()));
}

QList<QRect> AbstractSearchGroupContainer::startupAnimationAreaOfGroups() const
{
    QList<QRect> list;

    foreach(SearchGroup* const group, m_groups)
    {
        list += group->startupAnimationArea();
    }

    return list;
}

// -------------------------------------------------------------------------

class SearchView::Private
{
public:

    Private() :
        needAnimationForReadIn(false),
        layout(0),
        timeline(0),
        bar(0)
    {
    }

    bool                     needAnimationForReadIn;

    QVBoxLayout*             layout;
    QCache<QString, QPixmap> pixmapCache;
    QTimeLine*               timeline;

    SearchViewBottomBar*     bar;
};

SearchView::SearchView()
    : d(new Private)
{
    d->pixmapCache.setMaxCost(4);
}

SearchView::~SearchView()
{
    delete d;
}

void SearchView::setup()
{
    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(setTheme()));

    setTheme();

    d->layout = new QVBoxLayout;
    d->layout->setContentsMargins(QMargins());
    d->layout->setSpacing(0);

    // add stretch at bottom
    d->layout->addStretch(1);

    // create initial group
    addSearchGroup();

    setLayout(d->layout);

    // prepare animation
    d->timeline = new QTimeLine(500, this);
    d->timeline->setFrameRange(0, 100);

    connect(d->timeline, SIGNAL(finished()),
            this, SLOT(timeLineFinished()));

    connect(d->timeline, SIGNAL(frameChanged(int)),
            this, SLOT(animationFrame(int)));
}

void SearchView::setBottomBar(SearchViewBottomBar* const bar)
{
    d->bar = bar;

    connect(d->bar, SIGNAL(okPressed()),
            this, SIGNAL(searchOk()));

    connect(d->bar, SIGNAL(cancelPressed()),
            this, SIGNAL(searchCancel()));

    connect(d->bar, SIGNAL(tryoutPressed()),
            this, SIGNAL(searchTryout()));

    connect(d->bar, SIGNAL(addGroupPressed()),
            this, SLOT(slotAddGroupButton()));

    connect(d->bar, SIGNAL(resetPressed()),
            this, SLOT(slotResetButton()));
}

void SearchView::read(const QString& xml)
{
    SearchXmlCachingReader reader(xml);

    startReadingGroups(reader);
    SearchXml::Element element;

    while (!reader.atEnd())
    {
        element = reader.readNext();

        if (element == SearchXml::Group)
        {
            readGroup(reader);
        }
    }

    finishReadingGroups();

    if (isVisible())
    {
        startAnimation();
    }
    else
    {
        d->needAnimationForReadIn = true;
    }
}

void SearchView::addGroupToLayout(SearchGroup* group)
{
    // insert at last-but-one position; leave stretch at the bottom
    d->layout->insertWidget(d->layout->count() - 1, group);
}

SearchGroup* SearchView::createSearchGroup()
{
    SearchGroup* const group = new SearchGroup(this);
    group->setup(m_groups.isEmpty() ? SearchGroup::FirstGroup : SearchGroup::ChainGroup);
    return group;
}

void SearchView::slotAddGroupButton()
{
    addSearchGroup();
}

void SearchView::slotResetButton()
{
    while (m_groups.size() > 1)
    {
        delete m_groups.takeLast();
    }

    if (!m_groups.isEmpty())
    {
        if (m_groups.first())
        {
            m_groups.first()->reset();
        }
    }
}

QString SearchView::write() const
{
    SearchXmlWriter writer;
    writeGroups(writer);
    writer.finish();
    qCDebug(DIGIKAM_GENERAL_LOG) << writer.xml();
    return writer.xml();
}

void SearchView::startAnimation()
{
    d->timeline->setCurveShape(QTimeLine::EaseInCurve);
    d->timeline->setDuration(500);
    d->timeline->setDirection(QTimeLine::Forward);
    d->timeline->start();
}

void SearchView::animationFrame(int)
{
    update();
}

void SearchView::timeLineFinished()
{
    if (d->timeline->direction() == QTimeLine::Forward)
    {
        d->timeline->setDirection(QTimeLine::Backward);
        d->timeline->start();
    }
    else
    {
        update();
    }
}

void SearchView::showEvent(QShowEvent*)
{
    if (d->needAnimationForReadIn)
    {
        d->needAnimationForReadIn = false;
        startAnimation();
    }
}

void SearchView::paintEvent(QPaintEvent*)
{
    if (d->timeline->state() == QTimeLine::Running)
    {
        QList<QRect> rects = startupAnimationAreaOfGroups();

        if (rects.isEmpty())
        {
            return;
        }

        int animationStep = d->timeline->currentFrame();
        const int margin = 2;

        QRadialGradient grad(0.5, 0.5, 1, 0.5, 0.3);
        grad.setCoordinateMode(QGradient::ObjectBoundingMode);
        QColor color = qApp->palette().color(QPalette::Link);
        QColor colorStart(color), colorEnd(color);
        colorStart.setAlphaF(0);
        colorEnd.setAlphaF(color.alphaF()  * animationStep / 100.0);
        grad.setColorAt(0, colorEnd);
        grad.setColorAt(1, colorStart);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(Qt::NoPen));
        p.setBrush(grad);

        foreach(QRect rect, rects) // krazy:exclude=foreach
        {
            rect.adjust(-margin, -margin, margin, margin);
            p.drawRoundedRect(rect, 4, 4);
        }
    }
}

void SearchView::setTheme()
{
    // settings with style sheet results in extremely slow painting
    setBackgroundRole(QPalette::Base);

    QFont f = font();
    QString fontSizeLarger;
    QString fontSizeSmaller;

    if (f.pointSizeF() == -1)
    {
        // set pixel size
        fontSizeLarger  = QString::number(f.pixelSize() + 2) + QLatin1String("px");
        fontSizeSmaller = QString::number(f.pixelSize() - 2) + QLatin1String("px");
    }
    else
    {
        fontSizeLarger  = QString::number(f.pointSizeF() + 2) + QLatin1String("pt");
        fontSizeSmaller = QString::number(f.pointSizeF() - 2) + QLatin1String("pt");
    }

    QString sheet =
        QLatin1String("#SearchGroupLabel_MainLabel "
                      " { font-weight: bold; font-size: ")
        + fontSizeLarger + QLatin1Char(';') +
        QLatin1String("   color: ")
        + qApp->palette().color(QPalette::HighlightedText).name() + QLatin1Char(';') +
        QLatin1String(" } "
                      "#SearchGroupLabel_SimpleLabel "
                      " { font-size: ")
        + fontSizeLarger + QLatin1Char(';') +
        QLatin1String("   color: ")
        + qApp->palette().color(QPalette::HighlightedText).name() + QLatin1Char(';') +
        QLatin1String(" } "
                      "#SearchGroupLabel_GroupOpLabel "
                      " { font-weight: bold; font-size: ")
        + fontSizeLarger + QLatin1Char(';') +
        QLatin1String("   color: ")
        + qApp->palette().color(QPalette::HighlightedText).name() + QLatin1Char(';') +
        QLatin1String("   text-decoration: underline; "
                      " } "
                      "#SearchGroupLabel_CheckBox "
                      " { color: ")
        + qApp->palette().color(QPalette::HighlightedText).name() + QLatin1Char(';') +
        QLatin1String(" } "
                      "#SearchGroupLabel_RemoveLabel "
                      " { color: ")
        + qApp->palette().color(QPalette::HighlightedText).name() + QLatin1Char(';') +
        QLatin1String("   font-style: italic; "
                      "   text-decoration: underline; "
                      " } "
                      "#SearchGroupLabel_OptionsLabel "
                      " { color: ")
        + qApp->palette().color(QPalette::HighlightedText).name() + QLatin1Char(';') +
        QLatin1String("   font-style: italic; "
                      "   text-decoration: underline; font-size: ")
        + fontSizeSmaller + QLatin1Char(';') +
        QLatin1String(" } "
                      "#SearchFieldGroupLabel_Label "
                      " { color: ")
        + qApp->palette().color(QPalette::Link).name() + QLatin1Char(';') +
        QLatin1String("   font-weight: bold; "
                      " } "
                      "#SearchField_MainLabel "
                      " { font-weight: bold; } "
                      "#SearchFieldChoice_ClickLabel "
                      " { color: ")
        + qApp->palette().color(QPalette::Link).name() + QLatin1Char(';') +
        QLatin1String("   font-style: italic; "
                      "   text-decoration: underline; "
                      " } "
                      "QComboBox#SearchFieldChoice_ComboBox"
                      " {  border-width: 0px; border-style: solid; padding-left: 5px; "
                      " } "
                      "QComboBox::drop-down#SearchFieldChoice_ComboBox"
                      " {  subcontrol-origin: padding; subcontrol-position: right top; "
                      "    border: 0px; background: rgba(0,0,0,0); width: 0px; height: 0px; "
                      " } ");

    QWidget::setStyleSheet(sheet);

    d->pixmapCache.clear();
}

QPixmap SearchView::cachedBannerPixmap(int w, int h) const
{
    QString key  = QLatin1String("BannerPixmap-") + QString::number(w) + QLatin1Char('-') + QString::number(h);
    QPixmap* pix = d->pixmapCache.object(key);

    if (!pix)
    {
        QPixmap pixmap(w, h);
        pixmap.fill(qApp->palette().color(QPalette::Highlight));
        d->pixmapCache.insert(key, new QPixmap(pixmap));
        return pixmap;
    }
    else
    {
        return *pix;
    }
}

QPixmap SearchView::groupLabelPixmap(int w, int h)
{
    return cachedBannerPixmap(w, h);
}

QPixmap SearchView::bottomBarPixmap(int w, int h)
{
    return cachedBannerPixmap(w, h);
}

// -------------------------------------------------------------------------

SearchViewBottomBar::SearchViewBottomBar(SearchViewThemedPartsCache* const cache, QWidget* const parent)
    : QWidget(parent),
      m_themeCache(cache)
{
    m_mainLayout      = new QHBoxLayout;

    m_addGroupsButton = new QPushButton(i18n("Add Search Group"));
    m_addGroupsButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));

    connect(m_addGroupsButton, SIGNAL(clicked()),
            this, SIGNAL(addGroupPressed()));

    m_mainLayout->addWidget(m_addGroupsButton);

    m_resetButton = new QPushButton(i18n("Reset"));
    m_resetButton->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));

    connect(m_resetButton, SIGNAL(clicked()),
            this, SIGNAL(resetPressed()));

    m_mainLayout->addWidget(m_resetButton);
    m_mainLayout->addStretch(1);

    m_buttonBox = new QDialogButtonBox(this);

    QPushButton* const ok = m_buttonBox->addButton(QDialogButtonBox::Ok);

    connect(ok, SIGNAL(clicked()),
            this, SIGNAL(okPressed()));

    QPushButton* const cancel = m_buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(cancel, SIGNAL(clicked()),
            this, SIGNAL(cancelPressed()));

    QPushButton* const aBtn = m_buttonBox->addButton(i18n("Try"), QDialogButtonBox::ApplyRole);

    connect(aBtn, SIGNAL(clicked()),
            this, SIGNAL(tryoutPressed()));

    m_mainLayout->addWidget(m_buttonBox);

    setLayout(m_mainLayout);
}

void SearchViewBottomBar::paintEvent(QPaintEvent*)
{
    // paint themed background
    QPainter p(this);
    p.drawPixmap(0, 0, m_themeCache->bottomBarPixmap(width(), height()));
}

} // namespace Digikam
