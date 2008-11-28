=begin
***************************************************************************
*   Copyright (C) 2008 by Volker Lanz <vl@fidra.de>                       *
*   Copyright (C) 2007-2008 by Harald Sitter <harald@getamarok.com>       *
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

require 'fileutils'
require 'application.rb'

class Tagger
	def initialize(repositorySource, repositoryTag, app, version)
		@app = app
		@version = version
		@repositorySource = "#{repositorySource}/#{@app.component}/#{@app.section}/#{@app.name}"
		@repositoryTag = repositoryTag
		@tmpDirName = 'tagging_dir'
	end

	def tagSource
		puts "Tagging source files..."
		system "svn mkdir --parents -m 'Directory for tag #{@version}' #{@repositoryTag} >/dev/null 2>&1"
		system "svn cp -m 'Tag #{@version}' #{@repositorySource} #{@repositoryTag} >/dev/null 2>&1"
		puts "Done tagging source files."
	end

	def tagTranslations(translations)
		return if translations == nil or translations.length == 0

		puts "Tagging translations..."

		system "svn co --depth immediates #{@repositoryTag} #{@tmpDirName} >/dev/null 2>&1"
		system "svn mkdir --parents -m 'Translations for #{@version}' #{@repositoryTag}/po >/dev/null 2>&1"
		system "svn up #{@tmpDirName}/po >/dev/null 2>&1"

		translations.each do |trans|
			system "svn mkdir #{@tmpDirName}/po/#{trans} >/dev/null 2>&1 >/dev/null 2>&1"
			system "svn cp po/#{trans}/#{@app.name}.po #{@tmpDirName}/po/#{trans}/ >/dev/null 2>&1"
		end
		
		system "svn ci -m 'Tag translations for #{@version}' #{@tmpDirName}/po >/dev/null 2>&1"
		
		FileUtils.rm_rf @tmpDirName
		
		puts "Done tagging translations."
	end
	
	def tagDocumentation(docs)
		return if docs == nil or docs.length == 0

		puts "Tagging documentation..."
		
		system "svn co --depth immediates #{@repositoryTag} #{@tmpDirName} >/dev/null 2>&1"
		system "svn mkdir --parents -m 'Documentation for #{@version}' #{@repositoryTag}/doc >/dev/null 2>&1"
		system "svn up #{@tmpDirName}/doc >/dev/null 2>&1"

		docs.each { |doc| system "svn cp doc/#{doc} #{@tmpDirName}/doc/ >/dev/null 2>&1" }
		
		system "svn ci -m 'Tag documentation for #{@version}' #{@tmpDirName}/doc >/dev/null 2>&1"

		FileUtils.rm_rf @tmpDirName
	
		puts "Done tagging documentation."
	end
end
