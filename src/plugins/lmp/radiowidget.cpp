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

#include "radiowidget.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <util/gui/clearlineeditaddon.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iradiostationprovider.h>
#include "core.h"
#include "player.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class StationsFilterModel : public QSortFilterProxyModel
		{
		public:
			StationsFilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const
			{
				const auto& pat = this->filterRegExp ().pattern ();
				if (pat.isEmpty ())
					return true;

				const auto& idx = sourceModel ()->index (row, 0, parent);
				if (idx.data (Media::RadioItemRole::ItemType).toInt () == Media::RadioType::None)
					return true;

				return idx.data ().toString ().contains (pat, Qt::CaseInsensitive);
			}
		};
	}

	RadioWidget::RadioWidget (QWidget *parent)
	: QWidget (parent)
	, Player_ (0)
	, StationsModel_ (new QStandardItemModel (this))
	, StationsProxy_ (new StationsFilterModel (this))
	{
		Ui_.setupUi (this);

		StationsProxy_->setDynamicSortFilter (true);
		StationsProxy_->setSourceModel (StationsModel_);
		Ui_.StationsView_->setModel (StationsProxy_);

		connect (Ui_.StationsSearch_,
				SIGNAL (textChanged (QString)),
				StationsProxy_,
				SLOT (setFilterFixedString (QString)));

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.StationsSearch_);
	}

	void RadioWidget::InitializeProviders ()
	{
		auto providerObjs = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableRoots<Media::IRadioStationProvider*> ();
		Q_FOREACH (auto provObj, providerObjs)
		{
			auto prov = qobject_cast<Media::IRadioStationProvider*> (provObj);
			Q_FOREACH (auto item, prov->GetRadioListItems ())
			{
				StationsModel_->appendRow (item);
				Root2Prov_ [item] = prov;
			}
		}
	}

	void RadioWidget::SetPlayer (Player *player)
	{
		Player_ = player;
	}

	void RadioWidget::on_StationsView__doubleClicked (const QModelIndex& unmapped)
	{
		const auto& index = StationsProxy_->mapToSource (unmapped);
		const auto item = StationsModel_->itemFromIndex (index);
		auto root = item;
		while (auto parent = root->parent ())
			root = parent;
		if (!Root2Prov_.contains (root))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown provider for index"
					<< index;
			return;
		}

		QString param;
		switch (item->data (Media::RadioItemRole::ItemType).toInt ())
		{
		case Media::RadioType::None:
			return;
		case Media::RadioType::Predefined:
			break;
		case Media::RadioType::SimilarArtists:
			param = QInputDialog::getText (this,
					tr ("Similar artists radio"),
					tr ("Enter artist name for which to tune the similar artists radio station:"));
			if (param.isEmpty ())
				return;
			break;
		case Media::RadioType::GlobalTag:
			param = QInputDialog::getText (this,
					tr ("Global tag radio"),
					tr ("Enter global tag name:"));
			if (param.isEmpty ())
				return;
			break;
		}

		auto station = Root2Prov_ [root]->GetRadioStation (item, param);
		if (station)
			Player_->SetRadioStation (station);
	}
}
}
