/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-19
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef RENAMECUSTOMIZER_H
#define RENAMECUSTOMIZER_H

// Qt includes.

#include <qbuttongroup.h>

class QLineEdit;
class QCheckBox;
class QRadioButton;
class QComboBox;
class QTimer;
class QHBox;

namespace Digikam
{

class RenameCustomizer : public QButtonGroup
{
    Q_OBJECT

public:

    enum Case
    {
        NONE = 0,
        UPPER,
        LOWER
    };

    RenameCustomizer(QWidget* parent);
    ~RenameCustomizer();

    void    setUseDefault(bool val);
    bool    useDefault() const;
    QString nameTemplate() const;
    Case    changeCase() const;



signals:

    void signalChanged();
    
private:

    void readSettings();
    void saveSettings();
    
    QRadioButton*  m_renameDefault;
    QRadioButton*  m_renameCustom;

    QGroupBox*     m_renameDefaultBox;
    QLabel*        m_renameDefaultCase;
    QComboBox*     m_renameDefaultCaseType;

    QGroupBox*     m_renameCustomBox;
    QLineEdit*     m_renameCustomPrefix;
    QCheckBox*     m_renameCustomExif;
    QCheckBox*     m_renameCustomSeq;

    QTimer*        m_changedTimer;

private slots:

    void slotRadioButtonClicked(int);
    void slotPrefixChanged(const QString&);
    void slotExifChanged(bool);
    void slotSeqChanged(bool);
    void slotCaseTypeChanged(const QString&);
};

}  // namespace Digikam

#endif /* RENAMECUSTOMIZER_H */
