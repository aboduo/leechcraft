/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "core.h"
#include <QtDebug>
#include <QTimer>
#include <QMessageBox>
#include <QMainWindow>
#include <interfaces/iplugin2.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/util.h>
#include <util/notificationactionhandler.h>
#include "interfaces/blogique/iaccount.h"
#include "interfaces/blogique/ibloggingplatformplugin.h"
#include "interfaces/blogique/ibloggingplatform.h"
#include "pluginproxy.h"
#include "storagemanager.h"
#include "backupmanager.h"
#include "blogiquewidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
	Core::Core ()
	: UniqueID_ ("org.LeechCraft.Blogique")
	, PluginProxy_ (std::make_shared<PluginProxy> ())
	, StorageManager_ (new StorageManager (UniqueID_, this))
	, BackupManager_ (new BackupManager (this))
	, AutoSaveTimer_ (new QTimer (this))
	{
		connect (AutoSaveTimer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (checkAutoSave ()));
		XmlSettingsManager::Instance ().RegisterObject ("AutoSave",
				this, "handleAutoSaveIntervalChanged");
		handleAutoSaveIntervalChanged ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	QByteArray Core::GetUniqueID () const
	{
		return UniqueID_;
	}

	QIcon Core::GetIcon () const
	{
		static QIcon icon (":/plugins/blogique/resources/images/blogique.svg");
		return icon;
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetCoreProxy ()
	{
		return Proxy_;
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Blogique.Plugins.IBlogPlatformPlugin";
		return classes;
	}

	void Core::AddPlugin (QObject *plugin)
	{
		IPlugin2 *plugin2 = qobject_cast<IPlugin2*> (plugin);
		if (!plugin2)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "isn't a IPlugin2";
			return;
		}

		QByteArray sig = QMetaObject::normalizedSignature ("initPlugin (QObject*)");
		if (plugin->metaObject ()->indexOfMethod (sig) != -1)
			QMetaObject::invokeMethod (plugin,
					"initPlugin",
					Q_ARG (QObject*, PluginProxy_.get ()));

		QSet<QByteArray> classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Plugins.Blogique.Plugins.IBlogPlatformPlugin"))
			AddBlogPlatformPlugin (plugin);
	}

	QList<IBloggingPlatform*> Core::GetBloggingPlatforms () const
	{
		QList<IBloggingPlatform*> result;
		std::for_each (BlogPlatformPlugins_.begin (), BlogPlatformPlugins_.end (),
				[&result] (decltype (BlogPlatformPlugins_.front ()) bpp)
				{
					const auto& protos = qobject_cast<IBloggingPlatformPlugin*> (bpp)->
							GetBloggingPlatforms ();
					Q_FOREACH (QObject *obj, protos)
						result << qobject_cast<IBloggingPlatform*> (obj);
				});

		result.removeAll (0);
		return result;
	}

	QList<IAccount*> Core::GetAccounts () const
	{
		auto bloggingPlatforms = GetBloggingPlatforms ();
		QList<IAccount*> result;
		std::for_each (bloggingPlatforms.begin (), bloggingPlatforms.end (),
				[&result] (decltype (bloggingPlatforms.front ()) bp)
				{
					const auto& accountsObjList = bp->GetRegisteredAccounts ();
					std::transform (accountsObjList.begin (), accountsObjList.end (),
							std::back_inserter (result),
							[] (decltype (accountsObjList.front ()) accountObj)
							{
								return qobject_cast<IAccount*> (accountObj);
							});
				});
		result.removeAll (0);
		return result;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	void Core::DelayedProfilesUpdate ()
	{
		QTimer::singleShot (15000, this, SLOT (updateProfiles ()));
	}

	StorageManager* Core::GetStorageManager () const
	{
		return StorageManager_;
	}

	BackupManager* Core::GetBackupManager () const
	{
		return BackupManager_;
	}

	BlogiqueWidget* Core::CreateBlogiqueWidget ()
	{
		auto newTab = new BlogiqueWidget;
		connect (newTab,
				SIGNAL (removeTab (QWidget*)),
				&Core::Instance (),
				SIGNAL (removeTab (QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (checkAutoSave ()),
				newTab,
				SLOT (handleAutoSave ()));
		connect (&Core::Instance (),
				SIGNAL (entryPosted ()),
				newTab,
				SLOT (handleEntryPosted ()));
		connect (&Core::Instance (),
				SIGNAL (entryRemoved ()),
				newTab,
				SLOT (handleEntryRemoved ()));

		return newTab;
	}

	void Core::AddBlogPlatformPlugin (QObject *plugin)
	{
		IBloggingPlatformPlugin *ibpp = qobject_cast<IBloggingPlatformPlugin*> (plugin);
		if (!ibpp)
			qWarning () << Q_FUNC_INFO
					<< "plugin"
					<< plugin
					<< "tells it implements the IBlogPlatformPlugin but cast failed";
		else
		{
			BlogPlatformPlugins_ << plugin;
			handleNewBloggingPlatforms (ibpp->GetBloggingPlatforms ());
		}
	}

	void Core::handleNewBloggingPlatforms (const QObjectList& platforms)
	{
		Q_FOREACH (QObject *platformObj, platforms)
		{
			IBloggingPlatform *platform =
					qobject_cast<IBloggingPlatform*> (platformObj);

			Q_FOREACH (QObject *accObj, platform->GetRegisteredAccounts ())
				addAccount (accObj);

			connect (platform->GetQObject (),
					SIGNAL (accountAdded (QObject*)),
					this,
					SLOT (addAccount (QObject*)));
			connect (platform->GetQObject (),
					SIGNAL (accountRemoved (QObject*)),
					this,
					SLOT (handleAccountRemoved (QObject*)));
			connect (platform->GetQObject (),
					SIGNAL (accountValidated (QObject*, bool)),
					this,
					SLOT (handleAccountValidated (QObject*, bool)));
		}
	}

	void Core::addAccount (QObject *accObj)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObj
					<< sender ();
			return;
		}

		connect (accObj,
				SIGNAL (requestEntriesBegin ()),
				this,
				SIGNAL (requestEntriesBegin ()));
		connect (accObj,
				SIGNAL (entryPosted (QList<Entry>)),
				this,
				SLOT (handleEntryPosted (QList<Entry>)));
		connect (accObj,
				SIGNAL (entryRemoved (int)),
				this,
				SLOT (handleEntryRemoved (int)));
		connect (accObj,
				SIGNAL (entryUpdated (QList<Entry>)),
				this,
				SLOT (handleEntryUpdated (QList<Entry>)));
		connect (accObj,
				SIGNAL (gotEntries2Backup (QList<Entry>)),
				this,
				SLOT (handleGotEntries2Backup (QList<Entry>)));
		connect (accObj,
				SIGNAL (gettingEntries2BackupFinished ()),
				this,
				SLOT (handleGettingEntries2BackupFinished ()));

		emit accountAdded (accObj);
	}

	void Core::handleAccountRemoved (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObj
					<< sender ();
			return;
		}

		emit accountRemoved (accObj);
	}

	void Core::handleAccountValidated (QObject *accObj, bool validated)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObj
					<< sender ();
			return;
		}

		emit accountValidated (accObj, validated);
	}

	void Core::updateProfiles ()
	{
		for (auto acc : GetAccounts ())
			acc->updateProfile ();
	}

	void Core::handleEntryPosted (const QList<Entry>& entries)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;

		auto e = Util::MakeNotification ("Blogique",
				tr ("Entry was posted successfully:") +
					QString (" <a href=\"%1\">%1</a>\n")
						.arg (entries.value (0).EntryUrl_.toString ()),
				Priority::PInfo_);
		Util::NotificationActionHandler *nh = new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Open Link"),
				[this, entries] ()
				{
					Entity urlEntity = Util::MakeEntity (entries.value (0).EntryUrl_,
							QString (),
							static_cast<TaskParameters> (OnlyHandle | FromUserInitiated));
					SendEntity (urlEntity);
				});
		emit gotEntity (e);
		acc->RequestStatistics ();
		emit entryPosted ();
	}

	void Core::handleEntryRemoved (int)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;

		SendEntity (Util::MakeNotification ("Blogique",
				tr ("Entry was removed successfully."),
				Priority::PInfo_));
		acc->RequestStatistics ();
		emit entryRemoved ();
	}

	void Core::handleEntryUpdated (const QList<Entry>& entries)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;

		if (entries.isEmpty ())
			return;

		SendEntity (Util::MakeNotification ("Blogique",
				tr ("Entry was updated successfully."),
				Priority::PInfo_));
		acc->RequestStatistics ();
	}

	void Core::handleGotEntries2Backup (const QList<Entry>&)
	{
		auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
			return;
		//TODO
	}

	void Core::handleGettingEntries2BackupFinished ()
	{
		SendEntity (Util::MakeNotification ("Blogique",
				tr ("Entries were backuped successfully."),
				Priority::PInfo_));
	}

	void Core::handleAutoSaveIntervalChanged ()
	{
		AutoSaveTimer_->start (XmlSettingsManager::Instance ()
				.property ("AutoSave").toInt () * 1000);
	}
}
}

