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

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>

namespace LeechCraft
{
namespace LMP
{
namespace MPRIS
{
	class FDOPropsAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT

		Q_CLASSINFO ("D-Bus Interface", "org.freedesktop.DBus.Properties")
	public:
		FDOPropsAdaptor (QObject*);

		void Notify (const QString& iface, const QString& prop, const QVariant& val);
	public slots:
		QDBusVariant Get (const QString& iface, const QString& prop);
		void Set (const QString& iface, const QString& prop, const QDBusVariant&);
	private:
		bool GetProperty (const QString&, const QString&, QMetaProperty*, QObject**) const;
	signals:
		void PropertiesChanged (const QString&, const QVariantMap&, const QStringList&);
	};
}
}
}
