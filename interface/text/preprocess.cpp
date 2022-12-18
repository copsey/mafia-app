#include <concepts>

#include "../../util/algorithm.hpp"
#include "../../util/char.hpp"
#include "../../util/memory.hpp"
#include "../../util/misc.hpp"
#include "../../util/optional.hpp"
#include "../../util/parse.hpp"
#include "../../util/string.hpp"
#include "../../util/type_traits.hpp"
#include "../../util/variant.hpp"
#include "../../util/vector.hpp"

#include "../format.hpp"

namespace maf::_preprocess_text_impl {
	using iterator = string_view::iterator;
	using error = preprocess_text_error;
	using errc = error::error_code;

	inline bool is_brace(char ch) {
		return ch == '{' || ch == '}';
	}

	// # Example
	// Given the following input:
	// ```
	//        begin  next                end
	//          |    |                     |
	// "easy {grace  } of her attitude, ..."
	// ```
	// return `next`.
	inline iterator find_brace(iterator begin, iterator end) {
		return std::find_if(begin, end, is_brace);
	}

	// # Example
	// Given the following input:
	// ```
	//      begin  next             end
	//         |   |                  |
	// "Wemmick    explained to me ..."
	// ```
	// return `next`.
	inline iterator skip_whitespace(iterator begin, iterator end) {
		return std::find_if_not(begin, end, util::is_whitespace);
	}


	// Get the parameter from `params` whose key is equal to `name`.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with code `parameter_not_available` if
	//   there is no parameter in `params` whose key is equal to `name`.
	inline auto get_param(string_view name, TextParams const& params)
	-> TextParam const& {
		auto iter = params.find(name);

		if (iter == params.end()) {
			throw error{errc::parameter_not_available, name};
		}

		return (*iter).second;
	}


	// Get the parameter from `params` whose key is equal to `name`, and
	// ensure that its type is `ParamType`.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with code `parameter_not_available` if
	//   there is no parameter in `params` whose key is equal to `name`.
	// - Throws `preprocess_text_error` with code `wrong_parameter_type` if a
	//   parameter was found, but its type is not `ParamType`.
	template <typename ParamType>
	auto get_param_as(string_view name, TextParams const& params)
	-> ParamType const& {
		auto& param = get_param(name, params);

		if (auto ptr = std::get_if<ParamType>(&param)) {
			return *ptr;
		} else {
			throw error{errc::wrong_parameter_type, name};
		}
	}


	// Visit the parameter from `params` whose key is equal to `name` using
	// the provided function.
	//
	// # Exceptions
	// - Throws `preprocess_text_error` with code `parameter_not_available` if
	//   there is no parameter in `params` whose key is equal to `name`.
	template <typename Visitor>
		requires std::invocable<Visitor, TextParam>
	auto visit_param(Visitor && f, string_view name, TextParams const& params)
	-> decltype(visit(f, TextParam{})) {
		auto& param = get_param(name, params);
		return visit(f, param);
	}


	// Check if `token` is a textual representation of an integer.
	// This is true if it matches the regex `/^-?\d+$/`.
	inline bool is_integer(string_view token) {
		if (!token.empty() && token.front() == '-') {
			token.remove_prefix(1);
		}

		return !token.empty() && util::all_of(token, util::is_digit);
	}

	void read_into(int & integer, string_view token);

	// # Example
	// Given the following input:
	// ```
	//           begin  next                   end
	//           |       |                       |
	// "{!if x < 12345678}Miss Havisham{!end} ..."
	// ```
	// set `integer` to `12345678` and return `next`.
	iterator parse_into(int & integer, iterator begin, iterator end);


	enum class relation {
		equals,
		less_than,
		greater_than
	};

	void read_into(relation & rel, string_view token);

	// # Example
	// Given the following input:
	// ```
	//         begin                     end
	//         |                           |
	// "{!if x = 3} Mother by adoption, ..."
	//          |
	//          next
	// ```
	// set `rel` to `equal_to` and return `next`.
	iterator parse_into(relation & rel, iterator begin, iterator end);


