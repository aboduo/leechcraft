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

#include "roomclentry.h"
#include <QImage>
#include <QtDebug>
#include <QXmppMucManager.h>
#include <QXmppBookmarkManager.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/azothutil.h>
#include "glooxaccount.h"
#include "glooxprotocol.h"
#include "roompublicmessage.h"
#include "roomhandler.h"
#include "roomconfigwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	namespace
	{
	}

	RoomCLEntry::RoomCLEntry (RoomHandler *rh, bool isAutojoined, GlooxAccount *account)
	: QObject (rh)
	, IsAutojoined_ (isAutojoined)
	, Account_ (account)
	, RH_ (rh)
	, ActionRequestVoice_ (0)
	{
		connect (Account_,
				SIGNAL (statusChanged (const EntryStatus&)),
				this,
				SLOT (reemitStatusChange (const EntryStatus&)));

		Perms_ ["permclass_role"] << "norole";
		Perms_ ["permclass_role"] << "visitor";
		Perms_ ["permclass_role"] << "participant";
		Perms_ ["permclass_role"] << "moderator";

		Perms_ ["permclass_aff"] << "outcast";
		Perms_ ["permclass_aff"] << "noaffiliation";
		Perms_ ["permclass_aff"] << "member";
		Perms_ ["permclass_aff"] << "admin";
		Perms_ ["permclass_aff"] << "owner";

		Role2Str_ [QXmppMucItem::NoRole] = "norole";
		Role2Str_ [QXmppMucItem::VisitorRole] = "visitor";
		Role2Str_ [QXmppMucItem::ParticipantRole] = "participant";
		Role2Str_ [QXmppMucItem::ModeratorRole] = "moderator";

		Aff2Str_ [QXmppMucItem::OutcastAffiliation] = "outcast";
		Aff2Str_ [QXmppMucItem::NoAffiliation] = "noaffiliation";
		Aff2Str_ [QXmppMucItem::MemberAffiliation] = "member";
		Aff2Str_ [QXmppMucItem::AdminAffiliation] = "admin";
		Aff2Str_ [QXmppMucItem::OwnerAffiliation] = "owner";

		Translations_ ["permclass_role"] = tr ("Role");
		Translations_ ["permclass_aff"] = tr ("Affiliation");
		Translations_ ["norole"] = tr ("Kicked");
		Translations_ ["visitor"] = tr ("Visitor");
		Translations_ ["participant"] = tr ("Participant");
		Translations_ ["moderator"] = tr ("Moderator");
		Translations_ ["outcast"] = tr ("Banned");
		Translations_ ["noaffiliation"] = tr ("None");
		Translations_ ["member"] = tr ("Member");
		Translations_ ["admin"] = tr ("Admin");
		Translations_ ["owner"] = tr ("Owner");

		connect (Account_->GetClientConnection ()->GetBMManager (),
				SIGNAL (bookmarksReceived (QXmppBookmarkSet)),
				this,
				SLOT (handleBookmarks (QXmppBookmarkSet)));
	}

	RoomHandler* RoomCLEntry::GetRoomHandler () const
	{
		return RH_;
	}

	QObject* RoomCLEntry::GetQObject ()
	{
		return this;
	}

	QObject* RoomCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features RoomCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType RoomCLEntry::GetEntryType () const
	{
		return ETMUC;
	}

	QString RoomCLEntry::GetEntryName () const
	{
		for (const auto& bm : Account_->GetClientConnection ()->GetBookmarks ().conferences ())
			if (bm.jid () == RH_->GetRoomJID () && !bm.name ().isEmpty ())
				return bm.name ();

		return RH_->GetRoomJID ();
	}

	void RoomCLEntry::SetEntryName (const QString&)
	{
	}

	QString RoomCLEntry::GetEntryID () const
	{
		return Account_->GetAccountID () + '_' + RH_->GetRoomJID ();
	}

	QString RoomCLEntry::GetHumanReadableID () const
	{
		return RH_->GetRoomJID ();
	}

	QStringList RoomCLEntry::Groups () const
	{
		return QStringList () << tr ("Multiuser chatrooms");
	}

	void RoomCLEntry::SetGroups (const QStringList&)
	{
	}

	QStringList RoomCLEntry::Variants () const
	{
		QStringList result;
		result << "";
		return result;
	}

	QObject* RoomCLEntry::CreateMessage (IMessage::MessageType,
			const QString& variant, const QString& text)
	{
		if (variant == "")
			return new RoomPublicMessage (text, this);
		else
			return 0;
	}

	QList<QObject*> RoomCLEntry::GetAllMessages () const
	{
		return AllMessages_;
	}

	void RoomCLEntry::PurgeMessages (const QDateTime& before)
	{
		Util::StandardPurgeMessages (AllMessages_, before);
	}

	void RoomCLEntry::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus RoomCLEntry::GetStatus (const QString&) const
	{
		return Account_->GetState ();
	}

	QList<QAction*> RoomCLEntry::GetActions () const
	{
		QList<QAction*> result;
		RoomParticipantEntry *self = RH_->GetSelf ();
		if (self &&
				self->GetRole () == QXmppMucItem::VisitorRole)
		{
			if (!ActionRequestVoice_)
			{
				ActionRequestVoice_ = new QAction (tr ("Request voice"),
						RH_);
				connect (ActionRequestVoice_,
						SIGNAL (triggered ()),
						RH_,
						SLOT (requestVoice ()));
			}

			result << ActionRequestVoice_;
		}
		return result;
	}

	QImage RoomCLEntry::GetAvatar () const
	{
		return QImage ();
	}

	QString RoomCLEntry::GetRawInfo () const
	{
		return QString ();
	}

	void RoomCLEntry::ShowInfo ()
	{
	}

	QMap<QString, QVariant> RoomCLEntry::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void RoomCLEntry::MarkMsgsRead ()
	{
	}

	IMUCEntry::MUCFeatures RoomCLEntry::GetMUCFeatures () const
	{
		return MUCFCanBeConfigured | MUCFCanInvite;
	}

	QString RoomCLEntry::GetMUCSubject () const
	{
		return RH_->GetSubject ();
	}

	void RoomCLEntry::SetMUCSubject (const QString& subj)
	{
		RH_->SetSubject (subj);
	}

	QList<QObject*> RoomCLEntry::GetParticipants ()
	{
		return RH_->GetParticipants ();
	}

	bool RoomCLEntry::IsAutojoined () const
	{
		return IsAutojoined_;
	}

	void RoomCLEntry::Join ()
	{
		RH_->Join ();
	}

	void RoomCLEntry::Leave (const QString& msg)
	{
		RH_->Leave (msg);
	}

	QString RoomCLEntry::GetNick () const
	{
		return RH_->GetOurNick ();
	}

	void RoomCLEntry::SetNick (const QString& nick)
	{
		RH_->SetOurNick (nick);
	}

	QString RoomCLEntry::GetGroupName () const
	{
		return tr ("%1 participants").arg (RH_->GetRoomJID ());
	}

	QVariantMap RoomCLEntry::GetIdentifyingData () const
	{
		QVariantMap result;
		const QStringList& list = RH_->
				GetRoomJID ().split ('@', QString::SkipEmptyParts);
		const QString& room = list.at (0);
		const QString& server = list.value (1);
		result ["HumanReadableName"] = QString ("%2@%3 (%1)")
				.arg (GetNick ())
				.arg (room)
				.arg (server);
		result ["AccountID"] = Account_->GetAccountID ();
		result ["Nick"] = GetNick ();
		result ["Room"] = room;
		result ["Server"] = server;
		return result;
	}

	QString RoomCLEntry::GetRealID (QObject *obj) const
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (obj);
		if (!entry)
			return QString ();

		const QString& jid = entry->GetRealJID ();
		QString bare;
		QString resource;
		ClientConnection::Split (jid, &bare, &resource);
		return bare;
	}

	void RoomCLEntry::InviteToMUC (const QString& id, const QString& msg)
	{
		RH_->GetRoom ()->sendInvitation (id, msg);
	}

	QMap<QByteArray, QList<QByteArray>> RoomCLEntry::GetPossiblePerms () const
	{
		return Perms_;
	}

	QMap<QByteArray, QList<QByteArray>> RoomCLEntry::GetPerms (QObject *participant) const
	{
		if (!participant)
			participant = RH_->GetSelf ();

		QMap<QByteArray, QList<QByteArray>> result;
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			result ["permclass_role"] << "norole";
			result ["permclass_aff"] << "noaffiliation";
		}
		else
		{
			result ["permclass_role"] << Role2Str_.value (entry->GetRole (), "invalid");
			result ["permclass_aff"] << Aff2Str_.value (entry->GetAffiliation (), "invalid");
		}
		return result;
	}

	QPair<QByteArray, QByteArray> RoomCLEntry::GetKickPerm () const
	{
		return qMakePair<QByteArray, QByteArray> ("permclass_role",
				Role2Str_ [QXmppMucItem::Role::NoRole]);
	}

	QPair<QByteArray, QByteArray> RoomCLEntry::GetBanPerm () const
	{
		return qMakePair<QByteArray, QByteArray> ("permclass_aff",
				Aff2Str_ [QXmppMucItem::Affiliation::OutcastAffiliation]);
	}

	QByteArray RoomCLEntry::GetAffName (QObject *participant) const
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return "noaffiliation";
		}

		return Aff2Str_ [entry->GetAffiliation ()];
	}

	namespace
	{
		bool MayChange (QXmppMucItem::Role ourRole,
				QXmppMucItem::Affiliation ourAff,
				RoomParticipantEntry *entry,
				QXmppMucItem::Role newRole)
		{
			QXmppMucItem::Affiliation aff = entry->GetAffiliation ();
			QXmppMucItem::Role role = entry->GetRole ();

			if (role == QXmppMucItem::UnspecifiedRole ||
					ourRole == QXmppMucItem::UnspecifiedRole ||
					newRole == QXmppMucItem::UnspecifiedRole ||
					aff == QXmppMucItem::UnspecifiedAffiliation ||
					ourAff == QXmppMucItem::UnspecifiedAffiliation)
				return false;

			if (ourRole != QXmppMucItem::ModeratorRole)
				return false;

			if (ourAff < aff)
				return false;

			return true;
		}

		bool MayChange (QXmppMucItem::Role,
				QXmppMucItem::Affiliation ourAff,
				RoomParticipantEntry *entry,
				QXmppMucItem::Affiliation aff)
		{
			if (ourAff < QXmppMucItem::AdminAffiliation)
				return false;

			if (ourAff == QXmppMucItem::OwnerAffiliation)
				return true;

			QXmppMucItem::Affiliation partAff = entry->GetAffiliation ();
			if (partAff >= ourAff)
				return false;

			if (aff >= QXmppMucItem::AdminAffiliation)
				return false;

			return true;
		}
	}

	bool RoomCLEntry::MayChangePerm (QObject *participant,
			const QByteArray& permClass, const QByteArray& perm) const
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return false;
		}

		const QXmppMucItem::Role ourRole =
				RH_->GetSelf ()->GetRole ();
		const QXmppMucItem::Affiliation ourAff =
				RH_->GetSelf ()->GetAffiliation ();

		if (permClass == "permclass_role")
			return MayChange (ourRole, ourAff, entry, Role2Str_.key (perm));
		else if (permClass == "permclass_aff")
			return MayChange (ourRole, ourAff, entry, Aff2Str_.key (perm));
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown perm class"
					<< permClass;
			return false;
		}
	}

	void RoomCLEntry::SetPerm (QObject *participant,
			const QByteArray& permClass,
			const QByteArray& perm,
			const QString& reason)
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return;
		}

		if (permClass == "permclass_role")
			RH_->SetRole (entry, Role2Str_.key (perm), reason);
		else if (permClass == "permclass_aff")
			RH_->SetAffiliation (entry, Aff2Str_.key (perm), reason);
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown perm class"
					<< permClass;
			return;
		}
	}

	bool RoomCLEntry::IsLessByPerm (QObject *p1, QObject *p2) const
	{
		RoomParticipantEntry *e1 = qobject_cast<RoomParticipantEntry*> (p1);
		RoomParticipantEntry *e2 = qobject_cast<RoomParticipantEntry*> (p2);
		if (!e1 || !e2)
		{
			qWarning () << Q_FUNC_INFO
					<< p1
					<< "or"
					<< p2
					<< "is not a RoomParticipantEntry";
			return false;
		}

		return e1->GetRole () < e2->GetRole ();
	}

	bool RoomCLEntry::IsMultiPerm (const QByteArray&) const
	{
		return false;
	}

	QString RoomCLEntry::GetUserString (const QByteArray& id) const
	{
		return Translations_.value (id, id);
	}

	QWidget* RoomCLEntry::GetConfigurationWidget ()
	{
		return new RoomConfigWidget (this);
	}

	void RoomCLEntry::AcceptConfiguration (QWidget *w)
	{
		RoomConfigWidget *cfg = qobject_cast<RoomConfigWidget*> (w);
		if (!cfg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< w
					<< "to RoomConfigWidget";
			return;
		}

		cfg->accept ();
	}

	void RoomCLEntry::HandleMessage (RoomPublicMessage *msg)
	{
		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());
		proxy->PreprocessMessage (msg);

		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void RoomCLEntry::HandleNewParticipants (const QList<ICLEntry*>& parts)
	{
		QObjectList objs;
		Q_FOREACH (ICLEntry *e, parts)
			objs << e->GetQObject ();
		emit gotNewParticipants (objs);
	}

	void RoomCLEntry::HandleSubjectChanged (const QString& subj)
	{
		emit mucSubjectChanged (subj);
	}

	void RoomCLEntry::handleBookmarks (const QXmppBookmarkSet& set)
	{
		for (const auto& bm : set.conferences ())
			if (bm.jid () == RH_->GetRoomJID () && !bm.name ().isEmpty ())
			{
				emit nameChanged (bm.name ());
				break;
			}
	}

	void RoomCLEntry::reemitStatusChange (const EntryStatus& status)
	{
		emit statusChanged (status, QString ());
	}
}
}
}
