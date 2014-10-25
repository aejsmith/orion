/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		TGA texture loader.
 *
 * @todo		Support compressed images, 16bpp images (need 16-bit
 *			packed pixel formats).
 */

#include "texture_loader.h"

/** TGA texture loader class. */
class TGALoader : public Texture2DLoader {
public:
	bool loadData() override;
private:
	/** TGA image file header. */
	struct Header {
		uint8_t idLength;
		uint8_t colourMapType;
		uint8_t imageType;
		uint16_t colourMapOrigin;
		uint16_t colourMapLength;
		uint8_t colourMapDepth;
		uint16_t xOrigin;
		uint16_t yOrigin;
		uint16_t width;
		uint16_t height;
		uint8_t depth;
		uint8_t imageDescriptor;
	} PACKED;
};

IMPLEMENT_ASSET_LOADER(TGALoader, "tga");

/** Load a TGA file.
 * @return		Whether the texture data was loaded sucessfully. */
bool TGALoader::loadData() {
	Header header;
	if(!m_data->read(reinterpret_cast<char *>(&header), sizeof(header), 0)) {
		logError("%s: Failed to read asset data", m_path);
		return false;
	}

	/* Only support uncompressed RGB images for now. */
	if(header.imageType != 2) {
		logError("%s: Unsupported image format (%u)", m_path, header.imageType);
		return false;
	}

	if(header.depth != 24 && header.depth != 32) {
		logError("%s: Unsupported depth (%u)", m_path, header.depth);
		return false;
	}

	/* Determine image properties. */
	m_width = header.width;
	m_height = header.height;
	m_format = (header.depth == 32) ? PixelFormat::kB8G8R8A8 : PixelFormat::kB8G8R8;

	/* Read in the data, which is after the ID and colour map. */
	size_t size = m_width * m_height * (header.depth / 8);
	uint64_t offset = header.idLength + (header.colourMapLength * (header.colourMapDepth / 8));
	m_buffer.reset(new char[size]);
	if(!m_data->read(m_buffer.get(), size, offset)) {
		logError("%s: Failed to read asset data", m_path);
		return false;
	}

	return true;
}
