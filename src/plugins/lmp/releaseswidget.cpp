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

#include "releaseswidget.h"
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/util.h>
#include <util/qml/colorthemeproxy.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/irecentreleases.h>
#include <interfaces/media/idiscographyprovider.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "util.h"
#include "sysiconsprovider.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class ReleasesModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				AlbumName = Qt::UserRole + 1,
				ArtistName,
				AlbumImageThumb,
				AlbumImageFull,
				ReleaseDate,
				ReleaseURL,
				TrackList
			};

			ReleasesModel (QObject *parent = 0)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [AlbumName] = "albumName";
				names [ArtistName] = "artistName";
				names [AlbumImageThumb] = "albumThumbImage";
				names [AlbumImageFull] = "albumFullImage";
				names [ReleaseDate] = "releaseDate";
				names [ReleaseURL] = "releaseURL";
				names [TrackList] = "trackList";
				setRoleNames (names);
			}
		};
	}

	ReleasesWidget::ReleasesWidget (QWidget *parent)
	: QWidget (parent)
	, ReleasesModel_ (new ReleasesModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ReleasesView_->engine ()->addImageProvider ("sysIcons",
				new SysIconProvider (Core::Instance ().GetProxy ()));
		Ui_.ReleasesView_->rootContext ()->setContextProperty ("releasesModel", ReleasesModel_);
		Ui_.ReleasesView_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ().GetProxy ()->GetColorThemeManager (), this));
		Ui_.ReleasesView_->setSource (QUrl ("qrc:/lmp/resources/qml/ReleasesView.qml"));

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));
		connect (Ui_.WithRecs_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (request ()));

		connect (Ui_.ReleasesView_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
		connect (Ui_.ReleasesView_->rootObject (),
				SIGNAL (albumPreviewRequested (int)),
				this,
				SLOT (previewAlbum (int)));
	}

	void ReleasesWidget::InitializeProviders ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();

		const auto& lastProv = ShouldRememberProvs () ?
				XmlSettingsManager::Instance ()
					.Property ("LastUsedReleasesProvider", QString ()).toString () :
				QString ();

		bool lastFound = false;

		Providers_ = pm->GetAllCastableTo<Media::IRecentReleases*> ();
		Q_FOREACH (auto prov, Providers_)
		{
			Ui_.InfoProvider_->addItem (prov->GetServiceName ());

			if (prov->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.InfoProvider_->setCurrentIndex (idx);
				request ();
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.InfoProvider_->setCurrentIndex (-1);

		DiscoProviders_ = pm->GetAllCastableTo<Media::IDiscographyProvider*> ();
	}

	void ReleasesWidget::handleRecentReleases (const QList<Media::AlbumRelease>& releases)
	{
		TrackLists_.resize (releases.size ());

		auto discoProv = DiscoProviders_.value (0);
		Q_FOREACH (const auto& release, releases)
		{
			auto item = new QStandardItem ();
			item->setData (release.Title_, ReleasesModel::Role::AlbumName);
			item->setData (release.Artist_, ReleasesModel::Role::ArtistName);
			item->setData (release.ThumbImage_, ReleasesModel::Role::AlbumImageThumb);
			item->setData (release.FullImage_, ReleasesModel::Role::AlbumImageFull);
			item->setData (release.Date_.date ().toString (Qt::DefaultLocaleLongDate),
						ReleasesModel::Role::ReleaseDate);
			item->setData (release.ReleaseURL_, ReleasesModel::Role::ReleaseURL);
			item->setData (QString (), ReleasesModel::Role::TrackList);
			ReleasesModel_->appendRow (item);

			if (discoProv)
			{
				auto pending = discoProv->GetReleaseInfo (release.Artist_, release.Title_);
				connect (pending->GetQObject (),
						SIGNAL (ready ()),
						this,
						SLOT (handleReleaseInfo ()));
				Pending2Release_ [pending->GetQObject ()] = { release.Artist_, release.Title_ };
			}
		}
	}

	void ReleasesWidget::handleReleaseInfo ()
	{
		const auto& pendingRelease = Pending2Release_.take (sender ());

		auto pending = qobject_cast<Media::IPendingDisco*> (sender ());
		const auto& infos = pending->GetReleases ();
		if (infos.isEmpty ())
			return;

		const auto& info = infos.at (0);

		QStandardItem *item = 0;
		for (int i = 0; i < ReleasesModel_->rowCount (); ++i)
		{
			auto candidate = ReleasesModel_->item (i);
			if (candidate->data (ReleasesModel::Role::ArtistName) != pendingRelease.Artist_ ||
				candidate->data (ReleasesModel::Role::AlbumName) != pendingRelease.Title_)
				continue;

			item = candidate;
			QList<Media::ReleaseTrackInfo> trackList;
			for (const auto& list : info.TrackInfos_)
				trackList += list;
			TrackLists_ [i] = trackList;
			break;
		}

		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "item not found for"
					<< pendingRelease.Artist_
					<< pendingRelease.Title_;
			return;
		}

		item->setData (MakeTrackListTooltip (info.TrackInfos_), ReleasesModel::Role::TrackList);
	}

	void ReleasesWidget::request ()
	{
		Pending2Release_.clear ();
		TrackLists_.clear ();
		ReleasesModel_->clear ();

		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		Q_FOREACH (auto prov, Providers_)
			disconnect (dynamic_cast<QObject*> (prov),
					0,
					this,
					0);

		const bool withRecs = Ui_.WithRecs_->checkState () == Qt::Checked;
		auto prov = Providers_.at (idx);
		connect (dynamic_cast<QObject*> (prov),
				SIGNAL (gotRecentReleases (QList<Media::AlbumRelease>)),
				this,
				SLOT (handleRecentReleases (const QList<Media::AlbumRelease>&)));
		prov->RequestRecentReleases (15, withRecs);

		XmlSettingsManager::Instance ()
				.setProperty ("LastUsedReleasesProvider", prov->GetServiceName ());
	}

	void ReleasesWidget::previewAlbum (int index)
	{
		auto item = ReleasesModel_->item (index);
		const auto& artist = item->data (ReleasesModel::Role::ArtistName).toString ();
		for (const auto& track : TrackLists_.value (index))
			emit previewRequested (track.Name_, artist, track.Length_);
	}

	void ReleasesWidget::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
