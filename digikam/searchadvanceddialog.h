/* ============================================================
 * Authors: Tom Albers <tomalbers@kde.nl>
 *          Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-01-01
 * Description : a dialog to perform advanced search in albums
 * 
 * Copyright 2005 by Tom Albers and Renchi Raju
 *           2006 by Gilles Caulier
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


/** @file searchadvanceddialog.h */

#ifndef SEARCHADVANCEDDIALOG_H
#define SEARCHADVANCEDDIALOG_H

// KDE includes.

#include <kdialogbase.h>

class KURL;

namespace Digikam
{

class SearchAdvancedDialogPriv;

/** @class SearchAdvancedDialog
 * 
 * This is the dialog for the advanced search
 * @author Tom Albers
 * @author Renchi Raju
 * @author Gilles Caulier
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

    KURL&                     m_url;

    SearchAdvancedDialogPriv* d;
};

}  // namespace Digikam

#endif /* SEARCHADVANCEDDIALOG_H */
