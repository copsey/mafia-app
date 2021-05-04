#ifndef MAFIA_CORE_PLAYER_H
#define MAFIA_CORE_PLAYER_H

#include "../util/memory.hpp"
#include "../util/vector.hpp"

#include "role.hpp"
#include "time.hpp"

namespace maf::core {
	class Player {
	public:
		using ID = index;

		/// Create a player with the given ID and role.
		///
		/// Each status is set to its default value.
		Player(ID id, const Role & role) : _id{id}, _role{&role} { }

		// Unique ID given to the player for the game they're in.
		ID id() const { return _id; }

		const Role & role() const { return *_role; }
		// Change the player's role.
		void assign_role(const Role & role) { _role = &role; }

		Alignment alignment() const { return _role->alignment(); }
		Win_condition win_condition() const { return _role->win_condition(); }
		Peace_condition peace_condition() const { return _role->peace_condition(); }
		double duel_strength() const { return _role->duel_strength(); }
		bool is_role_faker() const { return _role->is_role_faker(); }
		bool is_troll() const { return _role->is_troll(); }

		bool is_alive() const { return _alive; }
		bool is_dead() const { return !_alive; }
		/// The date on which the player died.
		///
		/// @warning Undefined behaviour if the player is still alive.
		Date date_of_death() const { return _date_of_death; }
		/// The time at which the player died.
		///
		/// @warning Undefined behaviour if the player is still alive.
		Time time_of_death() const { return _time_of_death; }
		/// Kill the player, effectively removing them from the game.
		///
		/// @param date The date on which the player should be killed.
		/// @param time The time at which the player should be killed.
		void kill(Date date, Time time);

		bool is_present() const { return _present; }
		/// Make the player leave the town, without killing them.
		void leave() { _present = false; }

		bool has_been_kicked() const { return _kicked; }
		/// Kick the player from the game they are in, without killing them.
		void kick();

		bool has_been_lynched() const {	return _lynched; }
		/// Lynch the player, effectively removing them from the game.
		///
		/// @param date The date on which the player should be lynched.
		void lynch(Date date);


		bool has_fake_role() const { return _fake_role; }
		/// The role that the player must claim to have, or `nullptr` if none
		/// has been set.
		const Role * fake_role() const { return _fake_role; }
		/// Change the player's fake role.
		void give_fake_role(const Role & role) { _fake_role = &role; }

		/// Clear all temporary modifiers from the player, so that they are in a
		/// fresh state ready for the next day.
		void refresh();

		/// The abilities that the player must respond to before the game can
		/// continue.
		const vector<Ability> & compulsory_abilities() const {
			return _compulsory_abilities;
		}

		/// Add a compulsory ability that the player must respond to before the
		/// game can continue.
		void add_compulsory_ability(Ability ability);
		/// Remove a compulsory ability from the player.
		///
		/// This should be done when the player has responded to the compulsory
		/// ability.
		void remove_compulsory_ability(Ability ability);

		/// The player's current lynch vote, or `nullptr` if the player is not
		/// voting to lynch anybody.
		const Player * lynch_vote() const { return _lynch_vote; }
		/// Whether the player has a lynch vote.
		bool has_lynch_vote() const { return _lynch_vote != nullptr; }
		/// Choose a target for the player to vote against in the upcoming lynch.
		void cast_lynch_vote(const Player & target) { _lynch_vote = &target; }
		/// Remove the player's current lynch vote.
		void clear_lynch_vote() { _lynch_vote = nullptr; }

		/// Whether the player has won a duel at any point.
		bool has_won_duel() const { return _won_duel; }
		/// Make the player win a duel.
		void win_duel() { _won_duel = true; }

		/// Whether the player has been healed this night.
		bool is_healed() const { return _healed; }
		/// Heal the player for the current night.
		void heal() { _healed = true; }

		/// Whether the player currently appears as suspicious.
		bool is_suspicious() const { return role().is_suspicious() || _on_drugs; }
		/// Give the player some drugs for the current night.
		/// This forces the player to appear as suspicious.
		void give_drugs() { _on_drugs = true; }

		/// The ghost who is haunting this player, or `nullptr` if
		/// none exists.
		const Player * haunter() const { return _haunter; }
		/// Whether the player is being haunted by a ghost.
		bool is_haunted() const { return _haunter; }
		/// Haunt the player with a ghost!
		void haunt(const Player & haunter) { _haunter = &haunter; }

		/// Whether the player has won the game they are in.
		bool has_won() const { return _won_game; }
		/// Make the player win the game they are in.
		void win() { _won_game = true; }
		/// Make the player lose the game they are in.
		void lose() { _won_game = false; }

	private:
		ID _id;
		not_null<const Role *> _role;

		const Role * _fake_role = nullptr;

		bool _alive = true;
		bool _present = true;
		bool _kicked = false;
		Date _date_of_death;
		Time _time_of_death;

		vector<Ability> _compulsory_abilities = {};

		const Player * _lynch_vote = nullptr;
		const Player * _haunter = nullptr;

		bool _lynched = false;
		bool _won_duel = false;
		bool _won_game = false;

		// Temporary statuses
		bool _healed = false;
		bool _on_drugs = false;
	};

	/// Check if two players are the same.
	///
	/// This is fully determined by checking if they have the same ID.
	inline bool operator==(const Player & p1, const Player & p2) {
		return p1.id() == p2.id();
	}
}

#endif
