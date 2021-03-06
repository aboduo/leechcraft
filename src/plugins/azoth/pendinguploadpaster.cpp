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

#include "pendinguploadpaster.h"
#include <QUrl>
#include "interfaces/azoth/iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	PendingUploadPaster::PendingUploadPaster (QObject *sharer,
			ICLEntry *entry, const QString& variant, const QString& filename, QObject *parent)
	: QObject (parent)
	, Entry_ (entry)
	, EntryVariant_ (variant)
	, Filename_ (filename)
	{
		connect (sharer,
				SIGNAL (fileUploaded (QString, QUrl)),
				this,
				SLOT (handleFileUploaded (QString, QUrl)));
	}

	void PendingUploadPaster::handleFileUploaded (const QString& filename, const QUrl& url)
	{
		if (filename != Filename_)
			return;

		const auto msgType = Entry_->GetEntryType () == ICLEntry::ETMUC ?
					IMessage::MTMUCMessage :
					IMessage::MTChatMessage;
		auto msgObj = Entry_->CreateMessage (msgType, EntryVariant_, url.toEncoded ());
		auto msg = qobject_cast<IMessage*> (msgObj);
		msg->Send ();

		deleteLater ();
	}
}
}
