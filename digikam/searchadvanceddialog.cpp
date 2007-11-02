/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-01
 * Description : a dialog to perform advanced search in albums
 * 
 * Copyright (C) 2005 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 
/** @file searchadvanceddialog.cpp */

// Qt includes.

#include <QList>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

// KDE includes.

#include <kvbox.h>
#include <kiconloader.h>
#include <kurl.h>
#include <klocale.h>
#include <klineedit.h>

// Local includes.

#include "ddebug.h"
#include "searchwidgets.h"
#include "searchresultsview.h"
#include "searchadvanceddialog.h"
#include "searchadvanceddialog.moc"

namespace Digikam
{

class SearchAdvancedDialogPriv
{

public:

    SearchAdvancedDialogPriv()
    {
        timer         = 0;
        title         = 0;
        optionsCombo  = 0;
        resultsView   = 0;
        ungroupButton = 0;
        groupButton   = 0;
        delButton     = 0;
        addButton     = 0;
        rulesBox      = 0;
        vlayRulesBox  = 0;
    }

    QGroupBox                  *rulesBox;

    QVBoxLayout                *vlayRulesBox;

    QPushButton                *addButton;
    QPushButton                *delButton;
    QPushButton                *groupButton;
    QPushButton                *ungroupButton;

    QComboBox                  *optionsCombo;

    QList<SearchAdvancedBase*>  baseList;

    QTimer                     *timer;

    KLineEdit                  *title;

    SearchResultsView          *resultsView;
};

SearchAdvancedDialog::SearchAdvancedDialog(QWidget* parent, KUrl& url)
                    : KDialog(parent), m_url(url)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setCaption(i18n("Advanced Search"));
    setHelp("advancedsearchtool.anchor", "digikam");

    d = new SearchAdvancedDialogPriv;
    d->timer = new QTimer(this);

    QWidget *page = new QWidget( this );
    setMainWidget(page);

    restoreDialogSize(KGlobal::config()->group("AdvancedSearch Dialog"));

    // -------------------------------------------------------------
    // two columns, one for the rules, one for the preview.

    QHBoxLayout* hbox     = new QHBoxLayout(page);
    QVBoxLayout* leftSide = new QVBoxLayout();

    d->resultsView = new SearchResultsView(page);
    d->resultsView->setMinimumSize(QSize(200, 200));
    d->resultsView->setWhatsThis(i18n("<p>Here you can review the images found "
                                      "using the current search settings."));

    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->addLayout(leftSide, 10);
    hbox->addWidget(d->resultsView, 5);

    // -------------------------------------------------------------
    // Box for all the rules

    d->rulesBox = new QGroupBox( i18n("Search Rules"), page);
    d->rulesBox->setWhatsThis( i18n("<p>Here you can review the search rules used to filter images "
                                    "searching in album library."));
    d->rulesBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    d->vlayRulesBox = new QVBoxLayout(d->rulesBox);
    d->vlayRulesBox->setMargin(0);
    d->vlayRulesBox->setSpacing(spacingHint());
    d->vlayRulesBox->setAlignment(Qt::AlignTop);

    // -------------------------------------------------------------
    // Box for the add/delete

    QGroupBox *groupbox1 = new QGroupBox(i18n("Add/Delete Option"), page);
    groupbox1->setWhatsThis( i18n("<p>You can edit the search rules "
                                  "by adding/removing criteria."));

    QVBoxLayout *vlay2 = new QVBoxLayout(groupbox1);
                                         
    d->optionsCombo = new QComboBox(groupbox1);
    d->optionsCombo->insertItem(0, i18n("As well as"));
    d->optionsCombo->insertItem(1, i18n("Or"));
    d->optionsCombo->setEnabled(false);

    d->addButton = new QPushButton(i18n("&Add"), groupbox1);
    d->delButton = new QPushButton(i18n("&Del"), groupbox1);
    d->addButton->setIcon(SmallIcon("edit-add"));
    d->delButton->setIcon(SmallIcon("edit-delete"));
    d->addButton->setMaximumHeight( fontMetrics().height()+4 );
    d->delButton->setMaximumHeight( fontMetrics().height()+4 );

    QHBoxLayout* box1 = new QHBoxLayout();
    box1->setMargin(0);
    box1->setSpacing(0);
    box1->addWidget(d->optionsCombo);
    box1->addWidget(d->addButton);
    box1->addStretch(10);
    box1->addWidget(d->delButton);

    vlay2->setMargin(spacingHint());
    vlay2->setSpacing(spacingHint());
    vlay2->addLayout(box1);

    // -------------------------------------------------------------
    // Box for the group/ungroup

    QGroupBox *groupbox2 = new QGroupBox(i18n("Group/Ungroup Options"), page);
    groupbox2->setWhatsThis( i18n("<p>You can group or ungroup any search criteria "
                                  "from the Search Rule set."));

