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

#ifndef PLUGINS_AZOTH_INTERFACES_IADVANCEDCLENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IADVANCEDCLENTRY_H
#include "imucbookmarkeditorwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	/** This interface defines some advanced actions and signals on
	 * contact list entries, like methods for drawing attention and
	 * such.
	 *
	 * Entries implementing this interface should, of course, implement
	 * plain ICLEntry as well.
	 *
	 * @sa ICLEntry
	 */
	class IAdvancedCLEntry
	{
	public:
		virtual ~IAdvancedCLEntry () {}

		/** This enum represents some advanced features that may or may
		 * be not supported by advanced CL entries.
		 */
		enum AdvancedFeature
		{
			/** This entry supports drawing attention.
			 */
			AFSupportsAttention = 0x0001
		};

		Q_DECLARE_FLAGS (AdvancedFeatures, AdvancedFeature)

		/** Returns the OR-ed combination of advanced features supported
		 * by this contact list entry.
		 *
		 * @return The advanced features supported by this entry.
		 */
		virtual AdvancedFeatures GetAdvancedFeatures () const = 0;

		/** @brief Requests attention of the user behind this entry.
		 *
		 * This method, if called, should send request for attention to
		 * this entry, if supported by the protocol. An optional text
		 * message may be added to the attention request.
		 *
		 * If variant is an empty string, the variant with the highest
		 * priority should be used.
		 *
		 * @param[in] text Optional accompanying text.
		 * @param[in] variant The entry variant to buzz, or a null
		 * string for variant with highest priority.
		 *
		 * @sa attentionDrawn()
		 */
		virtual void DrawAttention (const QString& text, const QString& variant) = 0;

		/** @brief Notifies about attention request from this entry.
		 *
		 * This signal should be emitted by the entry whenever the user
		 * behind the entry decides to request our own attention.
		 *
		 * Depending on Azoth settings, the request may be displayed in
		 * some way or ignored completely.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] text Optional accompanying text.
		 * @param[out] variant Source variant of the entry that has
		 * requested our attention.
		 *
		 * @sa DrawAttention()
		 */
		virtual void attentionDrawn (const QString& text, const QString& variant) = 0;

		/** @brief Notifies that entry's user mood has changed.
		 *
		 * The actual mood information should be contained in the map
		 * returned from ICLEntry::GetClientInfo(). Please refer to its
		 * documentation for information about user moods.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose mood has
		 * changed.
		 *
		 * @sa activityChanged(), tuneChanged()
		 * @sa ICLEntry::GetClientInfo()
		 */
		virtual void moodChanged (const QString& variant) = 0;

		/** @brief Notifies that entry's user activity has changed.
		 *
		 * The actual activity information should be contained in the
		 * map returned from ICLEntry::GetClientInfo(). Please refer to
		 * its documentation for information about user activities.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose activity has
		 * changed.
		 *
		 * @sa moodChanged(), tuneChanged()
		 * @sa ICLEntry::GetClientInfo()
		 */
		virtual void activityChanged (const QString& variant) = 0;

		/** @brief Notifies that entry's user tune has changed.
		 *
		 * The actual tune information should be contained in the map
		 * returned from ICLEntry::GetClientInfo(). Please refer to its
		 * documentation for information about user tunes.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose tune has
		 * changed.
		 *
		 * @sa moodChanged(), activityChanged()
		 * @sa ICLEntry::GetClientInfo()
		 */
		virtual void tuneChanged (const QString& variant) = 0;

		/** @brief Notifies that entry's geolocation has changed.
		 *
		 * The actual geolocation information could be obtained via
		 * ISupportGeolocation::GetUserGeolocationInfo() method.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose location has
		 * changed.
		 * @sa ISupportGeolocation
		 */
		virtual void locationChanged (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IAdvancedCLEntry,
		"org.Deviant.LeechCraft.Azoth.IAdvancedCLEntry/1.0");

#endif
