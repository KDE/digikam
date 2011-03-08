##
# Read/Write configuration files, with [sections], keyword = value pairs
#
# Copyright (c) 2005 Dr Balwinder S Dheeman (bsd.SANSPAM@cto.homelinux.net)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc., 675
# Mass Ave, Cambridge, MA 02139, USA.
#
# See Config for class documentation.
#
# History:
#    v0.1.0	alpha code	Sat May  7 23:39:59 IST 2005 bsd
#    v0.1.2	bugfix, rdoc	Sun May 22 01:01:33 IST 2005 bsd

# = Config
#
# == What is This Library?
#
# This library provides functionality to retrieve and, or save human readable
# configuration files which may have sections, keyword and value pairs.
#
#
# == What This Library is NOT?
#
# This library does NOT provide functions to add 'comments' to +[sections]+,
# +keywords+ and, or +value+ pairs as yet; but removes all such comments! Be
# WARNED!!
#
# All values are returned as "Strings"; appropriate conversions might be done
# explicitly wheresoever applicable.
#
# == Examples
#
# Reading in and viewing a config file.
#
#     require 'config'
#
#     conf = Config::read('/etc/yate/h343chan.conf')
#     conf.cat				# -> <<EOT
#
#     [ep]
#       alias = yate
#       ep = true
#       gkclient = false
#       ident = yate
#
#     [gk]
#       interface1 = 203.152.135.66
#       server = false
#
#     [incoming]
#       called = 8989989
#       context = default
#     EOT
#
# Get values:
#
#     conf.value('ep','alias')		# -> "yete"
#     conf.value('ep','ep')		# -> "true"
#     conf.value('gk','interface1')	# -> "203.152.135.66"
#
# Test keyword, section and, or values:
#
#     conf.keyword?('ep','ident')	# -> true
#     conf.keyword?('ep','name')	# -> false
#     conf.section?('outgoing')		# -> false
#     conf.value?('ep','name')		# -> false
#
class Config
    # User definable +operator+, e.g. '=>', ':=' or ':' etc., default is '='.
    attr_accessor :operator

    # Read in +file+ configuration file, with optional +operator+ for keyword
    # values pairs, if ommited '=' (assignment) is used as a +operator+ by
    # default.
    def initialize(file, operator = '=')
	@operator = operator
	@sections = {}

	begin
	    section = ""; keywords = {}
	    File.open(file).readlines.each do |line|
		line.chop!
		next if line =~ /^\s*[!\/;#]|^$/	# Skip comments
		if line =~ /^\[.*\]$/			# Section?
	    	    keywords = {}
		    section = line.gsub(/(^\[)(.*?[^\[])(\]$)/, '\2')
		    next
		elsif line =~ /(.*?)#{@operator}(.*?)/	# Keyword?
		    keyword, value = line.split(/\s*#{@operator}\s*/)
		    keyword.gsub!(/\s/, '')
		    value.gsub!(/^\s|\s$|[#;].*$/, '') if value
		    keywords.store(keyword, value)
		end
		if section.length > 0 and keywords.size > 0
	    	    @sections.store(section, keywords)
		end
	    end
	    true
	rescue Errno::ENOENT
	    STDERR.printf "%s\n", $!
	    false
	rescue Errno::EACCES
	    STDERR.printf "%s\n", $!
	    false
	ensure
	    p @sections if $DEBUG
	end
    end

    # A wrapper method to trick out +Config#new+ and read in a configuration
    # +file+ directly.
    def self.read(file, operator = '=')
	self.new(file, operator)
    end

    # Concatinate to $stdout and, or save configurattion to +file+ file.
    def cat(file = STDOUT)
	begin
	    f = file != STDOUT ? open(file, 'w') : file

	    for section in @sections.keys.sort
		f.printf "\n[%s]\n", section
		keywords = @sections[section]
		for keyword in keywords.keys.sort
		    f.printf "  %s %s %s\n", keyword, @operator, keywords[keyword]
		end
	    end
	    f.close if file != STDOUT
	    true
	rescue
	    STDERR.printf "%s\n", $!
	    false
	end
    end
    alias save cat

    # Iterate through +sections+ of a configuration
    def each(&block)
	@sections.each(&block)
    end

    # Does a configuration has a keyword +keyword+ and, or value +value+ in
    # section +section+?
    def keyword?(section, keyword)
	if section?(section)
	    keywords = @sections[section]
	    keywords.has_key?(keyword)
	end
    end
    alias value? keyword?

    # Add a new section +section+ to configuration.
    def add_section(section)
	if !@sections.has_key?(section)
	    @sections.store(section, {})
	    return true
	end
	false
    end

    # Does configuration has a section +section+?
    def section?(section)
	@sections.has_key?(section)
    end

    # Add a new value +value+ to keyword +keyword+ in section +section+ to
    # configuration.
    def add_value(section, keyword, value)
	if !value?(section, keyword)
	    @sections[section].store(keyword, value)
	    return true
	end
	false
    end

    # Get a value for keyword +keyword+ from section +section+ of
    # configuration.
    def value(section, keyword)
	if section?(section)
	    keywords = @sections[section]
	    if keywords.has_key?(keyword)
		keywords[keyword]
	    end
	end
    end
end

