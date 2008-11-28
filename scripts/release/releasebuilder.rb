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

require 'application.rb'
require 'tagger.rb'
require 'translationstatsbuilder.rb'
require 'fileutils'
require 'getoptlong'

class ReleaseBuilder

private
	Apps = [
		Application.new('Amarok', 'extragear', 'multimedia', 'amarok'),
		Application.new('Digikam', 'extragear', 'graphics', 'digikam'),
		Application.new('Partition Manager', 'extragear', 'sysadmin', 'partitionmanager')
	]

public
	def initialize(workingDir, repository, product, version)
		@workingDir = workingDir
		@repository = repository
		@version = version
		
		@app = ReleaseBuilder.findAppByProduct(product)
		if not @app then raise "Product #{product} not found" end
		
		@outputDir = "#{@app.name}-#{@version}"
		
		FileUtils.rm_rf @outputDir
		FileUtils.rm_rf "#{@outputDir}.tar.bz2"
	end

	def run(protocol, user, createTarball, getTranslations, getDocs, createTag)
		checkoutSource
		translations = checkoutTranslations if getTranslations
		docs = checkoutDocumentation if getDocs
		
		if createTag
			repositoryTags = ReleaseBuilder.repository(@app.product, protocol, user, @version)
			tagger = Tagger.new(@repository, repositoryTags, @app, @version)
			tagger.tagSource
			tagger.tagTranslations(translations)
			tagger.tagDocumentation(docs)
		end
		
		self.createTarball if createTarball
	end

	def checkoutSource
		Dir.chdir @workingDir
		svnDir = "#{@app.component}/#{@app.section}/#{@app.name}"

		puts "Checking out source from #{@repository}/#{svnDir}..."

		system "svn co #{@repository}/#{svnDir} #{@outputDir} >/dev/null 2>&1"
	end

	def checkoutTranslations
		Dir.chdir "#{@workingDir}/#{@outputDir}"
	
		FileUtils.rm_rf 'l10n'
		FileUtils.rm_rf 'po'
		
		Dir.mkdir 'l10n'
		Dir.mkdir 'po'

		subdirs = `svn cat #{@repository}/l10n-kde4/subdirs 2>/dev/null`.chomp!
		translations = []

		subdirs.each do |lang|
			lang.chomp!
			next if lang == 'x-test'

			FileUtils.rm_rf 'l10n'
			system "svn co #{@repository}/l10n-kde4/#{lang}/messages/#{@app.component}-#{@app.section} l10n >/dev/null 2>&1"
			next unless FileTest.exists? "l10n/#{@app.name}.po"

			puts "Adding translations for #{lang}..."
			
			dest = "po/#{lang}"
			Dir.mkdir dest
			
			FileUtils.mv("l10n/#{@app.name}.po", dest)
			FileUtils.mv('l10n/.svn', dest)

			File.open("#{dest}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC) do |f|
				f.print <<END_OF_TEXT
file(GLOB _po_files *.po)
GETTEXT_PROCESS_PO_FILES(#{lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR} ${_po_files})
END_OF_TEXT
			end

			system "svn add #{dest}/CMakeLists.txt >/dev/null 2>&1"
			translations << lang
		end

		if translations.length > 0
			File.open('po/CMakeLists.txt', File::CREAT | File::RDWR | File::TRUNC) do |f|
				f.print <<END_OF_TEXT

find_package(Gettext REQUIRED)

if (NOT GETTEXT_MSGMERGE_EXECUTABLE)
	message(FATAL_ERROR "Please install the msgmerge program from the gettext package.")
endif (NOT GETTEXT_MSGMERGE_EXECUTABLE)

if (NOT GETTEXT_MSGFMT_EXECUTABLE)
	message(FATAL_ERROR "Please install the msgfmt program from the gettext package.")
endif (NOT GETTEXT_MSGFMT_EXECUTABLE)

END_OF_TEXT

				translations.each { |lang| f.print "add_subdirectory(#{lang})\n" }
			end

			File.open('CMakeLists.txt', File::APPEND | File::RDWR) do |f|
				f.print <<END_OF_TEXT
include(MacroOptionalAddSubdirectory)
macro_optional_add_subdirectory(po)
END_OF_TEXT
			end

			TranslationStatsBuilder.new(@app.name, @version, @workingDir, @outputDir).run
		else
			FileUtils.rm_rf 'po'
		end

		FileUtils.rm_rf 'l10n'

		return translations
	end

	def checkoutDocumentation
		Dir.chdir "#{@workingDir}/#{@outputDir}"
	
		system "svn co #{@repository}/#{@app.component}/#{@app.section}/doc/#{@app.name} doc/en_US >/dev/null 2>&1"

		if not File.exists? 'doc/en_US/index.docbook'
			FileUtils.rm_rf 'doc'
			return nil
		end
		
		File.open("doc/en_US/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC) do |f|
			f << "kde4_create_handbook(index.docbook INSTALL_DESTINATION \${HTML_INSTALL_DIR}/en_US/ SUBDIR #{@app.name})\n"
		end

		docs = [ "en_US" ]

		subdirs = `svn cat #{@repository}/l10n-kde4/subdirs 2>/dev/null`.chomp!
		
		subdirs.each do |lang|
			lang.chomp!

			FileUtils.rm_rf 'l10n'
			system "svn co #{@repository}/l10n-kde4/#{lang}/docs/#{@app.component}-#{@app.section}/#{@app.name} l10n >/dev/null 2>&1"
			next unless FileTest.exists? 'l10n/index.docbook'

			puts "Adding documentation for #{lang}..."

			dest = "doc/#{lang}"
			FileUtils.mv('l10n', dest)

			File.open("doc/#{lang}/CMakeLists.txt", File::CREAT | File::RDWR | File::TRUNC) do |f|
				f << "kde4_create_handbook(index.docbook INSTALL_DESTINATION \${HTML_INSTALL_DIR}/#{lang}/ SUBDIR #{@app.name})\n"
			end

			system "svn add doc/#{lang}/CMakeLists.txt >/dev/null 2>&1"
			docs << lang
		end

		File.open('doc/CMakeLists.txt', File::CREAT | File::RDWR | File::TRUNC) do |f|
			docs.each { |lang| f << "add_subdirectory(#{lang})\n" }
		end

		File.open('CMakeLists.txt', File::APPEND | File::RDWR) do |f|
			f << "include(MacroOptionalAddSubdirectory)\n" unless File.exists? 'po'
			f << "macro_optional_add_subdirectory(doc)\n"
		end

		FileUtils.rm_rf 'l10n'

		return docs
	end

	def createTarball
		Dir.chdir @workingDir
	
		tarFileName = "#{@outputDir}.tar.bz2"
		
		system "find #{@outputDir} -name .svn | xargs rm -rf"
		system "tar cfj #{tarFileName} #{@outputDir}"
		
		FileUtils.rm_rf @outputDir
		
		puts "MD5:  " + `md5sum #{tarFileName}`.split[0]
		puts "SHA1: " + `sha1sum #{tarFileName}`.split[0]
	end

	def self.repository(product, protocol, user, tag)
		appName = findAppByProduct(product).name

		if protocol == 'anonsvn'
			protocol = 'svn'
			user = 'anon'
		else
			user += "@"
		end
	
		if tag == 'stable'
			branch = 'branches/stable'
		elsif tag == 'trunk'
			branch = 'trunk'
		else
			branch = "tags/#{appName}/#{tag}"
		end

#		return "file://localhost/home/vl/tmp/svn/#{branch}"
		return "#{protocol}://#{user}svn.kde.org/home/kde/#{branch}"
	end

	def self.apps
		return Apps
	end

	def self.sortedProducts
		rval = []
		Apps.each { |a| rval << a.product }
		return rval.sort
	end

	def self.sortedAppNames
		rval = []
		Apps.each { |a| rval << a.name }
		return rval.sort
	end
	
	def self.findAppByProduct(product)
		Apps.each do |a|
			if a.product == product
				return a
			end
		end
		return nil
	end

	def self.findAppByName(name)
		Apps.each do |a|
			if a.name == name
				return a
			end
		end
		return nil
	end
end

