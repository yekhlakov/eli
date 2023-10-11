#include <sstream>
#include "eli.h"

#define VALUES(x) x->list()->values
#define VAL_SIZE VALUES(tree).size()
#define CHECK_ARG_COUNT(c) if (VAL_SIZE < c) throw Insufficient_arguments{tree}
#define BUILTIN_SIGNATURE [](NodePtr tree, SymbolTable sym, ELI* eli)
#define EVAL_ARG(idx) eli->eval(VALUES(tree)[idx], sym)
#define ENSURE_ATOM(x) if (!x->is_atom()) throw Invalid_argument{x}
#define ENSURE_LIST(x) if (!x->is_list()) throw Invalid_argument{x}
#define ENSURE_FUNC(x) if (!x->is_func()) throw Invalid_argument{x}
#define ENSURE_NOT_EMPTY(x) if (x->is_empty()) throw Invalid_argument{x}

namespace maxy
{
	namespace control
	{
		namespace ELI
		{
			// exceptions
			struct ELI::Invalid_argument
			{
				std::string message;
				Invalid_argument(NodePtr node) : message{ node->to_string() } {}
			};
			struct ELI::Insufficient_arguments
			{
				std::string message;
				Insufficient_arguments(NodePtr node) : message{ node->to_string() } {}
			};
			struct ELI::Variable_not_found
			{
				std::string var;
				Variable_not_found(std::string s) : var{ s } {};
			};
			struct ELI::Write_to_readonly_variable
			{
				std::string var;
				Write_to_readonly_variable(std::string s) : var{ s } {};
			};
			struct ELI::Function_not_found
			{
				std::string func;
				Function_not_found(std::string s) : func{ s } {};
			};

			std::string ELI::Node::to_string(void)
			{
				std::ostringstream os;
				this->output(os);
				return os.str();
			}

			bool ELI::Node::compare(ELI::NodePtr other)
			{
				if (type() != other->type()) return false;

				switch (type())
				{
				case Type::Atom:
					return atom()->value == other->atom()->value;

				case Type::Func:
					return false; // todo: compare functions

				case Type::List:
					if (list()->values.size() != other->list()->values.size()) return false;

					for (size_t i = 0; i < list()->values.size(); i++)
						if (!list()->values[i]->compare(other->list()->values[i]))
							return false;
				}
				return true;
			}

			ELI::Atom* ELI::Node::atom() { return dynamic_cast<Atom*>(this); }
			ELI::List* ELI::Node::list() { return dynamic_cast<List*>(this); }
			ELI::Func* ELI::Node::func() { return dynamic_cast<Func*>(this); }
			ELI::Builtin* ELI::Node::builtin() { return dynamic_cast<Builtin*>(this); }

			void ELI::Atom::output(std::ostream& os)
			{
				os << value;
			}

			ELI::Atom::operator bool()
			{
				auto d = std::atof(value.c_str());
				return d != 0.0l || value == "true";
			}

			ELI::Atom::operator double()
			{
				return std::atof(value.c_str());
			}

			void ELI::List::output(std::ostream& os)
			{
				os << '(';

				for (size_t i = 0; i < values.size(); i++)
				{
					values[i]->output(os);

					if (i < values.size() - 1) os << ' ';
				}

				os << ')';
			}

			ELI::List::operator bool()
			{
				return values.size() > 0;
			}

			// CALL Lisp function
			ELI::NodePtr ELI::Func::call(ELI::NodePtr tree, ELI::SymbolTable sym, ELI *eli)
			{
				auto head = VALUES(tree)[0];
				auto count = head->func()->parameter_names.size();

				if (VAL_SIZE < count + 1) throw Insufficient_arguments{ tree };

				std::vector<NodePtr> params;
				for (size_t i = 0; i < count; i++)
				{
					params.push_back(eli->eval(VALUES(tree)[1 + i], sym));
				}

				for (size_t i = 0; i < head->func()->parameter_names.size(); i++)
				{
					sym[head->func()->parameter_names[i]] = params[i];
				}

				return eli->eval(head->func()->body, sym);
			}

