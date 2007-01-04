/* ============================================================
 * Authors: Tom Albers <tomalbers@kde.nl>
 *          Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2005-01-01
 * Description : 
 * 
 * Copyright 2005 by Tom Albers and Renchi Raju
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

/** @file searchwidgets.h */

#ifndef SEARCHWIDGETS_H
#define SEARCHWIDGETS_H

class QHBox;
class QVBox;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QVGroupBox;
class QLabel;

class KURL;

class KDateEdit;

namespace Digikam
{

class RatingWidget;
class SqueezedComboBox;

/** @class SearchRuleLabel
 *
 * This class inherits everything from QLabel, and adds one
 * signal to it, when double clicked on it.
 */
class SearchRuleLabel: public QLabel
{
    Q_OBJECT

public:

    /** Constructor. See for more info the QLabel clas.
     * @param text Text of the label
     * @param parent The parent widget
     * @param name The name
     * @param f WFlags
     */
    SearchRuleLabel( const QString & text,
               QWidget * parent,
               const char * name=0,
               WFlags f=0 );
private:

    void mouseDoubleClickEvent( QMouseEvent * e );

signals:

    /**
     * Signal which gets emitted when a double clicked
     * event occurs
     * @param e the mouse event received
     */
    void signalDoubleClick( QMouseEvent * e );
};

/** @class SearchAdvancedBase
 *
 * This class is a common class for SearchAdvancedRule and
 * SearchAdvancedGroup. It contains the basic functionality
 * for the advanced search dialog.
 * @author Renchi Raju
 * @author Tom Albers
 * 
 */
class SearchAdvancedBase : public QObject
{
    Q_OBJECT

public:

    /** @enum Type
     * Possible types. A Base item can either be a Rule on its own
     * or hold a group of rules.
     */
    enum Type
    {
        RULE = 0,
        GROUP
    };

    /** @enum Option
     * Possible Options. A Rule or a group of rules can have a relation
     * to the previous rule or group of rules. None can be used for the
     * first rule or group.
     */
    enum Option
    {
        NONE = 0,
        AND,
        OR
    };

    /**
     * Constructor
     * @param type Determines which type of base item to be created.
     */
    SearchAdvancedBase(Type type): m_type(type) {}
    
    /**
     * Destructor
     */
    virtual ~SearchAdvancedBase() {}

    /**
     * Returns the type of the base item
     * @return The type which is a enum, see above
     */
    Type type() const { return m_type; }

    /**
     * Returns a pointer to the widget holding the base item
     * @return Pointer to the widget
     */
    virtual QWidget* widget() const = 0;

    /**
     * Determines if the base item is checked or not
     * @return true if the base item is checked and false if not
     */
    virtual bool     isChecked() const = 0;

    /**
     * Returns the option which is accociated with the base item
     * @return The Option which is a enum, see above
     */
    Option  option() const { return m_option; }

    /**
     * Sets the option of the base item
     * @param option The enum of the option to be set
     */
    virtual void addOption(Option option) = 0;

    /**
     * Removes the option of the base item
     */
    virtual void removeOption() = 0;

signals:
    /**
     * This signal is emitted when a rule or group is checked or unchecked
     * This is used to determine the state of the buttons of the dialog
     */
    void signalBaseItemToggled();

    /**
     * This signal is emitted when there is a change in the rule.
     * This is used in the dialog to recalculate the url to pass to the
     * preview widget
     */
    void signalPropertyChanged();

protected:

    enum Option m_option;
    enum Type m_type;
};

class SearchAdvancedGroup;

/** @class SearchAdvancedRule
 * This inherits SearchAdvancedBase and is one rule in the search dialog
 * it contains all widgets to create a rule
 */
class SearchAdvancedRule :  public SearchAdvancedBase
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent The parent
     * @param option Holds the Option of the rule, see the enum.
     *
     */
    SearchAdvancedRule(QWidget* parent, Option option);

    /**
     * destructor
     */
    ~SearchAdvancedRule();

