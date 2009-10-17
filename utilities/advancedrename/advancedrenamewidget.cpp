/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "advancedrenamewidget.h"
#include "advancedrenamewidget.moc"

// Qt includes

#include <QAction>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kdialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "advancedrenameinput.h"
#include "dcursortracker.h"
#include "defaultparser.h"
#include "themeengine.h"
#include "rexpanderbox.h"

namespace Digikam
{

class AdvancedRenameWidgetPriv
{
public:

    AdvancedRenameWidgetPriv() :
        inputColumns(2),
        optionsExpandedDefault(false),
        tooltipToggleButton(0),
        tokenToolButton(0),
        modifierToolButton(0),
        btnContainer(0),
        tooltipTrackerAlignment(Qt::AlignLeft),
        tooltipTracker(0),
        renameInputWidget(0),
        parser(0),
        optionsLabel(0),
        controlWidgetsMask(AdvancedRenameWidget::TokenButtons  |
                           AdvancedRenameWidget::ToolTipButton |
                           AdvancedRenameWidget::ModifierToolButton)
    {}

    int                                  inputColumns;
    bool                                 optionsExpandedDefault;

    QToolButton*                         tooltipToggleButton;
    QToolButton*                         tokenToolButton;
    QToolButton*                         modifierToolButton;

    QWidget*                             btnContainer;

    Qt::Alignment                        tooltipTrackerAlignment;

    DTipTracker*                         tooltipTracker;
    AdvancedRenameInput*                 renameInputWidget;
    Parser*                              parser;
    RLabelExpander*                      optionsLabel;

    AdvancedRenameWidget::ControlWidgets controlWidgetsMask;
};

AdvancedRenameWidget::AdvancedRenameWidget(QWidget* parent)
                 : QWidget(parent), d(new AdvancedRenameWidgetPriv)
{
    setupWidgets();
    setParser(new DefaultParser());
}

AdvancedRenameWidget::~AdvancedRenameWidget()
{
    writeSettings();

    // we need to delete it manually, because it has no parent
    delete d->tooltipTracker;

    delete d->parser;
    delete d;
}

QString AdvancedRenameWidget::text() const
{
    return d->renameInputWidget->text();
}

void AdvancedRenameWidget::setText(const QString& text)
{
    d->renameInputWidget->setText(text);
}

void AdvancedRenameWidget::setTooltipAlignment(Qt::Alignment alignment)
{
    d->tooltipTrackerAlignment = alignment;
    d->tooltipTracker->setTrackerAlignment(alignment);
}

void AdvancedRenameWidget::clear()
{
    d->renameInputWidget->clear();
}

void AdvancedRenameWidget::slotHideToolTipTracker()
{
    d->tooltipToggleButton->setChecked(false);
    slotToolTipButtonToggled(false);
}

QString AdvancedRenameWidget::parse(ParseInformation& info) const
{
    if (!d->parser)
    {
        return QString();
    }

    QString parseString = d->renameInputWidget->text();

    QString parsed;
    parsed = d->parser->parse(parseString, info);

    return parsed;
}

void AdvancedRenameWidget::createToolTip()
{
#define TOOLTIP_HEADER(str)                                                                     \
    do                                                                                          \
    {                                                                                           \
        tooltip += QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"                              \
                           "<nobr><font color=\"%2\"><center><b>")                              \
                           .arg(ThemeEngine::instance()->baseColor().name())                    \
                           .arg(ThemeEngine::instance()->textRegColor().name());                \
        tooltip += QString(str);                                                                \
        tooltip += QString("</b></center></font></nobr></td></tr>");                            \
    } while (0)                                                                                 \


#define TOOLTIP_ENTRIES(type, data)                                                             \
    do                                                                                          \
    {                                                                                           \
        foreach (type* t, data)                                                                 \
        {                                                                                       \
            foreach (Token* token, t->tokens())                                                 \
            {                                                                                   \
                tooltip += QString("<tr>"                                                       \
                                   "<td bgcolor=\"%1\">"                                        \
                                       "<font color=\"%2\"><b>&nbsp;%3&nbsp;</b></font></td>"   \
                                   "<td>&nbsp;%4&nbsp;</td></tr>")                              \
                                   .arg(ThemeEngine::instance()->baseColor().name())            \
                                   .arg(ThemeEngine::instance()->textRegColor().name())         \
                                   .arg(token->id())                                            \
                                   .arg(token->description());                                  \
            }                                                                                   \
        }                                                                                       \
    } while (0)