			// CALL Builtin function
			ELI::NodePtr ELI::Builtin::call(ELI::NodePtr tree, ELI::SymbolTable sym, ELI * eli)
			{
				return fn(tree, sym, eli);
			}

			// Create a new Atom node from string
			ELI::NodePtr ELI::new_atom(std::string v)
			{
				return std::make_shared<Atom>(v);
			}

			// Create a new Atom node from string
			ELI::NodePtr ELI::new_atom(const char* v)
			{
				return std::make_shared<Atom>(std::string{ v });
			}

			// Create a new Atom node from a double
			ELI::NodePtr ELI::new_atom(double d)
			{
				if (std::isnan(d))
				{
					return std::make_shared<Atom>("nan");
				}

				char chars[64];

				std::snprintf(chars, 64, "%.15f", d);
				std::string buf(chars);

				// remove trailing zeroes
				buf.erase(buf.find_last_not_of("0") + 1);

				// remove trailing decimal point
				buf.erase(buf.find_last_not_of(".") + 1);

				return std::make_shared<Atom>(buf);
			}

			// Create a new Atom node from a double
			ELI::NodePtr ELI::new_atom(long long ll)
			{
				return std::make_shared<Atom>(std::to_string(ll));
			}

			// Create a new Atom node from a double
			ELI::NodePtr ELI::new_atom(unsigned long long ull)
			{
				return std::make_shared<Atom>(std::to_string(ull));
			}

			// Create a new Atom node from a double
			ELI::NodePtr ELI::new_atom(bool b)
			{
				return std::make_shared<Atom>(b ? "1" : "");
			}

			// Create a new (empty) List node
			ELI::NodePtr ELI::new_list()
			{
				return std::make_shared<List>();
			}

			// Create a new List node from a vector of strings (the strings are converted to Atoms)
			ELI::NodePtr ELI::new_list(std::vector<std::string> s)
			{
				auto a = std::make_shared<List>();
				for (auto str : s)
				{
					a->list()->values.push_back(new_atom(str));
				}
				return a;
			}

			// Create a new Func node
			ELI::NodePtr ELI::new_func()
			{
				return std::make_shared<Func>();
			}

			// Create a new Builtin node
			ELI::NodePtr ELI::new_builtin(std::string name, ELI::BuiltinFunc fn)
			{
				return std::make_shared<Builtin>(name, fn);
			}


			// Get variable value (this is called from within Lisp)
			ELI::NodePtr ELI::get_var(const char* name)
			{
				auto vararg = variables.find(name);

				if (vararg == variables.end())
					throw Variable_not_found{ name };

				auto var = vararg->second;

				auto list = new_list();

				for (size_t i = 0; i < var.components; i++)
					switch (var.type)
					{
					case ExtVar::Type::Double:
						list->list()->values.push_back(new_atom(var.dptr[i]));
						break;
					case ExtVar::Type::Float:
						list->list()->values.push_back(new_atom(var.fptr[i]));
						break;
					case ExtVar::Type::Long:
						list->list()->values.push_back(new_atom(var.lptr[i]));
						break;
					case ExtVar::Type::Int:
						list->list()->values.push_back(new_atom((long long)var.iptr[i]));
						break;
					case ExtVar::Type::Ulong:
						list->list()->values.push_back(new_atom(var.ulptr[i]));
						break;
					case ExtVar::Type::Uint:
						list->list()->values.push_back(new_atom((unsigned long long)var.uiptr[i]));
						break;
					case ExtVar::Type::Bool:
						list->list()->values.push_back(new_atom((long long)var.bptr[i]));
						break;
					}

				return list;
			}


