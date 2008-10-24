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

    createChangeLog()
end

# Creates a changelog using svn2cl.
# Gets invoked from fetchSource once the SVN checkout is finished.
# Adds a header afterwards and commits the updated changelog.
# Expects the constant CHANGELOG to be set and svn2cl to be in your $PATH.
def createChangeLog()
# TODO: not initialized
    return unless @changelog
    if not %x[which svn2cl.sh] == ""
        svn2cl = "svn2cl.sh"
    elsif not %x[which svn2cl] == ""
        svn2cl = "svn2cl"
    else
        puts "NO svn2cl in your $PATH, can't generate CHANGELOG!"
        return
    end

    srcDir()

    puts("running svn2cl...")
    cl = %x[#{svn2cl} --stdout]

    puts("generating new changelog...")
    file = File.new(@changelog, File::RDWR)
    str = file.read()

# TODO: .include isn't precise enough, we need to have a complete match of linestart-version-whitespace-isodate
#     if str.include?(@version)
#         puts "ChangeLog already lists #{@version}, aborting svn2cl update!"
#         file.close()
#         return
#     end

    file.rewind()
    file.truncate( 0 )

    lines   = cl.split("\n")
    escape  = lines.index(str.split("\n")[2])
    counter = 1

    file << "#{@version} #{Time.now.utc.strftime("%Y-%m-%d")}\n"
    file << "--------------------------------------------------------------------------------\n\n"
    for line in lines
        if counter < escape
            file << line + "\n"
            counter += 1
        end
    end
    file << "\n"
    file << str

    file.close()

    puts("committing changelog...")
    %x[svn ci ChangeLog "Update changelog for #{@version}."]
end

# Removes all .svn directories, creates a tar.bz2 and removes the source folder.
# You probably want to run this command as one of the last actions, since for
# example tagging heavily depends on the presence of the .svn directories.
def createTar()
    puts("creating tarball...")
    baseDir()
    system("find #{@folder} -name .svn | xargs rm -rf")
    system("tar -cf #{@folder}.tar #{@folder}")
    system("bzip2 #{@folder}.tar")
    FileUtils.rm_rf(@folder)
    puts("tarball created...")
end

# Create and output checksums for the created tarball
# * MD5
# * SHA1
def createCheckSums()
    puts("#########################################")

    @md5sum = %x[md5sum #{@folder}.tar.bz2]
    puts("MD5Sum: #{@md5sum.split(" ")[0]}")

    @sha1sum = %x[sha1sum #{@folder}.tar.bz2]
    puts("SHA1Sum: #{@sha1sum.split(" ")[0]}")
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
