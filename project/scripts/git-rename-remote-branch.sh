#!/bin/sh

# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2016-08-15
# @brief  Script to rename remote git branch.
#
# @author Copyright (C) 2017 by Gilles Caulier
#         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# ============================================================

if [ $# -ne 3 ]; then
    echo "Rationale : Rename a branch on the server without checking it out."
    echo "Usage     : $(basename $0) <remote> <old name> <new name>"
    echo "Example   : $(basename $0) origin master release"
    exit 1
fi

git push $1 $1/$2:refs/heads/$3 :$2
