/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

// Qt includes

#include <QAction>
#include <QApplication>
#include <QGridLayout>
#include <QIcon>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QStyle>
#include <QToolButton>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "advancedrenameinput.h"
#include "defaultrenameparser.h"
#include "dynamiclayout.h"
#include "tooltipcreator.h"
#include "tooltipdialog.h"

namespace Digikam
{

class AdvancedRenameWidget::Private
{
    typedef AdvancedRenameWidget::ControlWidgets CWMask;
    typedef AdvancedRenameWidget::LayoutStyle    LStyle;

public:

    Private() :
        configExpandedStateDefault(true),
        tooltipToggleButton(0),
        modifiersToolButton(0),
        optionsButton(0),
        modifiersButton(0),
        btnContainer(0),
        tooltipDialog(0),
        renameInput(0),
        parser(0),
        optionsLabel(0),
        controlWidgetsMask(AdvancedRenameWidget::DefaultControls),
        layoutStyle(AdvancedRenameWidget::LayoutNormal)
    {}

    static const QString configGroupName;
    static const QString configExpandedStateEntry;

    bool                 configExpandedStateDefault;

    QToolButton*         tooltipToggleButton;
    QToolButton*         modifiersToolButton;

    QPushButton*         optionsButton;
    QPushButton*         modifiersButton;

    QWidget*             btnContainer;

    TooltipDialog*       tooltipDialog;
    AdvancedRenameInput* renameInput;
    Parser*              parser;
    DLabelExpander*      optionsLabel;

    CWMask               controlWidgetsMask;
    LStyle               layoutStyle;
};

const QString AdvancedRenameWidget::Private::configGroupName(QLatin1String("AdvancedRename Widget"));
const QString AdvancedRenameWidget::Private::configExpandedStateEntry(QLatin1String("Options are expanded"));

// --------------------------------------------------------

AdvancedRenameWidget::AdvancedRenameWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setupWidgets();
}

AdvancedRenameWidget::~AdvancedRenameWidget()
{
    writeSettings();
    delete d;
}

QString AdvancedRenameWidget::parseString() const
{
    return d->renameInput->text();
}

void AdvancedRenameWidget::setParseString(const QString& text)
{
    d->renameInput->setText(text);
}

void AdvancedRenameWidget::setParseTimerDuration(int milliseconds)
{
    d->renameInput->setParseTimerDuration(milliseconds);
}

void AdvancedRenameWidget::clearParseString()
{
    d->renameInput->slotClearText();
}

void AdvancedRenameWidget::clear()
{
    d->renameInput->slotClearTextAndHistory();
}

QString AdvancedRenameWidget::parse(ParseSettings& settings) const
{
    if (!d->parser)
    {
        return QString();
    }

    settings.parseString = d->renameInput->text();

    QString parsed;
    parsed = d->parser->parse(settings);

    return parsed;
}

void AdvancedRenameWidget::createToolTip()
{
    d->tooltipDialog->clearTooltip();

    if (d->parser)
    {
        d->tooltipDialog->setTooltip(TooltipCreator::getInstance().tooltip(d->parser));
    }
}

void AdvancedRenameWidget::slotToolTipButtonToggled(bool checked)
{
    Q_UNUSED(checked)

    if (!d->tooltipDialog->isVisible())
    {
        d->tooltipDialog->show();
    }

    d->tooltipDialog->raise();
}

void AdvancedRenameWidget::setLayoutStyle(LayoutStyle style)
{
    d->layoutStyle = style;
    calculateLayout();
}

void AdvancedRenameWidget::setControlWidgets(ControlWidgets mask)
{
    d->controlWidgetsMask = mask;

    // we need a parser and at least one renaming option to successfully use
    // this widget.
    bool enable       = d->parser && !(d->parser->options().isEmpty());

    // enable the modifier toolbutton if environment has been set up correctly
    bool enableModBtn = enable && !(d->parser->modifiers().isEmpty());

    d->renameInput->setEnabled(enable);
    d->tooltipToggleButton->setVisible(enable && (mask & ToolTipButton));

    // layout specific
    if (d->layoutStyle == LayoutNormal)
    {
        d->optionsLabel->setVisible(enable && (mask & TokenButtons));
        d->modifiersToolButton->setVisible(enableModBtn && (mask & ModifierToolButton));
    }
    else
    {
        d->optionsButton->setVisible(enableModBtn && (mask & TokenButtons));
        d->modifiersButton->setVisible(enableModBtn && (mask & ModifierToolButton));
    }
}

