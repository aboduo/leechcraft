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

#ifndef PLUGINS_BITTORRENT_PIECESMODEL_H
#define PLUGINS_BITTORRENT_PIECESMODEL_H
#define PLUGINS_BITTORRENT_WIN32_LEAN_AND_MEAN
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include <vector>
#include <libtorrent/torrent_handle.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class PiecesModel : public QAbstractItemModel
			{
				Q_OBJECT

				QStringList Headers_;
				struct Info
				{
					int Index_;
					libtorrent::partial_piece_info::state_t State_;
					int FinishedBlocks_;
					int TotalBlocks_;

					bool operator== (const Info&) const;
				};
				QList<Info> Pieces_;

				const int Index_;
			public:
				PiecesModel (int, QObject *parent = 0);

				virtual int columnCount (const QModelIndex&) const;
				virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
			public slots:
				void update ();
			private:
				void Clear ();
				void Update (const std::vector<libtorrent::partial_piece_info>&);
			};
		};
	};
};

#endif

