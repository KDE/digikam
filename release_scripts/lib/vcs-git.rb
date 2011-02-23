# Generic ruby library for KDE extragear/playground releases
#
# Copyright © 2009-2010 Harald Sitter <apachelogger@ubuntu.com>
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

module GIT
    module_function

    def getSrc(repo,folder)
        if ($options[:customsrc] == nil) then
            system("git clone --depth 1 git://gitorious.org/#{NAME}/#{NAME}.git #{folder}")
        else
            system("git clone --depth 1 #{$options[:customsrc]} #{folder}")
        end

        if $gitbranch != nil and $gitbranch != "master"
            puts "Switching to branch #{$gitbranch}..."
            system("cd #{folder} && git checkout #{$gitbranch}")
        end
    end

    def tagSrc(tag)
        puts "To tag with your standard Gitorious project that bans push -f:
*Ask an admin to uncheck the ban of push -f's
*git tag -a -m \"Some ever nice tage message\" v#{tag.split("/")[-1]}
*git push --tags git://git@gitorious.org:#{NAME}/#{NAME}.git
*tell admin to reenable the push -f ban"
    end
end
