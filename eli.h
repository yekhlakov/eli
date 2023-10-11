#pragma once
#ifndef _MAXY_CONTROL_ELI_
#define _MAXY_CONTROL_ELI_

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <mutex>
#include <ostream>

namespace maxy
{
	namespace control
	{
		namespace ELI
		{
			/**
			* Embedded Lisp Interpreter
			*/
			class ELI
			{
			public:
				// Public types

				// Syntax Tree elements
				struct Node;
				struct Atom;
				struct List;
				struct Func;
				struct Builtin;
			
				// Container for a tree node
				using NodePtr = std::shared_ptr<Node>;
				// Type for the Symbol Table 
				using SymbolTable = std::unordered_map<std::string, NodePtr>;
				// Type for the Builtin Function Pointer
				using BuiltinFunc = NodePtr(*)(NodePtr, SymbolTable, ELI*);
				// The type of a function that can be registered as an External Function callable from within Lisp
				using ExtFunc = std::vector<std::string>(*)(std::vector<std::string>);

				// Syntax Tree Node (abstract)
				struct Node
				{
					enum class Type : int
					{
						Atom,
						List,
						Func
					};

					virtual ~Node() { }
					virtual Type type() = 0;
					virtual bool is_empty() = 0;
					virtual bool is_list() = 0;
					virtual bool is_atom() = 0;
					virtual bool is_func() = 0;
					virtual operator bool() = 0;
					virtual operator double() = 0;
					virtual NodePtr call(NodePtr tree, SymbolTable sym, ELI * eli) = 0;

					Atom* atom();
					List* list();
					Func* func();
					Builtin* builtin();

					virtual void output(std::ostream& os) = 0;

					bool compare(NodePtr other);

					std::string to_string(void);
				};

				// Atom node
				struct Atom : Node
				{
					std::string value;
					Atom() : value{ "" } {}
					Atom(std::string v) : value{ v } {}

					virtual ~Atom() {}
					virtual Type type() { return Type::Atom; }
					virtual bool is_empty() { return value == ""; }
					virtual bool is_list() { return false; }
					virtual bool is_atom() { return true; }
					virtual bool is_func() { return false; }
					virtual void output(std::ostream& os);
					virtual operator bool();
					virtual operator double();
					virtual NodePtr call(NodePtr tree, SymbolTable, ELI *) { return tree; }
				};

				// List node
				struct List : Node
				{
					std::vector<ELI::NodePtr> values;

					List() {}

					virtual ~List() {}

					virtual Type type() { return Type::List; }
					virtual bool is_empty() { return values.size() == 0; }
					virtual bool is_list() { return true; }
					virtual bool is_atom() { return false; }
					virtual bool is_func() { return false; }
					virtual void output(std::ostream& os);
					virtual operator bool();
					virtual operator double() { return 0.0L; }
					virtual NodePtr call(NodePtr tree, SymbolTable, ELI *) { return tree; }

					void push(NodePtr node) { values.push_back(node); }
				};

				// Func node
				struct Func : Node
				{
					std::vector<std::string> parameter_names;
					NodePtr body;
					std::unordered_map<std::string, NodePtr> symbols;

					Func() : body{ NodePtr(nullptr) } {}
					virtual ~Func() {}

					virtual Type type() { return Type::Func; }
					virtual bool is_empty() { return body == nullptr || body->is_empty(); }
					virtual bool is_list() { return false; }
					virtual bool is_atom() { return false; }
					virtual bool is_func() { return true; }
					virtual void output(std::ostream& os) { os << "<fn>"; }
					virtual operator bool() { return true; }
					virtual operator double() { return 0.0L; }
					virtual NodePtr call(NodePtr tree, SymbolTable sym, ELI * eli);
				};

				struct Builtin : Node
				{
					std::string name;
					BuiltinFunc fn;
					Builtin(std::string s, BuiltinFunc f) : name{ s }, fn{ f } {};
					virtual ~Builtin() {}
					virtual Type type() { return Type::Func; }
					virtual bool is_empty() { return false; }
					virtual bool is_list() { return false; }
					virtual bool is_atom() { return false; }
					virtual bool is_func() { return true; }
					virtual void output(std::ostream& os) { os << name; }
					virtual operator bool() { return true; }
					virtual operator double() { return 0.0L; }
					virtual NodePtr call(NodePtr tree, SymbolTable sym, ELI * eli);
				};

