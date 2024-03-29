#include <algorithm>

#include "../util/algorithm.hpp"
#include "../util/map.hpp"
#include "../util/random.hpp"

#include "wildcard.hpp"
#include "role_ref.hpp"
#include "rulebook.hpp"

namespace maf::core {
	Wildcard::Wildcard(ID id, const std::map<Role::ID, double> & weights) :
		_id{id},
		_role_ids{util::key_begin(weights), util::key_end(weights)},
		_dist{}
	{
		auto is_zero = [](auto&& x) { return x == 0; };
		auto is_negative = [](auto&& x) { return x < 0; };

		if (std::any_of(util::item_begin(weights), util::item_end(weights), is_negative)) {
			string msg = "A wildcard with alias ";
			msg += alias();
			msg += " was created with a negative role weight.";

			throw std::invalid_argument{msg};
		}

		if (std::all_of(util::item_begin(weights), util::item_end(weights), is_zero)) {
			string msg = "A wildcard with alias ";
			msg += alias();
			msg += " was created with every role weight set to zero.";

			throw std::invalid_argument{msg};
		}

		_dist = {util::item_begin(weights), util::item_end(weights)};
	}

	string_view Wildcard::alias() const {
		return core::alias(_id);
	}

	bool Wildcard::matches_alignment(Alignment alignment, const Rulebook & rulebook) const {
		if (uses_evaluator()) {
			auto wrong_alignment = [&](const Role & role) {
				return ((role.alignment() == alignment) && (_evaluator(role) > 0.0));
			};

			return std::none_of(rulebook.roles_begin(), rulebook.roles_end(), wrong_alignment);
		} else {
			auto probs = _dist.probabilities();

			for (index i = 0, n = _role_ids.size(); i < n; ++i) {
				auto& role_id = _role_ids[i];
				auto& role = rulebook.look_up(role_id);
				auto& p = probs[i];

				if (role.alignment() != alignment && p > 0)
					return false;
			}

			return true;
		}
	}

	const Role & Wildcard::pick_role(const Rulebook & rulebook) const {
		if (uses_evaluator()) {
			vector_of_refs<const Role> roles{};
			vector<double> weights{};

			auto evaluate_role = [&](const Role & role) {
				double w = _evaluator(role);

				if (w < 0.0) {
					string msg = "A wildcard with alias ";
					msg += alias();
					msg += " returned the negative role weight of ";
					msg += std::to_string(w);
					msg += " for the role with alias ";
					msg += role.alias();
					msg += ".";

					throw std::logic_error{msg};
				} else if (w > 0.0) {
					roles.emplace_back(role);
					weights.push_back(w);
				}
			};

			rulebook.for_each_role(evaluate_role);

			if (roles.empty()) {
				string msg = "A wildcard with alias ";
				msg += alias();
				msg += " chose zero as the weight of every role in the rulebook.";

				throw std::logic_error{msg};
			}

			return *util::random::pick(roles, weights);
		} else {
			auto& mut_dist = const_cast<std::discrete_distribution<index> &>(_dist);
			auto i = mut_dist(util::random::default_generator);
			auto role_id = _role_ids[i];
			return rulebook.look_up(role_id);
		}
	}

	string_view alias(Wildcard::ID id) {
		switch (id) {
		case Wildcard::ID::any:
			return "random";
		case Wildcard::ID::village:
			return "any_village";
		case Wildcard::ID::village_basic:
			return "basic_village";
		case Wildcard::ID::mafia:
			return "any_mafia";
		case Wildcard::ID::freelance:
			return "any_freelance";
		}
	}
}
