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

#include "activitydialog.h"
#include <util/resourceloader.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	QString ActivityDialog::ToHumanReadable (const QString& str)
	{
		return tr (str.toLatin1 ());
	}

	ActivityDialog::ActivityDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		
		const char *genAct [] =
		{
			QT_TR_NOOP ("doing_chores"),
			QT_TR_NOOP ("drinking"),
			QT_TR_NOOP ("eating"),
			QT_TR_NOOP ("exercising"),
			QT_TR_NOOP ("grooming"),
			QT_TR_NOOP ("having_appointment"),
			QT_TR_NOOP ("inactive"),
			QT_TR_NOOP ("relaxing"),
			QT_TR_NOOP ("talking"),
			QT_TR_NOOP ("traveling"),
			QT_TR_NOOP ("working")
		};
		
		const char *specAct [] =
		{
			QT_TR_NOOP ("buying_groceries"),
			QT_TR_NOOP ("cleaning"),
			QT_TR_NOOP ("cooking"),
			QT_TR_NOOP ("doing_maintenance"),
			QT_TR_NOOP ("doing_the_dishes"),
			QT_TR_NOOP ("doing_the_laundry"),
			QT_TR_NOOP ("gardening"),
			QT_TR_NOOP ("running_an_errand"),
			QT_TR_NOOP ("walking_the_dog"),
			QT_TR_NOOP ("having_a_beer"),
			QT_TR_NOOP ("having_coffee"),
			QT_TR_NOOP ("having_tea"),
			QT_TR_NOOP ("having_a_snack"),
			QT_TR_NOOP ("having_breakfast"),
			QT_TR_NOOP ("having_dinner"),
			QT_TR_NOOP ("having_lunch"),
			QT_TR_NOOP ("cycling"),
			QT_TR_NOOP ("dancing"),
			QT_TR_NOOP ("hiking"),
			QT_TR_NOOP ("jogging"),
			QT_TR_NOOP ("playing_sports"),
			QT_TR_NOOP ("running"),
			QT_TR_NOOP ("skiing"),
			QT_TR_NOOP ("swimming"),
			QT_TR_NOOP ("working_out"),
			QT_TR_NOOP ("at_the_spa"),
			QT_TR_NOOP ("brushing_teeth"),
			QT_TR_NOOP ("getting_a_haircut"),
			QT_TR_NOOP ("shaving"),
			QT_TR_NOOP ("taking_a_bath"),
			QT_TR_NOOP ("taking_a_shower"),
			QT_TR_NOOP ("day_off"),
			QT_TR_NOOP ("hanging_out"),
			QT_TR_NOOP ("hiding"),
			QT_TR_NOOP ("on_vacation"),
			QT_TR_NOOP ("praying"),
			QT_TR_NOOP ("scheduled_holiday"),
			QT_TR_NOOP ("sleeping"),
			QT_TR_NOOP ("thinking"),
			QT_TR_NOOP ("fishing"),
			QT_TR_NOOP ("gaming"),
			QT_TR_NOOP ("going_out"),
			QT_TR_NOOP ("partying"),
			QT_TR_NOOP ("reading"),
			QT_TR_NOOP ("rehearsing"),
			QT_TR_NOOP ("shopping"),
			QT_TR_NOOP ("smoking"),
			QT_TR_NOOP ("socializing"),
			QT_TR_NOOP ("sunbathing"),
			QT_TR_NOOP ("watching_tv"),
			QT_TR_NOOP ("watching_a_movie"),
			QT_TR_NOOP ("in_real_life"),
			QT_TR_NOOP ("on_the_phone"),
			QT_TR_NOOP ("on_video_phone"),
			QT_TR_NOOP ("commuting"),
			QT_TR_NOOP ("cycling"),
			QT_TR_NOOP ("driving"),
			QT_TR_NOOP ("in_a_car"),
			QT_TR_NOOP ("on_a_bus"),
			QT_TR_NOOP ("on_a_plane"),
			QT_TR_NOOP ("on_a_train"),
			QT_TR_NOOP ("on_a_trip"),
			QT_TR_NOOP ("walking"),
			QT_TR_NOOP ("coding"),
			QT_TR_NOOP ("in_a_meeting"),
			QT_TR_NOOP ("studying"),
			QT_TR_NOOP ("writing"),
			QT_TR_NOOP ("other")
		};

		int sizes [] = { 9, 3, 4, 9, 6, 0, 8, 12, 3, 9, 4 };
		for (size_t i = 0, pos = 0; pos < sizeof (sizes) / sizeof (sizes [0]); ++pos)
			for (int j = 0; j < sizes [pos]; ++j, ++i)
				Gen2Specific_ [genAct [pos]] << specAct [i];
		
		Util::ResourceLoader *rl = Core::Instance ()
				.GetResourceLoader (Core::RLTActivityIconLoader);
		const QString& theme = XmlSettingsManager::Instance ()
				.property ("ActivityIcons").toString () + '/';
				
		Ui_.General_->addItem (tr ("<clear>"));
		
		for (size_t i = 0; i < sizeof (genAct) / sizeof (genAct [0]); ++i)
		{
			QIcon icon (rl->GetIconPath (theme + genAct [i]));
			Ui_.General_->addItem (icon, tr (genAct [i]), QString (genAct [i]));
		}
	}

	QString ActivityDialog::GetGeneral () const
	{
		return Ui_.General_->itemData (Ui_.General_->currentIndex ()).toString ();
	}

	void ActivityDialog::SetGeneral (const QString& general)
	{
		const int idx = std::max (0, Ui_.General_->findData (general));
		Ui_.General_->setCurrentIndex (idx);
		on_General__currentIndexChanged (idx);
	}

	QString ActivityDialog::GetSpecific () const
	{
		return Ui_.Specific_->itemData (Ui_.Specific_->currentIndex ()).toString ();
	}

	void ActivityDialog::SetSpecific (const QString& specific)
	{
		const int idx = std::max (0, Ui_.Specific_->findData (specific));
		Ui_.Specific_->setCurrentIndex (idx);
	}
	
	QString ActivityDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}
	
	void ActivityDialog::SetText (const QString& text)
	{
		Ui_.Text_->setText (text);
	}
	
	void ActivityDialog::on_General__currentIndexChanged (int idx)
	{
		Ui_.Specific_->clear ();
		
		Util::ResourceLoader *rl = Core::Instance ()
				.GetResourceLoader (Core::RLTActivityIconLoader);

		const QString& general = Ui_.General_->itemData (idx).toString ();
		const QString& theme = XmlSettingsManager::Instance ()
				.property ("ActivityIcons").toString () + '/';
		const QString& prefix = theme + general + '_';

		const QStringList& values = Gen2Specific_.value (general);
		Q_FOREACH (const QString& value, values)
		{
			QIcon icon (rl->GetIconPath (prefix + value));
			Ui_.Specific_->addItem (icon, tr (value.toLatin1 ()), value);
		}
	}
}
}
