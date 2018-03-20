# ===========================================================
#
# This file is a part of digiKam project
# <a href="http://www.digikam.org">http://www.digikam.org</a>
#
# @date   2011-11-02
# @brief  simple Bash script to replace CRLF by EOL
#
# @author Copyright (C) 2011-2016 by Gilles Caulier
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

#!/bin/bash

for f in `find -iname \*`; do
    if file $f | grep "CRLF"
    then
        echo "Patched EOL of file: $f"
        tr -d '\15\32' < $f > $f.tr
        mv $f.tr $f
    fi
done
