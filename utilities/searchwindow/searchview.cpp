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

#include "searchview.moc"

// Qt includes

#include <QGradient>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QTimeLine>
#include <QVBoxLayout>

// KDE includes

#include <kdialogbuttonbox.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandardguiitem.h>
#include <kdebug.h>
#include <kapplication.h>

// Local includes

#include "searchgroup.h"
#include "searchutilities.h"
#include "searchwindow.h"
#include "searchxml.h"
#include "thememanager.h"

namespace Digikam
{

AbstractSearchGroupContainer::AbstractSearchGroupContainer(QWidget* const parent)
    : QWidget(parent), m_groupIndex(0)
{
}

SearchGroup* AbstractSearchGroupContainer::addSearchGroup()
{
    SearchGroup* group = createSearchGroup();
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
        kWarning() << "Attempt to delete the primary search group";
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
    foreach(SearchGroup* group, m_groups)
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
    foreach(SearchGroup* group, m_groups)
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
    d->layout->setContentsMargins(0, 0, 0, 0);
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
    SearchGroup* group = new SearchGroup(this);
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
    kDebug() << writer.xml();
    return writer.xml();
}

void SearchView::startAnimation()
{
    d->timeline->setCurveShape(QTimeLine::EaseInCurve);
    d->timeline->setDuration(500);
    d->timeline->setDirection(QTimeLine::Forward);
#if QT_VERSION >= 0x040400
    d->timeline->start();
#endif
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
#if QT_VERSION >= 0x040400

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
        QColor color = kapp->palette().color(QPalette::Link);
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

#endif
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
        fontSizeLarger  = QString::number(f.pixelSize() + 2) + "px";
        fontSizeSmaller = QString::number(f.pixelSize() - 2) + "px";
    }
    else
    {
        fontSizeLarger  = QString::number(f.pointSizeF() + 2) + "pt";
        fontSizeSmaller = QString::number(f.pointSizeF() - 2) + "pt";
    }

    QString sheet =
        // ".SearchView { background-color: " + kapp->palette().color(QPalette::Base).name() + "; } "
        "#SearchGroupLabel_MainLabel "
        " { font-weight: bold; font-size: "
        + fontSizeLarger + ';' +
        "   color: "
        + kapp->palette().color(QPalette::HighlightedText).name() + ';' +
        " } "
        "#SearchGroupLabel_SimpleLabel "
        " { font-size: "
        + fontSizeLarger + ';' +
        "   color: "
        + kapp->palette().color(QPalette::HighlightedText).name() + ';' +
        " } "
        "#SearchGroupLabel_GroupOpLabel "
        " { font-weight: bold; font-size: "
        + fontSizeLarger + ';' +
        "   color: "
        + kapp->palette().color(QPalette::HighlightedText).name() + ';' +
        "   text-decoration: underline; "
        " } "
        "#SearchGroupLabel_CheckBox "
        " { color: "
        + kapp->palette().color(QPalette::HighlightedText).name() + ';' +
        " } "
        "#SearchGroupLabel_RemoveLabel "
        " { color: "
        + kapp->palette().color(QPalette::HighlightedText).name() + ';' +
        "   font-style: italic; "
        "   text-decoration: underline; "
        " } "
        "#SearchGroupLabel_OptionsLabel "
        " { color: "
        + kapp->palette().color(QPalette::HighlightedText).name() + ';' +
        "   font-style: italic; "
        "   text-decoration: underline; font-size: "
        + fontSizeSmaller + ';' +
        " } "
        "#SearchFieldGroupLabel_Label "
        " { color: "
        + kapp->palette().color(QPalette::Link).name() + ';' +
        "   font-weight: bold; "
        " } "
        "#SearchField_MainLabel "
        " { font-weight: bold; } "
        "#SearchFieldChoice_ClickLabel "
        " { color: "
        + kapp->palette().color(QPalette::Link).name() + ';' +
        "   font-style: italic; "
        "   text-decoration: underline; "
        " } "
        "QComboBox#SearchFieldChoice_ComboBox"
        " {  border-width: 0px; border-style: solid; padding-left: 5px; "
        " } "
        "QComboBox::drop-down#SearchFieldChoice_ComboBox"
        " {  subcontrol-origin: padding; subcontrol-position: right top; "
        "    border: 0px; background: rgba(0,0,0,0); width: 0px; height: 0px; "
        " } ";

    QWidget::setStyleSheet(sheet);

    d->pixmapCache.clear();
}

QPixmap SearchView::cachedBannerPixmap(int w, int h) const
{
    QString key  = "BannerPixmap-" + QString::number(w) + '-' + QString::number(h);
    QPixmap* pix = d->pixmapCache.object(key);

    if (!pix)
    {
        QPixmap pixmap(w, h);
        pixmap.fill(kapp->palette().color(QPalette::Highlight));
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

    m_addGroupsButton = new KPushButton(KStandardGuiItem::add());
    m_addGroupsButton->setText(i18n("Add Search Group"));

    connect(m_addGroupsButton, SIGNAL(clicked()),
            this, SIGNAL(addGroupPressed()));

    m_mainLayout->addWidget(m_addGroupsButton);

    m_resetButton = new KPushButton(KStandardGuiItem::reset());
    //m_addGroupsButton->setText(i18n("Reset"));

    connect(m_resetButton, SIGNAL(clicked()),
            this, SIGNAL(resetPressed()));

    m_mainLayout->addWidget(m_resetButton);

    m_mainLayout->addStretch(1);

    m_buttonBox = new KDialogButtonBox(this);
    m_buttonBox->addButton(KStandardGuiItem::ok(),
                           QDialogButtonBox::AcceptRole,
                           this,
                           SIGNAL(okPressed()));
    m_buttonBox->addButton(KStandardGuiItem::cancel(),
                           QDialogButtonBox::RejectRole,
                           this,
                           SIGNAL(cancelPressed()));
    KPushButton* aBtn = m_buttonBox->addButton(KStandardGuiItem::apply(),
                                               QDialogButtonBox::ApplyRole,
                                               this,
                                               SIGNAL(tryoutPressed()));
    aBtn->setText(i18n("Try"));
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
