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

#include "composemessagetab.h"
#include <QToolBar>
#include <QWebFrame>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QInputDialog>

#ifdef HAVE_MAGIC
#include <magic.h>
#endif

#include <util/util.h>
#include <interfaces/itexteditor.h>
#include <interfaces/core/ipluginsmanager.h>
#include "message.h"
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	QObject *ComposeMessageTab::S_ParentPlugin_ = 0;
	TabClassInfo ComposeMessageTab::S_TabClassInfo_;

	void ComposeMessageTab::SetParentPlugin (QObject *obj)
	{
		S_ParentPlugin_ = obj;
	}

	void ComposeMessageTab::SetTabClassInfo (const TabClassInfo& info)
	{
		S_TabClassInfo_ = info;
	}

	ComposeMessageTab::ComposeMessageTab (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar (tr ("Compose tab bar")))
	, MsgEditWidget_ (0)
	, MsgEdit_ (0)
	{
		Ui_.setupUi (this);

		QAction *send = new QAction (tr ("Send"), this);
		send->setProperty ("ActionIcon", "mail-send");
		connect (send,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSend ()));
		Toolbar_->addAction (send);

		AccountsMenu_ = new QMenu (tr ("Accounts"));
		AccountsMenu_->menuAction ()->setProperty ("ActionIcon", "system-users");
		QActionGroup *accsGroup = new QActionGroup (this);
		Q_FOREACH (Account_ptr account, Core::Instance ().GetAccounts ())
		{
			QAction *act = new QAction (account->GetName (), this);
			accsGroup->addAction (act);
			act->setCheckable (true);
			act->setChecked (true);
			act->setProperty ("Account", QVariant::fromValue<Account_ptr> (account));

			AccountsMenu_->addAction (act);
		}
		Toolbar_->addAction (AccountsMenu_->menuAction ());

		AttachmentsMenu_ = new QMenu (tr ("Attachments"));
		AttachmentsMenu_->menuAction ()->setProperty ("ActionIcon", "mail-attachment");
		AttachmentsMenu_->addSeparator ();
		QAction *add = AttachmentsMenu_->
				addAction (tr ("Add..."), this, SLOT (handleAddAttachment ()));
		add->setProperty ("ActionIcon", "list-add");
		Toolbar_->addAction (AttachmentsMenu_->menuAction ());

		Core::Instance ().GetProxy ()->UpdateIconset (QList<QAction*> ()
					<< AccountsMenu_->menuAction ()
					<< AttachmentsMenu_->menuAction ());

		QVBoxLayout *editFrameLay = new QVBoxLayout ();
		editFrameLay->setContentsMargins (0, 0, 0, 0);
		Ui_.MsgEditFrame_->setLayout (editFrameLay);

		auto plugs = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<ITextEditor*> ();
		Q_FOREACH (ITextEditor *plug, plugs)
		{
			if (!plug->SupportsEditor (ContentType::PlainText))
				continue;

			QWidget *w = plug->GetTextEditor (ContentType::PlainText);
			MsgEdit_ = qobject_cast<IEditorWidget*> (w);
			if (!MsgEdit_)
			{
				delete w;
				continue;
			}

			MsgEditWidget_ = w;
			editFrameLay->addWidget (w);
		}
	}

	TabClassInfo ComposeMessageTab::GetTabClassInfo () const
	{
		return S_TabClassInfo_;
	}

	QObject* ComposeMessageTab::ParentMultiTabs ()
	{
		return S_ParentPlugin_;
	}

	void ComposeMessageTab::Remove ()
	{
		emit removeTab (this);
		delete MsgEditWidget_;
		deleteLater ();
	}

	QToolBar* ComposeMessageTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void ComposeMessageTab::SelectAccount (Account_ptr account)
	{
		const auto& var = QVariant::fromValue<Account_ptr> (account);
		Q_FOREACH (QAction *action, AccountsMenu_->actions ())
			if (action->property ("Account") == var)
			{
				action->setChecked (true);
				break;
			}
	}

	void ComposeMessageTab::PrepareReply (Message_ptr msg)
	{
		auto address = msg->GetAddress (Message::Address::ReplyTo);
		if (address.second.isEmpty ())
			address = msg->GetAddress (Message::Address::From);
		Ui_.To_->setText (GetNiceMail (address));

		auto split = msg->GetBody ().split ('\n');
		for (int i = 0; i < split.size (); ++i)
		{
			QString str = split.at (i).trimmed ();
			if (str.at (0) != '>')
				str.prepend (' ');
			str.prepend ('>');
			split [i] = str;
		}

		QString subj = msg->GetSubject ();
		if (subj.left (3).toLower () != "re:")
			subj.prepend ("Re: ");
		Ui_.Subject_->setText (subj);

		QString plainContent = split.join ("\n");
		plainContent += "\n\n";
		MsgEdit_->SetContents (plainContent, ContentType::PlainText);
	}

	namespace
	{
		Message::Addresses_t FromUserInput (const QString& text)
		{
			Message::Addresses_t result;
			const QStringList& split = text.split (',', QString::SkipEmptyParts);

			Q_FOREACH (QString address, split)
			{
				address = address.trimmed ();

				QString name;

				const int idx = address.lastIndexOf (' ');
				if (idx > 0)
				{
					name = address.left (idx).trimmed ();
					address = address.mid (idx).simplified ();
				}

				if (address.startsWith ('<') &&
						address.endsWith ('>'))
				{
					address = address.mid (1);
					address.chop (1);
				}

				result.append ({ name, address });
			}

			return result;
		}
	}

	void ComposeMessageTab::handleSend ()
	{
		Account_ptr account;
		Q_FOREACH (QAction *act, AccountsMenu_->actions ())
		{
			if (!act->isChecked ())
				continue;

			account = act->property ("Account").value<Account_ptr> ();
			break;
		}
		if (!account)
			return;

		Message_ptr message (new Message);
		message->SetAddresses (Message::Address::To, FromUserInput (Ui_.To_->text ()));
		message->SetSubject (Ui_.Subject_->text ());
		message->SetBody (MsgEdit_->GetContents (ContentType::PlainText));
		message->SetHTMLBody (MsgEdit_->GetContents (ContentType::HTML));

#ifdef HAVE_MAGIC
		auto Magic_ = std::shared_ptr<magic_set> (magic_open (MAGIC_MIME_TYPE),
				magic_close);
		magic_load (Magic_.get (), NULL);
#endif

		Q_FOREACH (QAction *act, AttachmentsMenu_->actions ())
		{
			const QString& path = act->property ("Snails/AttachmentPath").toString ();
			if (path.isEmpty ())
				continue;

			const QString& descr = act->property ("Snails/Description").toString ();

#ifdef HAVE_MAGIC
			const QByteArray mime (magic_file (Magic_.get (), path.toUtf8 ().constData ()));
			const auto& split = mime.split ('/');
			const QByteArray type = split.value (0);
			const QByteArray subtype = split.value (1);
#else
			const QByteArray type = "application", subtype = "octet-stream";
#endif

			message->AddAttachment ({ path, descr, type, subtype, QFileInfo (path).size () });
		}

		account->SendMessage (message);
	}

	void ComposeMessageTab::handleAddAttachment ()
	{
		const QString& path = QFileDialog::getOpenFileName (this,
				tr ("Select file to attach"),
				QDir::homePath ());
		if (path.isEmpty ())
			return;

		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			QMessageBox::critical (this,
					tr ("Error attaching file"),
					tr ("Error attaching file: %1 cannot be read")
						.arg (path));
			return;
		}

		const QString& descr = QInputDialog::getText (this,
				tr ("Attachment description"),
				tr ("Enter optional attachment description (you may leave it blank):"));

		const QFileInfo fi (path);

		const QString& size = Util::MakePrettySize (file.size ());
		QAction *attAct = new QAction (QString ("%1 (%2)").arg (fi.fileName (), size), this);
		attAct->setProperty ("Snails/AttachmentPath", path);
		attAct->setProperty ("Snails/Description", descr);
		attAct->setIcon (QFileIconProvider ().icon (fi));
		connect (attAct,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRemoveAttachment ()));

		const auto& acts = AttachmentsMenu_->actions ();
		AttachmentsMenu_->insertAction (acts.at (acts.size () - 2), attAct);
	}

	void ComposeMessageTab::handleRemoveAttachment ()
	{
		QAction *act = qobject_cast<QAction*> (sender ());
		act->deleteLater ();
		AttachmentsMenu_->removeAction (act);
	}
}
}