    QVBoxLayout *vlay3 = new QVBoxLayout(groupbox2);
    
    d->groupButton   = new QPushButton(i18n("&Group"), groupbox2);
    d->ungroupButton = new QPushButton(i18n("&Ungroup"), groupbox2);
    d->groupButton->setMaximumHeight( fontMetrics().height()+4 );
    d->ungroupButton->setMaximumHeight( fontMetrics().height()+4 );

    QHBoxLayout *box2 = new QHBoxLayout();
    box2->setMargin(0);
    box2->setSpacing(0);
    box2->addWidget(d->groupButton);
    box2->addStretch(10);
    box2->addWidget(d->ungroupButton);

    vlay3->setMargin(spacingHint());
    vlay3->setSpacing(spacingHint());
    vlay3->addLayout(box2);

    // -------------------------------------------------------------
    // box for saving the search.

    KVBox *groupbox3 = new KVBox(page);
    groupbox3->layout()->setSpacing(KDialog::spacingHint());

    QLabel* label = new QLabel(i18n("&Save search as: "), groupbox3);
    d->title      = new KLineEdit(groupbox3);
    d->title->setWhatsThis( i18n("<p>Enter the name used to save the current search in "
                                 "\"My Searches\" view"));
    label->setBuddy(d->title);

    QHBoxLayout *box3 = new QHBoxLayout();
    box3->setMargin(0);
    box3->setSpacing(0);
    box3->addWidget(label);
    box3->addWidget(d->title);

    groupbox3->layout()->addItem(box3);

    // -------------------------------------------------------------

    leftSide->setMargin(spacingHint());
    leftSide->setSpacing(spacingHint());
    leftSide->addWidget(d->rulesBox);
    leftSide->addStretch(10);
    leftSide->addWidget(groupbox1);
    leftSide->addWidget(groupbox2);
    leftSide->addWidget(groupbox3);

    // -------------------------------------------------------------

    if ( url.isEmpty() )
    {
        d->title->setText(i18n("Last Search"));
        slotAddRule();
    }
    else
    {
        d->title->setText(url.queryItem("name"));
        fillWidgets( url );
    }

    slotChangeButtonStates();
    d->timer->setSingleShot(true);
    d->timer->start(0);

    // -------------------------------------------------------------

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddRule()));

    connect(d->delButton, SIGNAL(clicked()),
            this, SLOT(slotDelRules()));

    connect(d->groupButton, SIGNAL(clicked()),
            this, SLOT(slotGroupRules()));

    connect(d->ungroupButton, SIGNAL(clicked()),
            this, SLOT(slotUnGroupRules()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    connect(d->title, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotChangeButtonStates()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOk()));
}

SearchAdvancedDialog::~SearchAdvancedDialog()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("AdvancedSearch Dialog");
    saveDialogSize(group);

    delete d->timer;
    delete d;
}

void SearchAdvancedDialog::slotAddRule()
{
    SearchAdvancedBase::Option type = SearchAdvancedBase::NONE;
    if ( !d->baseList.isEmpty() )
    {
        if (d->optionsCombo->currentIndex() == 0 )
            type = SearchAdvancedBase::AND;
        else
            type = SearchAdvancedBase::OR;
    }

    SearchAdvancedRule* rule = new SearchAdvancedRule( d->rulesBox, type );
    d->vlayRulesBox->addWidget(rule->widget());
    d->baseList.append(rule);

    connect(rule, SIGNAL(signalBaseItemToggled()),
            this, SLOT(slotChangeButtonStates()));

    connect(rule, SIGNAL(signalPropertyChanged()),
            this, SLOT(slotPropertyChanged()));

    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotDelRules()
{
    if (d->baseList.isEmpty())
        return;

    typedef QList<SearchAdvancedBase*> BaseList;

    BaseList itemsToRemove;

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->isChecked())
        {
            itemsToRemove.append(base);
        }
    }

    for (BaseList::iterator it = itemsToRemove.begin();
         it != itemsToRemove.end(); ++it)
    {
        SearchAdvancedBase* base = (SearchAdvancedBase*) *it;
        d->baseList.removeAll(base);
        delete base;
    }

    BaseList::iterator it = d->baseList.begin();
    if (it != d->baseList.end())
        (*it)->removeOption();

    slotChangeButtonStates();
    slotPropertyChanged();
    if (d->baseList.isEmpty()) 
    {
        d->optionsCombo->setEnabled(false);
        d->addButton->setEnabled(true);
        enableButtonOk(false);
    }
}

