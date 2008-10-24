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

require 'lib/libkdialog'

@dlg = KDialog.new("#{NAME} release script","cookie")
@changelog = nil

# This will take you to the default execution directory (BASEPATH)
def baseDir()
    Dir.chdir(BASEPATH)
end

# This will take you to the default src directory, which is by default:
#  execution dir + / + src folder name (NAME-VERSION)
def srcDir()
    Dir.chdir(BASEPATH + "/" + @folder)
end

# Queries the executer for all sorts of so called information.
# By default it will query for:
#  - SVN location to checkout (trunk, stable, tag)
#  - Release version
#  - SVN protcol to use (ssh, https, anonsvn)
#  - If protocl is not 'anonsvn' it will also ask for a user name
#  - Whether to use svn2cl or not
#
# You can override the query by providing these information when calling the method.
# For example:
#    InformationQuery("trunk","1.0.0","https","sitter","yes")
def informationQuery(location=nil, version=nil, protocol=nil, user=nil, cl=nil)
    #     @version  = "2.0.0" #DEBUG.
    #     @protocol = "anonsvn" #DEBUG

    unless location
        checkoutLocation()
    else
        checkoutLocationDef(location)
    end

    unless version
        releaseVersion()
    else
        @version = version
    end

    unless protocol
        svnProtcol()
    else
        @protocol = protocol
    end

    unless user
        svnUsername()
    else
        @user = user + "@svn"
    end

    unless cl
        changeLog()
    end
end

private
def checkoutLocation()
    location = @dlg.combobox("Select checkout's place:", "Trunk Stable Tag")
    puts location #DEBUG
    checkoutLocationDef(location)
end

def checkoutLocationDef(location)
    if location == "Stable"
        @useStable = true
    elsif location == "Tag"
        @tag = @dlg.inputbox("Enter the tag name:")
        puts @tag #DEBUG
    end
end

def releaseVersion()
    if @tag and not @tag.empty?()
        @version = @tag
    else
        @version = @dlg.inputbox("Enter the release version:")
    end
    puts @version #DEBUG
end

def svnProtcol()
    @protocol = @dlg.radiolist("Do you use svn+ssh, https or anonsvn :",["svn+ssh","https","anonsvn"],1)
    puts @protocol #DEBUG
end

def svnUsername()
    if @protocol == "anonsvn"
        @protocol = "svn"
        @user = "anonsvn"
    else
        @user = @dlg.inputbox("Your SVN user:")
        @user += "@svn"
    end
    puts @user #DEBUG
end

def changeLog()
    if @protocol == "anonsvn" \
    or @user == "anonsvn" \
    or not @changelog
        @changelog = nil
        return
    end
    unless @dlg.yesno("Create changelog using svn2cl?<br /><br /><b>Note:</b> this will commit the changelog right after creating it,<br />so only use this feature when you really want to do a release")
        @changelog = nil
    else
        puts @changelog #DEBUG
    end
end
