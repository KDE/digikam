/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-02-06
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

#ifndef UNDOACTION_H
#define UNDOACTION_H

namespace Digikam
{
class ImlibInterface;
}

class UndoAction
{
public:

    UndoAction(Digikam::ImlibInterface* iface);
    virtual ~UndoAction();

    virtual void rollBack() = 0;
    
protected:

    Digikam::ImlibInterface* m_iface;
};

class UndoActionRotate : public UndoAction
{
public:

    enum Angle
    {
        R90,
        R180,
        R270
    };
    
    UndoActionRotate(Digikam::ImlibInterface* iface, Angle angle);
    ~UndoActionRotate();

    void rollBack();

private:

    int m_angle;
};

class UndoActionFlip : public UndoAction
{
public:

    enum Direction
    {
        Horizontal,
        Vertical
    };
    
    UndoActionFlip(Digikam::ImlibInterface* iface, Direction dir);
    ~UndoActionFlip();

    void rollBack();

private:

    int m_dir;
};

class UndoActionBCG : public UndoAction
{
public:

    UndoActionBCG(Digikam::ImlibInterface* iface,
                  double gamma, double brightness,
                  double contrast);
    ~UndoActionBCG();

    void rollBack();
    
private:

    double m_gamma;
    double m_brightness;
    double m_contrast;
};

class UndoActionIrreversible : public UndoAction
{
public:

    UndoActionIrreversible(Digikam::ImlibInterface* iface);
    ~UndoActionIrreversible();

    void rollBack();
};

#endif /* UNDOACTION_H */
