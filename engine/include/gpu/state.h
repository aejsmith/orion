/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		GPU state objects.
 */

#pragma once

#include "core/hash.h"

#include "gpu/defs.h"

/** Base GPU state object class.
 * @tparam Desc		Type of the state descriptor structure. */
template <typename Desc>
class GPUState : public GPUResource {
public:
	/** @return		Descriptor used to create the state object. */
	const Desc &desc() const { return m_desc; }
protected:
	GPUState(const Desc &desc) : m_desc(desc) {}
protected:
	Desc m_desc;			/**< State descriptor. */
};

/** GPU sampler state descriptor. */
struct GPUSamplerStateDesc {
	SamplerFilterMode filterMode;	/**< Filtering mode. */
	unsigned maxAnisotropy;		/**< Anisotropic filtering level. */
	SamplerAddressMode addressU;	/**< Addressing mode in U direction. */
	SamplerAddressMode addressV;	/**< Addressing mode in V direction. */
	SamplerAddressMode addressW;	/**< Addressing mode in W direction. */
public:
	/** Compare this descriptor with another. */
	bool operator ==(const GPUSamplerStateDesc &other) const {
		return filterMode == other.filterMode &&
			maxAnisotropy == other.maxAnisotropy &&
			addressU == other.addressU &&
			addressV == other.addressV &&
			addressW == other.addressW;
	}

	/** Get a hash from a sampler state descriptor. */
	friend size_t hashValue(const GPUSamplerStateDesc &desc) {
		size_t hash = hashValue(desc.filterMode);
		hash = hashCombine(hash, desc.maxAnisotropy);
		hash = hashCombine(hash, desc.addressU);
		hash = hashCombine(hash, desc.addressV);
		hash = hashCombine(hash, desc.addressW);
		return hash;
	}
};

/** Texture sampler state object. */
typedef GPUState<GPUSamplerStateDesc> GPUSamplerState;

/** Type of a pointer to a GPU sampler state. */
typedef GPUResourcePtr<GPUSamplerState> GPUSamplerStatePtr;
