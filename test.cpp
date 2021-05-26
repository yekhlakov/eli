#include <iostream>
#include <thread>

#include "eli.h"


std::vector<std::string> testfunc(std::vector<std::string> params)
{
	std::cout << "CALLED FROM INSIDE ELI\n";
	std::cout << "Params: ";
	for (auto s : params)
	{
		std::cout << s << ", ";
	}

	std::cout << "\n";

	return std::vector<std::string>{"Lol Kek"};
}

void test_classes()
{
	std::cout << "\n\nTesting classes\n";

	auto eli = new maxy::control::ELI::ELI();

	auto list = eli->new_list();
	list->list()->push(eli->new_atom("x"));

	auto func = eli->new_func();
	func->func()->body = list;

	std::vector<std::tuple<std::string, maxy::control::ELI::ELI::NodePtr, bool, bool, bool, bool>> test_cases =
	{
		std::make_tuple("empty atom", eli->new_atom(""), true, true, false, false),
		std::make_tuple("non-empty atom", eli->new_atom("LOL"), false, true, false, false),
		std::make_tuple("empty list", eli->new_list(), true, false, true, false),
		std::make_tuple("non-empty list", list, false, false, true, false),
		std::make_tuple("empty func", eli->new_func(), true, false, false, true),
		std::make_tuple("non-empty func", func, false, false, false, true),
	};

	auto count = 0, failed = 0;

	for (auto test_case : test_cases)
	{
		count++;
		auto failure = false;

		if (std::get<1>(test_case)->is_empty() != std::get<2>(test_case))
		{
			std::cout << std::get<0>(test_case) << ": is_empty failed\n";
			failure = true;
		}
		if (std::get<1>(test_case)->is_atom() != std::get<3>(test_case))
		{
			std::cout << std::get<0>(test_case) << ": is_atom failed\n";
			failure = true;
		}
		if (std::get<1>(test_case)->is_list() != std::get<4>(test_case))
		{
			std::cout << std::get<0>(test_case) << ": is_list failed\n";
			failure = true;
		}
		if (std::get<1>(test_case)->is_func() != std::get<5>(test_case))
		{
			std::cout << std::get<0>(test_case) << ": is_func failed\n";
			failure = true;
		}
		if (failure) failed++;
	}

	std::cout << "-----------------------------\n" << count << " TESTS, " << failed << " FAILURES\n\n";
}

