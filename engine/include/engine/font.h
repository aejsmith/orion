/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Font asset.
 */

#pragma once

#include "core/hash_table.h"

#include "engine/asset.h"
#include "engine/texture.h"

#include <vector>

class Font;

/** Structure containing details of a glyph within a font. */
struct FontGlyph {
    unsigned x;                         /**< X position of glyph in texture. */
    unsigned y;                         /**< Y position of glyph in texture. */
    unsigned width;                     /**< Width of the glyph image. */
    unsigned height;                    /**< Height of the glyph image. */

    /** Horizontal offset from cursor position to left of glyph image. */
    unsigned offsetX;

    /** Vertical offset from cursor position to top of glyph image. */
    unsigned offsetY;

    /** Horizontal distance to advance the cursor to the next glyph position. */
    unsigned advance;
};

/** Descriptor for a font variant. */
struct FontVariantDesc {
    unsigned pointSize;                 /**< Font size. */

    // TODO: Weight, italic. Not supported at the moment because the Font asset
    // can only load one font file. In future we want to be able to bundle
    // multiple files for different weights etc. into one font asset.
public:
    /** Compare this descriptor with another. */
    bool operator ==(const FontVariantDesc &other) const {
        return pointSize == other.pointSize;
    }

    /** Get a hash from a font variant descriptor. */
    friend size_t hashValue(const FontVariantDesc &desc) {
        return hashValue(desc.pointSize);
    }
};

/**
 * Font variant.
 *
 * A font variant is an instantiation of a font asset with specific properties,
 * i.e. size, weight, etc. This is what is actually used to draw with.
 */
class FontVariant {
public:
    ~FontVariant();

    /** Get metrics and atlas position information for a glyph.
     * @param ch            Glyph to look up.
     * @return              Information for the specified glyph. */
    const FontGlyph &getGlyph(unsigned char ch) const { return m_glyphs[ch]; }

    /** @return             Font that the variant belongs to. */
    Font *font() const { return m_font; }
    /** @return             Point size of the font. */
    unsigned pointSize() const { return m_desc.pointSize; }
    /** @return             Texture atlas containing glyph data. */
    Texture2D *texture() const { return m_texture; }
    /** @return             Vertical distance between rows. */
    unsigned height() const { return m_height; }
    /** @return             Maximum glyph width. */
    unsigned maxWidth() const { return m_maxWidth; }
private:
    FontVariant(Font *font, const FontVariantDesc &desc);

    bool load();
private:
    Font *m_font;                       /**< Font that the variant belongs to. */
    FontVariantDesc m_desc;             /**< Descriptor used to create the font variant. */
    Texture2DPtr m_texture;             /**< Texture atlas containing glyph data. */
    unsigned m_height;                  /**< Distance from one baseline to the next. */
    unsigned m_maxWidth;                /**< Maximum glyph width. */
    unsigned m_maxAscender;             /**< Maximum distance from baseline to top of glyph. */
    unsigned m_maxDescender;            /**< Maximum distance from baseline to bottom of glyph. */

    /** Array of glyph information. */
    std::vector<FontGlyph> m_glyphs;

    friend class Font;
};

/**
 * Font asset.
 *
 * A font asset represents a font file. Fonts cannot be used directly for
 * rendering: they must be instantiated into a FontVariant with specific
 * properties.
 */
class Font : public Asset {
public:
    Font();
    ~Font();

    FontVariant *getVariant(const FontVariantDesc &desc);

    bool setData(std::unique_ptr<char[]> &&data, size_t size);

    bool isFixedWidth() const;
private:
    std::unique_ptr<char[]> m_data;     /**< TTF file data. */
    size_t m_dataSize;                  /**< TTF file data size. */
    void *m_face;                       /**< FreeType face. */

    /** Variants of the font. */
    HashMap<FontVariantDesc, std::unique_ptr<FontVariant>> m_variants;

    friend class FontVariant;
};

/** Type of a font pointer. */
typedef TypedAssetPtr<Font> FontPtr;