			// Set the variable value (this is called from within Lisp)
			ELI::NodePtr ELI::set_var(const char* name, NodePtr value)
			{
				if (!value->is_list())
					throw Invalid_argument{ value };

				auto vararg = variables.find(name);

				if (vararg == variables.end())
					throw Variable_not_found{ name };

				auto var = vararg->second;

				if (var.readonly)
					throw Write_to_readonly_variable{ name };

				if (value->list()->values.size() < var.components)
					throw Insufficient_arguments{ value };

				for (size_t i = 0; i < var.components; i++)
					switch (var.type)
					{
					case ExtVar::Type::Double:
						var.dptr[i] = (double)*value->list()->values[i]->atom();
						break;
					case ExtVar::Type::Float:
						var.fptr[i] = (float)(double)*value->list()->values[i]->atom();
						break;
					case ExtVar::Type::Long:
						var.lptr[i] = (long long)(double)*value->list()->values[i]->atom();
						break;
					case ExtVar::Type::Int:
						var.iptr[i] = (unsigned int)(double)*value->list()->values[i]->atom();
						break;
					case ExtVar::Type::Ulong:
						var.ulptr[i] = (unsigned long long) (double) * value->list()->values[i]->atom();
						break;
					case ExtVar::Type::Uint:
						var.uiptr[i] = (unsigned int)(double)*value->list()->values[i]->atom();
						break;
					case ExtVar::Type::Bool:
						var.bptr[i] = (bool)*value->list()->values[i]->atom();
						break;
					}

				return new_atom("");
			}

			// Register an External Function
			void ELI::func(const char* name, ExtFunc p)
			{
				functions[name] = p;
			}

			// ELI constructor
			ELI::ELI()
			{
				// Language primitives
				builtins["seq"] = BUILTIN_SIGNATURE{
					if (VAL_SIZE == 0) return eli->new_atom("");

					for (size_t i = 1; i < VAL_SIZE - 1; i++)
						EVAL_ARG(i);

					return EVAL_ARG(VAL_SIZE - 1);
				};

				builtins["val"] = BUILTIN_SIGNATURE{
					auto list = eli->new_list();
					VALUES(list).insert(VALUES(list).end(), ++VALUES(tree).begin(), VALUES(tree).end());
					return list;
				};

#define BUILTIN_CHECK(chk) BUILTIN_SIGNATURE{\
					CHECK_ARG_COUNT(2);\
					auto a0 = EVAL_ARG(1);\
					return eli->new_atom(a0->chk() ? "1" : "");\
				}

				builtins["empty"] = BUILTIN_CHECK(is_empty);
				builtins["atom"] = BUILTIN_CHECK(is_atom);
				builtins["list"] = BUILTIN_CHECK(is_list);
				builtins["func"] = BUILTIN_CHECK(is_func);

				builtins["if"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(4);

					auto max_count = VAL_SIZE - 1;

					for (size_t i = 1; i < max_count; i += 2)
					{
						if ((bool)*EVAL_ARG(i))
							return EVAL_ARG(i + 1);
					}

					return EVAL_ARG(max_count);
				};

				builtins["id"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					return EVAL_ARG(1);
				};

				builtins["head"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto src = EVAL_ARG(1);
					ENSURE_LIST(src);
					ENSURE_NOT_EMPTY(src);
					return VALUES(src)[0];
				};

				builtins["tail"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto src = EVAL_ARG(1);
					ENSURE_LIST(src);
					if (src->is_empty()) return eli->new_list();
					auto list = eli->new_list();
					VALUES(list).insert(VALUES(list).end(), ++VALUES(src).begin(), VALUES(src).end());
					return list;
				};


				builtins["cons"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);

					auto src_list = EVAL_ARG(2);

					ENSURE_LIST(src_list);

					auto list = eli->new_list();

					VALUES(list).push_back(EVAL_ARG(1));

					VALUES(list).insert(VALUES(list).end(), VALUES(src_list).begin(), VALUES(src_list).end());

					return list;
				};

				builtins["fn"] = BUILTIN_SIGNATURE{
					auto fn = eli->new_func();

					CHECK_ARG_COUNT(2);

					for (size_t i = 1; i < VAL_SIZE - 1; i++)
					{
						// parameter names should be atoms
						if (!VALUES(tree)[i]->is_atom()) continue;

						fn->func()->parameter_names.push_back(VALUES(tree)[i]->atom()->value);
						fn->func()->body = VALUES(tree)[VAL_SIZE - 1];
					}

					return fn;
				};

