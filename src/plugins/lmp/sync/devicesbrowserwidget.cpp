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

#include "devicesbrowserwidget.h"
#include <algorithm>
#include <QMessageBox>
#include <QInputDialog>
#include <util/models/flattenfiltermodel.h>
#include <util/models/mergemodel.h>
#include <util/util.h>
#include <interfaces/iremovabledevmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/lmp/isyncplugin.h>
#include <interfaces/lmp/iunmountablesync.h>
#include "core.h"
#include "localcollection.h"
#include "uploadmodel.h"
#include "syncmanager.h"
#include "transcodingparams.h"
#include "unmountabledevmanager.h"
#include "syncunmountablemanager.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class MountableFlattener : public Util::FlattenFilterModel
		{
		public:
			MountableFlattener (QObject *parent)
			: Util::FlattenFilterModel (parent)
			{
			}

			QVariant data (const QModelIndex& index, int role) const
			{
				if (role != Qt::DisplayRole)
					return Util::FlattenFilterModel::data (index, role);

				const auto& mounts = index.data (DeviceRoles::MountPoints).toStringList ();
				const auto& mountText = mounts.isEmpty () ?
						tr ("not mounted") :
						tr ("mounted at %1").arg (mounts.join ("; "));

				const auto& size = index.data (DeviceRoles::TotalSize).toLongLong ();
				return QString ("%1 (%2, %3), %4")
						.arg (index.data (DeviceRoles::VisibleName).toString ())
						.arg (Util::MakePrettySize (size))
						.arg (index.data (DeviceRoles::DevFile).toString ())
						.arg (mountText);
			}
		protected:
			bool IsIndexAccepted (const QModelIndex& child) const
			{
				return child.data (DeviceRoles::IsMountable).toBool ();
			}
		};
	}

	DevicesBrowserWidget::DevicesBrowserWidget (QWidget *parent)
	: QWidget (parent)
	, DevUploadModel_ (new UploadModel (this))
	, Merger_ (new Util::MergeModel (QStringList ("Device name"), this))
	, UnmountableMgr_ (new UnmountableDevManager (this))
	, CurrentSyncer_ (0)
	{
		Ui_.setupUi (this);
		Ui_.UploadButton_->setIcon (Core::Instance ().GetProxy ()->GetIcon ("svn-commit"));

		DevUploadModel_->setSourceModel (Core::Instance ().GetLocalCollection ()->GetCollectionModel ());
		Ui_.OurCollection_->setModel (DevUploadModel_);

		connect (Core::Instance ().GetSyncManager (),
				SIGNAL (uploadLog (QString)),
				this,
				SLOT (appendUpLog (QString)));

		connect (Core::Instance ().GetSyncManager (),
				SIGNAL (transcodingProgress (int, int)),
				this,
				SLOT (handleTranscodingProgress (int, int)));
		connect (Core::Instance ().GetSyncManager (),
				SIGNAL (uploadProgress (int, int)),
				this,
				SLOT (handleUploadProgress (int, int)));

		connect (Core::Instance ().GetSyncUnmountableManager (),
				SIGNAL (transcodingProgress (int, int)),
				this,
				SLOT (handleTranscodingProgress (int, int)));
		connect (Core::Instance ().GetSyncUnmountableManager (),
				SIGNAL (uploadProgress (int, int)),
				this,
				SLOT (handleUploadProgress (int, int)));

		Ui_.TSProgress_->hide ();
		Ui_.UploadProgress_->hide ();

		Ui_.UnmountablePartsWidget_->hide ();
	}

	void DevicesBrowserWidget::InitializeDevices ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();

		const auto& mgrs = pm->GetAllCastableTo<IRemovableDevManager*> ();
		Q_FOREACH (const auto& mgr, mgrs)
		{
			auto flattener = new MountableFlattener (this);
			flattener->SetSource (mgr->GetDevicesModel ());
			Merger_->AddModel (flattener);
			Flattener2DevMgr_ [flattener] = mgr;
		}

		UnmountableMgr_->InitializePlugins ();
		Merger_->AddModel (UnmountableMgr_->GetDevListModel ());

		Ui_.DevicesSelector_->setModel (Merger_);
		connect (Merger_,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDevDataChanged (QModelIndex, QModelIndex)));
		connect (Merger_,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));

		if (Ui_.DevicesSelector_->count ())
			on_DevicesSelector__activated (0);
	}

	namespace
	{
		QList<ISyncPlugin*> FindSuitables (const QString& mountPath)
		{
			QList<ISyncPlugin*> suitables;
			auto syncers = Core::Instance ().GetSyncPlugins ();
			Q_FOREACH (auto syncer, syncers)
			{
				auto isp = qobject_cast<ISyncPlugin*> (syncer);
				if (isp->CouldSync (mountPath) != SyncConfLevel::None)
					suitables << isp;
			}
			return suitables;
		}
	}

	void DevicesBrowserWidget::UploadMountable (int idx)
	{
		const auto& to = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::MountPoints).toStringList ().value (0);
		if (to.isEmpty ())
			return;

		const auto& suitables = FindSuitables (to);
		if (suitables.size () == 1)
			CurrentSyncer_ = suitables.value (0);
		else
		{
			QStringList items;
			Q_FOREACH (ISyncPlugin *plugin, suitables)
				items << plugin->GetSyncSystemName ();

			const auto& name = QInputDialog::getItem (this,
					tr ("Select syncer"),
					tr ("Multiple different syncers can handle the device %1, what do you want to use?")
						.arg (Ui_.DevicesSelector_->itemText (idx)),
					items);
			if (name.isEmpty ())
				return;

			CurrentSyncer_ = suitables.value (items.indexOf (name));
		}

		const auto& selected = DevUploadModel_->GetSelectedIndexes ();
		QStringList paths;
		std::transform (selected.begin (), selected.end (), std::back_inserter (paths),
				[] (const QModelIndex& idx) { return idx.data (LocalCollection::Role::TrackPath).toString (); });
		paths.removeAll (QString ());

		Ui_.UploadLog_->clear ();
		Core::Instance ().GetSyncManager ()->AddFiles (CurrentSyncer_, to, paths, Ui_.TranscodingOpts_->GetParams ());
	}

	void DevicesBrowserWidget::UploadUnmountable (int idx)
	{
		int starting = 0;
		Merger_->GetModelForRow (idx, &starting);
		idx -= starting;

		const auto& selected = DevUploadModel_->GetSelectedIndexes ();
		QStringList paths;
		std::transform (selected.begin (), selected.end (), std::back_inserter (paths),
				[] (const QModelIndex& idx) { return idx.data (LocalCollection::Role::TrackPath).toString (); });
		paths.removeAll (QString ());

		auto syncer = qobject_cast<IUnmountableSync*> (UnmountableMgr_->GetDeviceManager (idx));
		const auto& info = UnmountableMgr_->GetDeviceInfo (idx);

		const int partIdx = Ui_.UnmountablePartsBox_->currentIndex ();
		const auto& storageId = Ui_.UnmountablePartsBox_->itemData (partIdx).toByteArray ();
		Core::Instance ().GetSyncUnmountableManager ()->AddFiles ({ syncer, info.ID_,
				storageId, paths, Ui_.TranscodingOpts_->GetParams () });
	}

	void DevicesBrowserWidget::HandleMountableSelected (int idx)
	{
		Ui_.MountButton_->show ();
		Ui_.TranscodingOpts_->SetMaskVisible (true);
		Ui_.UnmountablePartsWidget_->hide ();

		auto isMounted = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::IsMounted).toBool ();
		Ui_.MountButton_->setEnabled (!isMounted);

		if (!isMounted)
			return;

		const auto& mountPath = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::MountPoints).toStringList ().value (0);
		if (mountPath.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "device seems to be mounted, but no mount points available:"
					<< Ui_.DevicesSelector_->itemData (idx, DeviceRoles::DevID).toString ();
			return;
		}

		Ui_.SyncTabs_->setEnabled (!FindSuitables (mountPath).isEmpty ());
	}

	void DevicesBrowserWidget::HandleUnmountableSelected (int idx)
	{
		Ui_.MountButton_->hide ();
		Ui_.TranscodingOpts_->SetMaskVisible (false);
		Ui_.UnmountablePartsWidget_->show ();

		int starting = 0;
		Merger_->GetModelForRow (idx, &starting);
		idx -= starting;

		Ui_.UnmountablePartsBox_->clear ();
		const auto& info = UnmountableMgr_->GetDeviceInfo (idx);
		for (const auto& storage : info.Partitions_)
		{
			const auto& boxText = storage.TotalSize_ > 0 ?
					tr ("%1 (%2 available of %3)")
							.arg (storage.Name_)
							.arg (Util::MakePrettySize (storage.AvailableSize_))
							.arg (Util::MakePrettySize (storage.TotalSize_)) :
					storage.Name_;
			Ui_.UnmountablePartsBox_->addItem (boxText, storage.ID_);
		}
	}

	void DevicesBrowserWidget::handleDevDataChanged (const QModelIndex& from, const QModelIndex& to)
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < from.row () && idx > to.row ())
			return;

		on_DevicesSelector__activated (idx);
	}

	void DevicesBrowserWidget::handleRowsInserted (const QModelIndex&, int start, int end)
	{
		if (start - end + 1 == Merger_->rowCount ())
			on_DevicesSelector__activated (0);
	}

	void DevicesBrowserWidget::on_UploadButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		if (Flattener2DevMgr_.contains (*Merger_->GetModelForRow (idx)))
			UploadMountable (idx);
		else
			UploadUnmountable (idx);
	}

	void DevicesBrowserWidget::on_DevicesSelector__activated (int idx)
	{
		CurrentSyncer_ = 0;

		if (idx < 0)
		{
			Ui_.MountButton_->setEnabled (false);
			Ui_.UnmountablePartsWidget_->hide ();
			return;
		}

		if (Flattener2DevMgr_.contains (*Merger_->GetModelForRow (idx)))
			HandleMountableSelected (idx);
		else
			HandleUnmountableSelected (idx);
	}

	void DevicesBrowserWidget::on_MountButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		const auto model = *Merger_->GetModelForRow (idx);
		if (!Flattener2DevMgr_.contains (model))
			return;

		const auto& id = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::DevID).toString ();
		Flattener2DevMgr_ [model]->MountDevice (id);
	}

	void DevicesBrowserWidget::appendUpLog (QString text)
	{
		text.prepend (QTime::currentTime ().toString ("[HH:mm:ss.zzz] "));
		Ui_.UploadLog_->append ("<code>" + text + "</code>");
	}

	void DevicesBrowserWidget::handleTranscodingProgress (int done, int total)
	{
		Ui_.TSProgress_->setVisible (done < total);
		Ui_.TSProgress_->setMaximum (total);
		Ui_.TSProgress_->setValue (done);
	}

	void DevicesBrowserWidget::handleUploadProgress (int done, int total)
	{
		Ui_.UploadProgress_->setVisible (done < total);
		Ui_.UploadProgress_->setMaximum (total);
		Ui_.UploadProgress_->setValue (done);
	}
}
}
