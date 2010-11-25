#!/usr/bin/ruby

#
# Ruby script for generating amaroK tarball releases from KDE SVN
#
# (c) 2005 Mark Kretschmann <markey@web.de>
# (c) 2006-2008 Tom Albers <tomalbers@kde.nl>
# (c) 2007 Angelo Naselli <anaselli@linux.it> (command line parameters) 
# Some parts of this code taken from cvs2dist
# License: GNU General Public License V2

require 'optparse'
require 'ostruct'
require 'find'

# check command line parameters
options = OpenStruct.new
options.help  = false
options.https = false
options.ask   = true
options.translations = true

opts = OptionParser.new do |opts|
  opts.on("-u", "--user USERNAME", "svn account") do |u|
    options.username = u
  end
  opts.on("-w", "--https", "Using https instead of svn+ssh") do |w|
    options.https = w
  end
  opts.on("-n", "--noaccount", "Using svn://anonsvn.kde.org/ instead of svn+ssh") do |n|
    options.anonsvn = n
  end
  opts.on("-a", "--application APPL", "Application name (all for all, kde_release for apps that have kde_release=yes)") do |a|
    options.application = a
    options.ask = false
  end
  opts.on("-v", "--version VER", "Overwrite package version set in config.ini") do |v|
    options.ver = v
  end
  opts.on_tail("-h", "--help", "Show this usage statement") do |h|
    options.help = true
  end
  opts.on("-r", "--revision REV", "Use a specific revision of the repository") do |r|
    options.rev = r
  end
  opts.on("-t", "--no-translations", "Don't include translations") do |t|
    options.translations = false
  end
end

begin
  opts.parse!(ARGV)
rescue Exception => e
  puts e, "", opts
  puts
  exit
end

if (options.username)
  username = options.username + "@"
end

if (options.application)
  apps = Array.new
  apps << options.application
end

if (options.https)
  if (username)
    svnbase    = "https://#{username}svn.kde.org/home/kde"
  else
    puts opts
    puts
    puts "Username is mandatory with https"
    exit
  end
else
  svnbase    = "svn+ssh://#{username}svn.kde.org/home/kde"
end

if (options.anonsvn)
  if (options.https)
     puts opts
     puts
     puts "https or anonsvn please, not both"
     exit
   end
   svnbase    = "svn://anonsvn.kde.org/home/kde"
end

if (options.help)
  puts opts
  exit
end

############# START #############

kde_version  = `svn ls svn://anonsvn.kde.org/home/kde/tags/KDE | sort | tail -n1 | cut -d "/" -f1`.chomp
#kde_version = '4.0.4'

#----------------------------------------------------------------
# initiate. 
#----------------------------------------------------------------

f = File.new("config.ini")
app = Array.new
begin
    while (line = f.readline)
        aline = line.chomp
        if aline[0,1] == "[" 
            app << aline[1,(aline.length-2)]
        end
    end
rescue EOFError
    f.close
end

puts "Last KDE version found: " + kde_version
if (options.ask)
    puts "Which apps (multiple sep by space, posibilities: all kde_release " + app.join(" ") + ")?"
    apps = gets.split(" ")
end

kde_release = false;
if apps[0] == "all"
    apps = app
elsif apps[0] == "kde_release"
    apps = app
    kde_release = true;
end

puts "-> Considering " + apps.join(" & ")
if kde_release
    puts " -> Only applications which have kde_release = yes in config "
end
puts

#----------------------------------------------------------------
# retrieve apps. 
#----------------------------------------------------------------