    /**
     * Returns a pointer to the widget holding the rule
     * @return Pointer to the widget
     */
    QWidget* widget() const;

    /**
     * Determines if the rule is checked or not
     * @return True if the rule is checked, false if not
     */
    bool     isChecked() const;

    /**
     * Sets the values of the rule.
     * @param url The url which sets defaults for the rule.
     */
    void    setValues(const KURL& url);

    /**
     * Sets the option of the rule, so this holds the
     * relation ship to the previous rule or group
     * @param option the enum of the option to be set
     */
    void    addOption(Option option);

    /**
     * Removes the option of the rule
     */
    void    removeOption();

    /**
     * This adds the checkbox at the end of the rule. This is needed
     * for example when a group of rules get ungrouped.
     */
    void    addCheck();

    /**
     * This removes the checkbox at the end of the rule, this is
     * used if the rule is a part of a group. In a group the group
     * has the checkbox, not the individual rules.
     */
    void    removeCheck();

    /**
     * Gives back the value of the key part of the rule, which can
     * be used to build the correct url for the kio
     * @return The value of the key part of the rule
     */
    QString urlKey() const;

    /**
     * Gives back the value of the operator part of the rule, which can
     * be used to build the correct url for the kio
     * @return The value of the operator part of the rule
     */
    QString urlOperator() const;

    /**
     * Gives back the value of the value part of the rule, which can
     * be used to build the correct url for the kio
     * @return The value of the value part of the rule
     */
    QString urlValue() const;

    enum valueWidgetTypes
    {
        NOWIDGET = 0,
        LINEEDIT,
        DATE,
        ALBUMS,
        TAGS,
        RATING
    };

private:

    void setValueWidget(valueWidgetTypes oldType, valueWidgetTypes newType);

private slots:

    void slotKeyChanged(int);
    void slotLabelDoubleClick();

private:

    QLabel*                 m_label;

    QVBox*                  m_box;
    QWidget*                m_hbox;
    QHBoxLayout*            m_hboxLayout;

    QHBox*                  m_valueBox;

    QCheckBox*              m_check;

    QComboBox*              m_key;
    QComboBox*              m_operator;

    QLineEdit*              m_lineEdit;
    KDateEdit*              m_dateEdit;
    SqueezedComboBox*       m_valueCombo;
    RatingWidget*           m_ratingWidget;

    QMap<int, int>          m_itemsIndexIDMap;

    QHBox*                  m_optionsBox;

    enum valueWidgetTypes   m_widgetType;
};

/** @class SearchAdvancedGroup
 * This inherits SearchAdvancedBase and is a group of rules
 * in the search dialog
 */
class SearchAdvancedGroup : public SearchAdvancedBase
{

public:
    /**
     * Constructor
     * @param parent the parent
     */
    SearchAdvancedGroup(QWidget* parent);

    /**
     * Destructor
     */
    ~SearchAdvancedGroup();

     /**
     * Returns a pointer to the widget holding the group
     * @return pointer to the widget
     */
    QWidget* widget() const;

    /**
     * determines if the rule is checked or not
     * @return bool, which indicated if the base item is checked
     */
    bool isChecked() const;

    /**
     * sets the option of the group
     * @param option the enum of the option to be set
     */
    void addOption(Option option);

    /**
     * removes the option
     */
    void removeOption();

    /**
     * adds a rule to group
     * @param rule the pointer to the rule to be added to the group
     */
    void addRule(SearchAdvancedRule* rule);

    /**
     * removes all rules from the group
     */
    void removeRules();

    /**
     * gives back a list of pointers to the rules inside the group
     * @return a list of pointers to the childeren in the group
     */
    QValueList<SearchAdvancedRule*> childRules() const;

private:

    QHBox*                          m_box;
    QVGroupBox*                     m_groupbox;
    QCheckBox*                      m_check;
    QValueList<SearchAdvancedRule*> m_childRules;

};

}  // namespace Digikam

#endif /* SEARCHWIDGETS_H */