    // --------------------------------------------------------

    if (!d->parser)
    {
        d->tooltipTracker->setText(QString());
    }
    else
    {
        QString tooltip;
        tooltip += QString("<qt><table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">");

        // --------------------------------------------------------

        TOOLTIP_HEADER(i18n("Renaming Options"));
        TOOLTIP_ENTRIES(SubParser, d->parser->subParsers());

        tooltip += QString("<tr></tr>");

        TOOLTIP_HEADER(i18n("Modifiers"));
        TOOLTIP_ENTRIES(Modifier, d->parser->modifiers());

        // --------------------------------------------------------

        tooltip += QString("</table></qt>");
        tooltip += QString("<font size=\"-1\">%1</font>").arg(d->renameInputWidget->toolTip());

        d->tooltipTracker->setText(tooltip);
    }

    // --------------------------------------------------------

#undef TOOLTIP_HEADER
#undef TOOLTIP_ENTRIES
}

void AdvancedRenameWidget::slotToolTipButtonToggled(bool checked)
{
    d->tooltipTracker->setVisible(checked);
    slotUpdateTrackerPos();
}

void AdvancedRenameWidget::slotUpdateTrackerPos()
{
    d->tooltipTracker->refresh();
}

void AdvancedRenameWidget::setControlWidgets(ControlWidgets mask)
{
    bool enable       = d->parser && !(d->parser->subParsers().isEmpty());
    bool enableModBtn = enable && !(d->parser->modifiers().isEmpty());

    d->renameInputWidget->setEnabled(enable);
    d->optionsLabel->setVisible(enable && (mask & TokenButtons));
    d->tokenToolButton->setVisible(enable && (mask & TokenToolButton));
    d->modifierToolButton->setVisible(enableModBtn && (mask & ModifierToolButton));
    d->tooltipToggleButton->setVisible(enable && (mask & ToolTipButton));

    d->controlWidgetsMask = mask;
}

void AdvancedRenameWidget::registerParserControls()
{
   if (d->parser)
   {
       setupWidgets();

       QMenu* tokenToolBtnMenu    = new QMenu(d->tokenToolButton);
       QMenu* modifierToolBtnMenu = new QMenu(d->modifierToolButton);
       int column                 = 0;
       int row                    = 0;
       QPushButton* btn           = 0;
       QAction* action            = 0;
       QGridLayout* gridLayout    = new QGridLayout;
       gridLayout->setSpacing(KDialog::marginHint());
       gridLayout->setMargin(KDialog::marginHint());

       int maxParsers = d->parser->subParsers().count();
       foreach (SubParser* subparser, d->parser->subParsers())
       {
           btn    = subparser->registerButton(this);
           action = subparser->registerMenu(tokenToolBtnMenu);

           if (!btn || !action)
           {
               continue;
           }

           // set button tooltip
           btn->setToolTip(subparser->description());

           gridLayout->addWidget(btn, row, column, 1, 1);

           connect(subparser, SIGNAL(signalTokenTriggered(const QString&)),
                   d->renameInputWidget, SLOT(slotAddToken(const QString&)));

           ++column;

           if (column % d->inputColumns == 0)
           {
               ++row;
               column = 0;
           }
       }

       // --------------------------------------------------------

       // If the buttons don't fill up all columns, expand the last button to fit the layout
       if ((row >= (maxParsers / d->inputColumns)) && (column == 0))
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, (row - 1), (d->inputColumns - 1), 1, 1);
       }
       else if (column != d->inputColumns)
       {
           gridLayout->removeWidget(btn);
           gridLayout->addWidget(btn, row, (column - 1), 1, -1);
       }

       // --------------------------------------------------------

       // register modifiers
       foreach (Modifier* modifier, d->parser->modifiers())
       {
           QStringList avoidForNow;
           avoidForNow << QString("Range");

           if (!avoidForNow.contains(modifier->objectName()))
           {
               action = modifier->registerMenu(modifierToolBtnMenu);
           }

           if (!action)
           {
               continue;
           }

           connect(modifier, SIGNAL(signalTokenTriggered(const QString&)),
                   d->renameInputWidget, SLOT(slotAddModifier(const QString&)));
       }

       // --------------------------------------------------------

       d->btnContainer->setLayout(gridLayout);

