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

#ifndef PLUGINS_AZOTH_INTERFACES_IACCOUNT_H
#define PLUGINS_AZOTH_INTERFACES_IACCOUNT_H
#include <QFlags>
#include <QMetaType>
#include <QVariant>
#include <QStringList>
#include <interfaces/azoth/azothcommon.h>
#include "iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	class IProtocol;
	class ICLEntry;

	/** @brief Interface representing a single account.
	 *
	 * This interface represents an account — an entity within IProtocol
	 * owning some ICLEntry objects.
	 *
	 * The account may also implement IHaveServiceDiscovery if it
	 * supports some XMPP's service discovery-like functionality. Look
	 * at that documentation for more information.
	 *
	 * If the account supports something similar to XML console, it can
	 * also implement the IHaveConsole interface.
	 *
	 * If the account supports publishing user tune, it may implement
	 * ISupportTune.
	 *
	 * @sa IHaveServiceDiscovery, IHaveConsole
	 * @sa ISupportTune, ISupportMood, ISupportActivity
	 * @sa ISupportGeolocation
	 */
	class IAccount
	{
		bool IsShown_;
	public:
		IAccount ()
		: IsShown_ (true)
		{
		}

		virtual ~IAccount () {}

		/** Represents the features that may be supported by an acoount.
		 */
		enum AccountFeature
		{
			/** This account can be renamed, so calls to RenameAccount()
			 * would not be senseless.
			 */
			FRenamable = 0x01,

			/** This account supports Extended Away statuses.
			 */
			FSupportsXA = 0x02,

			/** This account has configuration dialog.
			 */
			FHasConfigurationDialog = 0x04,

			/** This account can add accounts to CL while being offline.
			 */
			FCanAddContactsInOffline = 0x08,

			/** This account supports viewing contacts information while
			 * offline.
			 */
			FCanViewContactsInfoInOffline = 0x10,

			/** This account supports sending and receiving files to or
			 * from MUCs.
			 */
			FMUCsSupportFileTransfers = 0x20
		};

		Q_DECLARE_FLAGS (AccountFeatures, AccountFeature)

		/** @brief Returns the account object as a QObject.
		 *
		 * @return Account object as QObject.
		 */
		virtual QObject* GetQObject () = 0;

		/** @brief Returns the pointer to the parent protocol that this
		 * account belongs to.
		 *
		 * @return The parent protocol of this account.
		 */
		virtual QObject* GetParentProtocol () const = 0;

		/** @brief Returns the OR-ed combination of features of this
		 * account.
		 *
		 * @return The features of this account.
		 */
		virtual AccountFeatures GetAccountFeatures () const = 0;

		/** @brief Returns the list of contact list entries of this
		 * account.
		 *
		 * Typically this would be the list of contacts added to the
		 * account plus any entries that represent multiuser chatrooms,
		 * their participants and such, if applicable.
		 *
		 * @return The list of contact list entries of this account.
		 */
		virtual QList<QObject*> GetCLEntries () = 0;

		/** @brief Returns the human-readable name of this account.
		 *
		 * @return Human-readable name of this account.
		 *
		 * @sa RenameAccount()
		 */
		virtual QString GetAccountName () const = 0;

		/** @brief Returns the nickname of our user.
		 *
		 * @return Nickname of our user.
		 */
		virtual QString GetOurNick () const = 0;

		/** @brief Sets the human-readable name of this account to the
		 * new name.
		 *
		 * @param[in] name The new name of the account.
		 *
		 * @sa GetAccountName()
		 */
		virtual void RenameAccount (const QString& name) = 0;

		/** @brief Returns the ID of this account.
		 *
		 * The returned ID should be unique among all accounts and
		 * should not depend on the value of GetAccountName()
		 * (the human-readable name of the account).
		 *
		 * @return The unique and persistent account ID.
		 */
		virtual QByteArray GetAccountID () const = 0;

		/** @brief Returns the list of actions for the account item.
		 *
		 * The list is showed, for example, when user calls the context
		 * menu on the account item, or such.
		 *
		 * @return The list of actions.
		 */
		virtual QList<QAction*> GetActions () const = 0;

		/** @brief Request message w/ info/vcard information for the given address.
		 *
		 * The address should be in format compatible with the result of
		 * ICLEntry::GetHumanReadableID() function whenever possible.
		 *
		 * @param[in] address Address or entry ID to query.
		 */
		virtual void QueryInfo (const QString& address) = 0;

		/** @brief Requests the account to open its configuration dialog.
		 */
		virtual void OpenConfigurationDialog () = 0;

		/** @brief Returns the current status of this account.
		 *
		 * @return Current status of this account.
		 */
		virtual EntryStatus GetState () const = 0;

		/** @brief Sets the status of this account.
		 *
		 * If the account was offline, it is expected to connect at this
		 * point automatically.
		 *
		 * @param[in] status The new status of this account.
		 */
		virtual void ChangeState (const EntryStatus& status) = 0;

		/** @brief Grants authorization to the given entry.
		 *
		 * entry is expected to be deleted in this function.
		 *
		 * @param[in] entry Entry object implementing ICLEntry.
		 */
		virtual void Authorize (QObject *entry) = 0;

		/** @brief Denies authorization for the given entry.
		 *
		 * entry is expected to be deleted in this function.
		 *
		 * @param[in] entry Entry object implementing ICLEntry.
		 */
		virtual void DenyAuth (QObject *entry) = 0;

		/** @brief Requests authorization from the given entry.
		 *
		 * entry is a human-readable identifier of the remote, as
		 * returned by ICLEntry::GetHumanReadableID().
		 *
		 * If applicable and msg is a non-empty string, the msg should
		 * be sent to the entry as the message sent along with the
		 * request.
		 *
		 * name should be the name under which the entry is added to the
		 * contact list, if applicable. If name is an empty string, a
		 * sane default should be used, like the human-readable ID —
		 * entry.
		 *
		 * groups is the list of groups that this entry is added to. If
		 * groups is an emptry list, the behavior should be defined by
		 * the plugin - the entry should either be added to some kind of
		 * default group or have no assigned groups at all. If protocol
		 * supports only one group per entry, the first one in the list
		 * should be used.
		 *
		 * @param[in] entry Human-readable ID of the remote.
		 * @param[in] msg Optional request string to display to remote,
		 * if applicable.
		 */
		virtual void RequestAuth (const QString& entry,
				const QString& msg = QString (),
				const QString& name = QString (),
				const QStringList& groups = QStringList ()) = 0;

		/** @brief Removes the given entry from the contact list.
		 *
		 * Removes the entry from the contact list and automatically
		 * synchronizes the changes in the contact list, if applicable.
		 *
		 * If the entry could not be removed at the moment, or if the
		 * parameter is invalid, this function should do nothing.
		 *
		 * @param[in] entry The entry to remove.
		 */
		virtual void RemoveEntry (QObject *entry) = 0;

		/** @brief Returns the object responsible for file transfers in
		 * this account.
		 *
		 * If file transfers aren't supported, NULL should be returned.
		 * The returned object, is not NULL, is expected to implement
		 * ITransferManager.
		 *
		 * @return The file transfer manager, or NULL if not supported.
		 */
		virtual QObject* GetTransferManager () const = 0;

		virtual bool IsShownInRoster () const
		{
			return IsShown_;
		}

		virtual void SetShownInRoster (bool shown)
		{
			IsShown_ = shown;
		}

		/** @brief This signal should be emitted when account is renamed.
		 *
		 * This signal should be emitted even after an explicit call to
		 * RenameAccount().
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] newName The new name of this account.
		 */
		virtual void accountRenamed (const QString& newName) = 0;

		/** @brief This signal should be emitted when new contact list
		 * items appear in this account.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] items The list of newly appeared items.
		 */
		virtual void gotCLItems (const QList<QObject*>& items) = 0;

		/** @brief This signal should be emitted after any contact list
		 * items are removed.
		 *
		 * The reason for removal doesn't matter. It could be a
		 * groupchat participant that exited or changed nickname, or
		 * some other stuff.
		 *
		 * @note This functions is expected to be a signal.
		 *
		 * @param[out] items The list of removed items.
		 */
		virtual void removedCLItems (const QList<QObject*>& items) = 0;

		/** @brief This signal should be emitted when another user
		 * requests authorization from this account.
		 *
		 * When a remote user requests authorization (or subscription in
		 * terms of XMPP, for example) from this account, this signal
		 * should be emitted. The entry is expected to represent the
		 * remote that requested the authorization and it must implement
		 * ICLEntry.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The object representing the requesting
		 * entry, must be an ICLEntry.
		 * @param[out] message Optional request message, if applicable.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * itemSubscribed(), itemUnsubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		virtual void authorizationRequested (QObject *entry,
				const QString& message) = 0;

		/** @brief This signal should be emitted when an already added
		 * entry has just subscribed to us.
		 *
		 * If the item didn't previously exist, the proper gotCLItems()
		 * signal should be emitted before this one, of course.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The object representing the just subscribed
		 * entry in the contact list, must be an ICLEntry.
		 * @param[out] message An optional reason message.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * authorizationRequested(), itemUnsubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		virtual void itemSubscribed (QObject *entry, const QString& message) = 0;

		/** @brief This signal should be emitted when an already added
		 * entry has just unsubscribed from us.
		 *
		 * If the item didn't exist in the roster, another overload of
		 * itemUnsubscribed() should be used, which takes the entry's ID
		 * as first parameter.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The object representing the unsubscribed
		 * entry in the contact list, must be an ICLEntry.
		 * @param[out] message An optional reason message.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * authorizationRequested(), itemSubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		virtual void itemUnsubscribed (QObject *entry, const QString& message) = 0;

		/** @brief This signal should be emitted when a non-roster item
		 * has just unsubscribed from us.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entryID The ID of just unsubscribed entry.
		 * @param[out] message An optional reason message.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * authorizationRequested(), itemSubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		virtual void itemUnsubscribed (const QString& entryID, const QString& message) = 0;

		/** @brief This signal should be emitted when a roster item
		 * cancels (or denies) our subscription.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The object representing the entry that
		 * granted the subscription, must be an ICLEntry.
		 * @param[out] message Optional reason message.
		 *
		 * @sa RequestAuth(), authorizationRequested(), itemSubscribed(),
		 * itemGrantedSubscription()
		 */
		virtual void itemCancelledSubscription (QObject *entry, const QString& message) = 0;

		/** @brief This signal should be emitted when a roster item
		 * grants us subscription.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The object representing the entry that
		 * granted the subscription, must be an ICLEntry.
		 * @param[out] message Optional reason message.
		 *
		 * @sa RequestAuth(), authorizationRequested(), itemSubscribed(),
		 * itemCancelledSubscription()
		 */
		virtual void itemGrantedSubscription (QObject *entry, const QString& message) = 0;

		/** @brief This signal should be emitted when state of this
		 * account changes for whatever reason.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] state New state of this account.
		 */
		virtual void statusChanged (const EntryStatus&) = 0;

		/** @brief This signal should be emitted whenever a MUC
		 * invitation has been received.
		 *
		 * The ident parameter contains the map with the identifying
		 * data suitable for the IMUCJoinWidget of this account. Refer
		 * to IMUCJoinWidget documentation for more information.
		 *
		 * @param[out] ident MUC identifying data for IMUCJoinWidget.
		 * @param[out] inviter The inviter's source ID or nickname.
		 * @param[out] reason An optional reason string.
		 */
		virtual void mucInvitationReceived (const QVariantMap& ident,
				const QString& inviter, const QString& reason) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IAccount::AccountFeatures);
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::IAccount*);
Q_DECLARE_INTERFACE (LeechCraft::Azoth::IAccount,
		"org.Deviant.LeechCraft.Azoth.IAccount/1.0");

#endif
