# Generic ruby library for KDE extragear/playground releases
#
# Copyright (C) 2009 Harald Sitter <apachelogger@ubuntu.com>
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

require 'fileutils'
require 'lib/release'
require 'lib/l10n'
require 'lib/doc'
require 'lib/tag'
require 'lib/kdialog'

include FileUtils

SRC      = "#{NAME}-#{@version}"
BASEPATH = Dir.getwd()
SRCPATH  = BASEPATH + "/" + SRC

if $srcvcs == "git" then
    require 'lib/vcs-git.rb'
    SRCVCS = GIT
else
    require 'lib/vcs-svn.rb'
    SRCVCS = SVN
end

if @useStable
    branch = "branches/stable"
elsif @tag and not @tag.empty?()
    branch = "tags/#{NAME}/#{@tag}"
else
    branch = "trunk"
end

@repo = "#{@protocol}://#{@user}.kde.org/home/kde/#{branch}"
puts "Fetching source from #{@repo}...\n\n"

# This will take you to the default execution directory (BASEPATH)
def base_dir
    Dir.chdir(BASEPATH)
end

# This will take you to the default src directory, which is by default:
# execution dir + / + src folder name (NAME-VERSION)
def src_dir
    Dir.chdir(SRCPATH)
end

def exit_checker(errcode,thing)
    if errcode != 0
        exit 1 unless $dlg.warningcontinuecancel("Fetching of #{thing} failed.")
    end
end

def sharper(string="")
    string = string + " " unless string.empty?
    i = 0
    l = 80 - string.length
    while i < l
        string += "#"
        i += 1
    end
    return string + "\n"
end

# Remove unnecessary stuff
def remover(toberemoved=[])
    src_dir()
    for object in toberemoved
        rm_rf(object)
    end
end
