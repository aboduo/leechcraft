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

#include "historyfiltermodel.h"
#include "historymodel.h"

namespace LeechCraft
{
namespace Poshuku
{
	HistoryFilterModel::HistoryFilterModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
	}
	
	bool HistoryFilterModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		if (sourceModel ()->rowCount (sourceModel ()->index (row, 0, parent)))
			return true;
		
		const auto& filter = filterRegExp ().pattern ();
		if (filter.isEmpty ())
			return true;
		
		auto source = sourceModel ();
		auto contains = [&filter, source, row, parent] (HistoryModel::Columns col)
		{
			return source->index (row, col, parent).data ()
					.toString ().contains (filter, Qt::CaseInsensitive);
		};
		return contains (HistoryModel::ColumnTitle) || contains (HistoryModel::ColumnURL);
	}
}
}
