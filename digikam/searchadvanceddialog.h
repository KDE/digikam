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

/** @file searchadvanceddialog.h */

#ifndef SEARCHADVANCEDDIALOG_H
#define SEARCHADVANCEDDIALOG_H

#include <kdialogbase.h>

class QVBox;
class QPushButton;
class QComboBox;
class QLineEdit;
class QTimer;
class SearchAdvancedBase;
class SearchResultsView;

/** @class SearchAdvancedDialog
 * 
 * This is the dialog for the advanced search
 * @author Tom Albers
 * @author Renchi Raju
 * 
 */
class SearchAdvancedDialog : public KDialogBase
{
    Q_OBJECT
    
public:
    /**
     * Constructor
     * @param parent parent window
     * @param url holds the url for the search
     */
    SearchAdvancedDialog(QWidget* parent, KURL& url);

    /**
     * Destructor
     */
    ~SearchAdvancedDialog();

private slots:
    void fillWidgets(const KURL& url);
    void slotAddRule();
    void slotDelRules();
    void slotGroupRules();
    void slotUnGroupRules();
    void slotChangeButtonStates();
    void slotTimeOut();
    void slotPropertyChanged();
    void slotOk();

private:
    QVBox*             m_rulesBox;
    QPushButton*       m_addButton;
    QPushButton*       m_delButton;
    QPushButton*       m_groupButton;
    QPushButton*       m_ungroupButton;
    QComboBox*         m_optionsCombo;
    QLineEdit*         m_title;
    QValueList<SearchAdvancedBase*>  m_baseList;
    SearchResultsView* m_resultsView;
    QTimer*            m_timer;
    KURL&              m_url;
};

#endif /* SEARCHADVANCEDDIALOG_H */
