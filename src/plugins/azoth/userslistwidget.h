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

#include <functional>
#include <QDialog>
#include "ui_userslistwidget.h"

class QStandardItemModel;
class QSortFilterProxyModel;

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	class UsersListWidget : public QDialog
	{
		Q_OBJECT

		Ui::UsersListWidget Ui_;

		QSortFilterProxyModel *Filter_;
		QStandardItemModel *PartsModel_;
	public:
		UsersListWidget (const QList<QObject*>&, std::function<QString (ICLEntry*)>, QWidget* = 0);

		QObject* GetActivatedParticipant () const;
	};
}
}