void SearchAdvancedDialog::slotGroupRules()
{
    typedef QList<SearchAdvancedBase*> BaseList;

    BaseList itemsToGroup;
    BaseList groupsToUnGroupAndGroup;

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if ( base->isChecked() )
        {
            itemsToGroup.append( base );
            if ( base->type() == SearchAdvancedBase::GROUP)
                groupsToUnGroupAndGroup.append( base );
        }
    }

    // ungroup every found group so it can be regrouped later on.
    for (BaseList::iterator it = groupsToUnGroupAndGroup.begin();
         it != groupsToUnGroupAndGroup.end(); ++it)
    {
        SearchAdvancedGroup* group = (SearchAdvancedGroup*)*it;
        int itemsToGroupPos        = itemsToGroup.indexOf(group);
        int itPos                  = d->baseList.indexOf(group);

        QList<SearchAdvancedRule*> childRules = group->childRules();
        for (QList<SearchAdvancedRule*>::iterator iter = childRules.begin();
             iter != childRules.end(); ++iter)
        {
            d->baseList.insert(itPos, *iter);
            itemsToGroup.insert(itemsToGroupPos, *iter);
        }
        group->removeRules();
        d->baseList.removeAll(group);
        itemsToGroup.removeAll(group);
        delete group;
    }

    // if there is only one or no item return
    if (itemsToGroup.size() < 2)
        return;

    BaseList::iterator it    = itemsToGroup.begin();
    SearchAdvancedRule* rule = (SearchAdvancedRule*)(*it);

    SearchAdvancedGroup* group = new SearchAdvancedGroup(d->rulesBox);
    int itPos = d->baseList.indexOf(rule);
    d->baseList.insert(itPos, group);

    for (BaseList::iterator it = itemsToGroup.begin();
         it != itemsToGroup.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::RULE)
        {
            SearchAdvancedRule* rule = (SearchAdvancedRule*)base;
            group->addRule(rule);
            d->baseList.removeAll(rule);
        }
    }

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        d->vlayRulesBox->removeWidget((*it)->widget());
        d->vlayRulesBox->addWidget((*it)->widget());
    }

    connect(group, SIGNAL(signalBaseItemToggled()),
            this, SLOT(slotChangeButtonStates()));

    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotUnGroupRules()
{
    typedef QList<SearchAdvancedBase*>  BaseList;
    typedef QList<SearchAdvancedGroup*> GroupList;

    GroupList itemsToUnGroup;

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::GROUP &&
            base->isChecked())
        {
            itemsToUnGroup.append((SearchAdvancedGroup*)base);
        }
    }

    if (itemsToUnGroup.isEmpty())
        return;

    for (GroupList::iterator it = itemsToUnGroup.begin();
         it != itemsToUnGroup.end(); ++it)
    {
        SearchAdvancedGroup *group = *it;
        QList<SearchAdvancedRule*> childRules = group->childRules();

        int itPos = d->baseList.indexOf(group);

        for (QList<SearchAdvancedRule*>::iterator iter = childRules.begin();
             iter != childRules.end(); ++iter)
        {
            d->baseList.insert(itPos, *iter);
        }

        group->removeRules();
        d->baseList.removeAll(group);
        delete group;
    }

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        d->vlayRulesBox->removeWidget((*it)->widget());
        d->vlayRulesBox->addWidget((*it)->widget());
    }

    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotPropertyChanged()
{
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

void SearchAdvancedDialog::slotOk()
{
    // calculate the latest url and name.
    slotTimeOut();

    // Since it's not possible to check the state of the ok button,
    // check the state of the add button.
    if ( d->addButton->isEnabled() )
        KDialog::close();
}

void SearchAdvancedDialog::slotTimeOut()
{
    if (d->baseList.isEmpty())
        return;

    typedef QList<SearchAdvancedBase*>  BaseList;

    QString grouping;
    int     count    = 0;
    bool    emptyVal = false;

    KUrl url;
    url.setProtocol("digikamsearch");

    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::RULE)
        {
            SearchAdvancedRule* rule = (SearchAdvancedRule*)base;
            QString val = rule->urlValue();
            if ( !val.isEmpty() )
            {
                if (rule->option() != SearchAdvancedBase::NONE && !count == 0 )
                    grouping += (rule->option() == SearchAdvancedBase::AND) ? " AND " : " OR ";

                grouping += QString::number(++count);
                url.addQueryItem( QString::number(count) + ".key", rule->urlKey());
                url.addQueryItem( QString::number(count) + ".op", rule->urlOperator());
                url.addQueryItem( QString::number(count) + ".val", val);
            }
            else
                emptyVal = true;
        }
        else
        {
            SearchAdvancedGroup* group = (SearchAdvancedGroup*)base;

            QString tempGrouping;
            int curCount = count;

            QList<SearchAdvancedRule*> childRules = group->childRules();
            for (QList<SearchAdvancedRule*>::iterator iter = childRules.begin();
                 iter != childRules.end(); ++iter)
            {
                SearchAdvancedRule* rule = (SearchAdvancedRule*)(*iter);
                QString val = rule->urlValue();
                if ( !val.isEmpty() )
                {
                    if (rule->option() != SearchAdvancedBase::NONE && !count == 0 )
                        tempGrouping += (rule->option() == SearchAdvancedBase::AND) ? " AND " : " OR ";

                    tempGrouping += QString::number(++count);
                    url.addQueryItem( QString::number(count) + ".key", rule->urlKey());
                    url.addQueryItem( QString::number(count) + ".op", rule->urlOperator());
                    url.addQueryItem( QString::number(count) + ".val", val);
                }
                else
                    emptyVal = true;
            }

            if (!tempGrouping.isEmpty())
            {
                if (group->option() != SearchAdvancedBase::NONE && !curCount == 0 )
                    grouping += (group->option() == SearchAdvancedBase::AND) ? " AND " : " OR ";

                grouping += " ( " + tempGrouping + " ) ";
            }
        }
    }

    url.setPath(grouping);
    url.addQueryItem("name", d->title->text());
    url.addQueryItem("count", QString::number(count));
    m_url = url;

    if (!count == 0)
        d->resultsView->openURL( url );

    DDebug() << url << endl;

    if (!d->baseList.isEmpty())
    {
        if (!d->title->text().isEmpty())
            enableButtonOk( !emptyVal );

        d->addButton->setEnabled( !emptyVal );
        d->optionsCombo->setEnabled( !emptyVal );
    }
}

