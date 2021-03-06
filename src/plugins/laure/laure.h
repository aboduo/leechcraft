/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
namespace Laure
{
	class LaureWidget;
	class LastFMSubmitter;

	/** @brief Main plugin instance class.
	 * 
	 *  @author Minh Ngo <nlminhtl@gmail.com>
	 * 
	 * @sa IInfo
	 * @sa IHaveTabs
	 * @sa IEntityHandler
	 * @sa IHaveSettings
	 */
	class Plugin : public QObject
				, public IInfo
				, public IHaveTabs
				, public IEntityHandler
				, public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IEntityHandler IHaveSettings)

		TabClasses_t TabClasses_;
		QList<LaureWidget*> Others_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray& tabClass);
		
		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);
		
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		LaureWidget* CreateTab ();
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	private slots:
		void handleNeedToClose ();
#ifdef HAVE_LASTFM
		void handleSubmitterInit ();
#endif
	};
}
}

