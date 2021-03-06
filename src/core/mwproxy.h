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

#ifndef MWPROXY_H
#define MWPROXY_H
#include <QObject>
#include "interfaces/imwproxy.h"

namespace LeechCraft
{
	class MainWindow;

	class MWProxy : public QObject
				  , public IMWProxy
	{
		Q_OBJECT
		Q_INTERFACES (IMWProxy)

		MainWindow *Win_;
	public:
		MWProxy (MainWindow*, QObject* = 0);

		void AddDockWidget (Qt::DockWidgetArea, QDockWidget*);
		void AssociateDockWidget (QDockWidget*, QWidget*);
		void ToggleViewActionVisiblity (QDockWidget*, bool);
		void SetViewActionShortcut (QDockWidget*, const QKeySequence&);
		void AddToolbar (QToolBar*, Qt::ToolBarArea);
		void AddSideWidget (QWidget*, WidgetArea);

		void ToggleVisibility ();

		QMenu* GetMainMenu ();
		void HideMainMenu ();
	};
}

#endif