				builtins["let"] = BUILTIN_SIGNATURE{

					CHECK_ARG_COUNT(4);

				    // create a local copy of the symbol table
				    auto local_sym = sym;

					for (size_t i = 1; i < VAL_SIZE - 2; i += 2)
					{
						// names should only be atoms
						if (!VALUES(tree)[i]->is_atom()) continue;

						local_sym[VALUES(tree)[i]->atom()->value] = eli->eval(VALUES(tree)[i + 1], local_sym);
					}

					return eli->eval(VALUES(tree)[VAL_SIZE - 1], local_sym);
				};

				builtins["def"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);

					for (size_t i = 1; i < VAL_SIZE - 1; i += 2)
					{
						// names should only be atoms
						if (!VALUES(tree)[i]->is_atom()) continue;

						auto x = std::lock_guard<std::mutex>(eli->symbol_mutex);
						eli->symbols[VALUES(tree)[i]->atom()->value] = EVAL_ARG(i + 1);
					}

					return eli->new_atom("");
				};

				// Operations on atoms
				builtins["!"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto a0 = EVAL_ARG(1);
					return eli->new_atom(!(bool)*a0);
				};

#define BUILTIN_BINARY(expr) BUILTIN_SIGNATURE{\
					CHECK_ARG_COUNT(3);\
					auto a0 = EVAL_ARG(1);\
					auto a1 = EVAL_ARG(2);\
					return eli->new_atom(expr);\
				};

				builtins["&"] = BUILTIN_BINARY((bool)*a0 && (bool)*a1);
				builtins["|"] = BUILTIN_BINARY((bool)*a0 || (bool)*a1);
				builtins["^"] = BUILTIN_BINARY((((bool)*a0) && !((bool)*a1)) || (!((bool)*a0) && ((bool)*a1)));
				builtins["+"] = BUILTIN_BINARY((double)*a0 + (double)*a1);
				builtins["*"] = BUILTIN_BINARY((double)*a0 * (double)*a1);
				builtins["-"] = BUILTIN_BINARY((double)*a0 - (double)*a1);
				builtins["/"] = BUILTIN_BINARY((double)*a0 / (double)*a1);
				builtins["%"] = BUILTIN_BINARY(std::fmod((double)*a0, (double)*a1));
				builtins["<"] = BUILTIN_BINARY((double)*a0 < (double)*a1);
				builtins[">"] = BUILTIN_BINARY((double)*a0 > (double)*a1);
				builtins["<="] = BUILTIN_BINARY((double)*a0 <= (double)*a1);
				builtins[">="] = BUILTIN_BINARY((double)*a0 >= (double)*a1);
				builtins["="] = BUILTIN_BINARY(a0->compare(a1));
				builtins["!="] = BUILTIN_BINARY(!a0->compare(a1));


				// Integrations
				builtins["get"] = BUILTIN_SIGNATURE{
					// Get value from external variable
					CHECK_ARG_COUNT(2);
					ENSURE_ATOM(VALUES(tree)[1]);
					return eli->get_var(VALUES(tree)[1]->atom()->value.c_str());
				};

				builtins["set"] = BUILTIN_SIGNATURE{
					// Set value of external variable
					CHECK_ARG_COUNT(3);
					ENSURE_ATOM(VALUES(tree)[1]);

					return eli->set_var
					(
						VALUES(tree)[1]->atom()->value.c_str(),
						EVAL_ARG(2)
					);
				};

				builtins["call"] = BUILTIN_SIGNATURE{
					// Call external function, return its result
					CHECK_ARG_COUNT(3);
					ENSURE_ATOM(VALUES(tree)[1]);

					auto funcname = VALUES(tree)[1]->atom()->value;
					auto funcptr = eli->functions.find(funcname);
					if (funcptr == eli->functions.end())
						throw Function_not_found{funcname};

					std::vector<std::string> v;

					auto a1 = EVAL_ARG(2);
					ENSURE_LIST(a1);

					for (auto param : a1->list()->values)
					{
						v.push_back (param->to_string ());
					}

					return eli->new_list((*(funcptr->second)) (v));
				};

				// cmath proxy

