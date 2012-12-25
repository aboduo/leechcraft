/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "inforequestpolicymanager.h"
#include "entrybase.h"
#include "xmlsettingsmanager.h"
#include "roomclentry.h"
#include "roomhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	InfoRequestPolicyManager::InfoRequestPolicyManager (QObject *parent)
	: QObject (parent)
	{
	}

	bool InfoRequestPolicyManager::IsRequestAllowed (InfoRequest req, EntryBase *entry) const
	{
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat)
		{
			if (!XmlSettingsManager::Instance ().property ("RequestVersionInMUCs").toBool ())
				return false;

			auto room = qobject_cast<RoomCLEntry*> (entry->GetParentCLEntry ());
			if (room->GetRoomHandler ()->IsGateway ())
				return false;
		}

		return true;
	}
}
}
}