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

#include "connection.h"
#include <QSslSocket>
#include <QTimer>
#include "conversions.h"
#include "packet.h"
#include "exceptions.h"
#include "message.h"
#include "typingmanager.h"
#include "../vaderutil.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	Connection::Connection (QObject *parent)
	: QObject (parent)
	, Socket_ (new QSslSocket (this))
	, PingTimer_ (new QTimer (this))
	, TM_ (new TypingManager (this))
	, Host_ ("94.100.187.24")
	, Port_ (443)
	, IsConnected_ (false)
	{
		connect (Socket_,
				SIGNAL (sslErrors (const QList<QSslError>&)),
				Socket_,
				SLOT (ignoreSslErrors ()));
		connect (Socket_,
				SIGNAL (connected ()),
				this,
				SLOT (greet ()));
		connect (Socket_,
				SIGNAL (readyRead ()),
				this,
				SLOT (tryRead ()));
		connect (Socket_,
				SIGNAL (error (QAbstractSocket::SocketError)),
				this,
				SLOT (handleSocketError (QAbstractSocket::SocketError)));

		connect (PingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handlePing ()));

		connect (&Balancer_,
				SIGNAL (gotServer (QString, int)),
				this,
				SLOT (handleGotServer (QString, int)));
		connect (&Balancer_,
				SIGNAL (error ()),
				this,
				SLOT (connectToStored ()));

		connect (TM_,
				SIGNAL (startedTyping (QString)),
				this,
				SIGNAL (userStartedTyping (QString)));
		connect (TM_,
				SIGNAL (stoppedTyping (QString)),
				this,
				SIGNAL (userStoppedTyping (QString)));
		connect (TM_,
				SIGNAL (needNotify (QString)),
				this,
				SLOT (handleOutTypingNotify (QString)));

		PacketActors_ [Packets::HelloAck] = [this] (HalfPacket hp) { HandleHello (hp); Login (); };
		PacketActors_ [Packets::LoginAck] = [this] (HalfPacket hp) { CorrectAuth (hp); };
		PacketActors_ [Packets::LoginRej] = [this] (HalfPacket hp) { IncorrectAuth (hp); };
		PacketActors_ [Packets::ConnParams] = [this] (HalfPacket hp) { ConnParams (hp); };

		PacketActors_ [Packets::UserInfo] = [this] (HalfPacket hp) { UserInfo (hp); };
		PacketActors_ [Packets::UserStatus] = [this] (HalfPacket hp) { UserStatus (hp); };
		PacketActors_ [Packets::ContactList2] = [this] (HalfPacket hp) { ContactList (hp); };

		PacketActors_ [Packets::WPInfo] = [this] (HalfPacket hp)
		{
			if (RequestedInfos_.contains (hp.Header_.Seq_))
				HandleWPInfo (hp, RequestedInfos_.take (hp.Header_.Seq_));
			else
				qWarning () << Q_FUNC_INFO << "WP info for unknown request";
		};

		PacketActors_ [Packets::MsgAck] = [this] (HalfPacket hp) { IncomingMsg (hp); };
		PacketActors_ [Packets::MsgStatus] = [this] (HalfPacket hp) { MsgStatus (hp); };
		PacketActors_ [Packets::SMSAck] = [this] (HalfPacket hp) { SMSAck (hp); };
		PacketActors_ [Packets::OfflineMsgAck] = [this] (HalfPacket hp) { OfflineMsg (hp); };
		PacketActors_ [Packets::MicroblogRecv] = [this] (HalfPacket hp) { MicroblogRecv (hp); };

		PacketActors_ [Packets::AuthorizeAck] = [this] (HalfPacket hp) { AuthAck (hp); };
		PacketActors_ [Packets::ContactAck] = [this] (HalfPacket hp) { ContactAdded (hp); };

		PacketActors_ [Packets::NewMail] = [this] (HalfPacket hp) { NewMail (hp); };
		PacketActors_ [Packets::MPOPSession] = [this] (HalfPacket hp) { MPOPKey (hp); };
	}

	void Connection::SetTarget (const QString& host, int port)
	{
		Host_ = host;
		Port_ = port;
	}

	void Connection::SetCredentials (const QString& login, const QString& pass)
	{
		Login_ = login;
		Pass_ = pass;
	}

	void Connection::SetUA (const QString& ua)
	{
		UA_ = ua;
	}

	void Connection::handleGotServer (const QString& server, int port)
	{
		qDebug () << Q_FUNC_INFO << server << port;
		Host_ = server;
		Port_ = port;

		connectToStored ();
	}

	void Connection::connectToStored ()
	{
		Socket_->connectToHost (Host_, Port_);
	}

	void Connection::tryRead ()
	{
		PE_ += Read ();

		auto defaultActor = [] (HalfPacket hp)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown packet type"
					<< hp.Header_.MsgType_;
		};

		while (PE_.MayGetPacket ())
		{
			const auto& hp = PE_.GetPacket ();

			try
			{
				PacketActors_.value (hp.Header_.MsgType_, defaultActor) (hp);
			}
			catch (const std::exception& e)
			{
				qDebug () << Q_FUNC_INFO
						<< "error parsing packet:"
						<< e.what ();
			}

			if (Socket_->bytesAvailable ())
				PE_ += Read ();
		}
	}

	bool Connection::IsConnected () const
	{
		return IsConnected_;
	}

	void Connection::Connect ()
	{
		if (Socket_->isOpen ())
			Socket_->disconnectFromHost ();

		Balancer_.GetServer ();
	}

	void Connection::SetState (const EntryStatus& status)
	{
		if (status.State_ == SOffline)
		{
			Disconnect ();
			emit statusChanged (status);
		}
		else if (!IsConnected_ && status.State_ != SOffline)
		{
			Connect ();
			PendingStatus_ = status;
		}
		else if (status.State_ != SOffline)
		{
			Write (PF_.SetStatus (VaderUtil::State2StatusID (status.State_),
						status.StatusString_).Packet_);
			emit statusChanged (status);
		}
	}

	void Connection::RequestInfo (const QString& email)
	{
		auto packet = PF_.RequestInfo (email);
		RequestedInfos_ [packet.Seq_] = email;
		Write (packet.Packet_);
	}

	quint32 Connection::SendMessage (const QString& to, const QString& message)
	{
		auto hp = PF_.Message (0, to, message);
		Write (hp.Packet_);
		return hp.Seq_;
	}

	quint32 Connection::SendSMS (const QString& to, const QString& message)
	{
		auto hp = PF_.Message (MsgFlag::SMS, to, message);
		Write (hp.Packet_);
		return hp.Seq_;
	}

	quint32 Connection::SendSMS2Number (const QString& phone, const QString& message)
	{
		auto hp = PF_.SMS (phone, message);
		Write (hp.Packet_);
		return hp.Seq_;
	}

	void Connection::SendAttention (const QString& to, const QString& message)
	{
		Write (PF_.Message (MsgFlag::Alarm, to, message).Packet_);
	}

	void Connection::SetTypingState (const QString& to, bool isTyping)
	{
		TM_->SetTyping (to, isTyping);
	}

	void Connection::PublishTune (const QString& tune)
	{
		Write (PF_.Microblog (BlogStatus::Music, tune).Packet_);
	}

	void Connection::Authorize (const QString& email)
	{
		Write (PF_.Authorize (email).Packet_);
	}

	quint32 Connection::AddContact (quint32 group, const QString& email, const QString& name)
	{
		const auto& p = PF_.AddContact (0, group, email, name);
		Write (p.Packet_);
		return p.Seq_;
	}

	void Connection::ModifyContact (quint32 contactId, quint32 groupId,
			const QString& email, const QString& name, const QString& phone)
	{
		const auto& p = PF_.ModifyContact (contactId,
				ContactOpFlag::None,
				groupId,
				email,
				name,
				phone.isEmpty () ? " " : phone);
		Write (p.Packet_);
	}

	void Connection::RemoveContact (quint32 id, const QString& email, const QString& name)
	{
		Write (PF_.RemoveContact (id, email, name).Packet_);
	}

	void Connection::RequestAuth (const QString& email, const QString& msg)
	{
		Write (PF_.Message (MsgFlag::Authorize | MsgFlag::NoRecv, email, msg).Packet_);
	}

	quint32 Connection::AddGroup (const QString& group, int groupNum)
	{
		const auto& p = PF_.AddGroup (group, groupNum);
		Write (p.Packet_);
		return p.Seq_;
	}

	void Connection::RequestPOPKey ()
	{
		Write (PF_.RequestKey ().Packet_);
	}

	void Connection::HandleHello (HalfPacket hp)
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO;
#endif
		quint32 timeout;
		FromMRIM (hp.Data_, timeout);

		PingTimer_->start (timeout * 1000);
	}

	void Connection::Login ()
	{
		const quint32 state = PendingStatus_.State_ == SOnline ?
				UserState::Online :
				UserState::Away;
		Write (PF_.Login (Login_, Pass_, state,
					PendingStatus_.StatusString_,
					UA_).Packet_);
	}

	void Connection::CorrectAuth (HalfPacket)
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO;
#endif
		emit statusChanged (PendingStatus_);
	}

	void Connection::IncorrectAuth (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO;
		Str1251 string;
		FromMRIM (hp.Data_, string);
		qDebug () << string;

		Disconnect ();

		emit authenticationError (string);
	}

	void Connection::ConnParams (HalfPacket hp)
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO;
#endif
		quint32 timeout;
		FromMRIM (hp.Data_, timeout);

		PingTimer_->stop ();
		PingTimer_->start (timeout * 1000);
	}

	void Connection::UserInfo (HalfPacket hp)
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO << hp.Data_.size ();
#endif

		QMap<QString, QString> info;
		while (!hp.Data_.isEmpty ())
		{
			try
			{
				Str1251 key;
				Str16 value;
				FromMRIM (hp.Data_, key, value);
				info [key] = value;
			}
			catch (const TooShortBA&)
			{
				break;
			}
		}
