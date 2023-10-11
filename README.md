
# embedded lisp interpreter

can be used to add powerful scripting to any application


# embedding

you may access C++ variables from the Lisp code:

```
double vec[4] = {665.l, 664.l, 663.l, 662.l};
auto eli = new ELI();
eli->var("extvec", &vec[0], 4);
eli->run("(set extvec (zipWith + (get extvec) (iota 4)))");
std::cout << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << "\n";
```

you also may call C++ functions from the Lisp code:


```
auto eli = new ELI();
eli->func("echo", [] (std::vector<std::string> params) { 
	for (auto p : params)
		std::cout << p << "\n";

	return std::vector<std::string>{}; 
});
eli->run("(call echo (iota 5))");
```


# builtin functions
## primitives

- `(seq x y ...)` - evaluate all arguments in order. Return the last evaluated argument.
- `(val x y ...)` - return `(x y ...)` as is without evaluating
- `(def xi yi ...)` - define `xi` to hold value of `yi` in global scope
- `(let xi yi ... z)` - evaluate `z` using names `xi` to hold values of `yi`
- `(empty x)` - check if `x` is empty. `()`, `0`, `0.0` and empty string are empty
- `(list x)` - check if `x` is a list
- `(atom x)` - check if `x` is an atom (that is a string or a number)
- `(func x)` - check if `x` is a function (a lambda or a builtin)
- `(if ci xi y)` - conditional (evaluate to `xi` if `ci` is true and finally to `y` if all `ci` are false)
- `(cons a b)` - construct a list of its head `a` and tail `b`
- `(head x)` - evaluate to head of `x`
- `(tail x)` - evaluate to tail of `x`
- `(fn ai ... x)` - evaluate to lambda with parameters `ai` and body `x`

## basic operations on atoms

- `(+ a b)` - addition
- `(* a b)` - multiplication
- `(- a b)` - subtraction
- `(/ a b)` - division
- `(% a b)` - modulo
- `(| a b)` - boolean or
- `(& a b)` - boolean and
- `(^ a b)` - boolean xor
- `(! a)` - boolean not
- `(= a b)` - equal
- `(!= a b)` - not equal
- `(> a b)` - greater
- `(< a b)` - less
- `(>= a b)` - greater or equal
- `(<= a b)` - less or equal

## math
- `(sqrt a)` - square root of `a`
- `(pow a b)` - `b` powered to `a`
- `(log a b)` - logarithm of `b` on base `a`
- `(sin a)`
- `(cos a)`
- `(sinCos a)` - return both sine and cosine of `a`
- `(tan a)`
- `(asin a)`
- `(acos a)`
- `(atan a)`
- `(atan2 y x)`
- `(abs a)`
- `(floor a)`
- `(ceil a)`


## standard library

- `(iota n)` - produce a list of numbers from `0` to `n - 1`
- `(reverse a)` - reverse `a`
- `(concat a b)` - concat two lists
- `(take n a)` - produce a list containing `n` first elements of a list `a`
- `(drop n a)` - drop `n` first elements of a list `a`, return the remaining elements
- `(map f a)` - map a function `f` over a list `a`
- `(filter f a)` - produce a list containing only those elements of `a` for which the predicate `f` returns true
- `(takeWhile f a)` - take elements from the beginning of the list while the predicate `f` holds
- `(dropWhile f a)` - remove elements from the beginning of the list while the predicate `f` holds
- `(zipWith f a b)` - combine two lists with binary function `f`
- `(foldl f a l)` - left fold a list `l` with a function `f` and starting value `a`
- `(foldl1 f l)` - left fold a list `l` with a function `f`
- `(foldr f a l)` - right fold a list `l` with a function `f` and starting value `a`
- `(foldr1 f l)` - right fold a list `l` with a function `f`

## external integration

- `(get v)` - read external variable `v` returning list of values
- `(set v x)` - set external variable `v` to `x` (`x` must be a list)
- `(call f x)` - call external function `f` with argument `x` (where `x` is a list)

# thread safety

`ELI::var` and `ELI::func` are *not* explicitly thread safe.

`ELI::run` and most lisp operations are implicitly thread-safe due to their local nature, except the following:

The `def` operation is thread safe by itself but it modifies the global symbol table. The modified table is immediately visible to other threads which
may alter the desired results (see `test.cpp` for an example).

The `get`, `set` and `call` operations are *not* explicitly thread-safe.