QMenu* AdvancedRenameWidget::createControlsMenu(QWidget* parent, const RulesList& rules)
{
    QMenu* const menu = new QMenu(parent);
    QAction* action   = 0;

    foreach(Rule* const rule, rules)
    {
        action = rule->registerMenu(menu);

        if (!action)
        {
            continue;
        }

        connect(rule, SIGNAL(signalTokenTriggered(QString)),
                d->renameInput, SLOT(slotAddToken(QString)));
    }

    return menu;
}

void AdvancedRenameWidget::registerParserControls()
{
    if (d->parser)
    {
        setupWidgets();

        RulesList optionsList   = d->parser->options();
        RulesList modifiersList = d->parser->modifiers();

        if (d->layoutStyle == LayoutNormal)
        {
            // register options
            QPushButton* btn      = 0;
            DynamicLayout* const layout = new DynamicLayout(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin),
                                                            QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));

            foreach(Rule* const p, d->parser->options())
            {
                btn = p->registerButton(this);

                if (!btn)
                {
                    continue;
                }

                // set button tooltip
                btn->setToolTip(p->description());

                layout->addWidget(btn);

                connect(p, SIGNAL(signalTokenTriggered(QString)),
                        d->renameInput, SLOT(slotAddToken(QString)));
            }

            d->btnContainer->setLayout(layout);
            setMinimumWidth(d->btnContainer->layout()->sizeHint().width());

            // register modifiers
            QMenu* const modifiersMenu = createControlsMenu(d->modifiersToolButton, modifiersList);
            d->modifiersToolButton->setMenu(modifiersMenu);
        }
        else    // LayoutCompact
        {
            // register options
            QMenu* const optionsMenu = createControlsMenu(d->optionsButton, optionsList);
            d->optionsButton->setMenu(optionsMenu);

            // register modifiers
            QMenu* const modifiersMenu = createControlsMenu(d->modifiersButton, modifiersList);
            d->modifiersButton->setMenu(modifiersMenu);
        }

        // --------------------------------------------------------

        d->renameInput->setParser(d->parser);
        createToolTip();
    }
}

Parser* AdvancedRenameWidget::parser() const
{
    return d->parser;
}

void AdvancedRenameWidget::setParser(Parser* parser)
{
    if (!parser)
    {
        return;
    }

    d->parser = parser;

    calculateLayout();
}