#ifdef PROTOCOL_LOGGING
		qDebug () << info;
#endif
	}

	void Connection::UserStatus (HalfPacket hp)
	{
		quint32 statusId = 0;
		Str1251 uri;
		Str16 title, desc;
		Str1251 email;
		quint32 features = 0;
		Str1251 ua;

		FromMRIM (hp.Data_, statusId, uri, title, desc, email, features, ua);

#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO << statusId << email << uri << title << desc << ua;
#endif

		emit userStatusChanged ({-1, 0, statusId, email,
				QString (), QString (), title, desc, features, ua});
	}

	void Connection::ContactList (HalfPacket hp)
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO << hp.Data_.size ();
#endif
		quint32 result = 0;
		FromMRIM (hp.Data_, result);

		switch (result)
		{
		case CLResponse::IntErr:
			qWarning () << Q_FUNC_INFO
					<< "internal server error";
			return;
		case CLResponse::Error:
			qWarning () << Q_FUNC_INFO
					<< "error";
			return;
		case CLResponse::OK:
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown response code"
					<< result;
			return;
		}

		quint32 groupsNum = 0;
		QByteArray gMask, cMask;
		FromMRIM (hp.Data_, groupsNum, gMask, cMask);

#ifdef PROTOCOL_LOGGING
		qDebug () << groupsNum << "groups; masks:" << gMask << cMask;
