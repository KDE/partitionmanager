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

require 'Qt4'

if not FileTest.exists?('ui_releasedialog.rb') or File::stat('ui_releasedialog.rb').mtime.to_i < File::stat('releasedialog.ui').mtime.to_i
	puts "rebuilding ui_releasedialog.rb..."
	system "rbuic4 releasedialog.ui > ui_releasedialog.rb"
end

require 'ui_releasedialog.rb'
require 'releasebuilder.rb'

class ReleaseDialog < Qt::Dialog
	slots 'on_comboAccess_currentIndexChanged(int)',
		'on_comboCheckout_currentIndexChanged(int)',
		'on_comboName_currentIndexChanged(int)'

	def initialize
		super

		@ui = Ui_ReleaseDialog.new()
		@ui.setupUi(self)

		ReleaseBuilder.apps.sort.each { |key, value| @ui.comboName.addItem(key) }
	end

	def validate
		if @ui.editVersion.text.empty?
			Qt::MessageBox.information(self, tr('Missing Information'), tr('The application\'s version can not be empty.'))
			return false
		end
	
		if @ui.comboAccess.currentIndex != 0 and @ui.editUser.text.empty?
			Qt::MessageBox.information(self, tr('Missing Information'), tr('Please provide a user name for the selected SVN access method.'))
			return false
		end
		
		return true
	end
	
	def accept
		return if not validate

		component = ReleaseBuilder.apps[@ui.comboName.currentText][1]
		section = ReleaseBuilder.apps[@ui.comboName.currentText][2]
		appName = ReleaseBuilder.apps[@ui.comboName.currentText][0]
		version = @ui.editVersion.text
		
		repository = ReleaseBuilder.repository(ReleaseBuilder.apps[@ui.comboName.currentText][0], @ui.comboAccess.currentText, @ui.editUser.text, @ui.comboCheckout.currentText != 'tag' ? @ui.comboCheckout.currentText : @ui.comboTag.currentText)
		
		releaseBuilder = ReleaseBuilder.new(Dir.getwd, repository, component, section, appName, version)
		releaseBuilder.run(@ui.comboAccess.currentText, @ui.editUser.text, @ui.checkTarball.isChecked, @ui.checkTranslations.isChecked, @ui.checkDocs.isChecked, @ui.checkTag.isChecked)
		
		super
	end

private
	def on_comboName_currentIndexChanged(index)
		on_comboCheckout_currentIndexChanged(@ui.comboCheckout.currentIndex)
	end
	
	def on_comboAccess_currentIndexChanged(index)
		@ui.editUser.setEnabled(index > 0)
	end
	
	def on_comboCheckout_currentIndexChanged(index)
		@ui.comboTag.setEnabled(index == 2)
		updateTags or @ui.comboTag.setEnabled(false) if index == 2
	end
	
	def updateTags
		@ui.comboTag.clear
	
		appName = ReleaseBuilder.apps[@ui.comboName.currentText][0]
		
#		tags = `svn ls file://localhost/home/vl/tmp/svn/tags/#{appName}`.chomp!
		tags = `svn ls svn://anonsvn.kde.org/home/kde/tags/#{appName}`.chomp!
		
		return false if not tags or tags.length == 0
		
		tags.sort.each { |t| @ui.comboTag.addItem(t.delete("/\n\r")) }
		return true
	end

end

if __FILE__ == $0
	app = Qt::Application.new(ARGV)
	window = ReleaseDialog.new
	window.show
	app.exec
end
