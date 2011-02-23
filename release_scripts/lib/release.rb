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

def fetch_source()
    rm_rf SRC
    for f in Dir.glob("#{SRC}*.tar.bz2")
        rm_rf f
    end

    SRCVCS.getSrc(@repo,SRC)
    exit_checker($?,"the whole freaking source tree")

    create_changelog()
end

# Creates a changelog using svn2cl.
# Gets invoked from fetchSource once the SVN checkout is finished.
# Adds a header afterwards and commits the updated changelog.
# Expects the constant CHANGELOG to be set and svn2cl to be in your $PATH.
def create_changelog()
# TODO: not initialized
    return unless @changelog

    src_dir()

    if File.exist?(@changelog)
        puts "using given changelog file"
        copy(@changelog, Dir.pwd)
        return
    end

    if not %x[which svn2cl.sh] == ""
        svn2cl = "svn2cl.sh"
    elsif not %x[which svn2cl] == ""
        svn2cl = "svn2cl"
    else
        puts "NO svn2cl in your $PATH, can't generate CHANGELOG!"
        return
    end

    puts("running svn2cl...")
    cl = %x[#{svn2cl} --stdout]

    puts("generating new changelog...")
    file = File.new(@changelog, File::RDWR)
    str  = file.read()

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
private :create_changelog

# Removes all .svn directories, creates a tar.bz2 and removes the source folder.
# You probably want to run this command as one of the last actions, since for
# example tagging heavily depends on the presence of the .svn directories.
def create_tar(suffix=nil,keep=false)
    base_dir()
    puts("creating tarball...")
    if suffix
        folder = SRC + "-" + suffix
        rm_rf(folder)
        cp_r(SRC,folder)
    else
        folder = SRC
    end
    for psvn in Dir.glob("#{folder}/**/.svn")
        FileUtils.rm_rf(psvn)
    end
    for gi in Dir.glob("#{folder}/**/.gitignore")
        FileUtils.rm_rf(gi)
    end
    FileUtils.rm_rf("#{folder}/.git")
    FileUtils.rm_rf("#{folder}/messages.mo")
    system("tar -cf #{folder}.tar #{folder}")
    system("bzip2 -9 #{folder}.tar")
    puts("tarball created for #{folder}...")
    create_checksums("#{folder}.tar.bz2")
    rm_rf(folder) unless keep
end

# Create and output checksums for the created tarball
# * MD5
# * SHA1
def create_checksums(tar)
    @checksums = {} if @checksums == nil

    md5sum = %x[md5sum #{tar}]
    puts("MD5Sum: #{md5sum.split(" ")[0]}")

    sha1sum = %x[sha1sum #{tar}]
    puts("SHA1Sum: #{sha1sum.split(" ")[0]}")

    @checksums[tar] = {
        "MD5Sum" => md5sum,
        "SHA1Sum" => sha1sum,
    }
end

def create_packager_notification()
    filename = "#{NAME}-PackagerNotification-#{@version}.txt"
    puts "Writing #{filename}..."

    file = File.new(filename, File::CREAT | File::RDWR | File::TRUNC)

    # checksum string composer
    unless @checksums == nil
        puts "Parsing checksums..."
        sumstring = "Checksums\n#{sharper}"
        @checksums.each_key{|key|
            @checksums[key].each_pair{|key,value|
                sumstring += "#{key}: #{value.chomp}\n"
            }
            sumstring += "\n"
        }
        file << sumstring + "\n"
    end

    # doc string composer
    unless @docs == nil
        puts "Parsing documentation..."
        docstring = "Documentation (#{@docs.count})\n#{sharper}"
        for doc in @docs
            docstring += doc + " "
        end
        file << docstring + "\n\n"
    end

    # translation string composer
    unless @l10n == nil
        puts "Parsing localization..."
        l10nstring = "Translations (#{@l10n.count})\n#{sharper}"
        for l10n in @l10n
            l10nstring += l10n + " "
        end
        file << l10nstring + "\n\n"
    end

    file.close
    puts "...done"
end

# TODO
def create_mail_announcement()
end

# TODO
def create_changelog_html()
end
