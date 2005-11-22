/* ============================================================
 * File  : fontchooserwidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-14
 * Description : a simple widget to choose a font based on 
 *               FontChooserWidget implementation.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original FontChooserWidget implementation is copyrighted by :
 * (C) 1997 Bernd Johannes Wuebben <wuebben@kde.org>
 * (c) 1999 Preston Brown <pbrown@kde.org>
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

#ifndef FONT_CHOOSER_WIDGET_H
#define FONT_CHOOSER_WIDGET_H

#include <qlineedit.h>
#include <qbutton.h>

class QComboBox;
class QCheckBox;
class QFont;
class QGroupBox;
class QLabel;
class QStringList;

class KListBox;
class KIntNumInput;

namespace DigikamInsertTextImagesPlugin
{

class FontChooserWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY( QFont font READ font WRITE setFont )

public:
  
    enum FontColumn 
       { 
       FamilyList=0x01, 
       StyleList=0x02, 
       SizeList=0x04
       };

    enum FontDiff 
       { 
       FontDiffFamily=0x01, 
       FontDiffStyle=0x02, 
       FontDiffSize=0x04 
       };
  
    enum FontListCriteria 
       {
       FixedWidthFonts=0x01, 
       ScalableFonts=0x02, 
       SmoothScalableFonts=0x04 
       };

public:
  
    FontChooserWidget(QWidget *parent = 0L, const char *name = 0L,
                      bool onlyFixed = false,
                      const QStringList &fontList = QStringList(),
                      int visibleListSize=8,
                      bool diff = false, 
                      QButton::ToggleState *sizeIsRelativeState = 0L );
    
    ~FontChooserWidget();
    
    void setFont( const QFont &font, bool onlyFixed = false );
    void setColor( const QColor & col );
    void setBackgroundColor( const QColor & col );
    void setSizeIsRelative( QButton::ToggleState relative );
    
    QFont font() const { return selFont; };
    QColor color() const;
    QColor backgroundColor() const;
    static void getFontList( QStringList &list, uint fontListCriteria);
    QButton::ToggleState sizeIsRelative() const;
    static QString getXLFD( const QFont &theFont ) { return theFont.rawName(); };
        
    int fontDiffFlags();
    void enableColumn( int column, bool state );
    virtual QSize sizeHint( void ) const;

signals:
  
    void fontSelected( const QFont &font );

private slots:
  
    void toggled_checkbox();
    void family_chosen_slot(const QString&);
    void size_chosen_slot(const QString&);
    void style_chosen_slot(const QString&);
    void displaySample(const QFont &font);
    void showXLFDArea(bool);
    void size_value_slot(int);
  
private:
  
    void fillFamilyListBox(bool onlyFixedFonts = false);
    void fillSizeList();
    
    void addFont( QStringList &list, const char *xfont );
    
    void setupDisplay();
    
    int minimumListWidth( const QListBox *list );
    int minimumListHeight( const QListBox *list, int numVisibleEntry );

private:
    
    bool                    usingFixed;
    int                     selectedSize;
    
    QMap<QString, QString>  currentStyles;
    
    // pointer to an optinally supplied list of fonts to
    // inserted into the fontdialog font-family combo-box
    QStringList             fontList;
    
    QLineEdit              *xlfdEdit;
    
    QLabel                 *familyLabel;
    QLabel                 *styleLabel;
    
    QCheckBox              *familyCheckbox;
    QCheckBox              *styleCheckbox;
    QCheckBox              *sizeCheckbox;
    QCheckBox              *sizeIsRelativeCheckBox;
    
    QComboBox              *charsetsCombo;
    
    QFont                   selFont;
    
    QString                 selectedStyle;
    
    QLabel                 *sizeLabel;
    
    KListBox               *familyListBox;
    KListBox               *styleListBox;
    KListBox               *sizeListBox;
    
    KIntNumInput           *sizeOfFont;
  
private:

    class FontChooserWidgetPrivate;
    FontChooserWidgetPrivate *d;
  
};

}  // NameSpace DigikamInsertTextImagesPlugin

#endif // FONT_CHOOSER_WIDGET_H
