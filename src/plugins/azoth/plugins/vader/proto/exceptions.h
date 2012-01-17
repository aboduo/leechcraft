/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_VADER_PROTO_EXCEPTIONS_H
#define PLUGINS_AZOTH_PLUGINS_VADER_PROTO_EXCEPTIONS_H
#include <stdexcept>

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class InvalidPacket : public std::runtime_error
	{
	public:
		explicit InvalidPacket (const std::string&);
	};

	class TooShortBA : public std::runtime_error
	{
	public:
		explicit TooShortBA (const std::string&);
	};

	class MsgParseError : public std::runtime_error
	{
	public:
		explicit MsgParseError (const std::string&);
	};
}
}
}
}

#endif
