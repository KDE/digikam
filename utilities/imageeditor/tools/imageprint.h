/* ============================================================
 * Authors: F.J. Cruz <fj.cruz@supercable.es>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2004-07-13
 * Description : image editor printing interface.
 *
 * Copyright 2004-2007 by Gilles Caulier
 * Copyright 2006 by F.J. Cruz
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

#ifndef IMAGEPRINT_H
#define IMAGEPRINT_H

// Qt lib includes

#include <qwidget.h>
#include <qstring.h>

// KDE lib includes

#include <kurl.h>
#include <kprinter.h>
#include <kdeprint/kprintdialogpage.h>

namespace Digikam
{

class ImagePrintPrivate;

class ImagePrint
{
public:

    ImagePrint(DImg& image, KPrinter& printer, const QString& fileName);
    ~ImagePrint();

    bool printImageWithQt();

private:

    QString minimizeString(QString text, const QFontMetrics& metrics, int maxWidth);
    void readSettings();
    
private:

    KPrinter&          m_printer;

    ImagePrintPrivate *d;
};

//-----------------------------------------------------------------------------

class ImageEditorPrintDialogPagePrivate;

class ImageEditorPrintDialogPage : public KPrintDialogPage
{
    Q_OBJECT

public:

    enum Unit 
    {
        DK_MILLIMETERS = 1,
        DK_CENTIMETERS,
        DK_INCHES
    };

    static inline double unitToMM(Unit unit);
    static inline Unit stringToUnit(const QString& unit);  
    static inline QString unitToString(Unit unit); 

public:

    ImageEditorPrintDialogPage(DImg& image, QWidget *parent=0L, const char *name=0);
    ~ImageEditorPrintDialogPage();

    virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
    virtual void setOptions(const QMap<QString,QString>& opts);

private slots:

    void toggleScaling( bool enable );
    void toggleRatio( bool enable );
    void slotUnitChanged(const QString& string);
    void slotHeightChanged(double value);
    void slotWidthChanged(double value);
    void slotSetupDlg();
    void slotAlertSettings(bool t);

private:

    void readSettings();
    int getPosition(const QString& align);
    QString setPosition(int align);

private:

    ImageEditorPrintDialogPagePrivate *d;
};

}  // namespace Digikam

#endif // IMAGEPRINT_H