			private:
				// External Variable link
				struct ExtVar
				{
					enum class Type : int
					{
						Double,
						Float,
						Long,
						Ulong,
						Int,
						Uint,
						Bool
					};

					union {
						double* dptr;
						float* fptr;
						long long* lptr;
						unsigned long long* ulptr;
						int* iptr;
						unsigned int* uiptr;
						bool* bptr;
					};

					Type type;
					size_t components;
					bool readonly;

					ExtVar() : dptr{ nullptr }, type{ Type::Double }, components{ 0 }, readonly{ true } {}
					ExtVar(double* ptr, size_t comp = 1, bool ro = false) : dptr{ ptr }, type{ Type::Double }, components{ comp }, readonly{ ro } {};
					ExtVar(float* ptr, size_t comp = 1, bool ro = false) : fptr{ ptr }, type{ Type::Float }, components{ comp }, readonly{ ro } {};
					ExtVar(long long* ptr, size_t comp = 1, bool ro = false) : lptr{ ptr }, type{ Type::Long }, components{ comp }, readonly{ ro } {};
					ExtVar(unsigned long long* ptr, size_t comp = 1, bool ro = false) : ulptr{ ptr }, type{ Type::Ulong }, components{ comp }, readonly{ ro } {};
					ExtVar(int* ptr, size_t comp = 1, bool ro = false) : iptr{ ptr }, type{ Type::Int }, components{ comp }, readonly{ ro } {};
					ExtVar(unsigned int* ptr, size_t comp = 1, bool ro = false) : uiptr{ ptr }, type{ Type::Uint }, components{ comp }, readonly{ ro } {};
					ExtVar(bool* ptr, size_t comp = 1, bool ro = false) : bptr{ ptr }, type{ Type::Bool }, components{ comp }, readonly{ ro } {};
				};

				// exceptions
				struct Invalid_argument;
				struct Insufficient_arguments;
				struct Variable_not_found;
				struct Write_to_readonly_variable;
				struct Function_not_found;

				// Registered external variables
				std::unordered_map<std::string, ExtVar> variables;

				// Registered external functions
				std::unordered_map<std::string, ExtFunc> functions;

				// Builtin functions
				std::unordered_map<std::string, BuiltinFunc> builtins;

				// Global symbol table
				SymbolTable symbols;

				// a mutex for thread-safe execution of `def` operations
				std::mutex symbol_mutex;

			public:

				// PUBLIC INTERFACE:

				// Create a new Atom node from string
				NodePtr new_atom(std::string v);

				// Create a new Atom node from string
				NodePtr new_atom(const char* v);

				// Create a new Atom node from a double
				NodePtr new_atom(double d);

				// Create a new Atom node from a double
				NodePtr new_atom(long long ll);

				// Create a new Atom node from a double
				NodePtr new_atom(unsigned long long ull);

				// Create a new Atom node from a double
				NodePtr new_atom(bool b);

				// Create a new (empty) List node
				NodePtr new_list();

				// Create a new List node from a vector of strings (the strings are converted to Atoms)
				NodePtr new_list(std::vector<std::string> s);

				// Create a new Func node
				NodePtr new_func();

				// Create a new Builtin node
				NodePtr new_builtin(std::string name, BuiltinFunc fn);

				// The Interpreter Constructor
				ELI();

				~ELI() = default;

				// Register an application variable to be accessible from Lisp
				template<typename Ty>
				void var(const char* name, Ty* ptr, size_t components = 1, bool readonly = false)
				{
					variables[name] = ExtVar{ ptr, components, readonly };
				}

				// Get variable value (this is called from within Lisp)
				NodePtr get_var(const char* name);

				// Set the variable value (this is called from within Lisp)
				NodePtr set_var(const char* name, NodePtr value);

				// Register an External Function
				void func(const char* name, ExtFunc p);

				// Evaluate a given Syntax Tree producing a new Node
				NodePtr eval(NodePtr tree, SymbolTable sym);

				// Execute Lisp code
				std::pair<std::string, std::string> run(const char* text);
			};
		}
	}
}


#endif //_MAXY_CONTROL_ELI_

