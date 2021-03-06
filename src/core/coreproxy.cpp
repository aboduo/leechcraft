/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "coreproxy.h"
#include <algorithm>
#include <interfaces/ifinder.h>
#include "core.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "iconthemeengine.h"
#include "tagsmanager.h"
#include "entitymanager.h"
#include "config.h"
#include "colorthemeengine.h"
#include "rootwindowsmanager.h"

namespace LeechCraft
{
	CoreProxy::CoreProxy (QObject *parent)
	: QObject (parent)
	, EM_ (new EntityManager (this))
	{
	}

	QNetworkAccessManager* CoreProxy::GetNetworkAccessManager () const
	{
		return Core::Instance ().GetNetworkAccessManager ();
	}

	IShortcutProxy* CoreProxy::GetShortcutProxy () const
	{
		return Core::Instance ().GetShortcutProxy ();
	}

	QModelIndex CoreProxy::MapToSource (const QModelIndex& index) const
	{
		return Core::Instance ().MapToSource (index);
	}

	Util::BaseSettingsManager* CoreProxy::GetSettingsManager () const
	{
		return XmlSettingsManager::Instance ();
	}

	IRootWindowsManager* CoreProxy::GetRootWindowsManager () const
	{
		return Core::Instance ().GetRootWindowsManager ();
	}

	QIcon CoreProxy::GetIcon (const QString& icon, const QString& iconOff) const
	{
		return IconThemeEngine::Instance ().GetIcon (icon, iconOff);
	}

	void CoreProxy::UpdateIconset (const QList<QAction*>& actions) const
	{
		IconThemeEngine::Instance ().UpdateIconSet (actions);
	}

	IColorThemeManager* CoreProxy::GetColorThemeManager () const
	{
		return &ColorThemeEngine::Instance ();
	}

	ITagsManager* CoreProxy::GetTagsManager () const
	{
		return &TagsManager::Instance ();
	}

	QStringList CoreProxy::GetSearchCategories () const
	{
		const QList<IFinder*>& finders = Core::Instance ().GetPluginManager ()->
			GetAllCastableTo<IFinder*> ();

		QStringList result;
		for (QList<IFinder*>::const_iterator i = finders.begin (),
				end = finders.end (); i != end; ++i)
			result += (*i)->GetCategories ();
		result.removeDuplicates ();
		std::sort (result.begin (), result.end ());
		return result;
	}

	int CoreProxy::GetID ()
	{
		return Pool_.GetID ();
	}

	void CoreProxy::FreeID (int id)
	{
		Pool_.FreeID (id);
	}

	IPluginsManager* CoreProxy::GetPluginsManager () const
	{
		return Core::Instance ().GetPluginManager ();
	}

	IEntityManager* CoreProxy::GetEntityManager () const
	{
		return EM_;
	}

	QString CoreProxy::GetVersion () const
	{
		return LEECHCRAFT_VERSION;
	}

	QObject* CoreProxy::GetSelf ()
	{
		return this;
	}

	void CoreProxy::RegisterSkinnable (QAction *act)
	{
		IconThemeEngine::Instance ().UpdateIconSet (QList<QAction*> () << act);
	}

	bool CoreProxy::IsShuttingDown ()
	{
		return Core::Instance ().IsShuttingDown ();
	}
}
