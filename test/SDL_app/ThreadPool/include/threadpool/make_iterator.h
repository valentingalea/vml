/** @file threadpool/make_iterator.h
 *
 * Threadpool for C++11, make iterator from function
 *
 * @copyright	2014 Ruediger Helsch, Ruediger.Helsch@t-online.de
 * @license	All rights reserved. Use however you want. No warranty at all.
 * $Revision: 2.1 $
 * $Date: 2014/05/15 23:55:22 $
 */
#ifndef THREADPOOL_MAKE_ITERATOR_H
#define THREADPOOL_MAKE_ITERATOR_H

#include <iterator>
#include <type_traits>
#include <stdexcept> // for std::out_of_range
#include <memory> // for std::shared_ptr





namespace ThreadPoolImpl {

    namespace impl {



	/**
	 * An assignable value-type for reference types
	 *
	 * @tparam T
	 *		The type to transport
	 *
	 * The ref_value is default-constructable, constructable and
	 * assignable from T references, copy- and move- constructable
	 * and -assignable. To get the value out of it, call get() for
	 * a const reference or move() for an rvalue reference. For
	 * lvalue references, move() falls back to an lvalue
	 * reference.
	 */
	template<class T, class Enable = void>
	class ref_value;

	template<class T>
	class ref_value<T, typename std::enable_if<!std::is_reference<T>::value>::type> {
	    typename std::remove_cv<T>::type value;
	public:
	    ref_value() { }
	    ref_value(const T& x) : value(x) { }
	    ref_value(T&& x) : value(std::forward<T>(x)) { }
	    ref_value(const ref_value&) = default;
	    ref_value(ref_value&&) = default;
	    ref_value& operator=(const T& x) { value = x; return *this; }
	    ref_value& operator=(T&& x) { value = std::forward<T>(x); return *this; }
	    ref_value& operator=(const ref_value&) = default;
	    ref_value& operator=(ref_value&&) = default;
	    const typename std::remove_volatile<T>::type& get() const { return value; }
	    typename std::remove_cv<T>::type&& move() { return std::move(value); }
	};
	 
	template<class T>
	class ref_value<T, typename std::enable_if<std::is_lvalue_reference<T>::value>::type> {
	    typename std::remove_reference<T>::type* value;
	public:
	    ref_value() { }
	    ref_value(T x) : value(&x) { }
	    ref_value& operator=(T x) { value = &x; return *this; }
	    T get() const { return *value; }
	    T move() { return *value; }
	};

	template<class T>
	class ref_value<T, typename std::enable_if<std::is_rvalue_reference<T>::value>::type> {
	    typename std::remove_reference<T>::type* value;
	public:
	    ref_value() { }
	    ref_value(T x) : value(&x) { }
	    ref_value& operator=(T x) { value = &x; return *this; }
	    const T& get() const { return std::move(*value); }
	    T move() { return std::move(*value); }
	};


	/**
	 * Wrap a function in a pointer.
	 *
	 * Keep R-values through a shared_ptr to the std::move()d object.
	 * Keep L-values through a simple pointer.
	 *
	 * Note that template parameter `Function` here has any
	 * rvalue reference stripped. Only the lvalue reference is
	 * remaining.
	 */
	template<class Function, class Enable = void>
	class FunctionPtr;

	template<class Function>
	class FunctionPtr<Function,
			  typename std::enable_if<!std::is_reference<Function>::value>::type> {
	    std::shared_ptr<Function> fun;
	public:
	    explicit FunctionPtr(Function&& fun) : fun(new Function(std::forward<Function>(fun))) { }
	    Function& operator*() { return *fun; }
	};

	template<class Function>
	class FunctionPtr<Function,
			  typename std::enable_if< std::is_reference<Function>::value>::type> {
	    typename std::add_pointer<Function>::type fun;
	public:
	    explicit FunctionPtr(Function& fun) : fun(&fun) { }
	    Function& operator*() { return *fun; }
	};


