# Generic ruby library for KDE extragear/playground releases
#
# Copyright (C) 2007-2010 Harald Sitter <apachelogger@ubuntu.com>
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

$: << File.dirname("../")

require 'lib/optparse'

require 'lib/kdialog'
$dlg = KDialog.new("#{NAME} release script","start-here-kde")

require 'lib/query'
if $srcvcs == "git"
    getGitBranch($options[:gitbranch])
end
checkout_location($options[:branch])
release_version($options[:version])
svn_protocol($options[:protocol])
svn_username($options[:user])
changelog($options[:changelog])

require 'lib/core'

fetch_source

fetch_l10n unless $options[:l10n] == false

fetch_doc unless $options[:doc] == false

create_tag if $options[:tag] == true

custom unless $options[:app] == false or not self.respond_to?("custom",include_private=true)

create_tar unless $options[:tar] == false

create_packager_notification unless $options[:pkgnotify] == false and $options[:tar] != true