	struct comparison {
		struct operand: variant<int, string_view> {
			using variant<int, string_view>::variant;

			// Determine if the next token in `{begin, end}` is an integer or
			// the name of a parameter, and then parse it into `operand`.
			iterator parse(iterator begin, iterator end);
		};

		operand arg_1;
		operand arg_2;
		relation rel;

		// # Example
		// Given the following input:
		// ```
		//       begin      next                   end
		//       |           |                       |
		// "{!if x < 12345678}Miss Havisham{!end} ..."
		// ```
		// set `this->arg_1` to `"x"`, set `this->arg_2` to `12345678`, set
		// `this->rel` to `less_than`, and return `next`.
		iterator parse(iterator begin, iterator end);

		// Perform the comparison on `this->arg_1` and `this->arg_2`.
		//
		// Each operand is first converted to an integer value, by either:
		// - looking for a parameter in `params` by name, or
		// - returning the operand directly (for `int`s).
		bool resolve(TextParams const& params) const {
			auto x = _get_value(arg_1, params);
			auto y = _get_value(arg_2, params);
			return this->_compare(x, y);
		}

	private:
		static int _get_value(int val, TextParams const& params) {
			return val;
		}

		static int _get_value(string_view name, TextParams const& params) {
			return get_param_as<int>(name, params);
		}

		static int _get_value(operand const& arg, TextParams const& params) {
			auto get_value = [&](auto&& x) -> int {
				return _get_value(x, params);
			};

			return visit(get_value, arg);
		}

		bool _compare(int x, int y) const {
			switch (rel) {
			case relation::equals: return x == y;
			case relation::less_than: return x < y;
			case relation::greater_than: return x > y;
			}
		}
	};


	struct logical_test {
		struct predicate: variant<bool, string_view, comparison> {
			using variant<bool, string_view, comparison>::variant;
		};

		predicate pred;

		iterator parse(iterator begin, iterator end);

		bool resolve(TextParams const& params) const {
			return _get_value(pred, params);
		}

	private:
		static bool _get_value(bool val, TextParams const& params) {
			return val;
		}

		static bool _get_value(string_view name, TextParams const& params) {
			return get_param_as<bool>(name, params);
		}

		static bool _get_value(comparison const& comp, TextParams const& params) {
			return comp.resolve(params);
		}

		static bool _get_value(predicate const& pred, TextParams const& params) {
			auto get_value = [&](auto&& x) -> bool {
				return _get_value(x, params);
			};

			return visit(get_value, pred);
		}
	};


	enum class command {
		if_,
		else_if,
		else_,
		list,
		end
	};

	void read_into(command & cmd, string_view token);

	// # Example
	// Given the following input:
	// ```
	//         begin  next      end
	//          |      |          |
	// "Lorem {!else_if ipsum} ..."
	// ```
	// set `cmd` to `else_if` and return `next`.
	iterator parse_into(command & cmd, iterator begin, iterator end);


	struct directive {
		enum class type_t {
			comment,
			substitution,
			command
		};

		static bool is_delimiter(char ch) {
			return is_brace(ch) || util::is_whitespace(ch);
		}

		// # Example
		// Given the following input:
		// ```
		//    begin  next               end
		//       |   |                    |
		// "The {flag} had been struck ..."
		// ```
		// return `next`.
		static iterator find_delimiter(iterator begin, iterator end) {
			return std::find_if(begin, end, is_delimiter);
		}

		// # Example
		// Given the following input:
		// ```
		//             begin           end
		//             |                 |
		// "It was the best of times, ..."
		// ```
		// return `"best"`.
		static string_view get_token(iterator begin, iterator end) {
			auto token_end = find_delimiter(begin, end);
			return util::make_string_view(begin, token_end);
		}

		// # Example
		// Given the following input:
		// ```
		//       begin          end
		//       |                |
		// "{!if house_number = 13}Watch out!{!end}"
		// ```
		// return `3`.
		static int count_tokens(iterator begin, iterator end) {
			int count = 0;

			for (auto i = skip_whitespace(begin, end);
				 i != end && !is_brace(*i); )
			{
				++count;

				i = find_delimiter(i, end);
				i = skip_whitespace(i, end);
			}

			return count;
		}

