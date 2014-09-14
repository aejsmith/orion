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

#include <memory>

/** TGA texture loader class. */
class TGALoader : public AssetLoader {
public:
	TGALoader() : AssetLoader("tga") {}
	AssetPtr load(DataStream *stream, rapidjson::Value &attributes, const char *path) const override;
private:
	static TGALoader m_instance;
};

/** TGA image file header. */
struct TGAHeader {
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

/** TGA loader instance. */
TGALoader TGALoader::m_instance;

/** Load a TGA file.
 * @param stream	Stream containing asset data.
 * @param attributes	Attributes specified in metadata.
 * @param path		Path to asset.
 * @return		Pointer to loaded asset, null on failure. */
AssetPtr TGALoader::load(DataStream *stream, rapidjson::Value &attributes, const char *path) const {
	TGAHeader header;
	if(!stream->read(reinterpret_cast<char *>(&header), sizeof(header), 0)) {
		orionLog(LogLevel::kError, "Failed to read asset '%s' data", path);
		return nullptr;
	}

	/* Only support uncompressed RGB images for now. */
	if(header.imageType != 2) {
		orionLog(LogLevel::kError, "TGA texture '%s' has unsupported format", path);
		return nullptr;
	}

	if(header.depth != 24 && header.depth != 32) {
		orionLog(LogLevel::kError, "TGA texture '%s' has unsupported depth", path);
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
	if(!stream->read(buf.get(), size, offset)) {
		orionLog(LogLevel::kError, "Failed to read asset '%s' data", path);
		return nullptr;
	}

	/* Create the texture, with mipmaps. */
	Texture2DPtr texture(new Texture2D(width, height, format));
	texture->update(buf.get());
	return texture;
}
