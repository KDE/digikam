# Generic ruby library for KDE extragear/playground releases
#
# Copyright (C) 2007-2009 Harald Sitter <apachelogger@ubuntu.com>
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

require 'lib/l10ncore.rb'

def fetch_doc
    src_dir
    ld = "l10n"
    dd = "doc"
    Dir.mkdir dd unless File.exists?("doc")

    l10nlangs = %x[svn cat #{@repo}/l10n-kde4/subdirs].split("\n")
    @docs     = []

    system("svn co #{@repo}/#{COMPONENT}/#{SECTION}/doc/#{NAME} doc/en_US")

    # No documentation avilable -> leave me alone
    if not File.exists?("doc/en_US") then
        puts("There is no documentation :(")
        puts("Leave me alone :(")
        return
    end

    cmakefile = File.new( "doc/en_US/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
    cmakefile << "kde4_create_handbook(index.docbook INSTALL_DESTINATION \${HTML_INSTALL_DIR}/en SUBDIR #{NAME} )\n"
    cmakefile.close

    # docs
    for lang in l10nlangs
        lang.chomp!

        docdirname = "l10n-kde4/#{lang}/docs/#{COMPONENT}-#{SECTION}/#{NAME}"
        # TODO: ruby-svn
        FileUtils.rm_rf( "l10n" )
        %x[svn co #{@repo}/#{docdirname} l10n 2> /dev/null]
        next unless FileTest.exists?( "l10n/index.docbook" ) # without index the translation is not worth butter

        dest = "doc/#{lang}"
        puts("Copying #{lang}'s #{NAME} documentation over...")
        FileUtils.mv( "l10n", dest )

        cmakefile = File.new( "doc/#{lang}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
        cmakefile << "kde4_create_handbook(index.docbook INSTALL_DESTINATION \${HTML_INSTALL_DIR}/#{lang} SUBDIR #{NAME} )\n"
        cmakefile.close

        # add to SVN in case we are tagging
        `svn add doc/#{lang}/CMakeLists.txt`
        @docs += [lang]

        puts( "done.\n" )
    end

    src_dir

    if not @docs.empty? # make sure we actually fetched docs
        # create doc's cmake file
        L10nCore.cmake_creator(dd)

        # change cmake file
        L10nCore.cmake_add_sub(dd)
    else
        rm_rf dd
    end

    rm_rf ld

#     compress_doc
end


def compress_doc
end