		type_t type;
		string_view content;
		optional<command> cmd;
		optional<string_view> param_name;
		optional<logical_test> test;

		bool is_instance_of(type_t type) const {
			return this->type == type;
		}

		bool is_instance_of(command cmd) const {
			return this->type == type_t::command && *(this->cmd) == cmd;
		}

		void verify_instance_of(type_t type) {
			if (!this->is_instance_of(type)) _throw_unexpected_type();
		}

		void verify_instance_of(command cmd) {
			if (!this->is_instance_of(cmd)) _throw_unexpected_type();
		}

		// Indicates whether the directive will be replaced by a (non-empty)
		// string in the output. Used to decide what to do with whitespace
		// surrounding the directive.
		bool vanishes() const {
			switch (type) {
			case type_t::comment:
			case type_t::command:
				return true;
			case type_t::substitution:
				return false;
			}
		}

		string_view extract_command_name() const;

		// # Examples
		// In all of the examples below, the function returns `next`.
		//
		// 1. Given the following input:
		// ```
		//     begin  next      end
		//      |      |          |
		// "You {stock} and stone!"
		// ```
		// set `this->type` to `substitution` and set `this->param_name` to
		// `"stock"`.
		//
		// 2. Given the following input:
		// ```
		//  begin       next             end
		//  |            |                 |
		// "{!if Estella} looked at her ..."
		// ```
		// set `this->type` to `command`, set `this->cmd` to `if_` and set
		// `this->test` to `"Estella"`.
		//
		// 3. Given the following input:
		// ```
		//  begin                   end
		//  |                         |
		// "{!else}                   |
		//  so hard and thankless, ..."
		//  |
		//  next
		// ```
		// set `this->type` to `command` and set `this->cmd` to `"else_"`. Note
		// that the newline after `"{!else}"` is skipped.
		iterator parse(iterator begin, iterator end, string_view input);

	private:
		void _throw_unexpected_type() const {
			switch (this->type) {
			case type_t::comment:
				{
					// FIXME: Throw a bespoke exception.
					break;
				}

			case type_t::substitution:
				{
					// FIXME: Throw a bespoke exception.
					break;
				}

			case type_t::command:
				{
					auto cmd_name = this->extract_command_name();
					throw error{errc::unexpected_command, cmd_name};
				}
			}
		}

		iterator _parse_as_comment(iterator begin, iterator directive_end);
		iterator _parse_as_substitution(iterator begin, iterator directive_end);
		iterator _parse_as_command(iterator begin, iterator directive_end);
	};


	// Check if `token` is an allowed name for a text parameter.
	// This is true if it matches the regex `/^[a-zA-Z_\.]+$/`.
	inline bool is_param_name(string_view token) {
		auto is_valid_char = [](char ch) -> bool {
			switch (ch) {
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
			case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
			case 's': case 't': case 'u': case 'v': case 'w': case 'x':
			case 'y': case 'z': case 'A': case 'B': case 'C': case 'D':
			case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
			case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
			case 'W': case 'X': case 'Y': case 'Z': case '_': case '.':
				return true;

			default:
				return false;
			}
		};

		return !token.empty() && util::all_of(token, is_valid_char);
	}


	// # Example
	// Given the following input:
	// ```
	//        begin  next                 end
	//           |    |                     |
	// "After a {short} pause of repose, ..."
	// ```
	// set `name` to `"short"` and return `next`.
	inline iterator parse_param_name(iterator begin, iterator end,
									 string_view & name)
	{
		auto token = directive::get_token(begin, end);

		if (token.empty()) {
			throw error{errc::missing_parameter, begin};
		}

		if (!is_param_name(token)) {
			throw error{errc::token_not_param_name, token};
		}

		name = token;

		return token.end();
	}


