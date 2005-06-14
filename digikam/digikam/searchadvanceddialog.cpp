////////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2005 Tom Albers <tomalbers@kde.nl>
//    Copyright (C) 2005 Renchi Raju <renchi@pooh.tam.uiuc.edu>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////////////

/** @file searchadvanceddialog.cpp */

#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qhgroupbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtimer.h>

#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>

#include "searchwidgets.h"
#include "searchadvanceddialog.h"
#include "searchresultsview.h"

SearchAdvancedDialog::SearchAdvancedDialog(QWidget* parent, KURL& url)
    : KDialogBase(parent, 0, true, i18n("Advanced Search"),
                  Ok|Cancel), m_url(url)
{

    QHBox* hbox = new QHBox(this);
    hbox->setSpacing(spacingHint());

    QVBox* vbox = new QVBox(hbox);
    vbox->setSpacing(spacingHint());
    
    QVBoxLayout* lay = new QVBoxLayout(vbox);
    lay->setSpacing(5);
    lay->setMargin(5);

    m_box = new QVBox(vbox);
    m_box->setFrameStyle(QFrame::Panel|QFrame::Raised);
    m_box->layout()->setSpacing(5);
    m_box->layout()->setMargin(5);
    m_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_box->layout()->setAlignment(Qt::AlignTop);
    lay->addWidget(m_box);

    QHGroupBox* box = 0;
    box = new QHGroupBox(vbox);
    box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_optionsCombo = new QComboBox( box );
    m_optionsCombo->insertItem(i18n("As Well As"));
    m_optionsCombo->insertItem(i18n("Or"));
    m_optionsCombo->setEnabled(false);
    m_addButton = new QPushButton(i18n("&Add"), box);
    m_delButton = new QPushButton(i18n("&Del"), box);
    lay->addWidget(box);

    box = new QHGroupBox(vbox);
    box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_groupButton   = new QPushButton(i18n("&Group"), box);
    m_ungroupButton = new QPushButton(i18n("&Ungroup"), box);
    lay->addWidget(box);

    box = new QHGroupBox(vbox);
    QLabel* label = new QLabel(i18n("Save Search As"), box);
    m_title = new QLineEdit(box,"searchTitle");
    label->setBuddy(m_title);
    lay->addWidget(box);

    m_resultsView = new SearchResultsView(hbox);
    m_timer = new QTimer(this);

    if ( url.isEmpty() )
    {
        m_title->setText(i18n("Last Search"));
        slotAddRule();
    }
    else
    {
        m_title->setText(url.queryItem("name"));
        fillWidgets( url );
    }

    setMainWidget(hbox);
    slotChangeButtonStates();
    m_timer->start(0, true);

    connect(m_addButton, SIGNAL(clicked()),
            SLOT(slotAddRule()));
    connect(m_delButton, SIGNAL(clicked()),
            SLOT(slotDelRules()));
    connect(m_groupButton, SIGNAL(clicked()),
            SLOT(slotGroupRules()));
    connect(m_ungroupButton, SIGNAL(clicked()),
            SLOT(slotUnGroupRules()));
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
    connect(m_title, SIGNAL ( textChanged(const QString&) ),
            SLOT(slotChangeButtonStates() ));
}

SearchAdvancedDialog::~SearchAdvancedDialog()
{
    delete m_timer;
}

void SearchAdvancedDialog::slotAddRule()
{
    SearchAdvancedRule* rule = new SearchAdvancedRule(
        m_box, m_baseList.isEmpty() ?
            SearchAdvancedRule::NONE :
            m_optionsCombo->currentText() == i18n("As Well As") ?
            SearchAdvancedRule::AND : SearchAdvancedRule::OR);
    m_baseList.append(rule);

    connect( rule, SIGNAL( signalBaseItemToggled() ) ,
             SLOT( slotChangeButtonStates() ) );
    connect( rule, SIGNAL( signalPropertyChanged() ),
             SLOT(slotPropertyChanged()));

    m_addButton->setEnabled(false);
    m_optionsCombo->setEnabled(false);
    if (!m_title->text().isEmpty())
        enableButtonOK( true );
    adjustSize();
}

