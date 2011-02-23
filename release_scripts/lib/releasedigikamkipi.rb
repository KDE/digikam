# Ruby library for Digikam and Kipi-Plugins pre-archiving changes
#
# Copyright (C) 2008 Harald Sitter <apachelogger@ubuntu.com>
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

@changelog = "ChangeLog"

def custom
    src_dir
    puts("Running #{NAME}-only changes")

    # change version in CMakeLists.txt
    version       = @version.split(".")
    majorversion  = version[0]
    minorversion  = version[1]
    unless version[2] == nil
        patchversion  = version[2].split("-")[0]
        suffixversion = version[2].split("-")[1]
    end
    name          = NAME.gsub("-","").upcase

    file = File.new( "CMakeLists.txt", File::RDWR )
    str = file.read
    file.rewind()
    file.truncate( 0 )
    str.sub!( /SET\(#{name}_MAJOR_VERSION \".*\"\)/, "SET\(#{name}_MAJOR_VERSION \"#{majorversion}\"\)" )
    str.sub!( /SET\(#{name}_MINOR_VERSION \".*\"\)/, "SET\(#{name}_MINOR_VERSION \"#{minorversion}\"\)" )
    str.sub!( /SET\(#{name}_PATCH_VERSION \".*\"\)/, "SET\(#{name}_PATCH_VERSION \"#{patchversion}\"\)" )
    unless suffixversion == nil
        str.sub!( /SET\(#{name}_SUFFIX_VERSION \".*\"\)/, "SET\(#{name}_SUFFIX_VERSION \"-#{suffixversion}\"\)" )
    end
    file << str
    file.close

    # remove unnecessary stuff from tarball
    remover(["project"])

    base_dir
end
