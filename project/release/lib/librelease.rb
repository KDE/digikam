#!/usr/bin/env ruby
#
# Generic ruby library for KDE extragear/playground releases
#
# Copyright (C) 2007-2008 Harald Sitter <harald@getamarok.com>
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

# DOC TODO
def fetchSource()
    bar  = @dlg.progressbar("fetching source code",1)
    FileUtils.rm_rf( @folder )
    FileUtils.rm_rf( "#{@folder}.tar.bz2" )

    branch = "trunk"

    if @useStable
        branch = "branches/stable"
    elsif @tag and not @tag.empty?()
        branch = "tags/#{NAME}/#{@tag}"
    end

    @repo = "#{@protocol}://#{@user}.kde.org/home/kde/#{branch}"
    #   @repo = "file:///home/kde/#{branch}"
    puts @repo #DEBUG

    puts "Fetching source from #{branch}...\n\n"
    # TODO: ruby-svn
    system("svn co #{@repo}/#{COMPONENT}/#{SECTION}/#{NAME} #{@folder}")

    bar.progress = 1
    bar.close
end

# Removes all .svn directories, creates a tar.bz2 and removes the source folder.
# You probably want to run this command as one of the last actions, since for
# example tagging heavily depends on the presence of the .svn directories.
def createTar()
    baseDir()
    bar  = @dlg.progressbar("creating tarball",4)
    system("find #{@folder} -name .svn | xargs rm -rf")
    bar.progress = 1
    system("tar -cf #{@folder}.tar #{@folder}")
    bar.progress = 2
    system("bzip2 #{@folder}.tar")
    bar.progress = 3
    FileUtils.rm_rf(@folder)
    bar.close
end

# Create and output checksums for the created tarball
# * MD5
# * SHA1
def createCheckSums()
    @md5sum = system("md5sum #{@folder}.tar.bz2")
    puts("MD5Sum: #{@md5sum}")

    @sha1sum = `sha1sum #{@folder}.tar.bz2`
    puts("SHA1Sum: #{@sha1sum}")
end

# TODO
def createMailNotification()
end

# TODO
def createMailAnnouncement()
end

# TODO
def createChangeLogHtml()
end
