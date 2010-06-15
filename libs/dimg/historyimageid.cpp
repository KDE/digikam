/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-02
 * Description : class holding properties of referenced files used in non-dest. editing
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "historyimageid.h"

namespace Digikam 
{

HistoryImageId::HistoryImageId()
{

}

HistoryImageId::HistoryImageId(const QString& originalUUID, const QString& fileUUID, const QString& fileName, const QDateTime& creationDate)
{
  m_originalUUID = originalUUID;
  m_fileUUID = fileUUID;
  m_fileName = fileName;
  m_creationDate = creationDate;
}

bool HistoryImageId::matches(const Digikam::HistoryImageId& other) const
{
  if(m_originalUUID == other.m_originalUUID && m_fileUUID == other.m_fileUUID)
  {
      return true;
  }
  else return false;
}

bool HistoryImageId::isEmpty() const
{
  if(m_originalUUID.isEmpty() && m_fileUUID.isEmpty())
  {
      return true;
  }
  else return false;
}

bool HistoryImageId::isOriginalFile() const
{
  if( ( !m_originalUUID.isEmpty() && m_fileUUID.isEmpty() ) || (!m_originalUUID.isEmpty() && (m_originalUUID == m_fileUUID) ) )
  {
      return true;
  }
  else return false;  
}

}