       d->tokenToolButton->setMenu(tokenToolBtnMenu);
       d->modifierToolButton->setMenu(modifierToolBtnMenu);

       d->renameInputWidget->setParser(d->parser);
       createToolTip();
   }
}

void AdvancedRenameWidget::setParser(Parser* parser)
{
    if (!parser)
    {
        return;
    }

    if (d->parser)
    {
        delete d->parser;
    }
    d->parser = parser;

    setInputColumns(d->inputColumns);
    registerParserControls();
    setControlWidgets(d->controlWidgetsMask);
}

void AdvancedRenameWidget::setInputColumns(int col)
{
    if (col == d->inputColumns)
    {
        return;
    }

    d->inputColumns = col;
    registerParserControls();
    setControlWidgets(d->controlWidgetsMask);
}

void AdvancedRenameWidget::setupWidgets()
{
    /*
     * This methods needs to delete all main widgets, do not remove the delete lines!
     * If a new parser is set or the layout has changed, we need to call setupWidgets() again.
     * So any widget that is created in here needs to be removed first, to avoid memory leaks and
     * duplicate signal/slot connections.
     */
    delete d->renameInputWidget;
    d->renameInputWidget = new AdvancedRenameInput;

    delete d->tooltipToggleButton;
    d->tooltipToggleButton = new QToolButton;
    d->tooltipToggleButton->setCheckable(true);
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));
    d->tooltipToggleButton->setToolTip(i18n("Show help"));

    // --------------------------------------------------------

    delete d->btnContainer;
    d->btnContainer = new QWidget(this);

    delete d->optionsLabel;
    d->optionsLabel = new RLabelExpander(this);
    d->optionsLabel->setText(i18n("Renaming Options"));
    d->optionsLabel->setWidget(d->btnContainer);
    d->optionsLabel->setLineVisible(false);

    // --------------------------------------------------------

    delete d->tokenToolButton;
    d->tokenToolButton = new QToolButton;
    d->tokenToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tokenToolButton->setIcon(SmallIcon("list-add"));
    d->tokenToolButton->setToolTip(i18n("Quickly add a renaming option"));

    delete d->modifierToolButton;
    d->modifierToolButton = new QToolButton;
    d->modifierToolButton->setPopupMode(QToolButton::InstantPopup);
    d->modifierToolButton->setIcon(SmallIcon("list-add-font"));
    d->modifierToolButton->setToolTip(i18n("Quickly add a modifier to the marked token"));

    // --------------------------------------------------------

    delete d->tooltipTracker;
    d->tooltipTracker = new DTipTracker(QString(), d->renameInputWidget, Qt::AlignLeft);
    d->tooltipTracker->setTextFormat(Qt::RichText);
    d->tooltipTracker->setEnable(false);
    d->tooltipTracker->setKeepOpen(true);
    d->tooltipTracker->setOpenExternalLinks(true);
    setTooltipAlignment(d->tooltipTrackerAlignment);

    // --------------------------------------------------------

    delete layout();
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->renameInputWidget,   0, 0, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton, 0, 1, 1, 1);
    mainLayout->addWidget(d->tokenToolButton,     0, 2, 1, 1);
    mainLayout->addWidget(d->modifierToolButton,  0, 3, 1, 1);
    mainLayout->addWidget(d->optionsLabel,        1, 0, 1,-1);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::marginHint());
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(toggled(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->renameInputWidget, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

    connect(d->renameInputWidget, SIGNAL(signalTokenMarked(bool)),
            this, SLOT(slotTokenMarked(bool)));

    slotTokenMarked(false);
    readSettings();
}

void AdvancedRenameWidget::slotTokenMarked(bool marked)
{
    bool enable    = marked && d->parser;
    bool enableMod = enable && !(d->parser->modifiers().isEmpty());
    d->modifierToolButton->setEnabled(enableMod);
}

void AdvancedRenameWidget::focusLineEdit()
{
    d->renameInputWidget->setFocus();
}

void AdvancedRenameWidget::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("AdvancedRenameWidget");
    d->optionsLabel->setExpanded(group.readEntry("Options are expanded", d->optionsExpandedDefault));
}

void AdvancedRenameWidget::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("AdvancedRenameWidget");
    group.writeEntry("Options are expanded", (d->optionsLabel) ?
                                              d->optionsLabel->isExpanded() :
                                              d->optionsExpandedDefault);
}

}  // namespace Digikam
