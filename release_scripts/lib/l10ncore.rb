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

module L10nCore
    def po_finder(pos=Array.new)
        Dir.glob("**/**/Messages.sh").each do |file|
            File.readlines(file).each do |line|
                line.match(/[^\/]*\.pot/).to_a.each do |match|
                    pos << match.sub(".pot",".po")
                end
            end
        end
        return pos
    end

    def cmake_creator(dir,l10n=false)
        cmakefile = File.new( "#{dir}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
        if l10n
            cmakefile << "find_package(Gettext REQUIRED)\n"
            cmakefile << "if (NOT GETTEXT_MSGMERGE_EXECUTABLE)\n"
            cmakefile << "MESSAGE(FATAL_ERROR \"Please install msgmerge binary\")\n"
            cmakefile << "endif (NOT GETTEXT_MSGMERGE_EXECUTABLE)\n"
            cmakefile << "if (NOT GETTEXT_MSGFMT_EXECUTABLE)\n"
            cmakefile << "MESSAGE(FATAL_ERROR \"Please install msgmerge binary\")\n"
            cmakefile << "endif (NOT GETTEXT_MSGFMT_EXECUTABLE)\n"
        end
        Dir.foreach(dir) {|lang|
            next if lang == '.' or lang == '..' or lang == 'CMakeLists.txt'
            cmakefile << "add_subdirectory(#{lang})\n"
        }
        cmakefile.close
    end

    def cmake_add_sub(dir)
        cmakefile = File.new( "CMakeLists.txt", File::APPEND | File::RDWR )
        cmakestr = cmakefile.read()
        cmakefile.rewind()
        cmakefile.truncate( 0 )
        macro = "\ninclude(MacroOptionalAddSubdirectory)\nmacro_optional_add_subdirectory( #{dir} )\n"
        if cmakestr.include?("##{dir.upcase}_SUBDIR")
            cmakestr = cmakestr.sub("##{dir.upcase}_SUBDIR",macro)
        # TODO: should be a regex for whitespace lovers
        elsif not cmakestr.include?("add_subdirectory(#{dir})") and not cmakestr.include?("macro_optional_add_subdirectory(#{dir})")
            cmakestr << macro
        end
        cmakefile << cmakestr
        cmakefile.close
    end
end