#endif

		auto skip = [&hp] (const QByteArray& mask)
		{
			for (int i = 0; i < mask.size (); ++i)
				switch (mask [i])
				{
				case 'u':
				{
					quint32 dummy;
					FromMRIM (hp.Data_, dummy);
					break;
				}
				case 's':
				{
					QByteArray ba;
					FromMRIM (hp.Data_, ba);
					break;
				}
				}
		};

		gMask = gMask.mid (2);
		QStringList groups;
		for (quint32 i = 0; i < groupsNum; ++i)
		{
			quint32 flags = 0;
			Str16 name;
			FromMRIM (hp.Data_, flags, name);
			groups << name;

#ifdef PROTOCOL_LOGGING
			qDebug () << "got group" << name << flags;
#endif
			try
			{
				skip (gMask);
			}
			catch (const TooShortBA&)
			{
				qDebug () << "got premature end in additional groups part, but that's OK";
			}
		}
		emit gotGroups (groups);

		cMask = cMask.mid (12);
		QList<ContactInfo> contacts;
		quint32 contactId = 20;
		while (!hp.Data_.isEmpty ())
		{
			try
			{
				quint32 flags = 0, group = 0;
				Str1251 email;
				Str16 alias;
				quint32 serverFlags = 0, status = 0;
				Str1251 phones, statusURI;
				Str16 statusTitle, statusDesc;
				quint32 comSupport = 0;
				Str1251 ua;

				FromMRIM (hp.Data_, flags, group, email, alias, serverFlags,
						status, phones, statusURI, statusTitle, statusDesc, comSupport, ua);

#ifdef PROTOCOL_LOGGING
				qDebug () << "got buddy" << flags << group << email << alias
						<< serverFlags << status << phones << statusURI
						<< statusTitle << statusDesc << comSupport << ua;
#endif

				contacts << ContactInfo { contactId++, group, status, email, phones, alias, statusTitle, statusDesc, comSupport, ua };

				try
				{
					skip (cMask);
				}
				catch (const TooShortBA&)
				{
					qDebug () << "got premature end in additional CL part, but that's OK";
				}
			}
			catch (const TooShortBA&)
			{
				break;
			}
		}
		emit gotContacts (contacts);
	}

	void Connection::HandleWPInfo (HalfPacket hp, const QString& id)
	{
		quint32 status = 0;
		FromMRIM (hp.Data_, status);

#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO << status;
#endif
		if (status != static_cast<quint32> (AnketaInfoStatus::OK))
		{
			if (status == static_cast<quint32> (AnketaInfoStatus::NoUser) ||
				status == static_cast<quint32> (AnketaInfoStatus::DBErr) ||
				status == static_cast<quint32> (AnketaInfoStatus::RateLimit))
				emit gotUserInfoError (id, static_cast<AnketaInfoStatus> (status));
			else
				emit gotUserInfoError (id, AnketaInfoStatus::Other);
			return;
		}

		quint32 colsNum = 0;
		quint32 rowsNum = 0;
		quint32 date = 0;
		FromMRIM (hp.Data_, colsNum, rowsNum, date);
#ifdef PROTOCOL_LOGGING
		qDebug () << colsNum << rowsNum << date;
#endif
		if (rowsNum > 1)
			rowsNum = 1;

		QStringList colsHeaders;

		QVector<bool> unicodes (colsNum, false);
		QSet<QString> unicodeCols;
		unicodeCols << "Nickname" << "FirstName" << "LastName" << "Location" << "status_title" << "status_desc";
		for (quint32 i = 0; i < colsNum; ++i)
		{
			Str1251 str;
			FromMRIM (hp.Data_, str);
			colsHeaders << str;

			if (unicodeCols.contains (str))
				unicodes [i] = true;
		}

#ifdef PROTOCOL_LOGGING
		qDebug () << "got columns:" << colsHeaders;
#endif

		QList<QStringList> rows;
		for (quint32 i = 0; i < rowsNum; ++i)
		{
			QStringList row;
			for (quint32 j = 0; j < colsNum; ++j)
			{
				if (unicodes [j])
				{
					Str16 str;
					FromMRIM (hp.Data_, str);
					row << str;
				}
				else
				{
					Str1251 str;
					FromMRIM (hp.Data_, str);
					row << str;
				}
			}
#ifdef PROTOCOL_LOGGING
			qDebug () << "got row:" << row;
#endif
			rows << row;
		}

		QMap<QString, QString> map;
		const QStringList& row = rows.value (0);
		for (int i = 0, size = std::min (row.size (), colsHeaders.size ());
				i < size; ++i)
			map [colsHeaders.at (i)] = row.at (i);

		emit gotUserInfoResult (id, map);
	}

	void Connection::IncomingMsg (HalfPacket hp)
	{
		quint32 msgId = 0, flags = 0;
		Str1251 from;
		FromMRIM (hp.Data_, msgId, flags, from);

		QByteArray textBA;
		FromMRIM (hp.Data_, textBA);
		const QString& text = (flags & MsgFlag::CP1251) ?
				FromMRIM1251 (textBA) :
				FromMRIM16 (textBA);

		qDebug () << Q_FUNC_INFO << from << text << (flags & MsgFlag::NoRecv);

		if (!(flags & MsgFlag::NoRecv))
			Write (PF_.MessageAck (from, msgId).Packet_);

		if (flags & MsgFlag::Authorize)
			emit gotAuthRequest (from, text);
		else if (flags & MsgFlag::Notify)
			TM_->GotNotification (from);
		else if (flags & MsgFlag::Alarm)
			emit gotAttentionRequest (from, text);
		else if (flags & MsgFlag::Multichat)
		{
		}
		else
			emit gotMessage ({msgId, flags, from, text, QDateTime::currentDateTime ()});
	}

	void Connection::MsgStatus (HalfPacket hp)
	{
		quint32 seq = hp.Header_.Seq_;
		quint32 status = 0;
		FromMRIM (hp.Data_, status);

		if (status == MessageStatus::Delivered)
			emit messageDelivered (seq);
	}

	void Connection::SMSAck (HalfPacket hp)
	{
		quint32 seq = hp.Header_.Seq_;
		quint32 status = 0;
		FromMRIM (hp.Data_, status);

		switch (status)
		{
		case SMSStatus::OK:
			emit smsDelivered (seq);
			break;
		case SMSStatus::ServUnavail:
			emit smsServiceUnavailable (seq);
			break;
		case SMSStatus::InvalidParams:
			emit smsBadParms (seq);
			break;
		}
	}

	namespace
	{
		void EnparseMessage (const QString& msgStr,
				QMap<QString, QString>& headers, QString& text)
		{
			QStringList split = msgStr.split (QRegExp ("(?:\r\n){2,2}"));
			if (split.size () < 2)
				throw MsgParseError ("Incorrect message format " + msgStr.toStdString ());

			const QString& hdr = split.takeFirst ();
			Q_FOREACH (const QString& field, hdr.split ("\n"))
			{
				const int idx = field.indexOf (":");
				if (idx < 0)
				{
					qWarning () << Q_FUNC_INFO
							<< "failed to parse field"
							<< field;
					continue;
				}

				headers [field.left (idx).trimmed ()] = field.mid (idx + 1).trimmed ();
			}

			text = split.join ("\r\n\r\n");
			if (headers.contains ("Boundary"))
			{
				const QString& boundary = headers ["Boundary"];
				const int idx = text.indexOf (boundary);
				if (idx > 0)
					text = text.split (boundary, QString::SkipEmptyParts).first ();
			}
		}

		QString MakeReadableText (const QMap<QString, QString>& headers, const QString& text)
		{
			const QByteArray& arr = headers ["Content-Transfer-Encoding"] == "base64" ?
					QByteArray::fromBase64 (text.toUtf8 ()) :
					text.toUtf8 ();

			const QString& cType = headers ["Content-Type"];
			const QString& sep = "charset=";
			const int idx = cType.indexOf (sep);
			if (idx < 0)
				throw MsgParseError ("Failed to detect charset: " + cType.toStdString ());

			const QString& encoding = cType.mid (idx + sep.length ());
			QTextCodec *codec = QTextCodec::codecForName (encoding.toLatin1 ());
			if (!codec)
				throw MsgParseError ("No codec for encoding " + encoding.toStdString ());

			return codec->toUnicode (arr);
		}
	}

	void Connection::OfflineMsg (HalfPacket hp)
	{
		UIDL id;
		Str1251 message;
		FromMRIM (hp.Data_, id, message);

#ifdef PROTOCOL_LOGGING
		qDebug () << "got offline message";
#endif

		QMap<QString, QString> headers;
		QString rawText;
		EnparseMessage (message, headers, rawText);
		const QString& text = MakeReadableText (headers, rawText);

		QString date = headers ["Date"];
		if (date.right (6).startsWith (" +") || date.right (6).startsWith (" -"))
			date = date.left (date.size () - 6);

		QDateTime dt = QDateTime::fromString (date, "ddd, dd MMM yyyy HH:mm:ss");
		if (dt.isNull ())
			throw MsgParseError ("Failed to parse date/time: " + headers ["Date"].toStdString ());

		Message msg =
		{
			hp.Header_.Seq_,
			headers ["X-MRIM-Flags"].toUInt (0, 16),
			headers ["From"],
			text,
			dt
		};
		emit gotOfflineMessage (msg);

		Write (PF_.OfflineMessageAck (id).Packet_);
	}

	void Connection::MicroblogRecv (HalfPacket hp)
	{
		quint32 flags = 0, dummy = 0;
		Str1251 email;
		Str16 text;
		FromMRIM (hp.Data_, flags, email, dummy, dummy, dummy, text);

		if (flags & BlogStatus::Music)
			emit gotUserTune (email, text);
		else
			qDebug () << Q_FUNC_INFO << email << flags << text;
	}

	void Connection::AuthAck (HalfPacket hp)
	{
		Str1251 from;
		FromMRIM (hp.Data_, from);

		emit gotAuthAck (from);
	}

	void Connection::ContactAdded (HalfPacket hp)
	{
		quint32 status = 0, contactId = 0;
		FromMRIM (hp.Data_, status, contactId);

#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO << hp.Header_.Seq_ << status << contactId;
#endif

		if (status == Proto::ContactAck::Success)
			emit contactAdded (hp.Header_.Seq_, contactId);
		else
			emit contactAdditionError (hp.Header_.Seq_, status);
	}

	void Connection::NewMail (HalfPacket hp)
	{
		quint32 count = 0;
		FromMRIM (hp.Data_, count);
		if (!count)
			return;

		Str1251 from;
		Str1251 subj;
		FromMRIM (hp.Data_, from, subj);
#ifdef PROTOCOL_LOGGING
		qDebug () << from << subj;
#endif

		emit gotNewMail (from, subj);
	}

	void Connection::MPOPKey (HalfPacket hp)
	{
		quint32 status = 0;
		Str1251 key;
		FromMRIM (hp.Data_, status, key);

		if (status == MPOPSession::Success)
			emit gotPOPKey (key);
	}

	void Connection::Disconnect ()
	{
		if (PingTimer_->isActive ())
			PingTimer_->stop ();
		Socket_->disconnectFromHost ();

		PE_.Clear ();

		IsConnected_ = false;
	}

	QByteArray Connection::Read ()
	{
		QByteArray res = Socket_->readAll ();
#ifdef PROTOCOL_LOGGING
		qDebug () << "MRIM READ" << res.toBase64 ();
#endif
		return res;
	}

	void Connection::Write (const QByteArray& ba)
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << "MRIM WRITE" << ba.toBase64 ();
#endif
		Socket_->write (ba);
		Socket_->flush ();
	}

	void Connection::greet ()
	{
		IsConnected_ = true;
		Write (PF_.Hello ().Packet_);
	}

	void Connection::handlePing ()
	{
		Write (PF_.Ping ().Packet_);
	}

	void Connection::handleOutTypingNotify (const QString& to)
	{
		Write (PF_.Message (MsgFlag::Notify, to, " ").Packet_);
	}

	void Connection::handleSocketError (QAbstractSocket::SocketError err)
	{
		qWarning () << Q_FUNC_INFO << err << Socket_->errorString ();
		qWarning () << "was connected with" << Host_ << Port_ << "as" << Login_;
		IsConnected_ = false;
	}
}
}
}
}
