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

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kdateedit.h>

#include "searchwidgets.h"

static struct
{
    QString keyText;
    QString key;
    bool    precise;
}
RuleKeyTable[] =
{
    { "Album",            "album",           true  },
    { "Album Name",       "albumname",       false },
    { "Album Caption",    "albumcaption",    false },
    { "Album Collection", "albumcollection", false },
    { "Tag",              "tag",             true  },
    { "Tag Name",         "tagname",         false },
    { "Image Name",       "imagename",       false },
    { "Image Date",       "imagedate",       true  },
    { "Image Caption",    "imagecaption",    false },
    { "Keyword",          "keyword",         false },
};
static const int RuleKeyTableCount = 10;

/* unused atm 
static struct
{
    QString keyText;
    QString key;
}

RuleOpTable[] =
{
    { "contains",           "LIKE" },
    { "does not contain",   "NLIKE"},
    { "equals",             "EQ"},
    { "does not equal",     "NE"},
    { ">",                  "GT"},
    { "<",                  "LT"},
    { "=",                  "EQ"},
};
static const int RuleOpTableCount = 7;
*/


SearchAdvancedRule::SearchAdvancedRule(QWidget* parent,
                                       SearchAdvancedRule::Option option)
    : SearchAdvancedBase(SearchAdvancedBase::RULE)
{
    m_box = new QVBox(parent);
    m_box->layout()->setSpacing(5);

    m_optionsBox   = 0;
    m_option       = option;
    if (option != NONE)
    {
        m_optionsBox  = new QHBox(m_box);
        QLabel* label = new QLabel( i18n(option == AND ? "As well as" : "Or"),
                                    m_optionsBox);
        QFrame* hline = new QFrame(m_optionsBox);
        hline->setFrameStyle(QFrame::HLine|QFrame::Sunken);

        label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        hline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    m_hbox = new QHBox(m_box);

    m_key = new QComboBox( m_hbox, "key" );
    for (int i=0; i< RuleKeyTableCount; i++)
        m_key->insertItem( RuleKeyTable[i].keyText, i );
    m_operator = new QComboBox( m_hbox );
    slotKeyChanged( 0 );


    m_valueBox = new QHBox(m_hbox);
    m_value = new QLineEdit( m_valueBox, "lineedit" );
    m_valueWidget = LINEEDIT;

    m_check = new QCheckBox(m_hbox);
    m_check->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_box->show();

    connect( m_key, SIGNAL( activated(int) ), this, SLOT(slotKeyChanged(int)));
    connect( m_key, SIGNAL( activated(int) ), this, SLOT(slotRuleChanged()));
    connect( m_operator, SIGNAL( activated(int) ), this, SLOT(slotRuleChanged()));
    connect( m_check, SIGNAL( toggled( bool ) ), this, SLOT(slotToggle()));
    connect( m_value, SIGNAL ( textChanged(const QString&) ),
             this, SLOT(slotRuleChanged()));
}

SearchAdvancedRule::~SearchAdvancedRule()
{
    delete m_box;
}

void SearchAdvancedRule::slotKeyChanged(int id)
{
    QString currentOperator = m_operator->currentText();
    m_operator->clear();

    if (RuleKeyTable[id].precise)
    {
        if (RuleKeyTable[id].key == "imagedate")
        {
            m_operator->insertItem("<");
            m_operator->insertItem("=");
            m_operator->insertItem(">");
            if ( currentOperator == "<" || currentOperator == "=" ||
                 currentOperator == ">" )
                m_operator->setCurrentText( currentOperator );
            delete m_value;
            
            m_dateEdit = new KDateEdit(m_valueBox,"datepicker");
            m_valueWidget=DATE;
            m_dateEdit->show();

            connect( m_dateEdit,
                     SIGNAL( dateChanged(const QDate& ) ),
                     this, SLOT(slotRuleChanged()));
        }
        else
        {
            m_operator->insertItem(i18n("equals"));
            m_operator->insertItem(i18n("does not equal"));
            if ( currentOperator == i18n("equals") ||
                 currentOperator == i18n("does not equal"))
                m_operator->setCurrentText( currentOperator );
        }
    }
    else
    {
        m_operator->insertItem(i18n("contains"));
        m_operator->insertItem(i18n("does not contain"));
        m_operator->insertItem(i18n("equals"));
        m_operator->insertItem(i18n("does not equal"));
        if ( currentOperator == i18n("contains") ||
             currentOperator == i18n("does not contain") ||
             currentOperator == i18n("equals") ||
             currentOperator == i18n("does not equal"))
            m_operator->setCurrentText( currentOperator );
     }
}

QString SearchAdvancedRule::urlKey() const
{
    return RuleKeyTable[m_key->currentItem()].key;
}

QString SearchAdvancedRule::urlOperator() const
{
    QString string;
    if ( m_operator->currentText() == i18n("contains"))
        return("LIKE");
    if ( m_operator->currentText() == i18n("does not contain"))
        return("NLIKE");
    if ( m_operator->currentText() == i18n("equals"))
        return("EQ");
    if ( m_operator->currentText() == i18n("does not equal"))
        return("NE");
    if ( m_operator->currentText() == i18n(">"))
        return("GT");
    if ( m_operator->currentText() == i18n("<"))
        return("LT");
    if ( m_operator->currentText() == i18n("="))
        return("EQ");

    return string;
}

QString SearchAdvancedRule::urlValue() const
{
    QString string;
    if (m_valueWidget == LINEEDIT)
        string.append( m_value->text() );
    else if (m_valueWidget == DATE)
        string.append( m_dateEdit->date().toString(Qt::ISODate) );
    return string;
}

QWidget* SearchAdvancedRule::widget() const
{
    return m_box;
}

bool SearchAdvancedRule::isChecked() const
{
    return (m_check && m_check->isChecked());    
}

void SearchAdvancedRule::addOption(Option option)
{
    if (option == NONE)
    {
        removeOption();
        return;
    }
    
    m_box->layout()->remove(m_hbox);
    
    m_optionsBox = new QHBox(m_box);
    new QLabel(i18n( option == AND ? "As well as" : "Or" ),
               m_optionsBox);
    QFrame* hline = new QFrame(m_optionsBox);
    hline->setFrameStyle(QFrame::HLine|QFrame::Sunken);
    hline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_optionsBox->show();

    m_box->layout()->add(m_hbox);

    m_option =  option;
}

void SearchAdvancedRule::removeOption()
{
    m_option = NONE;
    delete m_optionsBox;
    m_optionsBox = 0;
}

void SearchAdvancedRule::addCheck()
{
    m_check = new QCheckBox(m_hbox);
    m_check->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_check->show();

    connect( m_check, SIGNAL( toggled( bool ) ), this, SLOT(slotToggle()));
}

void SearchAdvancedRule::removeCheck()
{
    delete m_check;
    m_check = 0;
}

SearchAdvancedGroup::SearchAdvancedGroup(QWidget* parent)
    : SearchAdvancedBase(SearchAdvancedBase::GROUP)
{
    m_box      = new QHBox(parent);
    m_box->layout()->setSpacing(5);
    m_groupbox = new QVGroupBox(m_box);
    m_check    = new QCheckBox(m_box);
    m_option   = SearchAdvancedRule::NONE;
    m_box->show();

    connect( m_check, SIGNAL( toggled( bool ) ), this, SLOT(slotToggle()));
}

SearchAdvancedGroup::~SearchAdvancedGroup()
{
    delete m_box;
}

QWidget* SearchAdvancedGroup::widget() const
{
    return m_box;    
}

bool SearchAdvancedGroup::isChecked() const
{
    return m_check->isChecked();
}

void SearchAdvancedGroup::addRule(SearchAdvancedRule* rule)
{
    if (m_childRules.isEmpty() &&
        rule->option() != SearchAdvancedRule::NONE)
    {
        // this is the first rule being inserted in this group.
        // get its option and remove ite option
        addOption(rule->option());
        rule->removeOption();
    }

    rule->removeCheck();
    
    m_childRules.append(rule);
    rule->widget()->reparent(m_groupbox, QPoint(0,0));
    rule->widget()->show();
}

void SearchAdvancedGroup::removeRules()
{
    typedef QValueList<SearchAdvancedRule*> RuleList;

    for (RuleList::iterator it = m_childRules.begin();
         it != m_childRules.end(); ++it)
    {
        SearchAdvancedRule* rule = (SearchAdvancedRule*)(*it);
        if (it == m_childRules.begin())
        {
            rule->addOption(m_option);
        }
        rule->addCheck();
        
        rule->widget()->reparent((QWidget*)m_box->parent(), QPoint(0,0));
        rule->widget()->show();
    }

    m_childRules.clear();
    removeOption();
}

QValueList<SearchAdvancedRule*> SearchAdvancedGroup::childRules() const
{
    return m_childRules;
}

void SearchAdvancedGroup::addOption(Option option)
{
    m_option = option;
    m_groupbox->setTitle(
            i18n(m_option == SearchAdvancedRule::AND ? "As well as" : "Or"));
}

void SearchAdvancedGroup::removeOption()
{
    m_option = NONE;
    m_groupbox->setTitle("");
}

#include "searchwidgets.moc"