apps.each do |app|
    puts
    puts "-> Processing " + app 

    found = false;
    appdata = Hash.new 
    f = File.new("config.ini")
    begin
        while (line = f.readline)
            aline = line.chomp
            if aline == "[" + app + "]"
                found = true; 
            elsif aline.length > 0 && found
                data = aline.split("=");
                temp = { data[0].strip => data[1].strip }
                appdata = appdata.merge(temp)
            else
                found = false
            end
        end
        rescue EOFError
        f.close
    end

    if (kde_release && appdata["kde_release"] != "yes")
      puts "  -> Skipping because kde_release is not set in the config.ini"
      next
    end

    if (options.ver)
      temp = { "version" => options.ver }
      appdata = appdata.merge(temp)
    else
      if !appdata["version"]
        temp = { "version" => kde_version }
        appdata = appdata.merge(temp)
      else
        if kde_release
           temp = { "version" => appdata["version"] + "-kde" + kde_version }
           appdata = appdata.merge(temp)
        end
      end
    end

    if !appdata["name"]
        temp = { "name" => app }
        appdata = appdata.merge(temp)
    end

    if appdata["folder"]
        app = appdata["folder"]
    end 

    if !appdata["folder"] || appdata["name"]
        temp = { "folder" => appdata["name"] + "-" + appdata["version"] }
    else
        temp = { "folder" => appdata["folder"] + "-" + appdata["version"] }
    end
    appdata = appdata.merge(temp)

    if appdata["addPo"] && appdata["addPo"].length > 0
        temp = { "addPo" =>  (appdata["addPo"]+" "+app).split(" ") }
    else
        temp = { "addPo" =>  app }
    end
    appdata = appdata.merge(temp)

    if appdata["addDocs"] && appdata["addDocs"].length > 0
        temp = { "addDocs" =>  (appdata["addDocs"]+" "+app).split(" ") }
    else
        temp = { "addDocs" =>  app }
    end
    appdata = appdata.merge(temp)

    if appdata["submodule"] && appdata["submodule"].length > 0
        temp = { "submodulepath" => appdata["submodule"] + "/", "l10nmodule" => appdata["mainmodule"] + "-" + appdata["submodule"] }
    else
        temp = { "submodulepath" => "", "l10nmodule" => appdata["mainmodule"] }
    end
    appdata = appdata.merge(temp)

    if !appdata["customlang"]
        temp = { "customlang" => [] }
    end
    appdata = appdata.merge(temp)

    # Preparing
    rev = ""
    revString = ""
    if (options.rev)
      rev = "-r " + options.rev
      revString = " Rev " + options.rev
    end

    puts "-> Fetching " + appdata["mainmodule"] + "/" + appdata["submodulepath"] + app + revString + " into " + appdata["folder"] + "..."
    # Remove old folder, if exists
    `rm -rf #{appdata["folder"]} 2> /dev/null`
    `rm -rf #{appdata["folder"]}.tar.bz2 2> /dev/null`
    Dir.mkdir( appdata["folder"] )
    Dir.chdir( appdata["folder"] )

    if appdata["mainmodule"][0,5] == "trunk" || appdata["mainmodule"][0,8] == "branches" 
        svnroot = "#{svnbase}/"
    else
        #trunk is assumed for all mainmodules that don't start with "trunk" or "branches"
	svnroot = "#{svnbase}/trunk/"
    end

    # Do the main checkouts.
    if appdata["wholeModule"]
        `svn co #{svnroot}/#{appdata["mainmodule"]}/#{appdata["submodulepath"]} #{rev} #{app}-tmp`
    else
        `svn co #{svnroot}/#{appdata["mainmodule"]}/#{appdata["submodulepath"]}#{app} #{rev} #{app}-tmp`
    end
    Dir.chdir( app + "-tmp" )

    if appdata["docs"] != "no"
        `svn co #{svnroot}/#{appdata["mainmodule"]}/#{appdata["submodulepath"]}doc/#{app} #{rev} doc`
    end

    # Move them to the toplevel
    `/bin/mv * ..`
    Dir.chdir( ".." )

    `find -name ".svn" | xargs rm -rf`
    `rm -rf #{app}-tmp`

    if appdata["translations"] != "no" && options.translations
        puts "-> Fetching l10n docs for #{appdata["submodulepath"]}#{app} #{revString}..."

        i18nlangs = `svn cat #{svnroot}/l10n-kde4/subdirs #{rev}`.split
        i18nlangsCleaned = []
        for lang in i18nlangs
            l = lang.chomp
            if (l != "x-test") && (appdata["customlang"].empty? || appdata["customlang"].include?(l))
                i18nlangsCleaned += [l];
            end
        end
        i18nlangs = i18nlangsCleaned

        Dir.mkdir( "doc-translations" )
        topmakefile = File.new( "doc-translations/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )

        Dir.mkdir( "l10n" )
        Dir.chdir( "l10n" )

        # docs
        for lang in i18nlangs
            lang.chomp!

            for dg in appdata["addDocs"].split
                dg.chomp!
                `rm -rf #{dg}`
                docdirname = "l10n-kde4/#{lang}/docs/#{appdata["l10nmodule"]}/#{dg}"
                if ( appdata["docs"] != "no" )
                    puts "  -> Checking if #{lang} has translated documentation...\n"
                    `svn co -q #{rev} #{svnroot}/#{docdirname} > /dev/null 2>&1`
                end
                next unless FileTest.exists?( dg + '/index.docbook' )

                print "    -> Copying #{lang}'s #{dg} documentation over...  "
                Dir.mkdir( "../doc-translations/#{lang}_#{dg}/" )
                `cp -R #{dg}/ ../doc-translations/#{lang}_#{dg}/#{dg}`
                topmakefile << "add_subdirectory( #{lang}_#{dg}/#{dg} )\n"

                makefile = File.new( "../doc-translations/#{lang}_#{dg}/#{dg}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
                makefile << "kde4_create_handbook( index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/#{lang}/)\n"
                Dir.chdir( "../doc-translations/#{lang}_#{dg}/#{dg}")
                `find -name ".svn" | xargs rm -rf`
                Find.find( "." ) do |path|
                    if File.basename(path)[0] == ?.
                        # skip this one...
                    else
                        if FileTest.directory?(path)
                            makefile << "add_subdirectory( " + File.basename(path) + " )\n"
                            submakefile = File.new(  File.basename(path) + "/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
                            submakefile << "kde4_create_handbook( index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/#{lang}/#{dg}/)\n"
                            submakefile.close()
                            Find.prune
                        end
                    end
                end
                Dir.chdir( "../../../l10n")
                makefile.close()

                puts( "done.\n" )
            end
        end
        topmakefile.close()

        # app translations
        puts "-> Fetching l10n po for #{appdata["submodulepath"]}#{app}...\n"

        Dir.chdir( ".." ) # in submodule now

        $subdirs = false
        Dir.mkdir( "po" )

        topmakefile = File.new( "po/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
        for lang in i18nlangs
            lang.chomp!
            dest = "po/#{lang}"

            for dg in appdata["addPo"].split
                dg.chomp!
                if appdata["wholeModule"]
                    print "  -> Copying #{lang}'s over ..\n"
                    pofolder = "l10n-kde4/#{lang}/messages/#{appdata["l10nmodule"]}"
                    `svn co #{svnroot}/#{pofolder} #{dest}`
                    if FileTest.exist?( dest )
                      topmakefile << "add_subdirectory( #{lang} )\n"
                    end
                    next if !FileTest.exist?( dest )
				
                elsif appdata["custompo"]
		                valid = false
                    for sp in appdata["custompo"].split(/,/)
		    	              pofilename = "l10n-kde4/#{lang}/messages/#{appdata["l10nmodule"]}/#{sp}.po"
                    	  `svn cat #{svnroot}/#{pofilename} #{rev} 2> /dev/null | tee l10n/#{sp}.po`
                    	  if not FileTest.size( "l10n/#{sp}.po" ) == 0
	   		                valid=true
                    	  if !FileTest.exist?( dest )
                            Dir.mkdir( dest )
                            topmakefile << "add_subdirectory( #{lang} )\n"
                    	  end
                    	  print "\n  -> Copying #{lang}'s #{sp}.po over ..  "
                    	  `mv l10n/#{sp}.po #{dest}`
			              end
                end
		            next if not valid
                else
                    pofilename = "l10n-kde4/#{lang}/messages/#{appdata["l10nmodule"]}/#{dg}.po"
                    `svn cat #{svnroot}/#{pofilename} #{rev} 2> /dev/null | tee l10n/#{dg}.po`
                    next if FileTest.size( "l10n/#{dg}.po" ) == 0
                    
                    if !FileTest.exist?( dest )
                        Dir.mkdir( dest )
                        topmakefile << "add_subdirectory( #{lang} )\n"
                    end

                    print "  -> Copying #{lang}'s #{dg}.po over ..  "
                    `mv l10n/#{dg}.po #{dest}`
                    puts( "done.\n" )
                end

                makefile = File.new( "#{dest}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC )
                makefile << "file(GLOB _po_files *.po)\n"
                makefile << "GETTEXT_PROCESS_PO_FILES( #{lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR} ${_po_files} )\n"
                makefile.close()
            end
        end
        topmakefile.close()

        `rm -rf l10n`

        # add l10n to compilation.
        `echo "find_package(Msgfmt REQUIRED)" >> CMakeLists.txt`
        `echo "find_package(Gettext REQUIRED)" >> CMakeLists.txt`
        `echo "add_subdirectory( po )" >> CMakeLists.txt`
        if appdata["docs"] != "no"
            `echo "add_subdirectory( doc-translations )" >> CMakeLists.txt`
        end
    end

    # add doc generation to compilation
    if appdata["docs"] != "no"
        `echo "add_subdirectory( doc )" >> CMakeLists.txt`
    end

    # Remove cruft 
    `find -name ".svn" | xargs rm -rf`
    if ( appdata["remove"] != "")
        `/bin/rm -rf #{appdata["remove"]}`
    end

    print "-> Compressing ..  "
    Dir.chdir( ".." ) # root folder
    `tar -jcf #{appdata["folder"]}.tar.bz2 --group=root --owner=root  #{appdata["folder"]}`
    #`rm -rf #{appdata["folder"]}`
    puts " done."
end 

