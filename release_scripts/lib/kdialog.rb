# = kdialog.rb
#
# KDE kdialog wrapper class.
#
# Author:: Ed Hames
# Documentation:: Ed Hames
# License::
#   This software is distributed under the terms of the GPL.
# Revision:: kdialog.rb 0.3

class KDialog

    YES    = 0
    NO     = 256
    CANCEL = 512

    @@exit_status = { YES => true, NO => false, CANCEL => nil}

    attr_reader :selection
    attr_writer :icon

    def initialize(title='', icon='')
        @dialog = 'kdialog '
        self.backtitle = title
        self.icon = icon
        self
    end

    def backtitle=(title)
        @backtitle = title.gsub(/\s/, '\ ')
    end

    def yesno(text)
        exit = perform('--yesno', text)[1]
        @selection = @@exit_status[exit]
    end

    def warningyesno(text)
        exit = perform('--warningyesno', text)[1]
        @selection = @@exit_status[exit]
    end

    def warningcontinuecancel(text)
        exit = perform('--warningcontinuecancel', text)[1]
        @selection = @@exit_status[exit]
    end

    def error(text)
        exit = perform('--error', text)[1]
        @selection = @@exit_status[exit]
    end

    def msgbox(text)
        exit = perform('--msgbox', text)[1]
        @selection = @@exit_status[exit]
    end

    def inputbox(text, init = '')
        (selection, exit) = perform('--inputbox', text, init.gsub(/\s/, '\ '))
        @selection = @@exit_status[exit] ? selection.chomp : init
    end

    def password(text)
        (selection, exit) = perform('--password', text)
        @selection = @@exit_status[exit] ? selection.chomp : ''
    end

    def combobox(text, args)
        (selection, exit) = perform('--combobox', text, args)
        @selection = @@exit_status[exit] ? selection.chomp : ''
    end

    def checklist(text, on_list, off_list)
        hash = Hash.new
        on_list.each{|l| hash[l] = " #{l} #{l} on"}
        off_list.each{|l| hash[l] = " #{l} #{l} off"}

        args = ''
        hash.keys.sort.each{|k| args += hash[k]}
        (selection, exit) = perform('--checklist', text, args)
        @selection = @@exit_status[exit] ? selection.delete('"').split(/\s/) : []
    end

    def radiolist(text, list, selected)
        args = ''
        list.sort.each{|l|
            args += " #{l} #{l} " + ((l == selected) ? "on" : "off")
        }
        (selection, exit) = perform('--radiolist', text, args)
        @selection = @@exit_status[exit] ? selection.chomp : ''
    end

    def passivepopup(text, timeout)
        exit = perform('--passivepopup', text, timeout)[1]
        @selection = @@exit_status[exit]
    end

    def progressbar(text, steps)
        dbus_ref = perform('--progressbar', text, steps)[0]
        bar = ProgressBar.new(dbus_ref)
    end

    def perform(cmd, cmd_text, args='')
        dlg_cmd = @dialog
        dlg_cmd += " --title #{@backtitle} " unless @backtitle.empty?
        dlg_cmd += " --icon #{@icon} " unless @icon.empty?
        dlg_cmd += " #{cmd} \"#{cmd_text}\" #{args}"
            puts "Executing: '#{dlg_cmd}'"
        selection = `#{dlg_cmd}`
        [selection, $?.to_i]
    end
    private :perform
end

class ProgressBar
    def initialize(dbus_ref)
        @dbus_ref = dbus_ref.chop!
    end

    def maxvalue=(maxvalue)
        `qdbus #{@dbus_ref} Set org.kde.kdialog.ProgressDialog maximum #{maxvalue}`
    end

    def progress=(value)
        `qdbus #{@dbus_ref} Set org.kde.kdialog.ProgressDialog value #{value}`
    end

    def label=(label)
        `qdbus #{@dbus_ref} setLabelText #{label.gsub(/\s/, '\ ')}`
    end

    def close
        `qdbus #{@dbus_ref} org.kde.kdialog.ProgressDialog.close`
    end
end

class IconGroup
    Desktop     = "Desktop"
    Toolbar     = "Toolbar"
    MainToolbar = "MainToolbar"
    Small       = "Small"
    Panel       = "Panel"
    User        = "User"
end

class IconContext
    Devices      = "Devices"
    MimeTypes    = "MimeTypes"
    FileSystems  = "FileSystems"
    Applications = "Applications"
    Actions      = "Actions"
end