	// # Example
	// Given the following input:
	// ```
	//            begin       end
	//            |           |
	//     "Lorem \{ ipsum ..."
	//              |
	//              next
	// ```
	// set `str` to `"\{"` and return `next`.
	inline iterator	parse_escape_sequence(iterator begin, iterator end,
										  string_view & esc_seq)
	{
		if (begin == end || *begin != '\\') {
			throw preprocess_text_error{errc::missing_escape_sequence, begin};
		}

		if (begin + 1 == end) {
			string_view token{begin,1};
			throw preprocess_text_error{errc::token_not_escape_sequence, token};
		}

		esc_seq = string_view{begin, 2};

		return esc_seq.end();
	}


	struct expression {
		virtual ~expression() = default;
		virtual void write(string & output, TextParams const& params) const = 0;
		virtual iterator parse(iterator begin, iterator end, string_view input) = 0;
	};


	struct plain_text: expression {
		string str;

		static bool is_delimiter(char ch) {
			return ch == '\\' || ch == '{' || ch == '}';
		}

		static iterator find_delimiter(iterator begin, iterator end) {
			return std::find_if(begin, end, is_delimiter);
		}

		void write(string & output, TextParams const& params) const override {
			output += str;
		}

		// Form the largest possible subrange of `{begin, end}` starting from
		// `begin` where none of the characters are braces. Append the
		// subrange to `this->str`.
		//
		// Escape sequences are left untouched.
		//
		// # Example
		// Given the following input:
		// ```
		//  begin       next       end
		//  |            |           |
		// "Donec \{quam {felis}, ..."
		// ```
		// append `"Donec \{quam "` to `this->str` and return `next`.
		iterator parse(iterator begin, iterator end, string_view input) override;
	};


	struct substitution: expression {
		string_view param_name;

		void write(string & output, TextParams const& params) const override;

		// # Example
		// Given the following input:
		// ```
		//          begin  next       end
		//          |      |            |
		// "If that {staid} old house..."
		// ```
		// set `this->param_name` to `"staid"` and return `next`.
		iterator parse(iterator begin, iterator end, string_view input) override;
	};


	struct sequence: expression {
		vector<unique_ptr<expression>> subexprs;

		void write(string & output, TextParams const& params) const override;

		iterator parse(iterator begin, iterator end, string_view input) override;

	private:
		template <typename ExprType>
			requires std::derived_from<ExprType, expression>
		iterator _parse_onto_end(iterator begin, iterator end, string_view input);
	};


	class conditional: public expression {
	public:
		using conditional_sequence = pair<logical_test, sequence>;

		vector<conditional_sequence> cond_subexprs;
		optional<sequence> default_subexpr;

		void write(string & output, TextParams const& params) const override;

		// # Example
		// Given the following input:
		// ```
		//        begin              next  end
		//        |                     |    |
		// "Lorem {!if ipsum} ... {!end} amet"
		// ```
		// append `{"ipsum", /* sequence parsed from "..." */}` to
		// `this->cond_subexprs` and return `next`.
		iterator parse(iterator begin, iterator end, string_view input) override;

	private:
		iterator _first;

		iterator _parse_first_cond_subexpr (iterator begin, iterator end,
			string_view input);
		iterator _parse_remaining_subexprs (iterator begin, iterator end,
			string_view input);
		iterator _parse_end (iterator begin, iterator end, string_view input);
	};


	class loop: public expression {
	public:
		string_view param_name;
		sequence subexpr;

		void write(string & output, TextParams const& params) const override;

		// # Example
		// Given the following input:
		// ```
		//       begin                  next           end
		//       |                       |               |
		// "easy {!list grace} ... {!end} of her attitude"
		// ```
		// set `this->param_name` to `"grace"`, set `this->subexpr` to the
		// result of parsing `...` as a sequence, and return `next`.
		iterator parse(iterator begin, iterator end, string_view input) override;

	private:
		iterator _first;

		iterator _parse_list_subexpr (iterator begin, iterator end,
			string_view input);
		iterator _parse_end (iterator begin, iterator end, string_view input);
	};


	/*
	 * Definitions of "write" functions
	 */


