/**
 * @file
 * @copyright           2015 Alex Smith
 * @brief               Font asset.
 *
 * TODO:
 *  - Unicode.
 *  - Can save space in the texture atlas by only using the individual glyph
 *    width rather than the maximum per glyph. Would need a lookup table for the
 *    X/Y coordinates in the atlas of each glyph.
 */

#include "core/engine_global.h"

#include "engine/font.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

/** FreeType library manager. */
class FreeTypeLibrary : Noncopyable {
public:
    /** Initialise the FreeType library manager. */
    FreeTypeLibrary() : m_library(nullptr) {}

    /** Destroy the FreeType library. */
    ~FreeTypeLibrary() { FT_Done_FreeType(m_library); }

    /** @return             FreeType library instance. */
    FT_Library get() {
        if (!m_library) {
            FT_Error ret = FT_Init_FreeType(&m_library);
            if (ret != 0)
                fatal("Failed to initialize FreeType: %d", ret);
        }

        return m_library;
    }
private:
    FT_Library m_library;           /**< FreeType library handle. */
};

/** DPI to render at. */
static const size_t kFontDPI = 96;

/** Global FreeType library instance. */
static FreeTypeLibrary g_freeType;

/** Initialise the font. */
Font::Font() :
    m_dataSize(0),
    m_face(nullptr)
{}

/** Destroy the font. */
Font::~Font() {
    FT_Face face = reinterpret_cast<FT_Face>(m_face);
    FT_Done_Face(face);
}

/**
 * Get a font variant.
 *
 * Instantiates a variant of this font with a set of properties. If the variant
 * has already been previously instantiated, the existing variant will be
 * returned. The returned variant will remain valid as long as the Font asset
 * is loaded. Therefore, users must ensure that they hold a reference to the
 * Font as long as they are using the variant.
 *
 * @param desc          Descriptor containing variant properties.
 *
 * @return              Pointer to created variant, or null if variant cannot
 *                      be loaded.
 */
FontVariant *Font::getVariant(const FontVariantDesc &desc) {
    /* See if the variant has already been created. */
    auto it = m_variants.find(desc);
    if (it != m_variants.end())
        return it->second.get();

    std::unique_ptr<FontVariant> variant(new FontVariant(this, desc));
    if (!variant->load())
        return nullptr;

    FontVariant *ret = variant.get();
    m_variants.insert(std::make_pair(desc, std::move(variant)));
    return ret;
}

/**
 * Set the font data.
 *
 * Sets the data for the font. The supplied buffer should contain a TTF font
 * file. On success, the object will take ownership of the supplied data. Once
 * a font has data, it cannot be replaced.
 *
 * @param data          Buffer containing TTF font data.
 * @param size          Size of the font data.
 *
 * @return              Whether the font data was valid.
 */
bool Font::setData(std::unique_ptr<char[]> &&data, size_t size) {
    checkMsg(!m_data, "Font data cannot be set more than once");

    FT_Library library = g_freeType.get();

    /* Open the face. */
    FT_Error ret = FT_New_Memory_Face(
        library,
        reinterpret_cast<const FT_Byte *>(data.get()),
        size,
        0,
        reinterpret_cast<FT_Face *>(&m_face));
    if (ret != 0) {
        logError("Failed to load font: %d", ret);
        return false;
    }

    m_data = std::move(data);
    m_dataSize = size;
    return true;
}

/** @return             Whether the font is fixed width. */
bool Font::isFixedWidth() const {
    FT_Face face = reinterpret_cast<FT_Face>(m_face);
    return FT_IS_FIXED_WIDTH(face);
}

/** Initialise the variant (does not load font).
 * @param font          Font this variant belongs to.
 * @param desc          Descriptor containing variant properties. */
FontVariant::FontVariant(Font *font, const FontVariantDesc &desc) :
    m_font(font),
    m_desc(desc),
    m_height(0),
    m_maxWidth(0),
    m_maxAscender(0),
    m_maxDescender(0)
{}

/** Destroy the variant. */
FontVariant::~FontVariant() {}

/** Load the font data (internal method called from Font::getVariant()).
 * @return              Whether the variant was successfully loaded. */
bool FontVariant::load() {
    FT_Face face = reinterpret_cast<FT_Face>(m_font->m_face);

    /* Set the point size of the font (size is given as 1/64th's of a point). */
    FT_Set_Char_Size(face, 0, pointSize() * 64, kFontDPI, kFontDPI);

    /* Determine maximum font heights. Divide by 64 to get pixels. */
    m_maxAscender = face->size->metrics.ascender / 64;
    m_maxDescender = -(face->size->metrics.descender / 64);
    m_height = std::max(
        m_maxAscender + m_maxDescender,
        static_cast<unsigned>(face->size->metrics.height / 64));

    /* Determine the maximum glyph width, which will be the spacing between
     * glyphs in the texture atlas.. For this we have to check each glyph in
     * turn. */
    for (unsigned i = 0; i < 256; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER) != 0) {
            /* If loading a glyph fails, load a question mark in its place. */
            if (FT_Load_Char(face, '?', FT_LOAD_RENDER) != 0) {
                logError("%s: Loading glyph %zu failed", m_font->path().c_str(), i);
                return false;
            }
        }

        /* Calculate the dimensions of the glyph. */
        m_maxWidth = std::max(
            m_maxWidth,
            face->glyph->bitmap_left + face->glyph->bitmap.width);
    }

    logDebug("%s: Loading point size %u", m_font->path().c_str(), pointSize());
    logDebug("  height = %u", m_height);
    logDebug("  maxAscender = %u", m_maxAscender);
    logDebug("  maxDescender = %u", m_maxDescender);
    logDebug("  maxWidth = %u", m_maxWidth);

    /* Create the texture atlas. */
    m_texture = new Texture2D(m_maxWidth * 256, m_height, PixelFormat::kR8);
    m_texture->clear();

    /* Load glyph data into the atlas. */
    m_glyphs.resize(256);
    for (unsigned i = 0; i < 256; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER) != 0) {
            if (FT_Load_Char(face, '?', FT_LOAD_RENDER) != 0) {
                logError("%s: Loading glyph %zu failed", m_font->path().c_str(), i);
                return false;
            }
        }

        /* Get glyph metrics. */
        m_glyphs[i].width = face->glyph->bitmap.width;
        m_glyphs[i].height = face->glyph->bitmap.rows;
        m_glyphs[i].offsetX = face->glyph->metrics.horiBearingX / 64;
        m_glyphs[i].offsetY = m_maxAscender - (face->glyph->metrics.horiBearingY / 64);
        m_glyphs[i].advance = face->glyph->metrics.horiAdvance / 64;

        /* Determine texture atlas position. We put all fonts on to one row for
         * now so we just have Y position as Y offset. */
        m_glyphs[i].x = (i * m_maxWidth) + face->glyph->bitmap_left;
        m_glyphs[i].y = m_glyphs[i].offsetY;

        /* Load the texture data. FIXME: Correctly handle bitmap formats. */
        IntRect area(m_glyphs[i].x, m_glyphs[i].y, m_glyphs[i].width, m_glyphs[i].height);
        m_texture->update(area, face->glyph->bitmap.buffer);
    }

    return true;
}
