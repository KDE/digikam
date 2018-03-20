#!/usr/bin/env ruby
#
# Ruby script for pulling l10n documentation translations for digikam
# Requires ruby version >= 1.9
#
# Copyright (c)      2005, Mark Kretschmann, <kretschmann at kde dot org>
# Copyright (c)      2014, Nicolas Lécureuil, <kde at nicolaslecureuil dot fr>
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

require 'rbconfig'
require 'fileutils'
isWindows = RbConfig::CONFIG['host_os'] =~ /mswin|mingw|cygwin/i
STDOUT.sync = true

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

i18nlangs = []

if isWindows
    i18nlangs = `type .\\project\\release\\subdirs`
else
    i18nlangs = `cat project/release/subdirs`
end

##########################################################################################
# EXTRACT TRANSLATED DOCUMENTATION FILES

if !(File.exists?("doc-translated") && File.directory?("doc-translated"))
    Dir.mkdir( "doc-translated" )
end

Dir.chdir( "doc-translated" )

# -- digiKam extraction ------------------------------------------------------

print("digikam: ")

l3makefile = File.new( "CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )

# Get list of docbook files from documentation repository
docbookfiles = `git archive --remote=git://anongit.kde.org/digikam-doc.git HEAD digikam | tar -t | grep docbook | cut -d / -f2 | cut -d . -f1`
list = docbookfiles.split(/\r?\n/)

i18nlangs.each_line do |lang|
    lang.chomp!()

    if (lang != nil && lang != "")

        print("#{lang}")

        if !(File.exists?(lang) && File.directory?(lang))
            Dir.mkdir(lang)
        end

        Dir.chdir(lang)
        Dir.mkdir("digikam")

        # This boolean variable is true if full documentation translation files can be fetch from repository.
        complete = true

        list.each do |part|

            if isWindows
                `svn cat svn://anonsvn.kde.org/home/kde/#{branch}/l10n-kf5/#{lang}/docs/extragear-graphics/digikam/#{part}.docbook > digikam/#{part}.docbook`
            else
                `svn cat svn://anonsvn.kde.org/home/kde/#{branch}/l10n-kf5/#{lang}/docs/extragear-graphics/digikam/#{part}.docbook 2> /dev/null | tee digikam/#{part}.docbook`
            end

            if File.exists?("digikam/#{part}.docbook") and FileTest.size( "digikam/#{part}.docbook" ) == 0
                File.delete( "digikam/#{part}.docbook" )
                complete = false
                break
            end

        end

        if (complete == true)
            makefile = File.new( "CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
            makefile << "KDOCTOOLS_CREATE_HANDBOOK( digikam/index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/#{lang}/ SUBDIR digikam )"
            makefile.close()
        end

        Dir.chdir("..")

        if (complete == true)
            # complete checkout
            l3makefile << "add_subdirectory( #{lang} )\n"
            print(" ")
        else
            # uncomplete checkout
            FileUtils.rm_r(lang)
            print("(u) ")
        end

     end
end

puts ("\n")

# -- Showfoto extraction ------------------------------------------------------

print("showfoto: ")

# Get list of docbook files from documentation repository
docbookfiles = `git archive --remote=git://anongit.kde.org/digikam-doc.git HEAD showfoto | tar -t | grep docbook | cut -d / -f2 | cut -d . -f1`
list = docbookfiles.split(/\r?\n/)

i18nlangs.each_line do |lang|
    lang.chomp!()

    if (lang != nil && lang != "")

        if (File.exists?(lang))

            print("#{lang}")

            Dir.chdir(lang)
            Dir.mkdir("showfoto")

            # This boolean variable is true if full documentation translation files can be fetch from repository.
            complete = true

            list.each do |part|

                if isWindows
                    `svn cat svn://anonsvn.kde.org/home/kde/#{branch}/l10n-kf5/#{lang}/docs/extragear-graphics/showfoto/#{part}.docbook > showfoto/#{part}.docbook`
                else
                    `svn cat svn://anonsvn.kde.org/home/kde/#{branch}/l10n-kf5/#{lang}/docs/extragear-graphics/showfoto/#{part}.docbook 2> /dev/null | tee showfoto/#{part}.docbook`
                end

                if File.exists?("showfoto/#{part}.docbook") and FileTest.size( "showfoto/#{part}.docbook" ) == 0
                    File.delete( "showfoto/#{part}.docbook" )
                    complete = false
                    break
                end

            end

            if (complete == true)
                makefile = File.open( "CMakeLists.txt", "a")
                makefile << "\nKDOCTOOLS_CREATE_HANDBOOK( showfoto/index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/#{lang}/ SUBDIR digikam )"
                makefile.close()
            end

            if (complete == true)
                # complete checkout
                print(" ")
            else
                # uncomplete checkout
                print("(u) ")
            end

            Dir.chdir("..")

        end

     end
end

Dir.chdir("..")
puts ("\n")
