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

#include "simpleactor.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QtDebug>
#include "account.h"
#include "authmanager.h"
#include "urls.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	SimpleActor::SimpleActor (const QUrl& url, const QByteArray& post, Account *acc)
	: ActorBase (acc)
	, URL_ (url)
	, Post_ (post)
	{
	}

	QNetworkReply* SimpleActor::MakeRequest ()
	{
		emit statusChanged (tr ("Requesting action..."));
		return Mgr_->post (A_->MakeRequest (URL_), Post_);
	}

	void SimpleActor::HandleReply (QNetworkReply*)
	{
		emit finished ();
	}
}
}
}
