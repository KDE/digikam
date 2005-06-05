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

#ifndef SEARCHWIDGETS_H
#define SEARCHWIDGETS_H

#include "qobject.h"
#include "qwidget.h"

class QHBox;
class QVBox;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QVGroupBox;
class QLabel;
class KDateEdit;

class SearchAdvancedBase : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        RULE = 0,
        GROUP
    };

    enum Option
    {
        NONE = 0,
        AND,
        OR
    };

    SearchAdvancedBase(Type type): m_type(type) {}
    virtual ~SearchAdvancedBase() {}

    Type type() const { return m_type; }

    virtual QWidget* widget() const = 0;
    virtual bool     isChecked() const = 0;

    Option  option() const { return m_option; }
    virtual void addOption(Option option) = 0;
    virtual void removeOption() = 0;

signals:
    void signalBaseItemToggled();
    void signalPropertyChanged();

protected slots:
    void slotToggle() { emit signalBaseItemToggled(); }
    void slotRuleChanged() { emit signalPropertyChanged(); }

protected:
    enum Option m_option;
    enum Type m_type;


};

class SearchAdvancedGroup;

class SearchAdvancedRule :  public SearchAdvancedBase
{
    Q_OBJECT

public:
    SearchAdvancedRule(QWidget* parent, Option option);
    ~SearchAdvancedRule();

    QWidget* widget() const;
    bool     isChecked() const;

    void    addOption(Option option);
    void    removeOption();

    void    addCheck();
    void    removeCheck();

    QString urlKey() const;
    QString urlOperator() const;
    QString urlValue() const;

private:
    QVBox*     m_box;

    QHBox*     m_hbox;
    QHBox*     m_valueBox;
    
    QLabel*    m_label;
    QCheckBox* m_check;

    QComboBox* m_key;
    QComboBox* m_operator;
    QLineEdit* m_value;
    KDateEdit* m_dateEdit;

    QHBox*     m_optionsBox;

    enum valueWidgetTypes
    {
        LINEEDIT= 0,
        DATE,
        COMBOBOX
    };
    enum valueWidgetTypes   m_valueWidget;

private slots:
    void slotKeyChanged(int);
};

class SearchAdvancedGroup : public SearchAdvancedBase
{
public:
    SearchAdvancedGroup(QWidget* parent);
    ~SearchAdvancedGroup();

    QWidget* widget() const;
    bool     isChecked() const;

    void addOption(Option option);
    void removeOption();

    void addRule(SearchAdvancedRule* rule);
    void removeRules();

    QValueList<SearchAdvancedRule*> childRules() const;

private:
    QHBox*            m_box;
    QVGroupBox*       m_groupbox;
    QCheckBox*        m_check;
    QValueList<SearchAdvancedRule*> m_childRules;

};

#endif /* SEARCHWIDGETS_H */
