/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Reference counted object base class.
 */

#include "core/refcounted.h"

/**
 * Destroy a reference counted object.
 *
 * Destroys the reference counted object. The reference count must currently be
 * 0, this is checked.
 */
Refcounted::~Refcounted() {
	orionAssert(m_refcount == 0);
}

/**
 * Decrease the object reference count.
 *
 * Decreases the object's reference count. If the reference count reaches 0,
 * the released() method will be called. The reference count must not currently
 * be 0.
 *
 * @return		New value of the reference count.
 */
int32_t Refcounted::release() const {
	orionAssert(m_refcount > 0);

	int32_t ret = --m_refcount;
	if(ret == 0)
		const_cast<Refcounted *>(this)->released();

	return ret;
}

/** Called when the object is released. */
void Refcounted::released() {
	delete this;
}
