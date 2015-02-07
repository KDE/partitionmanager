/*************************************************************************
 *  Copyright (C) 2010 by Volker Lanz <vl@fidra.de>                      *
 *                                                                       *
 *  This program is free software; you can redistribute it and/or        *
 *  modify it under the terms of the GNU General Public License as       *
 *  published by the Free Software Foundation; either version 3 of       *
 *  the License, or (at your option) any later version.                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 *************************************************************************/

#if !defined(COREBACKENDMANAGER__H)

#define COREBACKENDMANAGER__H

#include "util/libpartitionmanagerexport.h"

#include <KService>

class QString;
class QStringList;
class CoreBackend;

/**
  * The backend manager class.
  *
  * This is basically a singleton class to give the application access to the currently
  * selected backend and also to manage the available backend plugins.
  * @author Volker Lanz <vl@fidra.de>
  */
class LIBPARTITIONMANAGERPRIVATE_EXPORT CoreBackendManager
{
	private:
		CoreBackendManager();

	public:
		/**
		  * @return pointer to ourselves
		  */
		static CoreBackendManager* self();

		/**
		  * @return the name of the default backend plugin
		  */
		static QString defaultBackendName() { return QStringLiteral("pmlibpartedbackendplugin"); }

		/**
		  * @return a list of available backend plugins
		  */
		KService::List list() const;

		/**
		   * Loads the given backend plugin into the application.
		   * @param name the name of the plugin to load
		   * @return true on success
		   */
		bool load(const QString& name);

		/**
		  * Unload the current plugin.
		  */
		void unload();

		/**
		  * @return a pointer to the currently loaded backend
		  */
		CoreBackend* backend() { return m_Backend; }

	private:
		CoreBackend* m_Backend;
};

#endif