void AdvancedRenameWidget::calculateLayout()
{
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
    delete d->tooltipDialog;
    delete d->renameInput;
    delete d->tooltipToggleButton;
    delete d->optionsButton;
    delete d->modifiersButton;
    delete d->btnContainer;
    delete d->optionsLabel;
    delete d->modifiersToolButton;

    // --------------------------------------------------------

    d->tooltipDialog = new TooltipDialog(this);
    d->tooltipDialog->resize(650, 530);

    d->renameInput = new AdvancedRenameInput;
    d->renameInput->setToolTip(i18n("<p>Enter your renaming pattern here. Use the access buttons to quickly add renaming "
                                    "options and modifiers. For further explanation, use the information button.</p>"));

    // --------------------------------------------------------

    d->tooltipToggleButton = new QToolButton;
    d->tooltipToggleButton->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->tooltipToggleButton->setToolTip(i18n("Show a list of all available options"));

    // --------------------------------------------------------

    QString modifiersStr     = i18n("Modifiers");
    QIcon   modifiersIcon    = QIcon::fromTheme(QLatin1String("document-edit"));
    QString modifiersTooltip = i18n("<p>Add a modifier to a renaming option. "
                                    "To activate this button, place the cursor behind a renaming option "
                                    "or an already assigned modifier.</p>");

    // --------------------------------------------------------

    delete layout();
    QGridLayout* const mainLayout = new QGridLayout;

    if (d->layoutStyle == LayoutNormal)
    {
        d->btnContainer = new QWidget(this);

        d->optionsLabel = new DLabelExpander(this);
        d->optionsLabel->setText(i18n("Renaming Options"));
        d->optionsLabel->setWidget(d->btnContainer);
        d->optionsLabel->setLineVisible(false);

        d->modifiersToolButton = new QToolButton;
        d->modifiersToolButton->setPopupMode(QToolButton::InstantPopup);
        d->modifiersToolButton->setText(modifiersStr);
        d->modifiersToolButton->setIcon(modifiersIcon);
        d->modifiersToolButton->setToolTip(modifiersTooltip);

        mainLayout->addWidget(d->renameInput,           0, 0, 1, 1);
        mainLayout->addWidget(d->modifiersToolButton,   0, 1, 1, 1);
        mainLayout->addWidget(d->tooltipToggleButton,   0, 2, 1, 1);
        mainLayout->addWidget(d->optionsLabel,          1, 0, 1, -1);
        mainLayout->setColumnStretch(0, 10);
    }
    else
    {
        d->optionsButton = new QPushButton;
        d->optionsButton->setText(i18n("Options"));
        d->optionsButton->setIcon(QIcon::fromTheme(QLatin1String("configure")));
        d->optionsButton->setToolTip(i18n("<p>Add renaming options to the parse string.</p>"));

        // --------------------------------------------------------

        d->modifiersButton = new QPushButton;
        d->modifiersButton->setText(modifiersStr);
        d->modifiersButton->setIcon(modifiersIcon);
        d->modifiersButton->setToolTip(modifiersTooltip);

        mainLayout->addWidget(d->renameInput,           0, 0, 1, -1);
        mainLayout->addWidget(d->optionsButton,         1, 0, 1, 1);
        mainLayout->addWidget(d->modifiersButton,       1, 1, 1, 1);
        mainLayout->addWidget(d->tooltipToggleButton,   1, 3, 1, 1);
        mainLayout->setColumnStretch(2, 10);
    }

    mainLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultChildMargin));
    mainLayout->setContentsMargins(QMargins());
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(clicked(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->renameInput, SIGNAL(signalTextChanged(QString)),
            this, SIGNAL(signalTextChanged(QString)));

    connect(d->renameInput, SIGNAL(signalTokenMarked(bool)),
            this, SLOT(slotTokenMarked(bool)));

    connect(d->renameInput, SIGNAL(signalReturnPressed()),
            this, SIGNAL(signalReturnPressed()));

    slotTokenMarked(false);
    readSettings();
}

void AdvancedRenameWidget::slotTokenMarked(bool marked)
{
    bool enable    = marked && d->parser;
    bool enableMod = enable && !(d->parser->modifiers().isEmpty());

    if (d->layoutStyle == LayoutNormal)
    {
        d->modifiersToolButton->setEnabled(enableMod);
    }
    else
    {
        d->modifiersButton->setEnabled(enableMod);
    }
}

void AdvancedRenameWidget::focusLineEdit()
{
    d->renameInput->slotSetFocus();
}

void AdvancedRenameWidget::highlightLineEdit()
{
    d->renameInput->slotHighlightLineEdit();
}

void AdvancedRenameWidget::highlightLineEdit(const QString& word)
{
    d->renameInput->slotHighlightLineEdit(word);
}

void AdvancedRenameWidget::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    if (d->layoutStyle == LayoutNormal)
    {
        d->optionsLabel->setExpanded(group.readEntry(d->configExpandedStateEntry, d->configExpandedStateDefault));
    }
}

void AdvancedRenameWidget::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    if (d->layoutStyle == LayoutNormal)
    {
        group.writeEntry(d->configExpandedStateEntry, d->optionsLabel
                         ? d->optionsLabel->isExpanded()
                         : d->configExpandedStateDefault);
    }
}

}  // namespace Digikam
