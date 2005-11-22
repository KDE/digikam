#!/usr/bin/env ruby
#
# Ruby script for generating amaroK tarball releases from KDE SVN
#
# (c) 2005 Mark Kretschmann <markey@web.de>
# Some parts of this code taken from cvs2dist
# License: GNU General Public License V2


name       = "digikamimageplugins"
egmodule   = "graphics"
version    = "doc-0.8.0"
docs       = "yes"

svnbase    = "https://toma@svn.kde.org/home/kde"
svnroot    = "#{svnbase}/trunk"
adminroot  = "#{svnbase}/branches/KDE/3.5"

addDocs    = [""]
addPo      = [""]

#----------------------------------------------------------------

folder     = name + "-" + version
addPo      = [name] + addPo
addDocs    = [name] + addDocs

# Prevent using unsermake
oldmake = ENV["UNSERMAKE"]
ENV["UNSERMAKE"] = "no"

puts "Fetching #{egmodule}/#{name}..."
# Remove old folder, if exists
`rm -rf #{folder} 2> /dev/null`
`rm -rf folder.tar.bz2 2> /dev/null`

Dir.mkdir( folder )
Dir.chdir( folder )

`svn co -N #{svnroot}/extragear/#{egmodule}`
Dir.chdir( egmodule )
`svn up -N doc`

if ( docs != "no")
    for dg in addDocs
        dg.chomp!
        `svn up doc/#{dg}`
    end
end

`svn co #{adminroot}/kde-common/admin`
puts "done\n"

puts "\n"
puts "Fetching l10n docs for #{egmodule}/#{name}...\n"
puts "\n"

i18nlangs = `svn cat #{svnroot}/l10n/subdirs`
i18nlangsCleaned = []
for lang in i18nlangs
  l = lang.chomp
  if (l != "xx")
    i18nlangsCleaned += [l];
  end
end
i18nlangs = i18nlangsCleaned

Dir.mkdir( "l10n" )
Dir.chdir( "l10n" )

# docs
for lang in i18nlangs
  lang.chomp!

  for dg in addDocs
    dg.chomp!
    `rm -rf #{dg}`
    docdirname = "l10n/#{lang}/docs/extragear-#{egmodule}/#{dg}"
    if ( docs != "no")
        `svn co -q #{svnroot}/#{docdirname} > /dev/null 2>&1`
    end
    next unless FileTest.exists?( dg )
    print "Copying #{lang}'s #{dg} documentation over...  "
    `cp -R #{dg}/ ../doc/#{lang}_#{dg}`

    # we don't want KDE_DOCS = AUTO, cause that makes the
    # build system assume that the name of the app is the
    # same as the name of the dir the Makefile.am is in.
    # Instead, we explicitly pass the name..
    makefile = File.new( "../doc/#{lang}_#{dg}/Makefile.am", File::CREAT | File::RDWR | File::TRUNC )
    makefile << "KDE_LANG = #{lang}\n"
    makefile << "KDE_DOCS = #{dg}\n"
    makefile.close()

    puts( "done.\n" )
  end
end

Dir.chdir( ".." ) # root folder
`rm -rf l10n`

# Remove SVN data folder
`find -name ".svn" | xargs rm -rf`

`/bin/mv * ..`
Dir.chdir( ".." ) # name-version
`rmdir #{egmodule}`

puts "\n"
`rm Mainpage.dox`


# Generate makefiles
`find | xargs touch`

puts "\n"
puts "Generating Makefiles..  "
`make -f Makefile.cvs`
puts "done.\n"

`rm -rf autom4te.cache`
`rm stamp-h.in`

puts "\n"
puts "Compressing..  "
Dir.chdir( ".." ) # root folder
`tar -jcf #{folder}.tar.bz2 #{folder}`
`rm -rf #{folder}`
puts "done.\n"


ENV["UNSERMAKE"] = oldmake
