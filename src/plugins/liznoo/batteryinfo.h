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

#include <QString>
#include <QMetaType>

namespace LeechCraft
{
namespace Liznoo
{
	struct BatteryInfo
	{
		QString ID_;

		char Percentage_;

		/** Time until battery is fully charged in seconds, or 0 if
		 * battery isn't charging.
		 */
		qlonglong TimeToFull_;
		qlonglong TimeToEmpty_;
		double Voltage_;

		double Energy_;
		double EnergyFull_;
		double DesignEnergyFull_;
		double EnergyRate_;

		QString Technology_;

		double Temperature_;

		void Dump ();
	};
}
}

Q_DECLARE_METATYPE (LeechCraft::Liznoo::BatteryInfo);

