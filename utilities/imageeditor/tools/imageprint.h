/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-07-13
 * Description : image editor printing interface.
 *
 * Copyright (C) 2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
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

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <kurl.h>
#include <kprinter.h>
#include <kdeprint/kprintdialogpage.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImagePrintPrivate;

class DIGIKAM_EXPORT ImagePrint
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

class DIGIKAM_EXPORT ImageEditorPrintDialogPage : public KPrintDialogPage
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