	inline void substitution::write(string & output, TextParams const& params) const {
		auto print = [&](auto&& arg) {
			using T = decay<decltype(arg)>;

			if constexpr (is_same<T, int>) {
				output += std::to_string(arg);
			} else if constexpr (is_same<T, string>) {
				output += arg;
			} else {
				throw error{errc::wrong_parameter_type, param_name};
			}
		};

		visit_param(print, param_name, params);
	}


	inline void sequence::write(string & output, TextParams const& params) const {
		for (auto & expr: this->subexprs) {
			expr->write(output, params);
		}
	}


	inline void conditional::write(string & output, TextParams const& params) const {
		for (auto& [test, expr]: this->cond_subexprs) {
			bool use_this_subexpr = test.resolve(params);

			if (use_this_subexpr) {
				expr.write(output, params);
				return;
			}
		}

		if (default_subexpr) default_subexpr->write(output, params);
	}


	inline void loop::write(string & output, TextParams const& params) const {
		auto& vec = get_param_as<vector<TextParams>>(param_name, params);

		for (auto&& subparams: vec) subexpr.write(output, subparams);
	}


	/*
	 * Definitions of "parse" functions
	 */


	inline void read_into(int & integer, string_view token) {
		if (!is_integer(token)) {
			throw error{errc::token_not_integer, token};
		}

		if (auto result = util::from_chars(token, integer);
			result.ec == std::errc::invalid_argument)
		{
			throw error{errc::token_not_integer, token};
		} else if (result.ec == std::errc::result_out_of_range) {
			throw error{errc::integer_out_of_range, token};
		}
	}


	inline iterator parse_into(int & integer, iterator begin, iterator end) {
		auto token = directive::get_token(begin, end);

		if (token.empty()) {
			throw error{errc::missing_integer, begin};
		}

		read_into(integer, token);

		return token.end();
	}


	inline void read_into(relation & rel, string_view token) {
		if (token == "<") {
			rel = relation::less_than;
		} else if (token == "=") {
			rel = relation::equals;
		} else if (token == ">") {
			rel = relation::greater_than;
		} else {
			throw error{errc::token_not_relation, token};
		}
	}


	inline iterator parse_into(relation & rel, iterator begin, iterator end) {
		auto token = directive::get_token(begin, end);

		if (token.empty()) {
			throw error{errc::missing_relation, begin};
		}

		read_into(rel, token);

		return token.end();
	}


	inline iterator comparison::operand::parse(iterator begin, iterator end) {
		auto token = directive::get_token(begin, end);

		if (token.empty()) {
			throw error{errc::missing_comp_operand, begin};
		}

		if (is_integer(token)) {
			int integer;

			if (auto result = util::from_chars(token, integer);
				result.ec == std::errc::invalid_argument)
			{
				throw error{errc::token_not_integer, token};
			} else if (result.ec == std::errc::result_out_of_range) {
				throw error{errc::integer_out_of_range, token};
			}

			(*this) = integer;
		} else if (is_param_name(token)) {
			(*this) = token;
		} else {
			throw error{errc::token_not_comp_operand, token};
		}

		return token.end();
	}


	inline iterator comparison::parse(iterator begin, iterator end) {
		auto i = begin;
		i = this->arg_1.parse(i, end);
		i = skip_whitespace(i, end);
		i = parse_into(this->rel, i, end);
		i = skip_whitespace(i, end);
		i = this->arg_2.parse(i, end);
		return i;
	}


	inline void read_into(command & cmd, string_view token) {
		if (token == "if") {
			cmd = command::if_;
		} else if (token == "else_if") {
			cmd = command::else_if;
		} else if (token == "else") {
			cmd = command::else_;
		} else if (token == "list") {
			cmd = command::list;
		} else if (token == "end") {
			cmd = command::end;
		} else {
			throw error{errc::token_not_command, token};
		}
	}


	inline iterator parse_into(command & cmd, iterator begin, iterator end) {
		auto token = directive::get_token(begin, end);

		if (token.empty()) {
			throw error{errc::missing_command, begin};
		}

		read_into(cmd, token);

		return token.end();
	}


