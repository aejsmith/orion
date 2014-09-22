/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		TGA texture loader.
 *
 * @todo		Support compressed images, 16bpp images (need 16-bit
 *			packed pixel formats).
 */

#include "engine/asset_loader.h"
#include "engine/texture.h"

/** TGA texture loader class. */
class TGALoader : public AssetLoader {
public:
	AssetPtr load() override;
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
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr TGALoader::load() {
	Header header;
	if(!m_data->read(reinterpret_cast<char *>(&header), sizeof(header), 0)) {
		orionLog(LogLevel::kError, "%s: Failed to read asset data", m_path);
		return nullptr;
	}

	/* Only support uncompressed RGB images for now. */
	if(header.imageType != 2) {
		orionLog(LogLevel::kError, "%s: Unsupported image format (%u)", m_path, header.imageType);
		return nullptr;
	}

	if(header.depth != 24 && header.depth != 32) {
		orionLog(LogLevel::kError, "%s: Unsupported depth (%u)", m_path, header.depth);
		return nullptr;
	}

	/* Determine image properties. */
	uint32_t width = header.width;
	uint32_t height = header.height;
	PixelFormat format = (header.depth == 32) ? PixelFormat::kB8G8R8A8 : PixelFormat::kB8G8R8;

	/* Read in the data, which is after the ID and colour map. */
	size_t size = width * height * (header.depth / 8);
	uint64_t offset = header.idLength + (header.colourMapLength * (header.colourMapDepth / 8));
	std::unique_ptr<char []> buf(new char[size]);
	if(!m_data->read(buf.get(), size, offset)) {
		orionLog(LogLevel::kError, "%s: Failed to read asset data", m_path);
		return nullptr;
	}

	/* Create the texture, with mipmaps. */
	Texture2DPtr texture(new Texture2D(width, height, format));
	texture->update(buf.get());
	return texture;
}
