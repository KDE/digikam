#! /usr/bin/env ruby

# ============================================================
# 
# This file is a part of digiKam project
# http://www.digikam.org
# 
# Date        : 2012-07-14
# Description : a helper script for finding source code with no moc includes
# 
# Copyright (C) 2012 by Andi Clemens <andi dot clemens at gmail dot com>
# 
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# ============================================================ */


# get all header files containing the line "Q_OBJECT"
candidates = Dir.glob("**/*.h").select do |file_name|
  File.read(file_name) =~ /^\s*Q_OBJECT\s*$/ rescue false
end

# get all source files with missing MOC includes
missingMocIncludes = candidates.select do |file_name|
  source_file = file_name.sub(/\.h$/, ".cpp")
  moc_file = File.basename(file_name, '.h') + '.moc'
  pattern = /#include\s+[<"]#{moc_file}[>"]/

  (File.read(source_file) =~ pattern) == nil rescue false
end

# display missing MOC includes
puts "missing MOC include:"
width = missingMocIncludes.length.to_s.length

missingMocIncludes.each_with_index do |file_name, idx|
  puts "%#{width}s: #{file_name}" % (idx + 1)
end