	inline iterator logical_test::parse(iterator begin, iterator end) {
		auto i = begin;
		auto num_tokens = directive::count_tokens(begin, end);

		if (num_tokens >= 3) {
			comparison comp;
			i = comp.parse(i, end);
			this->pred = comp;
		} else {
			string_view param_name;
			i = parse_param_name(i, end, param_name);
			this->pred = param_name;
		}

		return i;
	}


	inline string_view directive::extract_command_name() const {
		if (!this->is_instance_of(type_t::command)) {
			// FIXME: Throw an exception.
		}

		auto begin = this->content.begin(), end = this->content.end();
		auto i = begin + "!"sv.size();
		i = skip_whitespace(i, end);
		return get_token(i, end);
	}


	inline iterator directive::parse(iterator begin, iterator end, string_view input) {
		if (char ch = *begin; ch == '}') {
			throw error{errc::too_many_closing_braces, begin};
		} else if (ch != '{') {
			// FIXME: Throw exception due to invalid syntax.
		}

		auto i = begin + "{"sv.size();
		auto dir_end = find_brace(i, end);

		if (dir_end == end || *dir_end != '}') {
			throw error{errc::unclosed_directive, begin};
		}

		this->content = util::make_string_view(i, dir_end);

		if (char prefix = *i; prefix == '!') {
			this->type = type_t::command;
			i = _parse_as_command(begin, dir_end);
		} else if (prefix == '-') {
			this->type = type_t::comment;
			i = _parse_as_comment(begin, dir_end);
		} else {
			this->type = type_t::substitution;
			i = _parse_as_substitution(begin, dir_end);
		}

		if (this->vanishes()) {
			bool no_text_before = (begin == input.begin() || *(begin - 1) == '\n');
			bool no_text_after = (i == input.end() || *i == '\n');
			if (no_text_before && no_text_after && i != end) i += "\n"sv.size();
		}

		return i;
	}


	inline iterator directive::_parse_as_comment(iterator begin, iterator dir_end) {
		return dir_end + "}"sv.size();
	}


	inline iterator directive::_parse_as_substitution(iterator begin, iterator dir_end) {
		auto i = begin + "{"sv.size();
		i = skip_whitespace(i, dir_end);

		string_view param_name;
		i = parse_param_name(i, dir_end, param_name);
		this->param_name = param_name;

		i = skip_whitespace(i, dir_end);

		if (i != dir_end) {
			auto token = directive::get_token(i, dir_end);
			throw error{errc::too_many_tokens, token};
		}

		return i + "}"sv.size();
	}


	inline iterator directive::_parse_as_command(iterator begin, iterator dir_end) {
		auto i = begin + "{!"sv.size();
		i = skip_whitespace(i, dir_end);

		command cmd;
		i = parse_into(cmd, i, dir_end);
		this->cmd = cmd;

		i = skip_whitespace(i, dir_end);

		if (cmd == command::if_ || cmd == command::else_if) {
			logical_test test;
			i = test.parse(i, dir_end);
			this->test = move(test);
		} else if (cmd == command::list) {
			string_view param_name;
			i = parse_param_name(i, dir_end, param_name);
			this->param_name = move(param_name);
		}

		i = skip_whitespace(i, dir_end);

		if (i != dir_end) {
			auto token = directive::get_token(i, dir_end);
			throw error{errc::too_many_tokens, token};
		}

		return i + "}"sv.size();
	}


	inline iterator plain_text::parse (iterator begin, iterator end,
		string_view input)
	{
		for (auto i = begin; ; ) {
			auto next = find_delimiter(i, end);
			str.append(i, next);
			i = next;

			if (i == end || *i != '\\') {
				return i;
			}

			string_view esc_seq;
			i = parse_escape_sequence(i, end, esc_seq);
			str += esc_seq;
		}
	}


	inline iterator substitution::parse(iterator begin, iterator end, string_view input) {
		directive dir;
		auto next = dir.parse(begin, end, input);

		// FIXME: Throw an exception if `dir.type` is not `substitution`.

		param_name = *dir.param_name;
		return next;
	}


