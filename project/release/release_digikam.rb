#!/usr/bin/env ruby
#
# Generates a release tarball from KDE SVN
#
# Copyright (C) 2008 Harald Sitter <harald@getamarok.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License or (at your option) version 3 or any later version
# accepted by the membership of KDE e.V. (or its successor approved
# by the membership of KDE e.V.), which shall act as a proxy
# defined in Section 14 of version 3 of the license.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

NAME      = "digikam"
COMPONENT = "extragear"
SECTION   = "graphics"
BASEPATH  = Dir.getwd()

require 'fileutils'
require 'lib/libbase.rb'
require 'lib/librelease.rb'
require 'lib/libl10n.rb'
require 'lib/libtag.rb'

# digikam-only changes
def digikam()
    puts("Running digikam-only changes")
    srcDir()

    # change version in CMakeLists.txt
    version       = @version.split(".")
    majorversion  = version[0]
    minorversion  = version[1]
    patchversion  = version[2].split("-")[0]
    suffixversion = version[2].split("-")[1]

    file = File.new( "CMakeLists.txt", File::RDWR )
    str = file.read()
    file.rewind()
    file.truncate( 0 )
    str.sub!( /SET\(DIGIKAM_MAJOR_VERSION \".*\"\)/, "SET\(DIGIKAM_MAJOR_VERSION \"#{majorversion}\"\)" )
    str.sub!( /SET\(DIGIKAM_MINOR_VERSION \".*\"\)/, "SET\(DIGIKAM_MINOR_VERSION \"#{minorversion}\"\)" )
    str.sub!( /SET\(DIGIKAM_PATCH_VERSION \".*\"\)/, "SET\(DIGIKAM_PATCH_VERSION \"#{patchversion}\"\)" )
    unless suffixversion.empty?()
        str.sub!( /SET\(DIGIKAM_SUFFIX_VERSION \".*\"\)/, "SET\(DIGIKAM_SUFFIX_VERSION \"-#{suffixversion}\"\)" )
    end
    file << str
    file.close()

    # remove unnecessary stuff from tarball
    toberemoved = ["project"]
    for object in toberemoved
        FileUtils.rm_rf(object)
    end

    baseDir()
end

informationQuery()

# TODO: why is this done here?
@folder = "#{NAME}-#{@version}" #create folder instance var

fetchSource()

fetchTranslations()

# fetchDocumentation()

createTranslationStats()

# TODO: currently broken
# createTag()

digikam()

createTar()

createCheckSums()
