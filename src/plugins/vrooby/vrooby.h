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

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iremovabledevmanager.h>
#include <interfaces/iactionsexporter.h>

namespace LeechCraft
{
namespace Vrooby
{
	class DevBackend;
	class TrayView;

	class Plugin : public QObject
				, public IInfo
				, public IRemovableDevManager
				, public IActionsExporter
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IRemovableDevManager IActionsExporter)

		DevBackend *Backend_;
		std::shared_ptr<QAction> ActionDevices_;
		TrayView *TrayView_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QAbstractItemModel* GetDevicesModel () const;
		void MountDevice (const QString&);

		QList<QAction*> GetActions (ActionsEmbedPlace) const;
	private slots:
		void checkAction ();
		void showTrayView (bool);
	signals:
		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