void test_basic_functions()
{
	std::cout << "\n\nTesting builtin functions\n";

	std::vector<std::vector<const char *>> test_cases =
	{
		// primitives
		{"(empty ())", "1", ""},
		{"(empty 666)", "", ""},
		{"(empty (fn x (x)))", "", ""},
		{"(empty (fn x ()))", "1", ""},
		{"(empty *)", "", ""},
		{"(atom ())", "", ""},
		{"(atom 666)", "1", ""},
		{"(atom (fn x (x)))", "", ""},
		{"(atom =)", "", ""},
		{"(list ())", "1", ""},
		{"(list 666)", "", ""},
		{"(list (fn x (x)))", "", ""},
		{"(list +", "", ""},
		{"(func ())", "", ""},
		{"(func 666)", "", ""},
		{"(func (fn x (x)))", "1", ""},
		{"(func -)", "1", ""},
		{"(val 1 2 3)", "(1 2 3)", ""},
		{"(val + 5 5)", "(+ 5 5)", ""},
		{"(seq 1 2)", "2", ""},
		{"(seq (def x 666) x)", "666", ""},
		{"(if 1 2 3)", "2", ""},
		{"(if 0 2 3)", "3", ""},
		{"(if 0 1 0 2 666)", "666", ""},
		{"(id 1)", "1", ""},
		{"(id (1 2 3))", "(1 2 3)", ""},
		{"(head 1)", "", "Invalid argument 1"},
		{"(head (fn x (1 2)))", "", "Invalid argument <fn>"},
		{"(head ())", "", "Invalid argument ()"},
		{"(head (1 2 3))", "1", ""},
		{"(tail 1)", "", "Invalid argument 1"},
		{"(tail (fn x (1 2)))", "", "Invalid argument <fn>"},
		{"(tail ())", "()", ""},
		{"(tail (1 2 3))", "(2 3)", ""},
		{"(cons 123 456)", "", "Invalid argument 456"},
		{"(cons 666 (fn x (1 2)))", "", "Invalid argument <fn>"},
		{"(cons 1 (2 3))", "(1 2 3)", ""},
		{"(cons (1 2) (3 4))", "((1 2) 3 4)", ""},
		{"(head (cons 1 (2 3)))", "1", ""},
		{"(tail (cons 1 (2 3)))", "(2 3)", ""},
		{"(fn x (+ x 1))", "<fn>", ""},
		{"((fn x (+ x 1)) 5)", "6", ""},
		{"(let x 42)", "", "Insufficient arguments (let x 42)"},
		{"(let x 666 x)", "666", ""},
		{"(let x 1 y 2 (+ x y))", "3", ""},
		{"(def x)", "", "Insufficient arguments (def x)"},
		{"(def x 1)", "", ""},
		{"(seq (def x 41) (+ x 1))", "42", ""},

		// operations
		{"(! 0)", "1", ""},
		{"(! 1)", "", ""},
		{"(! ())", "1", ""},
		{"(! (1 2))", "", ""},
		{"(& 0)", "", "Insufficient arguments (& 0)"},
		{"(& 0 0)", "", ""},
		{"(& 0 1)", "", ""},
		{"(& 2 ())", "", ""},
		{"(& 3 (4 5))", "1", ""},
		{"(| 0)", "", "Insufficient arguments (| 0)"},
		{"(| 0 0)", "", ""},
		{"(| 0 1)", "1", ""},
		{"(| 2 ())", "1", ""},
		{"(| 3 (4 5))", "1", ""},
		{"(^ 0)", "", "Insufficient arguments (^ 0)"},
		{"(^ 0 0)", "", ""},
		{"(^ 0 1)", "1", ""},
		{"(^ 2 ())", "1", ""},
		{"(^ 3 (4 5))", "", ""},
		{"(+ 0)", "", "Insufficient arguments (+ 0)"},
		{"(+ 0 0)", "0", ""},
		{"(+ 0 1)", "1", ""},
		{"(+ 1 0)", "1", ""},
		{"(+ 1 1)", "2", ""},
		{"(+ 1 2)", "3", ""},
		{"(+ 3 0.14)", "3.14", ""},
		{"(* 0)", "", "Insufficient arguments (* 0)"},
		{"(* 0 0)", "0", ""},
		{"(* 0 1)", "0", ""},
		{"(* 1 0)", "0", ""},
		{"(* 1 1)", "1", ""},
		{"(* 1 2)", "2", ""},
		{"(* 3 0.1)", "0.3", ""},
		{"(/ 0)", "", "Insufficient arguments (/ 0)"},
		{"(/ 0 0)", "nan", ""},
		{"(/ 0 1)", "0", ""},
		{"(/ 1 0)", "inf", ""},
		{"(/ (- 0 1) 0)", "-inf", ""},
		{"(/ 1 1)", "1", ""},
		{"(/ 1 2)", "0.5", ""},
		{"(/ 3 0.1)", "30", ""},
		{"(% 0)", "", "Insufficient arguments (% 0)"},
		{"(% 0 0)", "nan", ""},
		{"(% 0 1)", "0", ""},
		{"(% 1 0)", "nan", ""},
		{"(% 5 3)", "2", ""},
		{"(% 1 1)", "0", ""},
		{"(% 1 2)", "1", ""},
		{"(% 7 11)", "7", ""},
		{"(- 0)", "", "Insufficient arguments (- 0)"},
		{"(- 0 0)", "0", ""},
		{"(- 0 1)", "-1", ""},
		{"(- 1 0)", "1", ""},
		{"(- 1 1)", "0", ""},
		{"(- 1 2)", "-1", ""},
		{"(- 3 0.1)", "2.9", ""},
		{"(< 0)", "", "Insufficient arguments (< 0)"},
		{"(< 0 0)", "", ""},
		{"(< 0 1)", "1", ""},
		{"(< 1 0)", "", ""},
		{"(< 1 1)", "", ""},
		{"(< 1 2)", "1", ""},
		{"(< 3 3.14)", "1", ""},
		{"(> 0)", "", "Insufficient arguments (> 0)"},
		{"(> 0 0)", "", ""},
		{"(> 0 1)", "", ""},
		{"(> 1 0)", "1", ""},
		{"(> 1 1)", "", ""},
		{"(> 1 2)", "", ""},
		{"(> 3 3.14)", "", ""},
		{"(<= 0)", "", "Insufficient arguments (<= 0)"},
		{"(<= 0 0)", "1", ""},
		{"(<= 0 1)", "1", ""},
		{"(<= 1 0)", "", ""},
		{"(<= 1 1)", "1", ""},
		{"(<= 1 2)", "1", ""},
		{"(<= 3 3.14)", "1", ""},
		{"(>= 0)", "", "Insufficient arguments (>= 0)"},
		{"(>= 0 0)", "1", ""},
		{"(>= 0 1)", "", ""},
		{"(>= 1 0)", "1", ""},
		{"(>= 1 1)", "1", ""},
		{"(>= 1 2)", "", ""},
		{"(>= 3 3.14)", "", ""},
		{"(= 0)", "", "Insufficient arguments (= 0)"},
		{"(= 0 0)", "1", ""},
		{"(= 0 ())", "", "" },
		{"(= (1) 1)", "", "" },
		{"(= 0 1)", "", ""},
		{"(= 1 0)", "", ""},
		{"(= 1 1)", "1", ""},
		{"(= 1 2)", "", ""},
		{"(= 3 3.00001)", "", ""},
		{"(= () ())", "1", ""},
		{"(= (1 2) (3 4))", "", ""},
		{"(= (1 2) (1 2))", "1", ""},
		{"(!= 0)", "", "Insufficient arguments (!= 0)" },
		{"(!= 0 0)", "", "" },
		{"(!= 0 ())", "1", "" },
		{"(!= (1) 1)", "1", "" },
		{"(!= 0 1)", "1", "" },
		{"(!= 1 0)", "1", "" },
		{"(!= 1 1)", "", "" },
		{"(!= 1 2)", "1", "" },
		{"(!= 3 3.00001)", "1", "" },
		{"(!= () ())", "", "" },
		{"(!= (1 2) (3 4))", "1", "" },
		{"(!= (1 2) (1 2))", "", "" },

		// math proxy
		{"(sqrt ())", "", "Invalid argument ()"},
		{"(sqrt +)", "", "Invalid argument +" },
		{"(sqrt 0)", "0", "" },
		{"(sqrt 1)", "1", "" },
		{"(sqrt 4)", "2", "" },
		{"(sqrt -1)", "nan", "" },
		{"(abs ())", "", "Invalid argument ()" },
		{"(abs +)", "", "Invalid argument +" },
		{"(abs 0)", "0", "" },
		{"(abs 1)", "1", "" },
		{"(abs 4)", "4", "" },
		{"(abs -1)", "1", "" },
		{"(sin ())", "", "Invalid argument ()" },
		{"(sin +)", "", "Invalid argument +" },
		{"(sin 0)", "0", "" },
		{"(sin 1.5707963267948966192313216916398)", "1", "" },
		{"(sin 3.1415926535897932384626433832795)", "0", "" },
		{"(sin -1.5707963267948966192313216916398)", "-1", "" },
		{"(cos ())", "", "Invalid argument ()" },
		{"(cos +)", "", "Invalid argument +" },
		{"(cos 0)", "1", "" },
		{"(cos 1.5707963267948966192313216916398)", "0", "" },
		{"(cos 3.1415926535897932384626433832795)", "-1", "" },
		{"(cos -1.5707963267948966192313216916398)", "0", "" },
		{"(tan +)", "", "Invalid argument +" },
		{"(tan 0)", "0", "" },
		{"(tan 3.1415926535897932384626433832795)", "-0", "" },
		{"(tan 0.78539816339744830961566084581988)", "1", ""},
		{ "(asin ())", "", "Invalid argument ()" },
		{ "(asin +)", "", "Invalid argument +" },
		{ "(asin 0)", "0", "" },
		{ "(asin 1)",   "1.570796326794897", "" },
		{ "(asin -1)", "-1.570796326794897","" },
		{ "(acos ())", "", "Invalid argument ()" },
		{ "(acos +)", "", "Invalid argument +" },
		{ "(acos 0)",  "1.570796326794897", "" },
		{ "(acos 1)", "0", "" },
		{ "(acos -1)", "3.141592653589793", "" },
		{ "(atan ())", "", "Invalid argument ()" },
		{ "(atan +)", "", "Invalid argument +" },
		{ "(atan 0)", "0", "" },
		{ "(atan -0)", "-0", "" },
		{ "(atan 1)",  "0.785398163397448", "" },
		{ "(atan2 ())", "", "Insufficient arguments (atan2 ())" },
		{ "(atan2 0 0)", "0", "" },
		{ "(atan2 1 1)",  "0.785398163397448", "" },
		{ "(atan2 0 -1)",  "3.141592653589793", "" },
		{ "(atan2 1 0)",  "1.570796326794897", "" },
		{ "(atan2 -1 0)",  "-1.570796326794897", "" },
		{"(pow 0)", "", "Insufficient arguments (pow 0)"},
		{"(pow 0 0)", "1", "" },
		{"(pow 2 3)", "9", ""},
		{"(pow 0.5 16)", "4", ""},
		{"(log 0)", "", "Insufficient arguments (log 0)" },
		{"(log 0 0)", "nan", ""},
		{"(log 10 1000)", "3", ""},
		{"(log 2 16)", "4", ""},
		{"(log 16 2)", "0.25", ""},
		{ "(floor ())", "", "Invalid argument ()" },
		{ "(floor +)", "", "Invalid argument +" },
		{ "(floor 0)", "0", "" },
		{ "(floor 1)", "1", "" },
		{ "(floor -1.5)", "-2", "" },
		{ "(floor 6.666)", "6", "" },
		{ "(ceil ())", "", "Invalid argument ()" },
		{ "(ceil +)", "", "Invalid argument +" },
		{ "(ceil 0)", "0", "" },
		{ "(ceil 1)", "1", "" },
		{ "(ceil -1.5)", "-1", ""},
		{ "(ceil 6.666)", "7", "" },

		// stdlib
		{"(length 0)", "", "Invalid argument 0"},
		{"(length (fn x (1 2)))", "", "Invalid argument <fn>"},
		{"(length ())", "0", ""},
		{"(length (()))", "1", ""},
		{"(length (1 2 3))", "3", ""},
		{"(reverse 0)", "", "Invalid argument 0" },
		{"(reverse (fn x (1 2)))", "", "Invalid argument <fn>" },
		{"(reverse ())", "()", "" },
		{"(reverse (()))", "(())", "" },
		{"(reverse (1 2 3))", "(3 2 1)", "" },
		{"(concat ())", "", "Insufficient arguments (concat ())"},
		{"(concat 0 ())", "", "Invalid argument 0" },
		{"(concat () (fn x (1 2)))", "", "Invalid argument <fn>" },
		{"(concat () ())", "()", "" },
		{"(concat (1 2) ())", "(1 2)", "" },
		{"(concat () (1 2))", "(1 2)", "" },
		{"(concat (3 4) (5 6))", "(3 4 5 6)", "" },
		{"(iota ())", "", "Invalid argument ()"},
		{"(iota (fn x (1 2))", "", "Invalid argument <fn>" },
		{"(iota 0)", "()", "" },
		{"(iota -1)", "()", "" },
		{"(iota 3)", "(0 1 2)", "" },
		{"(take ())", "", "Insufficient arguments (take ())" },
		{"(take () (123 456))", "", "Invalid argument ()" },
		{"(take (fn x (1 2)) (1 2))", "", "Invalid argument <fn>" },
		{"(take 3 666", "", "Invalid argument 666" },
		{"(take 5 (fn x (1 2)))", "", "Invalid argument <fn>" },
		{"(take 0 (1 2 3))", "()", "" },
		{"(take 1 ())", "()", "" },
		{"(take 2 ())", "()", "" },
		{"(take 2 (1 2 3 4))", "(1 2)", "" },
		{"(drop ())", "", "Insufficient arguments (drop ())" },
		{"(drop () (123 456))", "", "Invalid argument ()" },
		{"(drop (fn x (1 2)) (1 2))", "", "Invalid argument <fn>" },
		{"(drop 3 666", "", "Invalid argument 666" },
		{"(drop 5 (fn x (1 2)))", "", "Invalid argument <fn>" },
		{"(drop 0 (1 2 3))", "(1 2 3)", "" },
		{"(drop 1 ())", "()", "" },
		{"(drop 2 ())", "()", "" },
		{"(drop 2 (1 2 3 4))", "(3 4)", "" },
		{"(drop 5 (1 2 3 4))", "()", "" },
		{"(map ())", "", "Insufficient arguments (map ())" },
		{"(map () (123 456))", "", "Invalid argument ()" },
		{"(map 666 (123 456))", "", "Invalid argument 666" },
		{"(map (fn x (1 2)) 123)", "", "Invalid argument 123" },
		{"(map (fn x (1 2)) (fn y (3 4)))", "", "Invalid argument <fn>" },
		{"(map (fn x (+ x 1)) ())", "()", "" },
		{"(map (fn x (+ x 1)) (1 2 3))", "(2 3 4)", "" },
		{"(filter ())", "", "Insufficient arguments (filter ())" },
		{"(filter () (123 456))", "", "Invalid argument ()" },
		{"(filter 666 (123 456))", "", "Invalid argument 666" },
		{"(filter (fn x (1 2)) 123)", "", "Invalid argument 123" },
		{"(filter (fn x (1 2)) (fn y (3 4)))", "", "Invalid argument <fn>" },
		{"(filter (fn x (> x 2)) ())", "()", "" },
		{"(filter (fn x (> x 2)) (1 2 3 4))", "(3 4)", "" },
		{"(filter (fn x (> x 2)) (5 0 6 1))", "(5 6)", "" },
		{"(zipWith 0)", "", "Insufficient arguments (zipWith 0)"},
		{"(zipWith 0 0)", "", "Insufficient arguments (zipWith 0 0)" },
		{"(zipWith 1 2 3)", "", "Invalid argument 1" },
		{"(zipWith + 2 3)", "", "Invalid argument 2" },
		{"(zipWith + (2) (3))", "(5)", "" },
		{"(zipWith * (iota 4) (iota 4))", "(0 1 4 9)", "" },
		{"(takeWhile ())", "", "Insufficient arguments (takeWhile ())" },
		{"(takeWhile () (123 456))", "", "Invalid argument ()" },
		{"(takeWhile 666 (123 456))", "", "Invalid argument 666" },
		{"(takeWhile (fn x (1 2)) 123)", "", "Invalid argument 123" },
		{"(takeWhile (fn x (1 2)) (fn y (3 4)))", "", "Invalid argument <fn>" },
		{"(takeWhile (fn x (<= x 2)) ())", "()", "" },
		{"(takeWhile (fn x (<= x 2)) (1 2 3 4))", "(1 2)", "" },
		{"(takeWhile (fn x (<= x 2)) (5 0 6 1))", "()", "" },

		{"(dropWhile ())", "", "Insufficient arguments (dropWhile ())" },
		{"(dropWhile () (123 456))", "", "Invalid argument ()" },
		{"(dropWhile 666 (123 456))", "", "Invalid argument 666" },
		{"(dropWhile (fn x (1 2)) 123)", "", "Invalid argument 123" },
		{"(dropWhile (fn x (1 2)) (fn y (3 4)))", "", "Invalid argument <fn>" },
		{"(dropWhile (fn x (<= x 2)) ())", "()", "" },
		{"(dropWhile (fn x (<= x 2)) (1 2 3 4))", "(3 4)", "" },
		{"(dropWhile (fn x (<= x 2)) (5 0 6 1))", "(5 0 6 1)", "" },
		{"(dropWhile (fn x (<= x 2)) (0 1 2 0 1 2))", "()", "" },

		{"(repeat ())", "",  "Insufficient arguments (repeat ())" },
		{"(repeat () ())", "",  "Invalid argument ()" },
		{"(repeat + ())", "",  "Invalid argument +" },
		{"(repeat 0 ())", "()",  "" },
		{"(repeat 1 ())", "(())",  "" },
		{"(repeat 2 666)", "(666 666)",  "" },
		{"(repeat 3 +)", "(+ + +)",  "" },

		{"(foldl1 ())", "",  "Insufficient arguments (foldl1 ())" },
		{"(foldl1 () (1 2 3))", "",  "Invalid argument ()" },
		{"(foldl1 + ())", "",  "Invalid argument ()" },
		{"(foldl1 + (666))", "666",  "" },
		{"(foldl1 + (1 2))", "3",  "" },
		{"(foldl1 * (1 2 3 4))", "24",  "" },
		{"(foldl1 / (1 2 3 4))", "0.041666666666667",  "" },

		{"(foldl ())", "",  "Insufficient arguments (foldl ())" },
		{"(foldl + ())", "",  "Insufficient arguments (foldl + ())" },
		{"(foldl () 666 (1 2 3))", "",  "Invalid argument ()" },
		{"(foldl + 666 ())", "666",  "" },
		{"(foldl + 1 (2))", "3",  "" },
		{"(foldl + 1 (2 3))", "6",  "" },
		{"(foldl * 2 (1 2 3 4))", "48",  ""},
		{"(foldl / 2 (1 2 3 4))", "0.083333333333333",  "" },

		{"(foldr1 ())", "",  "Insufficient arguments (foldr1 ())" },
		{"(foldr1 () (1 2 3))", "",  "Invalid argument ()" },
		{"(foldr1 + ())", "",  "Invalid argument ()" },
		{"(foldr1 + (666))", "666",  "" },
		{"(foldr1 + (1 2))", "3",  "" },
		{"(foldr1 * (1 2 3 4))", "24",  "" },
		{"(foldr1 / (1 2 3 4))", "0.375",  "" },

		{"(foldr ())", "",  "Insufficient arguments (foldr ())" },
		{"(foldr + ())", "",  "Insufficient arguments (foldr + ())" },
		{"(foldr () 666 (1 2 3))", "",  "Invalid argument ()" },
		{"(foldr + 666 ())", "666",  "" },
		{"(foldr + 1 (2))", "3",  "" },
		{"(foldr + 1 (2 3))", "6",  "" },
		{"(foldr * 2 (1 2 3 4))", "48",  "" },
		{"(foldr / 2 (1 2 3 4))", "0.75",  "" },
	};

	auto count = 0, failed = 0;

	for (auto test_case : test_cases)
	{
		auto eli = new maxy::control::ELI::ELI();

		auto result = eli->run(test_case[0]);

		auto failure = false;

		if (result.first != test_case[1])
		{
			failure = true;

			std::cout << "FAILURE FOR \"" << test_case[0] << "\"\n"
				<< "WRONG RESULT\n"
				<< "\texpected \"" << test_case[1] << "\"\n"
				<< "\treceived \"" << result.first << "\"\n";
		}

		if (result.second != test_case[2])
		{
			if (!failure)
			{
				std::cout << "FAILURE FOR \"" << test_case[0] << "\"\n";
			}

			failure = true;

			std::cout << "WRONG ERROR FOR \"" << test_case[0] << "\"\n"
				<< "\texpected \"" << test_case[2] << "\"\n"
				<< "\treceived \"" << result.second << "\"\n";
		}

		count++;
		if (failure) failed++;

		delete eli;
	}

	std::cout << "-----------------------------\n" << count << " TESTS, " << failed << " FAILURES\n\n";
}

