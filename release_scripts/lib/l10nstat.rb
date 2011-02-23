# Generic ruby library for KDE extragear/playground releases
#
# Copyright © 2007-2010 Harald Sitter <apachelogger@ubuntu.com>
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

# TODO: there are some rough edges :-S
# TODO: needs to go to seperate file

def l10nstat
@stats   = {}
@sorter  = {}
@counter = {"fuzzy"=>0,"untranslated"=>0,"notshown"=>0,"percentage"=>0.0,"language"=>0}
@file    = File.new("../#{NAME}-l10n-#{@version}.html", File::CREAT | File::RDWR | File::TRUNC )

# write HTML header part
def write_header
    @file.print <<EOT
    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"  "http://www.w3.org/TR/html4/loose.dtd">
    <html>
    <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <title>Statistics of #{NAME} #{@version} translations</title>
    </head>
    <body>
    <a name="__top"><p align="center"><a name="statistics of #{NAME} #{@version} translations">
    <h1>Statistics of #{NAME} #{@version} translations</h1><br/>
    <table border="1" cellspacing="0"dir="ltr">
    <tr><td align="left" valign="middle" width="60" height="12">
    <font color="#196aff"><i><b>Language</b></i></font>
    </td><td align="center" valign="middle" width="142" height="12">
    <font color="#196aff"><i><b>Fuzzy Strings</b></i></font>
    </td><td align="center" valign="middle" width="168" height="12">
    <font color="#196aff"><i><b>Untranslated Strings</b></i></font>
    </td><td align="center" valign="middle" width="163" height="12">
    <font color="#196aff"><i><b>All Not Shown Strings</b></i></font>
    </td><td align="center" valign="middle" width="163" height="12">
    <font color="#196aff"><i><b>Translated %</b></i></font>
    </td></tr>
EOT
end

# write HTML footer part
def write_footer
    @file.print <<EOT
    <tr><td align="left" valign="middle" width="60" height="12">
    <u><i><b>#{@counter["language"]}</b></i></u></td>
    <td align="center" valign="middle" width="142" height="12"><u><i><b>
    #{@counter["fuzzy"]}</b></i></u></td>
    <td align="center" valign="middle" width="168" height="12"><u><i><b>
    #{@counter["untranslated"]}</b></i></u></td>
    <td align="center" valign="middle" width="163" height="12"><u><i><b>
    #{@counter["notshown"]}</b></i></u></td>
    <td align="center" valign="middle" width="163" height="12"><u><i><b>
    #{(@counter["percentage"]/@counter["language"] unless @counter["language"] == 0 ).to_i.to_s + " %"}</b></i></u></td></tr>
EOT
end

def create_stat( lang )
    @name = NAME.split("-").join #strip hyphens (required for kipi-plugins)

    values = nil
    translated = fuzzy = untranslated = 0

    for file in Dir.glob("po/#{lang}/*.po")
        t = f = u = 0
        data = %x[LANG=C msgfmt --statistics #{file} > /dev/stdout 2>&1]

        # tear the data apart and create some variables
        data.split(",").each{|x|
            if x.include? "untranslated"
                u = x.scan(/[\d]+/)[0].to_i #don't ask
            elsif x.include? "fuzzy"
                f = x.scan(/[\d]+/)[0].to_i
            elsif x.include? "translated"
                t = x.scan(/[\d]+/)[0].to_i
            end
        }

        untranslated += u
        fuzzy += f
        translated += t
    end

    notshown = fuzzy + untranslated
    all      = translated + fuzzy + untranslated
    shown    = all - notshown
    per      = ((100.0 * shown.to_f) / all.to_f)

    colortable = [
        100, "#00B015", #green
        95, "#FF9900", #orange
        75, "#6600FF", #blue
        50, "#000000", #black
        0, "#FF0000"  #red
    ]

    match = false; fcolor = ""
    colortable.each_slice(2) { |x| # e.g. [ 100, "#00B015" ]
        if per >= x[0] and not match
            fcolor = x[1]
            match = true
        end
        fcolor
    }

    @stats[lang]  = [fuzzy, untranslated, notshown, per, fcolor]
    @sorter[lang] = per
end

puts "Entering Directory..."
src_dir

puts "Writing the header..."
write_header

@l10n.each {|lang|
    create_stat( lang )
}

# temp debug
p @sorter

puts "Sorting by percentage..."
tmp   = @sorter.sort{|a,b| b[1]<=>a[1]}
order = []
tmp.each_slice(1){|x|
    x.each_slice(2){|y,z|
        order += y
    }
}

puts "Writing the statistics..."
order.each_slice(2){|lang,x|
    @stats[lang].each_slice(5){| fuzzy, untranslated, notshown, per, fcolor |

        if $options[:barrier] and per < $options[:barrier]
            @l10n.delete(lang)
            @counter.delete(lang)
            rm_rf("po/#{lang}")
            puts "WARNING: #{lang} doesn't match the barrier of #{$options[:barrier]}%\n...removed"
            next
        end

        @counter["fuzzy"]        = @counter["fuzzy"] + fuzzy if fuzzy
        @counter["untranslated"] = @counter["untranslated"] + untranslated if untranslated
        @counter["notshown"]     = @counter["notshown"] + notshown if notshown
        @counter["percentage"]   = @counter["percentage"] + per if per
        @counter["language"]     = @counter["language"] + 1

        @file.print <<EOT
        <tr><td align="left" valign="middle" width="60" height="12">
        <font color="#{fcolor}">#{lang}</font></td>
        <td align="center" valign="middle" width="142" height="12">
        <font color="#{fcolor}">#{fuzzy}</font></td>
        <td align="center" valign="middle" width="168" height="12">
        <font color="#{fcolor}">#{untranslated}</font></td>
        <td align="center" valign="middle" width="163" height="12">
        <font color="#{fcolor}">#{notshown}</font></td>
        <td align="center" valign="middle" width="163" height="12">
        <font color="#{fcolor}">#{per.to_i.to_s + " %"}</font></td></tr>
EOT
    }
}

puts "Writing the footer..."
write_footer
@file.close

puts("Creation finished...")
end
