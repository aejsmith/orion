/**
 * @file
 * @copyright		2014 Alex Smith
 * @brief		TGA texture loader.
 *
 * @todo		Support compressed images, 16bpp images (need 16-bit
 *			packed pixel formats).
 */

#include "tga_loader.h"

#include "asset/texture.h"

#include "core/engine.h"

#include <memory>

/** TGA image file header. */
struct TGAHeader {
	uint8_t id_length;
	uint8_t colour_map_type;
	uint8_t image_type;
	uint16_t colour_map_origin;
	uint16_t colour_map_length;
	uint8_t colour_map_depth;
	uint16_t x_origin;
	uint16_t y_origin;
	uint16_t width;
	uint16_t height;
	uint8_t depth;
	uint8_t image_descriptor;
} PACKED;

/** Initialize the loader. */
TGALoader::TGALoader() : AssetLoader("tga") {}

/** Load a TGA file.
 * @param stream	Stream containing asset data.
 * @param attributes	Attributes specified in metadata.
 * @param path		Path to asset.
 * @return		Pointer to loaded asset, null on failure. */
Asset *TGALoader::load(DataStream *stream, rapidjson::Value &attributes, const char *path) {
	TGAHeader header;
	if(!stream->read(reinterpret_cast<char *>(&header), sizeof(header), 0)) {
		orion_log(LogLevel::kError, "Failed to read asset '%s' data", path);
		return nullptr;
	}

	/* Only support uncompressed RGB images for now. */
	if(header.image_type != 2) {
		orion_log(LogLevel::kError, "TGA texture '%s' has unsupported format", path);
		return nullptr;
	}

	if(header.depth != 24 && header.depth != 32) {
		orion_log(LogLevel::kError, "TGA texture '%s' has unsupported depth", path);
		return nullptr;
	}

	/* Determine image properties. */
	uint32_t width = header.width;
	uint32_t height = header.height;
	PixelFormat format = (header.depth == 32) ? PixelFormat::kB8G8R8A8 : PixelFormat::kB8G8R8;

	/* Read in the data, which is after the ID and colour map. */
	size_t size = width * height * (header.depth / 8);
	uint64_t offset = header.id_length + (header.colour_map_length * (header.colour_map_depth / 8));
	std::unique_ptr<char []> buf(new char[size]);
	if(!stream->read(buf.get(), size, offset)) {
		orion_log(LogLevel::kError, "Failed to read asset '%s' data", path);
		return nullptr;
	}

	/* Create the texture, with mipmaps. */
	Texture2D *texture = new Texture2D(width, height, format);
	texture->update(buf.get());

	return texture;
}