	inline iterator sequence::parse (iterator begin, iterator end,
		string_view input)
	{
		for (auto i = begin; i != end; ) {
			if (char ch = *i; !is_brace(ch)) {
				i = _parse_onto_end<plain_text>(i, end, input);
				continue;
			}

			directive dir;
			auto next = dir.parse(i, end, input);

			using type = directive::type_t;
			if (dir.is_instance_of(type::comment)) {
				i = next;
			} else if (dir.is_instance_of(type::substitution)) {
				i = _parse_onto_end<substitution>(i, end, input);
			} else if (dir.is_instance_of(command::if_)) {
				i = _parse_onto_end<conditional>(i, end, input);
			} else if (dir.is_instance_of(command::list)) {
				i = _parse_onto_end<loop>(i, end, input);
			} else {
				return i; // stop when directive is not recognised.
			}
		}

		return end;
	}


	template <typename ExprType>
		requires std::derived_from<ExprType, expression>
	iterator sequence::_parse_onto_end(iterator begin, iterator end,
		string_view input)
	{
		auto expr = make_unique<ExprType>();
		auto next = expr->parse(begin, end, input);
		this->subexprs.push_back(move(expr));
		return next;
	}


	inline iterator conditional::parse(iterator begin, iterator end,
		string_view input)
	{
		auto i = this->_first = begin;
		i = _parse_first_cond_subexpr(i, end, input);
		i = _parse_remaining_subexprs(i, end, input);
		i = _parse_end(i, end, input);
		return i;
	}


	inline iterator conditional::_parse_first_cond_subexpr (iterator begin,
		iterator end, string_view input)
	{
		directive dir;
		auto next = dir.parse(begin, end, input);
		dir.verify_instance_of(command::if_);
		auto& test = *(dir.test);

		sequence expr;
		next = expr.parse(next, end, input);
		cond_subexprs.push_back({move(test), move(expr)});

		return next;
	}


	inline iterator conditional::_parse_remaining_subexprs (iterator
		begin, iterator end, string_view input)
	{
		auto i = begin;

		for ( ; i != end; ) {
			directive dir;
			auto subexpr_begin = dir.parse(i, end, input);

			if (dir.is_instance_of(command::else_if)) {
				auto& test = *(dir.test);

				sequence expr;
				i = expr.parse(subexpr_begin, end, input);
				cond_subexprs.push_back({move(test), move(expr)});
			} else if (dir.is_instance_of(command::else_)) {
				sequence expr;
				i = expr.parse(subexpr_begin, end, input);
				default_subexpr = move(expr);

				break;
			} else {
				break; // stop when the directive is not recognised
					   // note: `i` still points to the start of the directive
			}
		}

		return i;
	}


	inline iterator conditional::_parse_end (iterator begin, iterator end,
		string_view input)
	{
		if (begin == end) {
			throw error{errc::unclosed_expression, this->_first};
		}

		directive dir;
		auto next = dir.parse(begin, end, input);
		dir.verify_instance_of(command::end);

		return next;
	}


	inline iterator loop::parse (iterator begin, iterator end,
		string_view input)
	{
		auto i = this->_first = begin;
		i = _parse_list_subexpr(i, end, input);
		i = _parse_end(i, end, input);
		return i;
	}


	inline iterator loop::_parse_list_subexpr (iterator begin, iterator end,
		string_view input)
	{
		directive dir;
		auto next = dir.parse(begin, end, input);
		dir.verify_instance_of(command::list);
		this->param_name = *(dir.param_name);

		next = this->subexpr.parse(next, end, input);

		return next;
	}


	inline iterator loop::_parse_end (iterator begin, iterator end,
		string_view input)
	{
		if (begin == end) {
			throw error{errc::unclosed_expression, this->_first};
		}

		directive dir;
		auto next = dir.parse(begin, end, input);
		dir.verify_instance_of(command::end);

		return next;
	}
}


maf::string maf::preprocess_text(string_view input, TextParams const& params) {
	using namespace _preprocess_text_impl;

	string output;

	try {
		sequence expr;

		if (auto next = expr.parse(input.begin(), input.end(), input);
			next != input.end())
		{
			directive dir;
			dir.parse(next, input.end(), input);

			auto cmd_name = dir.extract_command_name();
			throw error{errc::unexpected_command, cmd_name};
		}

		expr.write(output, params);
	} catch (preprocess_text_error & error) {
		error.input = input;
		throw;
	}

	return output;
}


