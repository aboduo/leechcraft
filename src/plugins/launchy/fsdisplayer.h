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
#include <QHash>
#include <QDeclarativeView>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;
class QStandardItemModel;
class QDeclarativeView;

namespace LeechCraft
{
struct Entity;

namespace Launchy
{
	class ItemsFinder;
	class ItemIconsProvider;
	class ItemsSortFilterProxyModel;
	class FavoritesManager;

	class Item;
	typedef std::shared_ptr<Item> Item_ptr;

	class FSDisplayer : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		ItemsFinder *Finder_;
		FavoritesManager *FavManager_;

		QStandardItemModel *CatsModel_;
		QStandardItemModel *ItemsModel_;
		ItemsSortFilterProxyModel *ItemsProxyModel_;

		QDeclarativeView *View_;
		ItemIconsProvider *IconsProvider_;

		typedef std::function<void ()> Executor_f;
		struct ItemInfo
		{
			Executor_f Exec_;
			QString PermanentID_;
		};
		QHash<QString, ItemInfo> ItemInfos_;
	public:
		FSDisplayer (ICoreProxy_ptr, ItemsFinder *finder, FavoritesManager*, QObject* = 0);
		~FSDisplayer ();
	private:
		void MakeStdCategories ();
		void MakeStdItems ();
		void MakeCategories (const QStringList&);
		void MakeItems (const QList<QList<Item_ptr>>&);
		QStandardItem* FindItem (const QString&) const;
	private slots:
		void handleFinderUpdated ();
		void handleCategorySelected (int);
		void handleExecRequested (const QString&);
		void handleItemBookmark (const QString&);
		void handleViewStatus (QDeclarativeView::Status);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
