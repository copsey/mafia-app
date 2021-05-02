#include "player.hpp"

namespace maf::core {
	void Player::kill(Date date, Time time) {
		_alive = false;
		_present = false;
		_date_of_death = date;
		_time_of_death = time;
	}

	void Player::kick() {
		leave();
		_kicked = true;
	}

	void Player::lynch(Date date) {
		kill(date, Time::day);
		_lynched = true;
	}

	void Player::refresh() {
		_compulsory_abilities.clear();

		_lynch_vote = nullptr;

		_healed = false;
		_on_drugs = false;
	}

	void Player::add_compulsory_ability(Ability ability) {
		_compulsory_abilities.push_back(ability);
	}

	void Player::remove_compulsory_ability(Ability ability) {
		for (auto it = _compulsory_abilities.begin(); it != _compulsory_abilities.end(); ++it) {
			if ((*it).id == ability.id) {
				_compulsory_abilities.erase(it);
				return;
			}
		}
		/* FIXME: throw exception if compulsory ability is not currently stored. */
	}
}