#define MATH_UNARY(op) BUILTIN_SIGNATURE{\
					CHECK_ARG_COUNT(2);\
					auto a0 = EVAL_ARG(1);\
					ENSURE_ATOM(a0);\
					return eli->new_atom((double)std::op((long double) (double)*a0));\
				}

				builtins["sqrt"] = MATH_UNARY(sqrt);
				builtins["abs"] = MATH_UNARY(abs);
				builtins["sin"] = MATH_UNARY(sin);
				builtins["cos"] = MATH_UNARY(cos);
				builtins["tan"] = MATH_UNARY(tan);
				builtins["asin"] = MATH_UNARY(asin);
				builtins["acos"] = MATH_UNARY(acos);
				builtins["atan"] = MATH_UNARY(atan);
				builtins["sinCos"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto a0 = EVAL_ARG(1);
					ENSURE_ATOM(a0);

					auto v = (double)*a0;
					auto list = eli->new_list();
					VALUES(list).push_back(eli->new_atom(std::sin(v)));
					VALUES(list).push_back(eli->new_atom(std::cos(v)));

					return list;
				};

				builtins["atan2"] = BUILTIN_BINARY(std::atan2((double)*a0, (double)*a1));
				builtins["pow"] = BUILTIN_BINARY(std::pow((double)*a1, (double)*a0));
				builtins["log"] = BUILTIN_BINARY(std::log((double)*a1) / std::log((double)*a0));
				builtins["floor"] = MATH_UNARY(floor);
				builtins["ceil"] = MATH_UNARY(ceil);

				// More complex functions aka STDLIB
				builtins["length"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto a0 = EVAL_ARG(1);
					ENSURE_LIST(a0);

					return eli->new_atom((unsigned long long) VALUES(a0).size());
				};

				builtins["reverse"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto a0 = EVAL_ARG(1);
					ENSURE_LIST(a0);
					auto list = eli->new_list();

					std::reverse_copy(VALUES(a0).begin(), VALUES(a0).end(), std::back_inserter(VALUES(list)));
					return list;
				};

				builtins["concat"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_LIST(a0);
					ENSURE_LIST(a1);

					auto list = eli->new_list();
					std::copy(VALUES(a0).begin(), VALUES(a0).end(), std::back_inserter(VALUES(list)));
					std::copy(VALUES(a1).begin(), VALUES(a1).end(), std::back_inserter(VALUES(list)));
					return list;
				};

				builtins["iota"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(2);
					auto a0 = EVAL_ARG(1);
					ENSURE_ATOM(a0);
					auto list = eli->new_list();

					for (unsigned long long i = 0; i < (double)*a0; i++)
						VALUES(list).push_back(eli->new_atom(i));

					return list;
				};

				builtins["take"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_ATOM(a0);
					ENSURE_LIST(a1);

					if (a1->is_empty()) return a1;

					auto list = eli->new_list();

					for (size_t i = 0; i < (double)*a0 && i < VALUES(a1).size(); i++)
						VALUES(list).push_back(VALUES(a1)[i]);

					return list;
				};

				builtins["drop"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_ATOM(a0);
					ENSURE_LIST(a1);

					if (a1->is_empty()) return a1;

					auto list = eli->new_list();

					for (auto i = (size_t)(double)*a0; i < VALUES(a1).size(); i++)
						VALUES(list).push_back(VALUES(a1)[i]);

					return list;
				};

				builtins["map"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);

					ENSURE_FUNC(a0);
					ENSURE_LIST(a1);

					if (a1->is_empty()) return a1;

					auto list = eli->new_list();
					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(eli->new_atom(""));

					for (auto v : VALUES(a1))
					{
						VALUES(invocation)[1] = v;
						VALUES(list).push_back(eli->eval(invocation, sym));
					}

					return list;
				};

				builtins["filter"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);

					ENSURE_FUNC(a0);
					ENSURE_LIST(a1);

					if (a1->is_empty()) return a1;

					auto list = eli->new_list();
					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(eli->new_atom(""));

					for (auto v : VALUES(a1))
					{
						VALUES(invocation)[1] = v;
						auto predicate = eli->eval(invocation, sym);

						if ((bool)*predicate)
							VALUES(list).push_back(v);
					}

					return list;
				};

				builtins["zipWith"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(4);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					auto a2 = EVAL_ARG(3);

					ENSURE_FUNC(a0);
					ENSURE_LIST(a1);
					ENSURE_LIST(a2);

					auto list = eli->new_list();

					if (a1->is_empty() || a2->is_empty()) return list;

					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(eli->new_atom(""));
					VALUES(invocation).push_back(eli->new_atom(""));

					for (size_t i = 0; i < VALUES(a1).size() && i < VALUES(a2).size(); i++)
					{
						VALUES(invocation)[1] = VALUES(a1)[i];
						VALUES(invocation)[2] = VALUES(a2)[i];
						VALUES(list).push_back(eli->eval(invocation, sym));
					}
					
					return list;
				};

				builtins["takeWhile"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_FUNC(a0);
					ENSURE_LIST(a1);

					if (a1->is_empty()) return a1;

					auto list = eli->new_list();
					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(eli->new_atom(""));

					for (auto v : VALUES(a1))
					{
						VALUES(invocation)[1] = v;
						auto predicate = eli->eval(invocation, sym);
						if (!(bool)*predicate) break;
						VALUES(list).push_back(v);
					}

					return list;
				};

				builtins["dropWhile"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_FUNC(a0);
					ENSURE_LIST(a1);

					if (a1->is_empty()) return a1;

					auto list = eli->new_list();
					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(eli->new_atom(""));

					bool nodrop = false;

					for (auto v : VALUES(a1))
					{
						if (!nodrop)
						{
							VALUES(invocation)[1] = v;
							auto predicate = eli->eval(invocation, sym);
							if ((bool)*predicate) continue;

							nodrop = true;
						}
						VALUES(list).push_back(v);
					}

					return list;
				};

				builtins["repeat"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_ATOM(a0);

					auto list = eli->new_list();

					auto count = (size_t)(double)*a0;

					while (count--)
					{
						VALUES(list).push_back(a1);
					}

					return list;
				};

				builtins["foldl1"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1);
					auto a1 = EVAL_ARG(2);
					ENSURE_FUNC(a0);
					ENSURE_NOT_EMPTY(a1);
					ENSURE_LIST(a1);

					if (VALUES(a1).size() == 1) return VALUES(a1)[0];

					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(VALUES(a1)[0]);
					VALUES(invocation).push_back(VALUES(a1)[1]);

					auto accum = eli->eval(invocation, sym);

					for (size_t i = 2; i < VALUES(a1).size(); i++)
					{
						VALUES(invocation)[1] = accum;
						VALUES(invocation)[2] = VALUES(a1)[i];
						accum = eli->eval(invocation, sym);
					}

					return accum;
				};

				builtins["foldl"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(4);
					auto a0 = EVAL_ARG(1); // fn
					auto a1 = EVAL_ARG(2); // accum
					auto a2 = EVAL_ARG(3); // list

					ENSURE_FUNC(a0);
					ENSURE_LIST(a2);

					if (a2->is_empty()) return a1;

					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(a1);
					VALUES(invocation).push_back(VALUES(a2)[0]);

					auto accum = eli->eval(invocation, sym);

					for (size_t i = 1; i < VALUES(a2).size(); i++)
					{
						VALUES(invocation)[1] = accum;
						VALUES(invocation)[2] = VALUES(a2)[i];
						accum = eli->eval(invocation, sym);
					}

					return accum;
				};

				builtins["foldr"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(4);
					auto a0 = EVAL_ARG(1); // fn
					auto a1 = EVAL_ARG(2); // accum
					auto a2 = EVAL_ARG(3); // list

					ENSURE_FUNC(a0);
					ENSURE_LIST(a2);

					if (a2->is_empty()) return a1;

					auto count = VALUES(a2).size();
					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(VALUES(a2)[count - 1]);
					VALUES(invocation).push_back(a1);

					auto accum = eli->eval(invocation, sym);

					for (int i = count - 2; i >= 0; i--)
					{
						VALUES(invocation)[1] = VALUES(a2)[i];
						VALUES(invocation)[2] = accum;
						accum = eli->eval(invocation, sym);
					}

					return accum;
				};

				builtins["foldr1"] = BUILTIN_SIGNATURE{
					CHECK_ARG_COUNT(3);
					auto a0 = EVAL_ARG(1); // fn
					auto a1 = EVAL_ARG(2); // list

					ENSURE_FUNC(a0);
					ENSURE_NOT_EMPTY(a1);
					ENSURE_LIST(a1);

					auto count = VALUES(a1).size();

					if (count == 1) return VALUES(a1)[0];

					auto invocation = eli->new_list();
					VALUES(invocation).push_back(a0);
					VALUES(invocation).push_back(VALUES(a1)[count - 2]);
					VALUES(invocation).push_back(VALUES(a1)[count - 1]);

					auto accum = eli->eval(invocation, sym);

					for (int i = count - 3; i >= 0; i--)
					{
						VALUES(invocation)[1] = VALUES(a1)[i];
						VALUES(invocation)[2] = accum;
						accum = eli->eval(invocation, sym);
					}

					return accum;
				};
			}

			// Evaluate Lisp tree
			ELI::NodePtr ELI::eval(NodePtr tree, SymbolTable sym)
			{
				if (tree->is_func() || tree->is_empty())
				{
					return tree;
				}

				if (tree->is_atom())
				{
					// search local table
					auto x = sym.find(tree->atom()->value);
					if (x != sym.end()) return x->second;
					// search global table
					auto y = symbols.find(tree->atom()->value);
					if (y != symbols.end()) return y->second;
					// search builtin function
					auto bptr = builtins.find(tree->atom()->value);
					if (bptr != builtins.end()) return new_builtin(bptr->first, bptr->second);

					return tree;
				}

				if (tree->is_list())
				{
					auto head = eval(tree->list()->values[0], sym);
					VALUES(tree)[0] = head;
					if (head->is_func()) return head->call(tree, sym, this);
				}

				return tree;
			}

			// Lisp source code parser
			struct Parser
			{
				const char* const text;
				size_t i;
				ELI* eli;

				inline bool is_whitespace()
				{
					return text[i] == ' ' || text[i] == 9 || text[i] == 10 || text[i] == 13;
				}

				inline void skip_whitespace()
				{
					while (is_whitespace()) i++;
				}

				Parser(ELI* e, const char* const _text) : eli{ e }, text{ _text }, i{ 0 } {};

				inline ELI::NodePtr parse_comment()
				{
					while (text[i] && text[i] != '}') i++;
					if (text[i] == '}') i++;
					return parse();
				}

				inline bool is_token_separator()
				{
					return is_whitespace() || text[i] == '(' || text[i] == ')' || text[i] == '{';
				}

				inline ELI::NodePtr parse_token()
				{
					auto token_start = i;

					while (text[i] && !is_token_separator()) i++;

					return eli->new_atom(std::string(text + token_start, text + i));
				}

				inline ELI::NodePtr parse_list()
				{
					auto output = eli->new_list();
					while (true)
					{
						auto element = parse();

						skip_whitespace();

						if (element->is_atom() && element->is_empty())
						{
							if (text[i] == ')') i++;
							break;
						}

						output->list()->push(element);

						if (text[i] == ')')
						{
							i++;
							break;
						}
					}

					return output;
				}

				ELI::NodePtr parse()
				{
					skip_whitespace();

					switch (text[i])
					{
					case 0: return eli->new_atom("");
					case '(': i++;  return parse_list();
					case '{': i++;  return parse_comment();
					default: return parse_token();
					}
				}
			};

			std::pair<std::string, std::string> ELI::run(const char* text)
			{
				Parser parser(this, text);

				auto tree = parser.parse();

				try
				{
					auto result = eval(tree, SymbolTable{});

					return std::make_pair(result->to_string(), "");
				}
				catch (Invalid_argument invarg)
				{
					return std::make_pair("", "Invalid argument " + invarg.message);
				}
				catch (Insufficient_arguments insuff)
				{
					return std::make_pair("", "Insufficient arguments " + insuff.message);
				}
				catch (Variable_not_found vnf)
				{
					return std::make_pair("", "External variable not found " + vnf.var);
				}
				catch (Write_to_readonly_variable ro)
				{
					return std::make_pair("", "Attempted write to read-only variable " + ro.var);
				}
				catch (Function_not_found fnf)
				{
					return std::make_pair("", "Function not found " + fnf.func);
				}
			}
		}
	}
}
