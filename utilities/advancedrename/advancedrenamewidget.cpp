/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the AdvancedRename utility
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "advancedrenamewidget.moc"

// Qt includes

#include <QAction>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>
#include <QRegExp>
#include <QToolButton>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>

// LibKDcraw includes

#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "advancedrenameinput.h"
#include "defaultrenameparser.h"
#include "dynamiclayout.h"
#include "tooltipcreator.h"

using namespace KDcrawIface;

namespace Digikam
{

class AdvancedRenameWidgetPriv
{
    typedef AdvancedRenameWidget::ControlWidgets CWMask;

public:

    AdvancedRenameWidgetPriv() :
        configGroupName("AdvancedRename Widget"),
        configExpandedStateEntry("Options are expanded"),
        configExpandedStateDefault(true),

        tooltipToggleButton(0),
        modifierToolButton(0),
        btnContainer(0),
        tooltipDialog(0),
        renameInput(0),
        parser(0),
        optionsLabel(0),
        controlWidgetsMask(AdvancedRenameWidget::DefaultControls)
    {}

    const QString        configGroupName;
    const QString        configExpandedStateEntry;
    bool                 configExpandedStateDefault;

    QToolButton*         tooltipToggleButton;
    QToolButton*         modifierToolButton;

    QWidget*             btnContainer;

    KDialog*             tooltipDialog;
    AdvancedRenameInput* renameInput;
    Parser*              parser;
    RLabelExpander*      optionsLabel;

    CWMask               controlWidgetsMask;
};

AdvancedRenameWidget::AdvancedRenameWidget(QWidget* parent)
                 : QWidget(parent), d(new AdvancedRenameWidgetPriv)
{
    setupWidgets();
    setParser(new DefaultRenameParser());
}

AdvancedRenameWidget::~AdvancedRenameWidget()
{
    writeSettings();

    delete d->parser;
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
    QTextEdit *te = dynamic_cast<QTextEdit*>(d->tooltipDialog->mainWidget());
    if (te)
    {
        te->clear();

        if (d->parser)
        {
            te->setHtml(TooltipCreator::getInstance().tooltip(d->parser));
        }

        te->setReadOnly(true);
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

void AdvancedRenameWidget::setControlWidgets(ControlWidgets mask)
{
    // we need a parser and at least one renaming option to successfully use
    // this widget.
    bool enable       = d->parser && !(d->parser->options().isEmpty());
    // enable the modifier toolbutton if environment has been set up correctly
    bool enableModBtn = enable && !(d->parser->modifiers().isEmpty());

    d->renameInput->setEnabled(enable);
    d->optionsLabel->setVisible(enable && (mask & TokenButtons));
    d->modifierToolButton->setVisible(enableModBtn && (mask & ModifierToolButton));
    d->tooltipToggleButton->setVisible(enable && (mask & ToolTipButton));

    d->controlWidgetsMask = mask;
}

void AdvancedRenameWidget::registerParserControls()
{
   if (d->parser)
   {
       setupWidgets();

       QMenu* modifierToolBtnMenu = new QMenu(d->modifierToolButton);
       QPushButton* btn           = 0;
       QAction* action            = 0;
       DynamicLayout* layout      = new DynamicLayout(KDialog::marginHint(), KDialog::marginHint());

       foreach (Option* option, d->parser->options())
       {
           btn = option->registerButton(this);

           if (!btn)
           {
               continue;
           }

           // set button tooltip
           btn->setToolTip(option->description());

           layout->addWidget(btn);

           connect(option, SIGNAL(signalTokenTriggered(const QString&)),
                   d->renameInput, SLOT(slotAddToken(const QString&)));
       }

       // --------------------------------------------------------

       // register modifiers
       foreach (Modifier* modifier, d->parser->modifiers())
       {
           action = modifier->registerMenu(modifierToolBtnMenu);
           if (!action)
           {
               continue;
           }

           connect(modifier, SIGNAL(signalTokenTriggered(const QString&)),
                   d->renameInput, SLOT(slotAddToken(const QString&)));
       }

       // --------------------------------------------------------

       d->btnContainer->setLayout(layout);
       setMinimumWidth(d->btnContainer->layout()->sizeHint().width());

       d->modifierToolButton->setMenu(modifierToolBtnMenu);

       d->renameInput->setParser(d->parser);
       createToolTip();
   }
}

Parser* AdvancedRenameWidget::parser()
{
    return d->parser;
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
    d->tooltipDialog = new KDialog(this);
    d->tooltipDialog->setCaption(i18n("Information"));
    d->tooltipDialog->setButtons(KDialog::Close);
    d->tooltipDialog->resize(650, 530);
    d->tooltipDialog->setMainWidget(new QTextEdit());

    delete d->renameInput;
    d->renameInput = new AdvancedRenameInput;
    d->renameInput->setToolTip(i18n("<p>Enter your renaming pattern here. Use the access buttons to quickly add renaming "
                                    "options and modifiers. For further explanations, use the information toolbutton.</p>"));

    // --------------------------------------------------------

    delete d->tooltipToggleButton;
    d->tooltipToggleButton = new QToolButton;
    d->tooltipToggleButton->setIcon(SmallIcon("dialog-information"));
    d->tooltipToggleButton->setToolTip(i18n("Show a list of all available options"));

    // --------------------------------------------------------

    delete d->btnContainer;
    d->btnContainer = new QWidget(this);

    delete d->optionsLabel;
    d->optionsLabel = new RLabelExpander(this);
    d->optionsLabel->setText(i18n("Renaming Options"));
    d->optionsLabel->setWidget(d->btnContainer);
    d->optionsLabel->setLineVisible(false);

    // --------------------------------------------------------

    delete d->modifierToolButton;
    d->modifierToolButton = new QToolButton;
    d->modifierToolButton->setPopupMode(QToolButton::InstantPopup);
    d->modifierToolButton->setIcon(SmallIcon("document-edit"));
    d->modifierToolButton->setToolTip(i18n("<p>Add a modifier to a renaming option. "
                                           "To activate this button, place the cursor behind a renaming option "
                                           "or an already assigned modifier.</p>"));

    // --------------------------------------------------------

    delete layout();
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->renameInput,         0, 0, 1, 1);
    mainLayout->addWidget(d->modifierToolButton,  0, 1, 1, 1);
    mainLayout->addWidget(d->tooltipToggleButton, 0, 2, 1, 1);
    mainLayout->addWidget(d->optionsLabel,        1, 0, 1,-1);
    mainLayout->setColumnStretch(0, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::marginHint());
    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->tooltipToggleButton, SIGNAL(clicked(bool)),
            this, SLOT(slotToolTipButtonToggled(bool)));

    connect(d->renameInput, SIGNAL(signalTextChanged(const QString&)),
            this, SIGNAL(signalTextChanged(const QString&)));

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
    d->modifierToolButton->setEnabled(enableMod);
}

void AdvancedRenameWidget::focusLineEdit()
{
    d->renameInput->slotSetFocus();
}

void AdvancedRenameWidget::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->optionsLabel->setExpanded(group.readEntry(d->configExpandedStateEntry, d->configExpandedStateDefault));
}

void AdvancedRenameWidget::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    // remove duplicate entries and save pattern history, omit empty strings
    QString pattern = d->renameInput->text();
    group.writeEntry(d->configExpandedStateEntry, d->optionsLabel
                                                  ? d->optionsLabel->isExpanded()
                                                  : d->configExpandedStateDefault);
}

}  // namespace Digikam
