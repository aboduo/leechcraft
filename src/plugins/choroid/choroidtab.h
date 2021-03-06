/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
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

#include <functional>
#include <QWidget>
#include <QDeclarativeView>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_choroidtab.h"

class QFileSystemModel;
class QStandardItemModel;
class QStandardItem;
class QFileInfo;
class QDeclarativeView;

namespace LeechCraft
{
namespace Choroid
{
	class QMLItemModel;

	class ChoroidTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget);

		const TabClassInfo TabClass_;
		QObject *Parent_;
		ICoreProxy_ptr Proxy_;

		Ui::ChoroidTab Ui_;

		QDeclarativeView *DeclView_;

		QMLItemModel *QMLFilesModel_;

		QFileSystemModel *FSModel_;
		QStandardItemModel *FilesModel_;

		QUrl CurrentImage_;

		QToolBar *Bar_;
		QMenu *SortMenu_;

		enum CustomRoles
		{
			CRFilePath = Qt::UserRole + 1
		};

		enum ImagesListRoles
		{
			ILRFilename = Qt::UserRole + 1,
			ILRImage,
			ILRFileSize
		};

		std::function<bool (const QFileInfo&, const QFileInfo&)> CurrentSorter_;
	public:
		ChoroidTab (const TabClassInfo&, ICoreProxy_ptr, QObject*);
		~ChoroidTab ();

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void LoadQML ();
		void SetSortMenu ();
		void ShowImage (const QString&);
		void ShowImage (const QUrl&);
		QStandardItem* FindFileItem (const QString&);
	private slots:
		void sortByName ();
		void sortByDate ();
		void sortBySize ();
		void sortByNumber ();

		void reload ();

		void handleDirTreeCurrentChanged (const QModelIndex&);
		void handleFileChanged (const QModelIndex&);

		void handleQMLImageSelected (const QString&);
		void showNextImage ();
		void showPrevImage ();
		void goUp ();

		void handleStatusChanged (QDeclarativeView::Status);
	signals:
		void removeTab (QWidget*);
	};
}
}
