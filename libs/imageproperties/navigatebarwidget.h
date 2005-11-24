/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-07
 * Description :
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef NAVIGATEBARWIDGET_H
#define NAVIGATEBARWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// Local includes.

#include "digikam_export.h"

class QPushButton;
class KSqueezedTextLabel;

namespace Digikam
{

class DIGIKAM_EXPORT NavigateBarWidget : public QWidget
{
Q_OBJECT

public:

    enum CurrentItemPosition
    {
    ItemCurrent=0,
    ItemFirst,
    ItemLast
    };

public:

    NavigateBarWidget(QWidget *parent=0, bool show=true);
    ~NavigateBarWidget();
    
    void setFileName(QString filename);
    void setButtonsState(int itemType);

signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void);    
        
private:    

    QPushButton        *m_firstButton;
    QPushButton        *m_prevButton;
    QPushButton        *m_nextButton;
    QPushButton        *m_lastButton;
    
    KSqueezedTextLabel *m_filename;

};

}  // namespace Digikam

#endif /* NAVIGATEBARWIDGET_H */
