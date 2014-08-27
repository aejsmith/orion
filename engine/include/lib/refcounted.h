/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Reference counted object base class.
 */

#ifndef ORION_LIB_REFCOUNTED_H
#define ORION_LIB_REFCOUNTED_H

#include "core/defs.h"

#include <type_traits>

/**
 * Base class providing reference counting functionality.
 *
 * This class provides reference counting functionality to derived classes. It
 * maintains a reference count which is modified using the retain() and
 * release() methods. When the reference count reaches 0, the released() method
 * is called, this method can be overridden for custom functionality. The
 * default implementation deletes the object.
 *
 * The retain and release methods are marked const to allow for const pointers
 * to a reference counted object.
 *
 * @todo		Threading: does this need to be atomic?
 * @todo		Inline release() for non-debug builds (currently out of
 *			line due to assertion).
 */
class Refcounted {
public:
	Refcounted() : m_refcount(0) {}
	virtual ~Refcounted();

	/** Increase the object's reference count.
	 * @return		New value of the reference count. */
	int32_t retain() const { return ++m_refcount; }

	int32_t release() const;

	/** @return		Current reference count. */
	int32_t refcount() const { return m_refcount; }
private:
	/** Called when the object is released. */
	virtual void released() {}
private:
	mutable int32_t m_refcount;	/**< Object reference count. */
};

/** Reference counting smart pointer.
 * @tparam T		Type of the reference counted object that the pointer
 *			should point to. */
template <typename T>
class ReferencePtr {
public:
	/** Create a null pointer. */
	constexpr ReferencePtr() : m_object(nullptr) {}

	/** Create a null pointer. */
	constexpr ReferencePtr(std::nullptr_t) : m_object(nullptr) {}

	/** Point to an object, increasing its reference count. */
	template <
		typename U,
		typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	explicit ReferencePtr(U *ptr) :
		m_object(ptr)
	{
		if(m_object)
			m_object->retain();
	}

	/** Copy another pointer, increasing the reference count. */
	ReferencePtr(const ReferencePtr &other) :
		m_object(other.m_object)
	{
		if(m_object)
			m_object->retain();
	}

	/** Copy another pointer of compatible type, increasing the reference count. */
	template <
		typename U,
		typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	ReferencePtr(const ReferencePtr<U> &other) :
		m_object(other.get())
	{
		if(m_object)
			m_object->retain();
	}

	/** Move another pointer to this one. */
	ReferencePtr(ReferencePtr &&other) :
		m_object(other.m_object)
	{
		other.m_object = nullptr;
	}

	/** Destroy the pointer, releasing the object. */
	~ReferencePtr() { reset(); }

	/** Check whether the pointer is valid. */
	explicit operator bool() const { return m_object != nullptr; }

	/** Copy another pointer, increasing the reference count. */
	ReferencePtr &operator =(const ReferencePtr &other) {
		reset(other.m_object);
		return *this;
	}

	/** Copy another pointer of compatible type, increasing the reference count. */
	template <typename U>
	typename std::enable_if<std::is_convertible<U *, T *>::value, ReferencePtr &>
	operator =(const ReferencePtr<U> &other) {
		reset(other.get());
		return *this;
	}

	/** Move another pointer to this one. */
	ReferencePtr &operator =(ReferencePtr &&other) {
		if(m_object)
			m_object->release();

		m_object = other.m_object;
		other.m_object = nullptr;
		return *this;
	}

	/** Move another pointer of compatible type to this one. */
	template <typename U>
	typename std::enable_if<std::is_convertible<U *, T *>::value, ReferencePtr &>
	operator =(ReferencePtr<U> &&other) {
		if(m_object)
			m_object->release();

		m_object = other.m_object;
		other.m_object = nullptr;
		return *this;
	}

	T &operator *() const { return *m_object; }
	T *operator ->() const { return m_object; }

	/** @return		Value of the pointer. */
	T *get() const { return m_object; }

	/** Release the current pointer and replace it.
	 * @param ptr		New pointer, defaults to null. */
	void reset(T *ptr = nullptr) {
		if(ptr)
			ptr->retain();

		std::swap(m_object, ptr);

		if(ptr)
			ptr->release();
	}

	/** Swap the pointer with another pointer.
	 * @param other		Pointer to swap with. */
	void swap(ReferencePtr &other) {
		std::swap(m_object, other.m_object);
	}
private:
	T *m_object;			/**< Referenced object. */
};

/** Compare ReferencePtrs for equality. */
template <typename T, typename U>
bool operator ==(const ReferencePtr<T> &a, const ReferencePtr<U> &b) {
	return a.get() == b.get();
}

/** Compare ReferencePtrs for inequality. */
template <typename T, typename U>
bool operator !=(const ReferencePtr<T> &a, const ReferencePtr<U> &b) {
	return !(a == b);
}

#endif /* ORION_LIB_REFCOUNTED_H */