void SearchAdvancedDialog::slotDelRules()
{
    if (m_baseList.isEmpty())
        return;

    typedef QValueList<SearchAdvancedBase*> BaseList;

    BaseList itemsToRemove;

    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
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
        m_baseList.remove(base);
        delete base;
    }

    BaseList::iterator it = m_baseList.begin();
    if (it != m_baseList.end())
        (*it)->removeOption();

    adjustSize();
    slotChangeButtonStates();
    slotPropertyChanged();
    if (m_baseList.isEmpty()) {
        m_optionsCombo->setEnabled(false);
        m_addButton->setEnabled(true);
        enableButtonOK( false );
    }
    adjustSize();
}

void SearchAdvancedDialog::slotGroupRules()
{
    typedef QValueList<SearchAdvancedBase*> BaseList;

    BaseList itemsToGroup;
    BaseList groupsToUnGroupAndGroup;

    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
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
        BaseList::iterator itemsToGroupPos = itemsToGroup.find(group);
        BaseList::iterator itPos = m_baseList.find(group);
        QValueList<SearchAdvancedRule*> childRules = group->childRules();
        for (QValueList<SearchAdvancedRule*>::iterator iter = childRules.begin();
                 iter != childRules.end(); ++iter)
        {
            m_baseList.insert(itPos, *iter);
            itemsToGroup.insert(itemsToGroupPos, *iter);
        }
        group->removeRules();
        m_baseList.remove(group);
        itemsToGroup.remove(group);
        delete group;
    }

    // if there is only one or no item return
    if (itemsToGroup.size() < 2)
        return;

    BaseList::iterator it = itemsToGroup.begin();
    SearchAdvancedRule* rule = (SearchAdvancedRule*)(*it);

    SearchAdvancedGroup* group = new SearchAdvancedGroup(m_box);
    BaseList::iterator itPos = m_baseList.find(rule);
    m_baseList.insert(itPos, group);    

    for (BaseList::iterator it = itemsToGroup.begin();
         it != itemsToGroup.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::RULE)
        {
            SearchAdvancedRule* rule = (SearchAdvancedRule*)base;
            group->addRule(rule);
            m_baseList.remove(rule);
        } 
    }

    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
    {
        m_box->layout()->remove((*it)->widget());
        m_box->layout()->add((*it)->widget());
    }

    connect( group, SIGNAL( signalBaseItemToggled() ) ,
             this, SLOT( slotChangeButtonStates() ) );
 
    adjustSize();
    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotUnGroupRules()
{
    typedef QValueList<SearchAdvancedBase*>  BaseList;
    typedef QValueList<SearchAdvancedGroup*> GroupList;

    GroupList itemsToUnGroup;

    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
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
        QValueList<SearchAdvancedRule*> childRules = group->childRules();

        BaseList::iterator itPos = m_baseList.find(group);

        for (QValueList<SearchAdvancedRule*>::iterator iter = childRules.begin();
             iter != childRules.end(); ++iter)
        {
            m_baseList.insert(itPos, *iter);
        }

        group->removeRules();
        m_baseList.remove(group);
        delete group;
    }


    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
    { 
        m_box->layout()->remove((*it)->widget());
        m_box->layout()->add((*it)->widget());
    }

    adjustSize();
    slotChangeButtonStates();
    slotPropertyChanged();
}

void SearchAdvancedDialog::slotPropertyChanged()
{
    m_timer->start(500, true);
}

void SearchAdvancedDialog::slotOk()
{
    // calculate the latest url and name.
    slotTimeOut();

    // Since it's not possible to check the state of the ok button,
    // check the state of the add button.
    if ( m_addButton->isEnabled() )
        KDialogBase::slotOk();
}

