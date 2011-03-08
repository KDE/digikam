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

module SVN
    module_function

    def getSrc(repo,folder)
        begin
            %x[svn co #{repo}/#{COMPONENT}/#{SECTION}/#{PATHPREFIX}/#{NAME} #{folder}]
        rescue NameError
            %x[svn co #{repo}/#{COMPONENT}/#{SECTION}/#{NAME} #{folder}]
        end
    end

    def tagSrc(tag)
        %x[svn mkdir -m "Create tag #{NAME} #{@version} root directory" #{tag}]
        %x[svn cp -m "Tag #{NAME} #{@version}." #{@repo}/#{COMPONENT}/#{SECTION}/#{NAME} #{tag}]
    end
end
