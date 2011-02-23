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

require 'lib/config'
require 'optparse'

$options = {} if $options == nil
@conf = Config::read(".#{NAME}rc") if File.exist? ".#{NAME}rc"
if @conf == nil
    @conf = Config::read("#{NAME}rc") if File.exist? "#{NAME}rc"
end

def cv(keyword)
    return @conf.value("main",keyword) unless @conf == nil
end

def cb(keyword)
    v = cv(keyword)
    return true if v == "true"
    return false if v == "false"
end

OptionParser.new do |opts|
    opts.banner = "Usage: #{File.basename($0)} [$options]"

    $options[:customsrc] = cv("customsrc")
    opts.on("--src PATH",
        "Defines custom source path to use for fetching") do |cs|
        $options[:customsrc] = cs.to_s
    end

    $options[:gitbranch] = cv("gitbranch")
    opts.on("--git-branch BRANCH",
        "Defines Git branch to release from") do |gb|
        $options[:gitbranch] = gb.to_s
    end

    $options[:branch] = cv("branch")
    opts.on("-b", "--branch BRANCH", [:trunk, :stable, :tag],
        "Select transfer type (trunk, stable, tag)") do |b|
        $options[:branch] = b.to_s
    end

    $options[:version] = cv("version")
    opts.on("-v", "--version VERSION", "Set release version") do |v|
        $options[:version] = v
    end

    $options[:protocol] = cv("protocol")
    opts.on("-p", "--protocol PROTOCOL", [:anonsvn, :https, :ssh],
        "Select protocol type (anonsvn, https, ssh)") do |pr|
        pr = "svn+ssh" if pr == :ssh
        $options[:protocol] = pr.to_s
    end

    $options[:user] = cv("user")
    opts.on("-u", "--user USER", "Set SVN username (not necessary if protocol is anonsvn)") do |u|
        $options[:user] = u
    end

    $options[:changelog] = cv("changelog")
    opts.on("-c", "--changelog CHANGELOG", "Set changelog file name") do |c|
        $options[:changelog] = c
    end

    $options[:barrier] = cv("barrier")
    opts.on("-m", "--min BARRIER", Integer, "Lowest accepted translation completeness (percentage, integer)") do |m|
        $options[:barrier] = m
    end

    $options[:l10n] = cb("l10n")
    opts.on("-l", "--[no-]l10n", "Fetch translations") do |l|
        $options[:l10n] = l
    end

    $options[:stat] = cb("stat")
    opts.on("-s", "--[no-]stat", "Create translation statistics") do |s|
        $options[:stat] = s
    end

    $options[:doc] = cb("doc")
    opts.on("-d", "--[no-]doc", "Fetch documentation") do |d|
        $options[:doc] = d
    end

    $options[:tag] = cb("tag")
    opts.on("-t", "--[no-]tag", "Create tag") do |t|
        $options[:tag] = t
    end

    $options[:app] = cb("app")
    opts.on("-a", "--[no-]app", "Apply application specific changes") do |a|
        $options[:app] = a
    end

    $options[:tar] = cb("tar")
    opts.on("-r", "--[no-]tar", "Create tarball") do |r|
        $options[:tar] = r
    end

    $options[:pkgnotify] = cb("pkgnotify")
    opts.on("--[no-]notification", "Create file containing information for packagers (depends on tarball creation)") do |n|
        $options[:pkgnotify] = n
    end
end.parse!

# DEBUG
# p $options
# p ARGV
