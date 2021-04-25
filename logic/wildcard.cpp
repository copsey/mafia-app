#include <algorithm>

#include "../util/map.hpp"
#include "../util/random.hpp"

#include "wildcard.hpp"
#include "role_ref.hpp"
#include "rulebook.hpp"


maf::Wildcard::Wildcard(ID id, const std::map<Role::ID, double> & weights) :
	_id{id},
	_role_ids{util::key_begin(weights), util::key_end(weights)},
	_dist{util::item_begin(weights), util::item_end(weights)}
{
	auto is_negative = [](double w) { return w < 0; };
	auto is_zero = [](double w) { return w == 0; };

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
}

maf::string_view maf::Wildcard::alias() const {
	return maf::alias(_id);
}

bool maf::Wildcard::matches_alignment(Alignment alignment, const Rulebook & rulebook) const {
	if (uses_evaluator()) {
		auto wrong_alignment = [&](const Role & role) {
			return ((role.alignment() == alignment) && (_evaluator(role) > 0.0));
		};

		return std::none_of(rulebook.roles_begin(), rulebook.roles_end(), wrong_alignment);
	} else {
		auto probs = _dist.probabilities();

		for (std::size_t i{0}; i < _role_ids.size(); ++i) {
			auto& r = rulebook.look_up(_role_ids[i]);
			if (r.alignment() != alignment) {
				double p = probs[i];
				if (p > 0.0) return false;
			}
		}

		return true;
	}
}

const maf::Role & maf::Wildcard::pick_role(const Rulebook & rulebook) const {
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

		std::discrete_distribution<std::size_t> dist{weights.begin(), weights.end()};
		return roles[dist(util::random_engine)];
	} else {
		auto& mut_dist = const_cast<std::discrete_distribution<decltype(_role_ids)::size_type> &>(_dist);
		auto role_id = _role_ids[mut_dist(util::random_engine)];
		return rulebook.look_up(role_id);
	}
}

maf::string_view maf::alias(Wildcard::ID id) {
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