void SearchAdvancedDialog::slotChangeButtonStates()
{
    bool group  = false;
    int counter = 0;

    typedef QList<SearchAdvancedBase*>  BaseList;
    for (BaseList::iterator it = d->baseList.begin();
         it != d->baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->isChecked())
        {
            ++counter;
            if (base->type() == SearchAdvancedBase::GROUP)
                group = true;
        }
    }

    d->ungroupButton->setEnabled( group );

    if ( counter == 0)
    {
        d->delButton->setEnabled(false);
        d->groupButton->setEnabled(false);
    } 
    else if ( counter == 1)
    {
        if (d->baseList.count() > 1)
            d->delButton->setEnabled(true);
        d->groupButton->setEnabled(false);
    }
    else if ( counter > 1 )
    {
        d->delButton->setEnabled(true);
        d->groupButton->setEnabled(true);
    }

    enableButtonOk( !d->title->text().isEmpty() );
}

void SearchAdvancedDialog::fillWidgets( const KUrl& url )
{
    int  count = url.queryItem("count").toInt();
    if (count <= 0)
        return;

    QMap<int, KUrl> rulesMap;

    for (int i=1; i<=count; i++)
    {
        KUrl newRule;

        QString key = url.queryItem(QString::number(i) + ".key");
        QString op  = url.queryItem(QString::number(i) + ".op");
        QString val = url.queryItem(QString::number(i) + ".val");

        newRule.setPath("1");
        newRule.addQueryItem("1.key",key);
        newRule.addQueryItem("1.op",op);
        newRule.addQueryItem("1.val",val);

        rulesMap.insert(i, newRule);
    }

    QStringList strList = url.path().split(" ", QString::SkipEmptyParts);

    SearchAdvancedGroup* group      = 0;
    bool groupingActive             = false;
    SearchAdvancedBase::Option type = SearchAdvancedBase::NONE;

    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
    {
        bool ok;
        int  num = (*it).toInt(&ok);
        if (ok)
        {
            SearchAdvancedRule* rule = new SearchAdvancedRule( d->rulesBox, type );
            rule->setValues( rulesMap[num] );

            connect(rule, SIGNAL(signalBaseItemToggled()),
                    this, SLOT(slotChangeButtonStates()));

            connect(rule, SIGNAL(signalPropertyChanged()),
                    this, SLOT(slotPropertyChanged()));

            if (groupingActive)
                group->addRule(rule);
            else
                d->baseList.append(rule);
        }
        else if (*it == "OR")
        {
            type = SearchAdvancedRule::OR;
        }
        else if (*it == "AND")
        {
            type = SearchAdvancedRule::AND;
        }
        else if (*it == "(")
        {
            group = new SearchAdvancedGroup(d->rulesBox);

            connect(group, SIGNAL(signalBaseItemToggled()),
                    this, SLOT(slotChangeButtonStates()));

            groupingActive = true;
        }
        else if (*it == ")")
        {
            groupingActive = false;
        }
        else
        {
            DDebug() << "IGNORED:" << *it << endl;
        }
    }

    enableButtonOk(true);
}

}  // namespace Digikam
