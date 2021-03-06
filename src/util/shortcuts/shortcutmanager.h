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

#pragma once

#include <QObject>
#include <QHash>
#include <util/utilconfig.h>
#include "interfaces/ihaveshortcuts.h"
#include "interfaces/core/icoreproxy.h"

class QAction;
class QShortcut;
class IShortcutProxy;

namespace LeechCraft
{
namespace Util
{
	/** @brief Aids in providing configurable shortcuts.
	 *
	 * This class serves as a "collector" for different QActions and
	 * QShortcuts. One typically instantiates an object of this class
	 * as a per-plugin global object (via a singleton, for example),
	 * registers all required actions via the RegisterShortcut(),
	 * RegisterAction() and RegisterActionInfo() functions and relays
	 * calls to the IHaveShortcuts::SetShortcut() and
	 * IHaveShortcuts::GetActionInfo() functions to this class.
	 *
	 * Though one can register actions at arbitrary points of time,
	 * only those "kinds" of actions registered during the IInfo::Init()
	 * will be visible to the LeechCraft core. Actions added later will
	 * still have customized shortcuts (if any), but only if another
	 * action with the same ID has been added during IInfo::Init().
	 *
	 * See the documentation for IHaveShortcuts for more information
	 * about actions and their IDs.
	 *
	 * @sa IHaveShortcuts
	 */
	class ShortcutManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr CoreProxy_;
		QObject *ContextObj_;

		QHash<QString, QList<QAction*>> Actions_;
		QHash<QString, QList<QShortcut*>> Shortcuts_;

		QMap<QString, ActionInfo> ActionInfo_;
	public:
		/** @brief Creates the shortcut manager.
		 *
		 * @param[in] proxy The proxy object passed to IInfo::Init() of
		 * your plugin.
		 * @param[in] parent The parent object of this object.
		 */
		UTIL_API ShortcutManager (ICoreProxy_ptr proxy, QObject *parent = 0);

		/** @brief Sets the plugin instance object of this manager.
		 *
		 * The plugin instance object serves as a kind of "context" for
		 * the shortcut manager.
		 *
		 * @param[in] pluginObj The plugin instance object.
		 */
		UTIL_API void SetObject (QObject *pluginObj);

		/** @brief Registers the given QAction by the given id.
		 *
		 * This function registers the given action at the given id and
		 * updates it if necessary. The ActionInfo structure is created
		 * automatically, and \em ActionIcon property of the action is
		 * used to fetch its icon.
		 *
		 * If update is set to true, the shortcut manager will request an
		 * actual and up-to-date shortcut from the LeechCraft core. There
		 * is no need to update an action registered during IInfo::Init()
		 * as the Core will update actions with custom shortcuts later on
		 * anyway.
		 *
		 * @note Updating the action (setting update to true) will
		 * trigger the Core to request the action map, and all new IDs
		 * past this point won't be known to LeechCraft core. Thus as a
		 * rule of thumb one should only set update to true only in calls
		 * happening after IInfo::Init().
		 *
		 * @param[in] id The ID of action to register.
		 * @param[in] action The action to register.
		 * @param[in] update Whether action shortcut should be updated
		 * immediately.
		 *
		 * @sa RegisterShortcut(), RegisterActionInfo()
		 */
		UTIL_API void RegisterAction (const QString& id, QAction *action, bool update = false);

		/** @brief Registers the given QShortcut with the given id.
		 *
		 * If update is set to true, the shortcut manager will request an
		 * actual and up-to-date shortcut from the LeechCraft core. There
		 * is no need to update an action registered during IInfo::Init()
		 * as the Core will update actions with custom shortcuts later on
		 * anyway.
		 *
		 * @note Updating the action (setting update to true) will
		 * trigger the Core to request the action map, and all new IDs
		 * past this point won't be known to LeechCraft core. Thus as a
		 * rule of thumb one should only set update to true only in calls
		 * happening after IInfo::Init().
		 *
		 * @param[in] id The ID of QShortcut to register.
		 * @param[in] info The additional ActionInfo about this shortcut.
		 * @param[in] shortcut The QShortcut to register.
		 * @param[in] update Whether action shortcut should be updated
		 * immediately.
		 *
		 * @sa RegisterAction(), RegisterActionInfo()
		 */
		UTIL_API void RegisterShortcut (const QString& id,
				const ActionInfo& info, QShortcut *shortcut, bool update = false);

		/** @brief Registers the given action info with the given id.
		 *
		 * This function can be used to register an action info with the
		 * given ID before any actions or shortcuts with this ID are
		 * really created. This function can be used, for example, to
		 * register shortcuts that will be available during some time
		 * after IInfo::Init(), like a reload action in a web page (as
		 * there are no web pages during plugin initialization).
		 *
		 * @param[in] id The ID of an action or QShortcut to register.
		 * @param[in] info The ActionInfo about this shortcut.
		 *
		 * @sa RegisterAction(), RegisterShortcut()
		 */
		UTIL_API void RegisterActionInfo (const QString& id, const ActionInfo& info);

		/** @brief Sets the key sequence for the given action.
		 *
		 * This function updates all the registered actions with the
		 * given ID. It is intended to be called only from
		 * IHaveShortcuts::SetShortcut(), user code should hardly ever
		 * need to call it elsewhere.
		 *
		 * @param[in] id The ID of the action to update.
		 * @param[in] sequences The list of sequences to for the action.
		 */
		UTIL_API void SetShortcut (const QString& id, const QKeySequences_t& sequences) const;

		/** @brief Returns the map with information about actions.
		 *
		 * The return result is suitable to be returned from
		 * IHaveShortcuts::GetActionInfo().
		 *
		 * @return Action info map.
		 */
		UTIL_API QMap<QString, ActionInfo> GetActionInfo () const;

		typedef QPair<QString, QAction*> IDPair_t;
		/** @brief Utility function equivalent to RegisterAction().
		 *
		 * This function is equivalent to calling
		 * <code>RegisterAction (pair.first, pair.second);</code>.
		 *
		 * @param[in] pair The pair of action ID and the action itself.
		 * @return The shortcut manager object.
		 */
		UTIL_API ShortcutManager& operator<< (const QPair<QString, QAction*>& pair);
	private slots:
		void handleActionDestroyed ();
		void handleShortcutDestroyed ();
	};
}
}
