# Generic ruby library for KDE extragear/playground releases
#
# Copyright (C) 2008-2009 Harald Sitter <apachelogger@ubuntu.com>
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

# Checks if tagging directory is actually available, if not it will create one
def check_tag
    %x[svn ls #{@tag1}]
    %x[svn mkdir -m "Create tag #{NAME} #{@version} root directory" #{@tag1}] if $? != 0
end

# Tags actual source content - 1st step
# * The tag itself will be placed in tags/NAME/VERSION
# * Therefore the source will be in tags/NAME/VERSION/NAME
# * Svn mkdir
# * Svn cp from the downloaded source (librelease)
def tag_source
    src_dir

    @tag1 = "#{@protocol}://#{@user}.kde.org/home/kde/tags/#{NAME}/#{@version}"

    SRCVCS.tagSrc(@tag1)
end

# Tags translation - 2nd step
# * Placed in tags/NAME/VERSION/po
# * Svn co the tag directory
# * Svn mkdir po
# * Svn mkdir TRANSLATION for all TRANSLATIONS (provided by libl10n. So, if no translation fetching did happen, it's going o break here)
# * Svn cp from fetched translations (libl10n)
# TODO: optionalify depend on libl10n
def tag_l10n
    @name = NAME.split("-").join

    src_dir
    check_tag
    `svn co -N #{@tag1} tagging`

    tag = "#{@protocol}://#{@user}.kde.org/home/kde/tags/#{NAME}/#{@version}/po"

    `svn mkdir -m "Create tag #{NAME} #{@version} po directory" #{tag}`
    `svn up tagging/po`
    for l10n in @l10n do
        `svn mkdir tagging/po/#{l10n}`
        for f in Dir.glob("po/#{l10n}/#{@name.chop}*.po")
            `svn cp #{f} tagging/po/#{l10n}/`
        end
    end
    `svn ci -m "Tag #{NAME} #{@version} - localizations." tagging/po`

    FileUtils.rm_rf("tagging")
end

# Tags documentation - 3rd step
# * Placed in tags/NAME/VERSION/doc
# * Svn co tag directory
# * Svn mkdir doc
# * Svn cp DOC for all DOCS (provided by libl10n. So, if no translation fetching did happen, it's going o break here)
def tag_docs
    src_dir
    check_tag
    `svn co -N #{@tag1} tagging`

    tag = "#{@protocol}://#{@user}.kde.org/home/kde/tags/#{NAME}/#{@version}/po"

    `svn mkdir -m "Create tag #{NAME} #{@version} doc directory" #{tag}`
    `svn up tagging/doc`
    for doc in @docs do
        `svn cp doc/#{doc} tagging/doc/`
    end
    `svn ci -m "Tag #{NAME} #{@version} - documentations." tagging/doc`

    FileUtils.rm_rf( "tagging" )
end

# Tagging wrapper
# 1. source
# 2. translations (depends on libl10n)
# 3. documentation (depends on libl10n)
def create_tag
    tag_source
    tag_l10n unless @l10n == nil or @l10n.empty?
    tag_docs unless @docs == nil or @docs.empty?
end
