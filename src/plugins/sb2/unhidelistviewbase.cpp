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

#include "unhidelistviewbase.h"
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sys/paths.h>
#include "unhidelistmodel.h"
#include "autoresizemixin.h"

namespace LeechCraft
{
namespace SB2
{
	UnhideListViewBase::UnhideListViewBase (const QPoint& orig,
			ViewManager *viewMgr, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new UnhideListModel (this))
	{
		new Util::UnhoverDeleteMixin (this);
		new AutoResizeMixin (orig, viewMgr, this);

		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "UnhideListView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			deleteLater ();
			return;
		}

		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("unhideListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		setSource (QUrl::fromLocalFile (file));

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
	}

	void UnhideListViewBase::BeginModelFill ()
	{
		QMetaObject::invokeMethod (Model_, "modelAboutToBeReset");
		Model_->blockSignals (true);
	}

	void UnhideListViewBase::EndModelFill ()
	{
		Model_->blockSignals (false);
		QMetaObject::invokeMethod (Model_, "modelReset");;
	}
}
}