	/**
	 * An input iterator made from a function
	 *
	 * The function is called with no arguments and must return
	 * the next value of the input sequence or throw an
	 * std::out_of_range exception if the input sequence has
	 * ended. After throwing the exception once, the function is
	 * no more called.
	 *
	 * Implements only two member functions doing something. The
	 * operator==() returns true if both iterators are at the end
	 * or both iterators are not at the end. The operator*()
	 * returns the next value by rvalue reference.
	 */
	template<class Function>
	class FunctionInputIterator
	    : public std::iterator<std::input_iterator_tag,
				   decltype(std::declval<Function>()())> {
	    typedef std::iterator<std::input_iterator_tag,
				  decltype(std::declval<Function>()())> Base;

	    /*
	      Must share the values between copied interators, because
	      shared values may be compared, and during comparision a
	      value may need to be read, which should be available
	      through the original value of the iterator.
	    */
	    struct Values {
		bool last = false;
		bool value_valid = false;
		FunctionPtr<Function> fun;
		ref_value<typename Base::value_type> value;
		explicit Values(FunctionPtr<Function>& fun) : fun(fun) { }
		Values(const Values& x) = default;
	    };

	    std::shared_ptr<Values> v;

	public:

	    FunctionInputIterator() = default;
	    explicit FunctionInputIterator(FunctionPtr<Function>& fun) : v(new Values(fun)) { }
	    FunctionInputIterator& operator++() { return *this; }
	    FunctionInputIterator& operator++(int) { return *this; }

	    typename Base::value_type&& operator*() const
	    {
		// No need to check for empty v, caller must have checked.
		if (v->value_valid) {
		    v->value_valid = false;
		} else if (!v->last) {
		    try {
			v->value = (*v->fun)();
		    } catch (std::out_of_range) {
			v->last = true;
		    }
		}

		if (v->last)
		    throw std::out_of_range("");

		return v->value.move();
	    }

	    bool operator==(const FunctionInputIterator& x) const {

		if (v != nullptr && !(v->last || v->value_valid)) {
		    try {
			v->value = (*v->fun)();
			v->value_valid = true;
		    } catch (std::out_of_range) {
			v->last = true;
		    }
		}

		if (x.v != nullptr && !(x.v->last || x.v->value_valid))
		    return x.operator==(*this);

		return (v == nullptr || v->last) == (x.v == nullptr || x.v->last);
	    }

	    bool operator!=(const FunctionInputIterator& x) const {
		return !operator==(x);
	    }

	};



	/*
	 * An input range made from a function
	 *
	 * The function is called with no arguments and must return
	 * the next value of the input sequence or throw an
	 * std::out_of_range exception if the input sequence has
	 * ended. After throwing the exception once, the function is
	 * no more called.
	 *
	 * Input iterators are frequently used as pairs delimiting a
	 * range. Use member functions begin() and end() of the range
	 * as `first` and `last` iterators for algorithms on iterator
	 * ranges, or use the range itself as input for algorithms
	 * working on containers or as the range argument in a
	 * range-based for statement.
	 */
	template<class Function>
	class FunctionInputIteratorRange {

	    FunctionPtr<Function> fun;

	public:

	    template <class Fun> explicit FunctionInputIteratorRange(Fun&& fun) : fun(std::forward<Fun>(fun)) { }
	    FunctionInputIterator<Function> begin() { return FunctionInputIterator<Function>(fun); }
	    FunctionInputIterator<Function> end() { return FunctionInputIterator<Function>(); }
	};



	/**
	 * An output iterator made from a function
	 *
	 * The function is called with one argument, which is whatever
	 * is assigned to the value_type of the iterator.
	 *
	 * Implements only one member function doing something: The
	 * operator=() calls the function with its argument.
	 */
	template<class Function>
	class FunctionOutputIterator
	    : public std::iterator<std::output_iterator_tag, void, void, void, void> {

	    FunctionPtr<Function> fun;

	public:
	    template<class Fun> explicit FunctionOutputIterator(Fun&& fun) : fun(std::forward<Fun>(fun)) { }
	    FunctionOutputIterator& operator++() { return *this; }
	    FunctionOutputIterator& operator++(int) { return *this; }
	    FunctionOutputIterator& operator*() { return *this; }
	    template<class Arg> void operator=(Arg&& arg) { (*fun)(std::forward<Arg>(arg)); }
	};



    } // End of namespace impl

} // End of namespace ThreadPoolImpl





namespace threadpool {

    /**
     * Create an input range from a function
     *
     * @param fun
     *		The function must be callable with no arguments and
     *		must return the values. The type of the return value
     *		of the function will become the `value_type` of the
     *		input iterator.
     *
     * @returns
     *		The input range. The input range has two member
     *		functions begin() and end() which return iterators
     *		that can be used as `first` and `last` value of
     *		algorithms. The range itself can be used like a
     *		container.
     */
    template<class Function>
    ThreadPoolImpl::impl::FunctionInputIteratorRange<Function>
    make_function_input_range(Function&& fun) {
	return ThreadPoolImpl::impl::FunctionInputIteratorRange<Function>(std::forward<Function>(fun));
    }

    /**
     * Create an output iterator from a function
     *
     * @param fun
     *		The function must be callable with one argument. This
     *		argument will receive the value assigned to the output
     *		iterator.
     *
     * @returns
     *		The output iterator.
     */
    template<class Function>
    ThreadPoolImpl::impl::FunctionOutputIterator<Function>
    make_function_output_iterator(Function&& fun) {
	return ThreadPoolImpl::impl::FunctionOutputIterator<Function>(std::forward<Function>(fun));
    }
 
} // End of namespace threadpool





#endif // !THREADPOOL_MAKE_ITERATOR_H
