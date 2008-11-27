#!/usr/bin/ruby
=begin
***************************************************************************
*   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
***************************************************************************
=end

require 'releasebuilder.rb'

def usage
	puts <<END_OF_TEXT
#{$0} [options]
where options are:
    --application-name (-a)*
    --version (-v)*
    --checkout-from (-c): trunk, stable, tag
    --tag (-t): name of tag
    --svn-access (-s): (https, svn+ssh, anonsvn)
    --get-docs (-d): also get documentation
    --get-translations (-r): also get translations
    --create-tag (-e): create a new tag
    --create-tarball (-b): create a tarball
    --help (-h): show this usage
Options with an asterisk are required.
Possible values for application-name:
END_OF_TEXT
	ReleaseBuilder.apps.sort.each { |a| puts '"' + a[0] + '"' }
end

opts = GetoptLong.new(
	[ '--application-name', '-a', GetoptLong::REQUIRED_ARGUMENT ],
	[ '--version', '-v', GetoptLong::REQUIRED_ARGUMENT ],
	[ '--checkout-from', '-c', GetoptLong::REQUIRED_ARGUMENT ],
	[ '--tag', '-t', GetoptLong::REQUIRED_ARGUMENT ],
	[ '--svn-access', '-s', GetoptLong::REQUIRED_ARGUMENT ],
	[ '--svn-user', '-u', GetoptLong::REQUIRED_ARGUMENT ],
	[ '--get-docs', '-d', GetoptLong::NO_ARGUMENT ],
	[ '--get-translations', '-r', GetoptLong::NO_ARGUMENT ],
	[ '--create-tag', '-e', GetoptLong::NO_ARGUMENT ],
	[ '--create-tarball', '-b', GetoptLong::NO_ARGUMENT ],
	[ '--help', '-h', GetoptLong::NO_ARGUMENT ]
)

appName = ''
version = ''
checkoutFrom = 'trunk'
tag = ''
protocol = 'anonsvn'
user = ''
getDocs = false
getTranslations = false
createTag = false
createTarball = false

opts.each do |opt, arg|
	case opt
		when '--application-name' then appName = arg
		when '--version' then version = arg
		when '--checkout-from' then checkoutFrom = arg
		when '--tag' then tag = arg
		when '--svn-access' then protocol = arg
		when '--svn-user' then user = arg
		when '--get-docs' then getDocs = true
		when '--get-translations' then getTranslations = true
		when '--create-tag' then createTag = true
		when '--create-tarball' then createTarball = true
		when '--help' then usage; exit
	end
end

if appName.empty? or version.empty?
	puts "Application and version can not be empty."
	exit
end

if not ReleaseBuilder.apps.key?(appName)
	puts "Unknown application '#{appName}'"
	exit
end

if protocol != 'anonsvn' and user.empty?
	puts "The selected SVN access protocol requires a user name."
	exit
end

if checkoutFrom == 'tag' and tag.empty?
	puts "Cannot check out from tag dir if tag is empty."
	exit
end

component = ReleaseBuilder.apps[appName][1]
section = ReleaseBuilder.apps[appName][2]

repository = ReleaseBuilder.repository(ReleaseBuilder.apps[appName][0], protocol, user, checkoutFrom != 'tag' ? checkoutFrom : tag)

releaseBuilder = ReleaseBuilder.new(Dir.getwd, repository, component, section, ReleaseBuilder.apps[appName][0], version)
releaseBuilder.run(protocol, user, createTarball, getTranslations, getDocs, createTag)

