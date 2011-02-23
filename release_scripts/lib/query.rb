# Generic ruby library for KDE extragear/playground releases
#
# Copyright (C) 2007-2010 Harald Sitter <apachelogger@ubuntu.com>
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


def getGitBranch(branch)
    branch = $dlg.inputbox("Enter the Git Branch to release from (e.g. master):") unless branch

    $gitbranch = branch
end

def checkout_location(location)
    location = $dlg.combobox("Select checkout's place:",
    "Trunk Stable Tag") unless location

    if location.downcase == "stable"
        @useStable = true
    elsif location.downcase == "tag"
        @tag = $dlg.inputbox("Enter the tag name:")
    end
end

def release_version(version)
    if @tag and not @tag.empty?
        version = @tag
    else
        version = $dlg.inputbox("Enter the release version:") unless version
    end
    @version = version
end

def svn_protocol(protocol)
    protocol = $dlg.radiolist("Do you use svn+ssh, https or anonsvn :",
    ["svn+ssh","https","anonsvn"],1) unless protocol
    @protocol = protocol
end

def svn_username(user)
    if @protocol == "anonsvn"
        @protocol = "svn"
        user = "anonsvn"
    else
        user = $dlg.inputbox("Your SVN username:") unless user
        user += "@svn"
    end
    @user = user
end

def changelog(cl)
    @changelog = cl if cl
    if @protocol == "anonsvn" \
    or @user == "anonsvn" \
    or not @changelog
        @changelog = nil
        return
    end
    if not File.exists(@changelog)
        unless $dlg.yesno("Create changelog using svn2cl?<br /><br /><b>Note:</b>
        this will commit the changelog right after creating it,<br />so only use this
        feature when you really want to do a release")
            @changelog = nil
        end
    end
end
