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

require 'release/application.rb'
require 'fileutils'

class PartitionManagerApp < Application
	def initialize(product, component, section, name)
		super
	end

	def applyFixes(workingDir, outputDir)
		puts "applying fixes for partition manager"
 
		Dir.chdir "#{workingDir}/#{outputDir}"

		return if !File.exists? 'doc'

		FileUtils.mv('CMakeLists.txt', 'CMakeLists.txt.orig')

		newFile = File.new('CMakeLists.txt', File::CREAT | File::TRUNC | File::RDWR)

		IO.foreach('CMakeLists.txt.orig')  do |line|
			if line =~ /\s*macro_optional_add_subdirectory\s*\(\s*doc\s*\)\s*/i
				newFile.print <<END_OF_TEXT
if (KDEVERSION VERSION_GREATER "4.1.3")
    macro_optional_add_subdirectory(doc)
else (KDEVERSION VERSION_GREATER "4.1.3")
    message (STATUS "No documentation built for KDE libs < 4.1.4")
endif (KDEVERSION VERSION_GREATER "4.1.3")
END_OF_TEXT
			else
				newFile << line
			end
		end

		newFile.close
	end
end
