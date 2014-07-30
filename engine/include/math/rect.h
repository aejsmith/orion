/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		Rectangle structure.
 */

#ifndef ORION_MATH_RECT_H
#define ORION_MATH_RECT_H

#include "core/defs.h"

/** Structure defining a 2D rectangle. */
template <typename T>
struct RectImpl {
	T x;			/**< X position. */
	T y;			/**< Y position. */
	T width;		/**< Width. */
	T height;		/**< Height. */
public:
	/** Vector of type T. */
	typedef glm::detail::tvec2<T, glm::highp> VecType;
public:
	RectImpl() :
		x(0),
		y(0),
		width(0),
		height(0)
	{}

	RectImpl(T _x, T _y, T _width, T _height) :
		x(_x),
		y(_y),
		width(_width),
		height(_height)
	{}

	RectImpl(VecType pos, VecType size) :
		x(pos.x),
		y(pos.y),
		width(size.x),
		height(size.y)
	{}

	/** Get the position of this rectangle.
	 * @return		Position of the rectangle. */
	VecType pos() const { return VecType(x, y); }

	/** Get the size of this rectangle.
	 * @return		Size of the rectangle. */
	VecType size() const { return VecType(width, height); }

	/** Check whether the rectangle contains a point.
	 * @param point		Point to check for.
	 * @return		Whether the point is within the rectangle. */
	bool contains(const VecType &point) const {
		return (point.x >= x && point.y >= y && point.x < (x + width) && point.y < (y + height));
	}

	/** Compare for equality with another rectangle.
	 * @param other		Rectangle to compare with.
	 * @return		Whether they are equal. */
	bool operator ==(const RectImpl& other) const {
		return (x == other.x && y == other.y && width == other.width && height == other.height);
	}

	/** Compare for inequality with another rectangle.
	 * @param other		Rectangle to compare with.
	 * @return		Whether they are not equal. */
	bool operator !=(const RectImpl& other) const {
		return !(*this == other);
	}
};

/** Rectangle using single precision floating point values. */
typedef RectImpl<float> Rect;

/** Rectangle using integer values. */
typedef RectImpl<int> IntRect;

#endif /* ORION_MATH_RECT_H */
