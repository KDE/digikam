# Generic ruby library for KDE extragear/playground releases
#
# Copyright © 2007-2010 Harald Sitter <apachelogger@ubuntu.com>
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

require 'lib/l10ncore'
require 'lib/l10nstat'

include L10nCore

def pofiledir?(lang)
    return "l10n-kde4/#{lang}/messages/#{COMPONENT}-#{SECTION}"
end

def strip_comments(file)
    # Strip #~ lines, which once were sensible translations, but then the
    # strings become removed, so they now stick around in case the strings
    # return, poor souls, waiting forever :(
    # Problem is that msgfmt does add those to the binary!
    file = File.new(file, File::RDWR)
    str = file.read
    file.rewind
    file.truncate(0)
    str.gsub!(/#~.*/, "")
    str = str.strip
    file << str
    file.close
end

def fetch_l10n_single(ld, pos, lang)
    pofiledir = pofiledir?(lang)
    pofilename = pos[0] #
    rm_rf ld
    Dir.mkdir(ld)
    system("svn export #{@repo}/#{pofiledir}/#{pofilename} #{ld}/#{pofilename}")
    # Do not exit check here, since a failed co on 

    files = Array.new
    if File.exist?("#{ld}/#{pofilename}")
        files << "#{ld}/#{pofilename}"
        strip_comments("#{ld}/#{pofilename}")
    end
    return files
end

def fetch_l10n_multiple(ld, pos, lang)
    pofiledir = pofiledir?(lang)
    rm_rf ld
    return Array.new if %x[svn ls #{@repo}/#{pofiledir}].empty?
    system("svn co #{@repo}/#{pofiledir} #{ld}")
    exit_checker($?,pofiledir)

    files = Array.new
    pos.each do |po|
        next if not File.exist?("#{ld}/#{po}")
        files << "#{ld}/#{po}"
        strip_comments("#{ld}/#{po}")
    end
    return files
end

def fetch_l10n
    src_dir
    ld    = "l10n"
    pd    = "po"
    Dir.mkdir pd

    pos       = po_finder
    l10nlangs = %x[svn cat #{@repo}/l10n-kde4/subdirs].split("\n")
    @l10n     = Array.new

    # Only do single fetches (svn export) if tagging is not used, or l10n could
    # not be tagged.
    if pos.length == 1 and not $options[:tag]
        multiple = false
    else
        multiple = true
    end

    for lang in l10nlangs
        next if lang == "x-test"

        if multiple
            files = fetch_l10n_multiple(ld, pos, lang)
        else
            files = fetch_l10n_single(ld, pos, lang)
        end

        next if files.empty?

        dest = pd + "/#{lang}"
        Dir.mkdir dest

        puts("Copying #{lang}\'s .po(s) over ...")
        mv( files, dest )
        mv( ld + "/.svn", dest ) if $options[:tag] # Must be fatal iff tagging

        # create lang's cmake files
        cmakefile = File.new( "#{dest}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
        cmakefile << "file(GLOB _po_files *.po)\n"
        cmakefile << "GETTEXT_PROCESS_PO_FILES(#{lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR} ${_po_files} )\n"
        cmakefile.close

        # add to SVN in case we are tagging
        %x[svn add #{dest}/CMakeLists.txt] if $options[:tag]
        @l10n += [lang]

        puts "done."
    end

    # the statistics depend on @l10n, so invoking it only within fetch_l10n makes most sense
    l10nstat unless $options[:stat] == false or @l10n.empty?

    if not @l10n.empty? # make sure we actually fetched languages
        # create po's cmake file
        cmake_creator(pd,true)

        # change cmake file
        cmake_add_sub(pd)
    else
        rm_rf pd
    end

    rm_rf ld
end
