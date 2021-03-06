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

#include "udisks2backend.h"
#include <memory>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QTimer>
#include <QtDebug>
#include <QMetaMethod>
#include <util/util.h>
#include "udisks2types.h"

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LeechCraft
{
namespace Vrooby
{
namespace UDisks2
{
	Backend::Backend (QObject *parent)
	: DevBackend (parent)
	, Valid_ (false)
	, DevicesModel_ (new QStandardItemModel (this))
	, UDisksObj_ (0)
	{
		qDBusRegisterMetaType<VariantMapMap_t> ();
		qDBusRegisterMetaType<EnumerationResult_t> ();
		qDBusRegisterMetaType<ByteArrayList_t> ();

		InitialEnumerate ();

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (updateDeviceSpaces ()));
		timer->start (10000);
	}

	bool Backend::IsValid () const
	{
		return Valid_;
	}

	QAbstractItemModel* Backend::GetDevicesModel () const
	{
		return DevicesModel_;
	}

	namespace
	{
		QDBusInterface_ptr GetBlockInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks2",
						path,
						"org.freedesktop.UDisks2.Block",
						QDBusConnection::systemBus ()));
		}

		QDBusInterface_ptr GetPartitionInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks2",
						path,
						"org.freedesktop.UDisks2.Partition",
						QDBusConnection::systemBus ()));
		}

		QDBusInterface_ptr GetFSInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks2",
						path,
						"org.freedesktop.UDisks2.Filesystem",
						QDBusConnection::systemBus ()));
		}

		QDBusInterface_ptr GetDevInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks2",
						path,
						"org.freedesktop.UDisks2.Drive",
						QDBusConnection::systemBus ()));
		}

		QDBusInterface_ptr GetPropsInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks2",
						path,
						"org.freedesktop.DBus.Properties",
						QDBusConnection::systemBus ()));
		}
	}

	void Backend::MountDevice (const QString& id)
	{
		auto iface = GetFSInterface (id);
		if (!iface)
			return;

		if (!iface->property ("DeviceIsMounted").toBool ())
		{
			auto async = iface->asyncCall ("FilesystemMount", QString (), QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (mountCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	void Backend::InitialEnumerate ()
	{
		auto sb = QDBusConnection::systemBus ();
		auto iface = sb.interface ();

		auto services = iface->registeredServiceNames ()
				.value ().filter ("org.freedesktop.UDisks2");
		if (services.isEmpty ())
		{
			iface->startService ("org.freedesktop.UDisks2");
			services = iface->registeredServiceNames ()
					.value ().filter ("org.freedesktop.UDisks2");
			if (services.isEmpty ())
				return;
		}

		UDisksObj_ = new org::freedesktop::DBus::ObjectManager ("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", sb);
		auto reply = UDisksObj_->GetManagedObjects ();
		auto watcher = new QDBusPendingCallWatcher (reply, this);
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleEnumerationFinished (QDBusPendingCallWatcher*)));

		connect (UDisksObj_,
				SIGNAL (InterfacesAdded (QDBusObjectPath, VariantMapMap_t)),
				this,
				SLOT (handleDeviceAdded (QDBusObjectPath, VariantMapMap_t)));
		connect (UDisksObj_,
				SIGNAL (InterfacesRemoved (QDBusObjectPath, QStringList)),
				this,
				SLOT (handleDeviceRemoved (QDBusObjectPath)));
	}

	bool Backend::AddPath (const QDBusObjectPath& path)
	{
		const auto& str = path.path ();
		if (Object2Item_.contains (str))
			return true;

		auto blockIface = GetBlockInterface (str);
		if (!blockIface->isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid interface for"
					<< str
					<< blockIface->lastError ().message ();
			return false;
		}

		const auto& driveId = blockIface->property ("Drive").value<QDBusObjectPath> ().path ();

		auto driveIface = driveId.isEmpty () ? QDBusInterface_ptr () : GetDevInterface (driveId);
		if (!driveIface || !driveIface->isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no associated device"
					<< str
					<< blockIface->property ("Drive")
					<< driveId;
			return false;
		}

		auto partitionIface = GetPartitionInterface (str);
		const bool isPartition = !partitionIface->property ("Type").toString ().isEmpty ();

		const auto& slaveTo = partitionIface->property ("Table").value<QDBusObjectPath> ();
		const bool isRemovable = driveIface->property ("Removable").toBool ();
		qDebug () << str << slaveTo.path () << isPartition << isRemovable;
		if ((!isPartition && !isRemovable) || Unremovables_.contains (slaveTo.path ()))
		{
			qDebug () << "detected as unremovable";
			Unremovables_ << str;
			return false;
		}

		QDBusConnection::systemBus ().connect ("org.freedesktop.UDisks2",
				path.path (), "org.freedesktop.DBus.Properties", "PropertiesChanged",
				this, SLOT (handleDeviceChanged (QDBusMessage)));

		auto item = new QStandardItem;
		Object2Item_ [str] = item;
		SetItemData ({ partitionIface, GetFSInterface (str), blockIface, driveIface, GetPropsInterface (str) }, item);
		if (slaveTo.path ().isEmpty ())
			DevicesModel_->appendRow (item);
		else
		{
			if (!Object2Item_.contains (slaveTo.path ()))
				if (!AddPath (slaveTo))
					return false;

			Object2Item_ [slaveTo.path ()]->appendRow (item);
		}
		return true;
	}

	void Backend::RemovePath (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		auto item = Object2Item_.take (path);
		if (!item)
			return;

		auto getChildren = [] (QStandardItem *item)
		{
			QList<QStandardItem*> result;
			for (int i = 0; i < item->rowCount (); ++i)
				result << item->child (i);
			return result;
		};

		QList<QStandardItem*> toRemove = getChildren (item);
		for (int i = 0; i < toRemove.size (); ++i)
			toRemove += getChildren (toRemove [i]);

		for (QStandardItem *item : toRemove)
			Object2Item_.remove (Object2Item_.key (item));

		if (item->parent ())
			item->parent ()->removeRow (item->row ());
		else
			DevicesModel_->removeRow (item->row ());
	}

	void Backend::SetItemData (const ItemInterfaces& ifaces, QStandardItem *item)
	{
		if (!item)
			return;

		const bool isRemovable = ifaces.Drive_->property ("Removable").toBool ();
		const bool isPartition = !ifaces.Partition_->property ("Type").toString ().isEmpty ();

		const auto& vendor = ifaces.Drive_->property ("Vendor").toString () +
				" " +
				ifaces.Drive_->property ("Model").toString ();
		const auto& partLabel = ifaces.Partition_->property ("Name").toString ().trimmed ();
		const auto& partName = partLabel.isEmpty () ?
				tr ("Partition %1")
						.arg (ifaces.Partition_->property ("Number").toInt ()) :
				partLabel;
		const auto& name = isPartition ? partName : vendor;
		const auto& fullName = isPartition ?
				QString ("%1: %2").arg (vendor, partName) :
				vendor;

		DevicesModel_->blockSignals (true);

		QStringList mountPaths;
		auto msg = ifaces.Props_->call ("Get",
				"org.freedesktop.UDisks2.Filesystem", "MountPoints");
		QDBusReply<QDBusVariant> reply (msg);

		if (reply.isValid ())
			for (const auto& point : qdbus_cast<ByteArrayList_t> (reply.value ().variant ()))
				mountPaths << QString::fromUtf8 (point);

		if (!mountPaths.isEmpty ())
		{
			qint64 space = -1;
			try
			{
				const auto& wstrMount = mountPaths.value (0).toStdWString ();
				space = static_cast<qint64> (boost::filesystem::space (wstrMount).free);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error obtaining free space info:"
						<< QString::fromUtf8 (e.what ());
			}
			item->setData (static_cast<qint64> (space), DeviceRoles::AvailableSize);
		}
		else
			item->setData (-1, DeviceRoles::AvailableSize);

		item->setText (name);
		item->setData (DeviceType::GenericDevice, DeviceRoles::DevType);
		item->setData (ifaces.Block_->property ("Device").toByteArray (), DeviceRoles::DevFile);
		item->setData (ifaces.Partition_->property ("PartitionType").toInt (), DeviceRoles::PartType);
		item->setData (isRemovable, DeviceRoles::IsRemovable);
		item->setData (isPartition, DeviceRoles::IsPartition);
		item->setData (isPartition && isRemovable, DeviceRoles::IsMountable);
		item->setData (!mountPaths.isEmpty (), DeviceRoles::IsMounted);
		item->setData (ifaces.Drive_->property ("MediaAvailable"), DeviceRoles::IsMediaAvailable);
		item->setData (ifaces.Block_->path (), DeviceRoles::DevID);
		item->setData (fullName, DeviceRoles::VisibleName);
		item->setData (ifaces.Partition_->property ("Size").toLongLong (), DeviceRoles::TotalSize);
		DevicesModel_->blockSignals (false);
		item->setData (mountPaths, DeviceRoles::MountPoints);
	}

	void Backend::toggleMount (const QString& id)
	{
		auto iface = GetFSInterface (id);
		if (!iface->isValid ())
			return;

		auto item = Object2Item_.value (id);
		if (!item)
			return;

		const bool isMounted = !item->data (DeviceRoles::MountPoints).toStringList ().isEmpty ();
		if (isMounted)
		{
			auto async = iface->asyncCall ("Unmount", QVariantMap ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (umountCallFinished (QDBusPendingCallWatcher*)));
		}
		else
		{
			auto async = iface->asyncCall ("Mount", QVariantMap ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (mountCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	namespace
	{
		QString GetErrorText (const QString& errorCode)
		{
			QMap<QString, QString> texts;
			texts ["org.freedesktop.UDisks.Error.PermissionDenied"] = Backend::tr ("permission denied");
			texts ["org.freedesktop.PolicyKit.Error.NotAuthorized"] = Backend::tr ("not authorized");
			texts ["org.freedesktop.PolicyKit.Error.Busy"] = Backend::tr ("the device is busy");
			texts ["org.freedesktop.PolicyKit.Error.Failed"] = Backend::tr ("the operation has failed");
			texts ["org.freedesktop.PolicyKit.Error.Cancelled"] = Backend::tr ("the operation has been cancelled");
			texts ["org.freedesktop.PolicyKit.Error.InvalidOption"] = Backend::tr ("invalid mount options were given");
			texts ["org.freedesktop.PolicyKit.Error.FilesystemDriverMissing"] = Backend::tr ("unsupported filesystem");
			return texts.value (errorCode, Backend::tr ("unknown error"));
		}
	}

	void Backend::mountCallFinished (QDBusPendingCallWatcher *watcher)
	{
		qDebug () << Q_FUNC_INFO;
		watcher->deleteLater ();
		QDBusPendingReply<QString> reply = *watcher;

		if (!reply.isError ())
		{
			emit gotEntity (Util::MakeNotification ("Vrooby",
						tr ("Device has been successfully mounted at %1.")
							.arg (reply.value ()),
						PInfo_));
			return;
		}

		const auto& error = reply.error ();
		qWarning () << Q_FUNC_INFO
				<< error.name ()
				<< error.message ();
		emit gotEntity (Util::MakeNotification ("Vrooby",
					tr ("Failed to mount the device: %1 (%2).")
						.arg (GetErrorText (error.name ()))
						.arg (error.message ()),
					PCritical_));
	}

	void Backend::umountCallFinished (QDBusPendingCallWatcher *watcher)
	{
		qDebug () << Q_FUNC_INFO;
		watcher->deleteLater ();
		QDBusPendingReply<void> reply = *watcher;

		if (!reply.isError ())
		{
			emit gotEntity (Util::MakeNotification ("Vrooby",
						tr ("Device has been successfully unmounted."),
						PInfo_));
			return;
		}

		const auto& error = reply.error ();
		qWarning () << Q_FUNC_INFO
				<< error.name ()
				<< error.message ();
		emit gotEntity (Util::MakeNotification ("Vrooby",
					tr ("Failed to unmount the device: %1 (%2).")
						.arg (GetErrorText (error.name ()))
						.arg (error.message ()),
					PCritical_));
	}

	void Backend::handleEnumerationFinished (QDBusPendingCallWatcher *watcher)
	{
		watcher->deleteLater ();
		QDBusPendingReply<EnumerationResult_t> reply = *watcher;
		if (reply.isError ())
		{
			Valid_ = false;
			qWarning () << reply.error ().message ();
			return;
		}

		for (const QDBusObjectPath& path : reply.value ().keys ())
			AddPath (path);
	}

	void Backend::handleDeviceAdded (const QDBusObjectPath& path, const VariantMapMap_t& map)
	{
		AddPath (path);
	}

	void Backend::handleDeviceRemoved (const QDBusObjectPath& path)
	{
		RemovePath (path);
	}

	void Backend::handleDeviceChanged (const QDBusMessage& msg)
	{
		const auto& path = msg.path ();

		auto item = Object2Item_.value (path);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no item for path"
					<< path;
			return;
		}

		auto blockIface = GetBlockInterface (path);
		const ItemInterfaces faces =
		{
			GetPartitionInterface (path),
			GetFSInterface (path),
			blockIface,
			GetDevInterface (blockIface->property ("Drive").value<QDBusObjectPath> ().path ()),
			GetPropsInterface (path)
		};
		SetItemData (faces, item);
	}

	void Backend::updateDeviceSpaces ()
	{
		for (QStandardItem *item : Object2Item_.values ())
		{
			const auto& mountPaths = item->data (DeviceRoles::MountPoints).toStringList ();
			if (mountPaths.isEmpty ())
				continue;

			const auto& space = boost::filesystem::space (mountPaths.value (0).toStdWString ());
			const auto free = static_cast<qint64> (space.free);
			if (free != item->data (DeviceRoles::AvailableSize).value<qint64> ())
				item->setData (static_cast<qint64> (free), DeviceRoles::AvailableSize);
		}
	}
}
}
}
