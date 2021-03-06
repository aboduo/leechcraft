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

#ifndef UTIL_SOCKETERRORSTRINGS_H
#define UTIL_SOCKETERRORSTRINGS_H
#include <QAbstractSocket>
#include "utilconfig.h"

class QString;

namespace LeechCraft
{
namespace Util
{
	/** @brief Returns an error string for the given socket error.
	 *
	 * This function returns a human-readable localized string describing
	 * the given socket \em error.
	 *
	 * @param[in] error The socket error to describe.
	 * @return The human-readable localized error string.
	 */
	UTIL_API QString GetSocketErrorString (QAbstractSocket::SocketError error);
}
}

#endif
