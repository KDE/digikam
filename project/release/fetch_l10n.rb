#!/usr/bin/env ruby
#
# Ruby script for generating Amarok tarball releases from KDE SVN
#
# (c) 2005 Mark Kretschmann <kretschmann@kde.org>
# Some parts of this code taken from cvs2dist
# License: GNU General Public License V2


branch = "trunk"
tag = ""

unless $*.empty?()
    case $*[0]
        when "--branch"
            branch = `kdialog --inputbox "Enter branch name: " "branches/stable"`.chomp()
        when "--tag"
            tag = `kdialog --inputbox "Enter tag name: "`.chomp()
        else
            puts("Unknown option #{$1}. Use --branch or --tag.\n")
    end
end

# Ask user for targeted application version
user = `kdialog --inputbox "Your SVN user:"`.chomp()
protocol = `kdialog --radiolist "Do you use https or svn+ssh?" https https 0 "svn+ssh" "svn+ssh" 1`.chomp()

puts "\n"
puts "**** l10n ****"
puts "\n"

i18nlangs = `cat subdirs`
Dir.mkdir( "po" )
Dir.chdir( "po" )
topmakefile = File.new( "CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
for lang in i18nlangs
    lang.chomp!()
    Dir.mkdir(lang)
    Dir.chdir(lang)
    for part in ['digikam']
        puts "Copying #{lang}'s #{part} over..  "
        `svn cat #{protocol}://#{user}@svn.kde.org/home/kde/#{branch}/l10n-kde4/#{lang}/messages/extragear-graphics/#{part}.po 2> /dev/null | tee #{part}.po `

        if FileTest.size( "#{part}.po" ) == 0
          File.delete( "#{part}.po" )
          puts "Delete File #{part}.po"
        end
 
        makefile = File.new( "CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
        makefile << "file(GLOB _po_files *.po)\n"
        makefile << "GETTEXT_PROCESS_PO_FILES( #{lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR} ${_po_files} )\n"
        makefile.close()
                                                        
        puts( "done.\n" )
    end
    Dir.chdir("..")
    topmakefile << "add_subdirectory( #{lang} )\n"
end

puts "\n"