maf::index maf::preprocess_text_error::pos() const {
	using namespace _preprocess_text_impl;

	auto get_iter = [&](auto&& arg) {
		using T = decay<decltype(arg)>;

		static_assert(is_same<T, iterator> || is_same<T, string_view>,
		"Unexpected type for 'info' parameter in 'preprocess_text_error'");

		if constexpr (is_same<T, iterator>) {
			return arg;
		} else if constexpr (is_same<T, string_view>) {
			return arg.begin();
		}
	};

	auto iter = visit(get_iter, param);
	return iter - input.begin();
}


maf::string maf::preprocess_text_error::message() const {
	string msg;
	auto pos = std::to_string(this->pos());

	switch (this->code) {
	case error_code::token_not_command:
		msg += "Invalid command \"";
		msg += std::get<string_view>(param);
		msg += "\" at position ";
		msg += pos;
		break;

	case error_code::token_not_comp_operand:
		msg += "Expected either an integer or a parameter, instead got \"";
		msg += std::get<string_view>(param);
		msg += "\", at position ";
		msg += pos;
		break;

	case error_code::token_not_escape_sequence:
		msg += "Invalid escape sequence \"";
		msg += std::get<string_view>(param);
		msg += "\" at position ";
		msg += pos;
		break;

	case error_code::token_not_integer:
		msg += "Expected an integer, instead got \"";
		msg += std::get<string_view>(param);
		msg += "\", at position ";
		msg += pos;
		break;

	case error_code::token_not_param_name:
		msg += "Expected a parameter, instead got \"";
		msg += std::get<string_view>(param);
		msg += "\", at position ";
		msg += pos;
		break;

	case error_code::token_not_relation:
		msg += "Expected a relation (e.g. '<'), instead got \"";
		msg += std::get<string_view>(param);
		msg += "\", at position ";
		msg += pos;
		break;

	case error_code::missing_command:
		msg += "Expected a command at position ";
		msg += pos;
		break;

	case error_code::missing_comp_operand:
		msg += "Expected an integer or parameter at position ";
		msg += pos;
		break;

	case error_code::missing_parameter:
		msg += "Expected a parameter at position ";
		msg += pos;
		break;

	case error_code::missing_relation:
		msg += "Expected a relation at position ";
		msg += pos;
		break;

	case error_code::parameter_not_available:
		msg += "Unrecognised parameter name \"";
		msg += std::get<string_view>(param);
		msg += "\" at position ";
		msg += pos;
		break;

	case error_code::too_many_closing_braces:
		msg += "Unexpected '}' at position ";
		msg += pos;
		break;

	case error_code::too_many_tokens:
		msg += "An extra token \"";
		msg += std::get<string_view>(param);
		msg += "\" appears at position ";
		msg += pos;
		break;

	case error_code::unclosed_directive:
		msg += "No closing brace found for '{' at position ";
		msg += pos;
		break;

	case error_code::unclosed_expression:
		msg += "No \"end\" directive found for the expression starting at position ";
		msg += pos;
		break;

	case error_code::unexpected_command:
		msg += "The command \"";
		msg += std::get<string_view>(param);
		msg += "\" cannot be used at position ";
		msg += pos;
		break;

	case error_code::wrong_parameter_type:
		msg += "The parameter \"";
		msg += std::get<string_view>(param);
		msg += "\" has the wrong type at position ";
		msg += pos;
		break;

	case error_code::missing_integer:
		msg += "Expected an integer at position ";
		msg += pos;
		break;

	case error_code::integer_out_of_range:
		msg += "The integer ";
		msg += std::get<string_view>(param);
		msg += " is too large, located at position ";
		msg += pos;
		break;

	case error_code::missing_escape_sequence:
		msg += "Expected an escape sequence at position ";
		msg += pos;
		break;
	}

	return msg;
}