void test_integrations()
{
	std::cout << "\n\nTesting integrations\n";

	auto count = 0, failed = 0;

	int i = -666;
	unsigned int ui = 666;
	long long ll = -666666;
	unsigned long long ull = 666999;
	float f = -666.0f;
	double d = 666.666l;
	bool b = false;
	int ro = 12345;

	float fvec2[2] = { 1.f, 2.f };
	double dvec3[3] = { 3.l, 4.l, 5.l };
	int ivec4[4] = { 6, 7, 8, 9 };

	std::vector<std::vector<const char *>> test_cases =
	{
		// non-existent variable
		{"(get xxx)", "", "External variable not found xxx"},

		// read
		{"(get i)", "(-666)", ""},
		{"(get ui)", "(666)", ""},
		{"(get ll)", "(-666666)", ""},
		{"(get ull)", "(666999)", ""},
		{"(get f)", "(-666)", ""},
		{"(get d)", "(666.666000000000054)", ""},
		{"(get b)", "(0)", ""},
		{"(get ro)", "(12345)", ""},
		{"(get fvec2)", "(1 2)", ""},
		{"(get dvec3)", "(3 4 5)", ""},
		{"(get ivec4)", "(6 7 8 9)", ""},

		// modify-read
		{"(seq (set i (-123)) (get i))", "(-123)", ""},
		{"(seq (set ui (-123)) (get ui))", "(4294967173)", ""},
		{"(seq (set ll (-123)) (get ll))", "(-123)", ""},
		{"(seq (set ull (-123)) (get ull))", "(18446744073709551493)", ""},
		{"(seq (set f (-123)) (get f))", "(-123)", ""},
		{"(seq (set d (-123)) (get d))", "(-123)", ""},
		{"(seq (set b (-123)) (get b))", "(1)", ""},
		{"(set fvec2 (666))", "", "Insufficient arguments (666)"},
		{"(seq (set fvec2 (666 999)) (get fvec2))", "(666 999)", ""},
		{"(set dvec3 (666 777))", "", "Insufficient arguments (666 777)"},
		{"(seq (set dvec3 (666 999 555.1)) (get dvec3))", "(666 999 555.100000000000023)", ""},
		{"(set ivec4 (-1 1 -1))", "", "Insufficient arguments (-1 1 -1)"},
		{"(seq (set ivec4 (4 3 2 1)) (get ivec4))", "(4 3 2 1)", ""},

		// readonly
		{"(set ro (999))", "", "Attempted write to read-only variable ro"},

		// func
		{"(call xxx ())", "", "Function not found xxx"},
		{"(call fun (1 2 3))", "(3 2 1 LOL)", ""}
	};

	for (auto test_case : test_cases)
	{
		auto eli = new maxy::control::ELI::ELI();

		eli->var("i", &i);
		eli->var("ui", &ui);
		eli->var("ll", &ll);
		eli->var("ull", &ull);
		eli->var("f", &f);
		eli->var("d", &d);
		eli->var("b", &b);
		eli->var("ro", &ro, 1, true);
		eli->var("fvec2", &fvec2[0], 2);
		eli->var("dvec3", &dvec3[0], 3);
		eli->var("ivec4", &ivec4[0], 4);
		eli->func("fun", [](std::vector<std::string> s) { return std::vector<std::string>{s[2], s[1], s[0], "LOL"}; });

		auto result = eli->run(test_case[0]);

		auto failure = false;

		if (result.first != test_case[1])
		{
			failure = true;

			std::cout << "FAILURE FOR \"" << test_case[0] << "\"\n"
				<< "WRONG RESULT\n"
				<< "\texpected \"" << test_case[1] << "\"\n"
				<< "\treceived \"" << result.first << "\"\n";
		}

		if (result.second != test_case[2])
		{
			if (!failure)
			{
				std::cout << "FAILURE FOR \"" << test_case[0] << "\"\n";
			}

			failure = true;

			std::cout << "WRONG ERROR FOR \"" << test_case[0] << "\"\n"
				<< "\texpected \"" << test_case[2] << "\"\n"
				<< "\treceived \"" << result.second << "\"\n";
		}

		count++;
		if (failure) failed++;

		delete eli;
	}

	std::cout << "-----------------------------\n" << count << " TESTS, " << failed << " FAILURES\n\n";
}

void test_threading()
{
	std::cout << "\n\nTesting multithreading\n";

	auto eli = new maxy::control::ELI::ELI();

	double v[4] = { 0.L, 0.L, 0.L, 0.L };

	eli->var("v0", &v[0]);
	eli->var("v1", &v[1]);
	eli->var("v2", &v[2]);
	eli->var("v3", &v[3]);

	std::thread([&] () {
		eli->run("(seq (def x 1) (set v0 (x)))");
	}).detach();

	std::thread([&] () {
		eli->run("(seq (def x 2) (set v1 (x)))");
	}).detach();

	std::thread([&] () {
		eli->run("(seq (def x 3) (set v2 (x)))");
	}).detach();

	std::thread([&] () {
		eli->run("(seq (def x 4) (set v3 (x)))");
	}).detach();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::cout << "Results: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << "\n"
	    << "(Can be any combination of 1, 2, 3 and 4)\n";

	delete eli;

	std::cout << "Done\n\n";
}

int main()
{
	test_classes();

	test_basic_functions();

	test_integrations();

	test_threading();

	return 0;
}