void SearchAdvancedDialog::slotTimeOut()
{
    if (m_baseList.isEmpty())
        return;

    typedef QValueList<SearchAdvancedBase*>  BaseList;

    QString grouping;
    int     count  = 0;
    bool    emptyVal = false;

    KURL url;
    url.setProtocol("digikamsearch");

    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->type() == SearchAdvancedBase::RULE)
        {
            SearchAdvancedRule* rule = (SearchAdvancedRule*)base;
            QString val = rule->urlValue();
            if ( !val.isEmpty() )
            {
                if (rule->option() != SearchAdvancedBase::NONE &&
                    !count == 0 )
                    grouping += (rule->option() == SearchAdvancedBase::AND) ?
                        " AND " : " OR ";
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

            QValueList<SearchAdvancedRule*> childRules = group->childRules();
            for (QValueList<SearchAdvancedRule*>::iterator iter =
                    childRules.begin();
                 iter != childRules.end(); ++iter)
            {
                SearchAdvancedRule* rule = (SearchAdvancedRule*)(*iter);
                QString val = rule->urlValue();
                if ( !val.isEmpty() )
                {
                    if (rule->option() != SearchAdvancedBase::NONE &&
                    !count == 0 )
                        tempGrouping += (rule->option() == SearchAdvancedBase::AND) ?
                            " AND " : " OR ";
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
                if (group->option() != SearchAdvancedBase::NONE &&
                    !curCount == 0 )
                    grouping += (group->option() == SearchAdvancedBase::AND) ?
                            " AND " : " OR ";
                    grouping += " ( " + tempGrouping + " ) ";
            }
        }
    }

    url.setPath(grouping);
    url.addQueryItem("name", m_title->text());
    url.addQueryItem("count", QString::number(count));
    m_url = url;
    if (!count == 0) 
        m_resultsView->openURL( url );
    kdDebug() << url << endl;

    if (!m_baseList.isEmpty())
    {
        if (!m_title->text().isEmpty())
            enableButtonOK( !emptyVal );
        m_addButton->setEnabled( !emptyVal );
        m_optionsCombo->setEnabled( !emptyVal );
    }
}


void SearchAdvancedDialog::slotChangeButtonStates()
{
    bool group = false;
    int counter = 0;

    typedef QValueList<SearchAdvancedBase*>  BaseList;
    for (BaseList::iterator it = m_baseList.begin();
         it != m_baseList.end(); ++it)
    {
        SearchAdvancedBase* base = *it;
        if (base->isChecked())
        {
            ++counter;
            if (base->type() == SearchAdvancedBase::GROUP)
                group = true;
        }
    }

    m_ungroupButton->setEnabled( group );

    if ( counter == 0)
    {
        m_delButton->setEnabled(false);
        m_groupButton->setEnabled(false);
    } else if ( counter == 1)
    {
        m_delButton->setEnabled(true);
        m_groupButton->setEnabled(false);
    } else if ( counter > 1 )
    {
        m_delButton->setEnabled(true);
        m_groupButton->setEnabled(true);
    }

    enableButtonOK( !m_title->text().isEmpty() );
}

void SearchAdvancedDialog::fillWidgets( const KURL& url )
{
    int  count = url.queryItem("count").toInt();
    if (count <= 0)
        return;

    QMap<int, KURL> rulesMap;

    for (int i=1; i<=count; i++)
    {
        KURL newRule;

        QString key = url.queryItem(QString::number(i) + ".key");
        QString op  = url.queryItem(QString::number(i) + ".op");
        QString val  = url.queryItem(QString::number(i) + ".val");

        newRule.setPath("1");
        newRule.addQueryItem("1.key",key);
        newRule.addQueryItem("1.op",op);
        newRule.addQueryItem("1.val",val);
        
        rulesMap.insert(i, newRule);
    }

    QStringList strList = QStringList::split(" ", url.path());

    SearchAdvancedGroup* group;
    bool groupingActive=false;
    SearchAdvancedBase::Option type = type=SearchAdvancedRule::NONE;

    for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
    {
        bool ok;
        int  num = (*it).toInt(&ok);
        if (ok)
        {
            SearchAdvancedRule* rule = new SearchAdvancedRule( m_box, type );
            rule->setValues( rulesMap[num] );

            connect( rule, SIGNAL( signalBaseItemToggled() ) ,
                     SLOT( slotChangeButtonStates() ) );
            connect( rule, SIGNAL( signalPropertyChanged() ),
                     SLOT(slotPropertyChanged()));

            if (groupingActive)
                group->addRule(rule);
            else
                m_baseList.append(rule);
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
            group = new SearchAdvancedGroup(m_box);
            m_baseList.append(group);
            connect( group, SIGNAL( signalBaseItemToggled() ) ,
                     this, SLOT( slotChangeButtonStates() ) );
            groupingActive = true;
        }
        else if (*it == ")")
        {
            groupingActive = false;
        }
        else
        {
            kdDebug() << "IGNORED:" << *it << endl;
        }
    }

    enableButtonOK( true );
    adjustSize();
}

#include "searchadvanceddialog.moc"
