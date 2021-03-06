/*
 * Copyright (C) 2015-2016 Alex Smith
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file
 * @brief               Object system metadata generator.
 */

#include "core/filesystem.h"
#include "core/string.h"

#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clang-c/Index.h>

#include <mustache/mustache.hpp>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#ifdef ORION_PLATFORM_WIN32
    #include <windows.h>
    #include <tchar.h>
#endif

#include "objgen.mustache.h"

using Mustache = Kainjow::BasicMustache<std::string>;

class ParsedTranslationUnit;

/** Details common to all parsed declarations. */
class ParsedDecl {
public:
    ParsedDecl(CXCursor _cursor, ParsedDecl *_parent = nullptr, bool nameFromType = false);
    virtual ~ParsedDecl() {}

    CXCursor cursor;                /**< Cursor for the declaration. */
    ParsedDecl *parent;             /**< Parent declaration. */
    std::string name;               /**< Name of the declaration. */
    bool isAnnotated;               /**< Whether the declaration is annotated. */

    bool isFromMainFile() const;
    ParsedTranslationUnit *getTranslationUnit();

    /** Generate this declaration.
     * @return              Generated Mustache data object. */
    virtual Mustache::Data generate() const { return Mustache::Data(); }

    /** Dump this declaration.
     * @param depth         Indentation depth. */
    virtual void dump(unsigned depth) const = 0;

    /** Type of the visitor function. */
    using VisitFunction = std::function<void (CXCursor, CXCursorKind)>;

    static void visitChildren(CXCursor cursor, ParsedDecl *decl);
    static void visitChildren(CXCursor cursor, const VisitFunction &function);
protected:
    /** Called when an annotation is observed on this declaration.
     * @param type          Type of the annotation.
     * @param attributes    Attributes specified for the annotation.
     * @return              Whether the annotation was consumed. */
    virtual bool handleAnnotation(const std::string &type, const rapidjson::Document &attributes) {
        return false;
    }

    /** Called when a (non-annotation) child is reached on this declaration.
     * @param cursor        Child cursor.
     * @param kind          Kind of the cursor. */
    virtual void handleChild(CXCursor cursor, CXCursorKind kind) {}
private:
    static CXChildVisitResult visitDeclCallback(CXCursor cursor, CXCursor parent, CXClientData data);
    static CXChildVisitResult visitFunctionCallback(CXCursor cursor, CXCursor parent, CXClientData data);
};

/** Details of a parsed property. */
class ParsedProperty : public ParsedDecl {
public:
    ParsedProperty(CXCursor cursor, ParsedDecl *parent);

    std::string type;               /**< Type of the property. */
    std::string getFunction;        /**< Getter function for the property (empty for direct access). */
    std::string setFunction;        /**< Setter function for the property (empty for direct access). */

    /** Behaviour flags. */
    bool transient;

    Mustache::Data generate() const override;
    void dump(unsigned depth) const override;
protected:
    bool handleAnnotation(const std::string &type, const rapidjson::Document &attributes) override;
};

/** Details of a parsed class. */
class ParsedClass : public ParsedDecl {
public:
    ParsedClass(CXCursor cursor, ParsedDecl *parent);

    bool isObjectDerived;           /**< Whether the class derives from Object. */
    ParsedClass *parentClass;       /**< Parent class. */

    /** List of child properties. */
    std::list<std::unique_ptr<ParsedProperty>> properties;

    bool isObject() const;
    bool isConstructable() const;
    bool isPublicConstructable() const;

    Mustache::Data generate() const override;
    void dump(unsigned depth) const override;
protected:
    bool handleAnnotation(const std::string &type, const rapidjson::Document &attributes) override;
    void handleChild(CXCursor cursor, CXCursorKind kind) override;
private:
    /** Whether the class is constructable. */
    enum class Constructability {
        kDefault,                   /**< No constructors have yet been declared. */
        kPublic,                    /**< Publically, the default when no constructor is declared. */
        kPrivate,                   /**< Private or protected. Only usable for deserialisation. */
        kNone,                      /**< None, if no suitable constructor found. */
        kForcedNone,                /**< Forced off by attribute. */
    };

    Constructability m_constructable;
    bool m_onMetaClass;             /**< Temporary state used while parsing. */
};

/** Details of a parsed enumeration. */
class ParsedEnum : public ParsedDecl {
public:
    bool shouldGenerate;            /**< Whether this enum is used and should have code generated. */

    Mustache::Data generate() const override;
    void dump(unsigned depth) const override;

    static void create(CXCursor cursor, ParsedDecl *parent);
protected:
    bool handleAnnotation(const std::string &type, const rapidjson::Document &attributes) override;
    void handleChild(CXCursor cursor, CXCursorKind kind) override;
private:
    /** Type of a pair describing an enum constant. */
    using EnumConstant = std::pair<std::string, long long>;

    ParsedEnum(CXCursor cursor, ParsedDecl *parent);

    /** Possible values of the enum. */
    std::list<EnumConstant> m_constants;
};

/** Details of a parsed translation unit. */
class ParsedTranslationUnit : public ParsedDecl {
public:
    explicit ParsedTranslationUnit(CXCursor cursor);

    /** List of child classes. */
    std::map<std::string, std::unique_ptr<ParsedClass>> classes;

    /** List of child enumerations (including ones nested within classes). */
    std::map<std::string, std::unique_ptr<ParsedEnum>> enums;

    Mustache::Data generate() const override;
    void dump(unsigned depth = 0) const override;
protected:
    void handleChild(CXCursor cursor, CXCursorKind kind) override;
};

/** Whether a parse error occurred. */
static bool g_parseErrorOccurred = false;

/** Raise a parse error.
 * @param cursor        AST node that the error occurred at.
 * @param fmt           Format string.
 * @param ...           Arguments to substitute into format. */
static void parseError(CXCursor cursor, const char *fmt, ...) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned line, column;
    clang_getSpellingLocation(location, &file, &line, &column, nullptr);
    CXString fileName = clang_getFileName(file);

    fprintf(stderr, "%s:%u:%u: error: ", clang_getCString(fileName), line, column);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    g_parseErrorOccurred = true;
}

/**
 * Mangle a name string for use in generated code.
 *
 * In the generated code we base the name of some of the variables we define
 * on the name of the class or enum. For a class or enum that is nested in a
 * namespace or inside another class, the name is of the form "Foo::Bar". This
 * cannot be directly used to name a variable, e.g. "Foo::Bar_data". This
 * function solves this by replacing "::" in the name string with "_" to give
 * a name suitable for naming our generated variables.
 *
 * @param name          Original name string.
 *
 * @return              Mangled name string.
 */
static std::string mangleName(std::string name) {
    size_t pos;
    while ((pos = name.find("::")) != std::string::npos)
        name.replace(pos, 2, "_");

    return name;
}

/** Initialise the declaration.
 * @param _cursor       Cursor to initialise from.
 * @param _parent       Parent declaration.
 * @param nameFromType  Whether to get name from the type rather than directly
 *                      from the cursor. */
ParsedDecl::ParsedDecl(CXCursor _cursor, ParsedDecl *_parent, bool nameFromType) :
    cursor(_cursor),
    parent(_parent),
    isAnnotated(false)
{
    CXString name;

    if (nameFromType) {
        CXType type = clang_getCursorType(cursor);
        name = clang_getTypeSpelling(type);
    } else {
        name = clang_getCursorSpelling(cursor);
    }

    this->name = clang_getCString(name);
    clang_disposeString(name);
}

/** @return             Whether this declaration is from the main file. */
bool ParsedDecl::isFromMainFile() const {
    return clang_Location_isFromMainFile(clang_getCursorLocation(this->cursor));
}

/** @return             The translation unit this declaration belongs to. */
ParsedTranslationUnit *ParsedDecl::getTranslationUnit() {
    ParsedDecl *decl = this;

    while (decl->parent)
        decl = decl->parent;

    return static_cast<ParsedTranslationUnit *>(decl);
}

/** Parse an annotation string.
 * @param cursor        Cursor for annotation.
 * @param type          Where to store annotation type.
 * @param attributes    Where to store annotation attributes.
 * @return              Whether parsed successfully. */
static bool parseAnnotation(CXCursor cursor, std::string &type, rapidjson::Document &attributes) {
    CXString str = clang_getCursorSpelling(cursor);
    std::string annotation(clang_getCString(str));
    clang_disposeString(str);

    std::vector<std::string> tokens;
    String::tokenize(annotation, tokens, ":", 3, false);

    if (tokens[0].compare("orion")) {
        /* Don't raise an error for annotations that aren't marked as being for
         * us, could be annotations for other reasons. */
        return false;
    } else if (tokens.size() != 3) {
        parseError(cursor, "malformed annotation");
        return false;
    }

    type = tokens[1];

    std::string json = std::string("{") + tokens[2] + std::string("}");

    attributes.Parse(json.c_str());
    if (attributes.HasParseError()) {
        const char *msg = rapidjson::GetParseError_En(attributes.GetParseError());
        parseError(cursor, "parse error in attributes (at %zu): %s", attributes.GetErrorOffset() - 1, msg);
        return false;
    }

    return true;
}

/** AST visitor callback function.
 * @param cursor        Current cursor.
 * @param parent        Parent cursor.
 * @param data          Next declaration pointer.
 * @return              Child visitor status code. */
CXChildVisitResult ParsedDecl::visitDeclCallback(CXCursor cursor, CXCursor parent, CXClientData data) {
    ParsedDecl *currentDecl = reinterpret_cast<ParsedDecl *>(data);

    CXCursorKind kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_AnnotateAttr) {
        std::string type;
        rapidjson::Document attributes;
        if (!parseAnnotation(cursor, type, attributes))
            return CXChildVisit_Continue;

        if (currentDecl->handleAnnotation(type, attributes)) {
            currentDecl->isAnnotated = true;
        } else {
            parseError(cursor, "unexpected '%s' annotation", type.c_str());
        }
    } else {
        currentDecl->handleChild(cursor, kind);
    }

    return CXChildVisit_Continue;
}

/** Visit child nodes.
 * @param cursor        Current cursor position.
 * @param decl          Declaration to visit with. */
void ParsedDecl::visitChildren(CXCursor cursor, ParsedDecl *decl) {
    clang_visitChildren(cursor, visitDeclCallback, decl);
}

/** AST visitor callback function.
 * @param cursor        Current cursor.
 * @param parent        Parent cursor.
 * @param data          Pointer to std::function to call.
 * @return              Child visitor status code. */
CXChildVisitResult ParsedDecl::visitFunctionCallback(CXCursor cursor, CXCursor parent, CXClientData data) {
    const VisitFunction &function = *reinterpret_cast<const VisitFunction *>(data);
    CXCursorKind kind = clang_getCursorKind(cursor);
    function(cursor, kind);
    return CXChildVisit_Continue;
}

/** Visit child nodes.
 * @param cursor        Current cursor position.
 * @param function      Function to call on each child. */
void ParsedDecl::visitChildren(CXCursor cursor, const VisitFunction &function) {
    clang_visitChildren(cursor, visitFunctionCallback, const_cast<VisitFunction *>(&function));
}

/** Initialise the property.
 * @param cursor        Cursor to initialise from.
 * @param _parent       Parent declaration. */
ParsedProperty::ParsedProperty(CXCursor cursor, ParsedDecl *parent) :
    ParsedDecl (cursor, parent),
    transient  (false)
{
    /* Remove prefixes from property names. */
    if (this->name.substr(0, 2) == "m_") {
        this->name = this->name.substr(2);
    } else if (this->name.substr(0, 6) == "vprop_") {
        this->name = this->name.substr(6);
    }

    /* Get the property type. */
    CXType type = clang_getCursorType(cursor);
    CXString str = clang_getTypeSpelling(type);
    this->type = clang_getCString(str);
    clang_disposeString(str);
}

/** Called when an annotation is observed on this declaration.
 * @param type          Type of the annotation.
 * @param attributes    Attributes specified for the annotation.
 * @return              Whether the annotation was consumed. */
bool ParsedProperty::handleAnnotation(const std::string &type, const rapidjson::Document &attributes) {
    if (type.compare("property") != 0)
        return false;

    /* Now that we know that we are really a property, if our type is an enum,
     * mark that enum for code generation. */
    CXType propertyType = clang_getCursorType(cursor);
    CXCursor propertyTypeDecl = clang_getTypeDeclaration(propertyType);
    if (clang_getCursorKind(propertyTypeDecl) == CXCursor_EnumDecl) {
        const ParsedTranslationUnit *translationUnit = getTranslationUnit();
        auto it = translationUnit->enums.find(this->type);
        if (it != translationUnit->enums.end()) {
            it->second->shouldGenerate = true;
        } else {
            parseError(
                this->cursor,
                "full declaration of enum '%s' must be available for property '%s'",
                this->type.c_str(), this->name.c_str());
            return true;
        }
    }

    ParsedClass *parent = static_cast<ParsedClass *>(this->parent);
    if (!parent->isObjectDerived) {
        parseError(
            this->cursor,
            "'property' annotation on field '%s' in non-Object class '%s'",
            this->name.c_str(), this->parent->name.c_str());
        return true;
    }

    static const char *kGetAttribute       = "get";
    static const char *kSetAttribute       = "set";
    static const char *kTransientAttribute = "transient";

    if (attributes.HasMember(kGetAttribute)) {
        const rapidjson::Value &value = attributes[kGetAttribute];

        if (!value.IsString()) {
            parseError(this->cursor, "'%s' attribute must be a string", kGetAttribute);
            return true;
        }

        this->getFunction = value.GetString();
    }

    if (attributes.HasMember(kSetAttribute)) {
        const rapidjson::Value &value = attributes[kSetAttribute];

        if (!value.IsString()) {
            parseError(this->cursor, "'%s' attribute must be a string", kSetAttribute);
            return true;
        }

        this->setFunction = value.GetString();
    }

    if (this->getFunction.empty() != this->setFunction.empty()) {
        parseError(this->cursor, "both 'get' and 'set' or neither of them must be specified");
        return true;
    }

    if (attributes.HasMember(kTransientAttribute)) {
        const rapidjson::Value &value = attributes[kTransientAttribute];

        if (!value.IsBool()) {
            parseError(this->cursor, "'%s' attribute must be a boolean", kTransientAttribute);
            return true;
        }

        this->transient = value.GetBool();
    }

    if (clang_getCXXAccessSpecifier(this->cursor) != CX_CXXPublic) {
        parseError(this->cursor, "property '%s' must be public", this->name.c_str());
        return true;
    }

    bool isVirtual = clang_getCursorKind(this->cursor) == CXCursor_VarDecl;
    if (isVirtual) {
        if (this->getFunction.empty()) {
            /* These require getters and setters. If they are omitted, default
             * names are used based on the property name. */
            this->getFunction = this->name;
            this->setFunction =
                std::string("set") +
                static_cast<char>(toupper(this->name[0])) +
                this->name.substr(1);
        }
    } else if (!this->getFunction.empty()) {
        /* This makes no sense - code can directly access/modify the property
         * so usage of getter/setter methods should not be required. */
        parseError(this->cursor, "public properties cannot have getter/setter methods");
        return true;
    }

    return true;
}

/** Generate this declaration.
 * @return              Generated Mustache data object. */
Mustache::Data ParsedProperty::generate() const {
    Mustache::Data data;

    /* Generate a flags string. */
    std::string flags;
    auto addFlag =
        [&] (const char *flag) {
            if (!flags.empty())
                flags += " | ";
            flags += "MetaProperty::";
            flags += flag;
        };

    if (this->transient)
        addFlag("kTransient");

    if (flags.empty())
        flags = "0";

    data.set("propertyName", this->name);
    data.set("propertyType", this->type);
    data.set("propertyFlags", flags);

    if (!this->getFunction.empty()) {
        data.set("propertyGet", this->getFunction);
        data.set("propertySet", this->setFunction);
    }

    return data;
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedProperty::dump(unsigned depth) const {
    printf(
        "%-*sProperty '%s' (type '%s', get '%s', set '%s')\n",
        depth * 2, "",
        this->name.c_str(), this->type.c_str(), this->getFunction.c_str(),
        this->setFunction.c_str());
}

/** Initialise the class.
 * @param cursor        Cursor to initialise from.
 * @param _parent       Parent declaration. */
ParsedClass::ParsedClass(CXCursor cursor, ParsedDecl *parent) :
    ParsedDecl(cursor, parent, true),
    isObjectDerived(name.compare("Object") == 0),
    parentClass(nullptr),
    m_constructable(Constructability::kDefault),
    m_onMetaClass(false)
{}

/**
 * Check whether the class is a valid object class.
 *
 * The return value indicates whether this class should have code generated for
 * it. If there are any code errors then the global parse error flag will be
 * set.
 *
 * @return              Whether the class is an object class.
 */
bool ParsedClass::isObject() const {
    if (this->isAnnotated && this->isObjectDerived) {
        return true;
    } else if (this->isObjectDerived) {
        parseError(
            this->cursor,
            "Object-derived class '%s' missing 'class' annotation; CLASS() macro missing?",
            this->name.c_str());
    }

    return false;
}

/** @return             Whether the class is constructable (public or otherwise). */
bool ParsedClass::isConstructable() const {
    switch (m_constructable) {
    case Constructability::kDefault:
    case Constructability::kPublic:
    case Constructability::kPrivate:
        return true;
    default:
        return false;
    }
};

/** @return             Whether the constructor is publically constructable. */
bool ParsedClass::isPublicConstructable() const {
    switch (m_constructable) {
    case Constructability::kDefault:
    case Constructability::kPublic:
        return true;
    default:
        return false;
    }
}

/** Called when an annotation is observed on this declaration.
 * @param type          Type of the annotation.
 * @param attributes    Attributes specified for the annotation.
 * @return              Whether the annotation was consumed. */
bool ParsedClass::handleAnnotation(const std::string &type, const rapidjson::Document &attributes) {
    if (!m_onMetaClass || type.compare("class") != 0)
        return false;

    if (!this->isObjectDerived) {
        parseError(
            cursor,
            "'class' annotation on non-Object class '%s'",
            this->name.c_str());
    }

    if (attributes.HasMember("constructable")) {
        const rapidjson::Value &value = attributes["constructable"];

        if (!value.IsBool()) {
            parseError(this->cursor, "'constructable' attribute must be a boolean");
            return true;
        }

        bool constructable = value.GetBool();
        if (constructable) {
            parseError(this->cursor, "constructability cannot be forced on, only off");
            return true;
        }

        m_constructable = Constructability::kForcedNone;
    }

    return true;
}

/** Called when a child is reached on this declaration.
 * @param cursor        Child cursor.
 * @param kind          Kind of the cursor. */
void ParsedClass::handleChild(CXCursor cursor, CXCursorKind kind) {
    if (m_onMetaClass)
        return;

    switch (kind) {
        case CXCursor_CXXBaseSpecifier:
        {
            /* Check if this class is derived from Object. This gives us the
             * fully-qualified name (with all namespaces) regardless of whether
             * it was specified that way in the source. */
            CXType type = clang_getCursorType(cursor);
            CXString str = clang_getTypeSpelling(type);
            std::string typeName(clang_getCString(str));
            clang_disposeString(str);

            /* The translation unit records all Object-derived classes seen,
             * even those outside the main file. Therefore, we look for the
             * base class name in there, and if it matches one of those, then
             * we are an Object-derived class as well. */
            const ParsedTranslationUnit *translationUnit = getTranslationUnit();
            auto it = translationUnit->classes.find(typeName);
            if (it != translationUnit->classes.end()) {
                /* If isObjectDerived is already set to true, then we have
                 * multiple inheritance, which is unsupported. */
                if (this->isObjectDerived) {
                    parseError(
                        cursor,
                        "Inheritance from multiple Object-derived classes is unsupported (on class '%s')",
                        this->name.c_str());
                }

                this->isObjectDerived = true;
                this->parentClass = it->second.get();
            }

            break;
        }

        case CXCursor_Constructor:
        {
            /* Ignore if forced to be non-constructable. */
            if (m_constructable == Constructability::kForcedNone)
                break;

            /* Determine the number of parameters to this constructor. */
            unsigned numParams = 0;
            visitChildren(
                cursor,
                [&numParams] (CXCursor cursor, CXCursorKind kind) {
                    if (kind == CXCursor_ParmDecl)
                        numParams++;
                });

            /* Only constructors with no parameters are suitable. */
            if (numParams == 0) {
                if (clang_getCXXAccessSpecifier(cursor) == CX_CXXPublic) {
                    m_constructable = Constructability::kPublic;
                } else {
                    m_constructable = Constructability::kPrivate;
                }
            } else {
                /* If no other constructors have been seen so far, mark as
                 * non-constructable. */
                if (m_constructable == Constructability::kDefault)
                    m_constructable = Constructability::kNone;
            }

            break;
        }

        case CXCursor_VarDecl:
        {
            /* Static class variables fall under VarDecl. The class annotation
             * is applied to the staticMetaClass member, so if we have that
             * variable, then descend onto children keeping the same current
             * declaration so we see the annotation below. */
            CXString str = clang_getCursorSpelling(cursor);
            std::string typeName(clang_getCString(str));
            clang_disposeString(str);
            if (!typeName.compare("staticMetaClass")) {
                m_onMetaClass = true;
                visitChildren(cursor, this);
                m_onMetaClass = false;
                break;
            }

            /* Fall through to check for virtual properties. */
        }

        case CXCursor_FieldDecl:
        {
            /* FieldDecl is an instance variable. Look for properties. */
            std::unique_ptr<ParsedProperty> parsedProperty(new ParsedProperty(cursor, this));
            visitChildren(cursor, parsedProperty.get());

            if (parsedProperty->isAnnotated)
                this->properties.emplace_back(std::move(parsedProperty));

            break;
        }

        case CXCursor_CXXMethod:
            /* Classes with pure virtual methods are not constructable.
             * TODO: This does not handle a class which is abstract because a
             * parent class has virtual methods that it does not override.
             * libclang doesn't appear to have an easy way to identify this, so
             * for now don't handle it. If it does become a problem it can be
             * worked around using the constructable attribute. */
            if (clang_CXXMethod_isPureVirtual(cursor))
                m_constructable = Constructability::kForcedNone;

            break;

        case CXCursor_EnumDecl:
            ParsedEnum::create(cursor, this);
            break;

        default:
            break;
    }
}

/** Generate this declaration.
 * @return              Generated Mustache data object. */
Mustache::Data ParsedClass::generate() const {
    Mustache::Data data;

    data.set("name", this->name);
    data.set("mangledName", mangleName(this->name));

    if (this->parentClass)
        data.set("parent", this->parentClass->name);

    if (isConstructable())
        data.set("isConstructable", Mustache::Data::Type::True);
    if (isPublicConstructable())
        data.set("isPublicConstructable", Mustache::Data::Type::True);

    Mustache::Data properties(Mustache::Data::List());
    for (const std::unique_ptr<ParsedProperty> &parsedProperty : this->properties)
        properties.push_back(parsedProperty->generate());

    data.set("properties", properties);

    return data;
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedClass::dump(unsigned depth) const {
    printf("%-*sClass '%s' (", depth * 2, "", this->name.c_str());

    if (this->parentClass)
        printf("parent '%s', ", this->parentClass->name.c_str());

    printf("constructable %d %d)\n", isConstructable(), isPublicConstructable());

    for (const std::unique_ptr<ParsedProperty> &parsedProperty : this->properties)
        parsedProperty->dump(depth + 1);
}

/** Initialise the enum.
 * @param cursor        Cursor to initialise from.
 * @param _parent       Parent declaration. */
ParsedEnum::ParsedEnum(CXCursor cursor, ParsedDecl *parent) :
    ParsedDecl(cursor, parent, true),
    shouldGenerate(false)
{}

/** Create an enum and add it to the translation unit.
 * @param cursor        Cursor to initialise from.
 * @param parent        Parent declaration. */
void ParsedEnum::create(CXCursor cursor, ParsedDecl *parent) {
    if (!clang_isCursorDefinition(cursor))
        return;

    /* We don't handle anonymous enums. There is no function that specifically
     * identifies this, so the way we do this is to check if the cursor
     * spelling is empty. Have to do this separately rather than checking the
     * name obtained by the constructor because that gets the type spelling
     * which is not empty for an anonymous enum. */
    CXString name = clang_getCursorSpelling(cursor);
    bool anonymous = std::strlen(clang_getCString(name)) == 0;
    clang_disposeString(name);
    if (anonymous)
        return;

    std::unique_ptr<ParsedEnum> parsedEnum(new ParsedEnum(cursor, parent));

    visitChildren(cursor, parsedEnum.get());
    parent->getTranslationUnit()->enums.insert(std::make_pair(parsedEnum->name, std::move(parsedEnum)));
}

/** Called when an annotation is observed on this declaration.
 * @param type          Type of the annotation.
 * @param attributes    Attributes specified for the annotation.
 * @return              Whether the annotation was consumed. */
bool ParsedEnum::handleAnnotation(const std::string &type, const rapidjson::Document &attributes) {
    if (type.compare("enum") != 0)
        return false;

    this->shouldGenerate = true;
    return true;
}

/** Called when a child is reached on this declaration.
 * @param cursor        Child cursor.
 * @param kind          Kind of the cursor. */
void ParsedEnum::handleChild(CXCursor cursor, CXCursorKind kind) {
    if (kind == CXCursor_EnumConstantDecl) {
        long long value = clang_getEnumConstantDeclValue(cursor);

        CXString str = clang_getCursorSpelling(cursor);
        std::string name(clang_getCString(str));
        clang_disposeString(str);

        m_constants.push_back(std::make_pair(name, value));
    }
}

/** Generate this declaration.
 * @return              Generated Mustache data object. */
Mustache::Data ParsedEnum::generate() const {
    Mustache::Data data;

    data.set("name", this->name);
    data.set("mangledName", mangleName(this->name));

    Mustache::Data constants(Mustache::Data::List());
    for (const EnumConstant &constant : m_constants) {
        std::stringstream valueStr;
        valueStr << constant.second;

        Mustache::Data constantData;
        constantData.set("constantName", constant.first);
        constantData.set("constantValue", valueStr.str());
        constants.push_back(constantData);
    }

    data.set("constants", constants);

    return data;
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedEnum::dump(unsigned depth) const {
    printf("%-*sEnum '%s'\n", depth * 2, "", this->name.c_str());

    for (auto &pair : m_constants)
        printf("%-*s'%s' = %lld\n", (depth + 1) * 2, "", pair.first.c_str(), pair.second);
}

/** Initialise the translation unit.
 * @param cursor        Cursor to initialise from. */
ParsedTranslationUnit::ParsedTranslationUnit(CXCursor cursor) :
    ParsedDecl(cursor)
{}

/** Called when a child is reached on this declaration.
 * @param cursor        Child cursor.
 * @param kind          Kind of the cursor. */
void ParsedTranslationUnit::handleChild(CXCursor cursor, CXCursorKind kind) {
    switch (kind) {
        case CXCursor_Namespace:
            /* Descend into namespaces. */
            visitChildren(cursor, this);
            break;

        case CXCursor_ClassDecl:
        case CXCursor_StructDecl:
            /* Ignore forward declarations. */
            if (clang_isCursorDefinition(cursor)) {
                std::unique_ptr<ParsedClass> parsedClass(new ParsedClass(cursor, this));
                visitChildren(cursor, parsedClass.get());

                if (parsedClass->isObject())
                    this->classes.insert(std::make_pair(parsedClass->name, std::move(parsedClass)));
            }

            break;

        case CXCursor_EnumDecl:
            ParsedEnum::create(cursor, this);
            break;

        default:
            break;
    }
}

/** Generate this declaration.
 * @return              Generated Mustache data object. */
Mustache::Data ParsedTranslationUnit::generate() const {
    Mustache::Data classes(Mustache::Data::List());
    for (auto &it : this->classes) {
        const std::unique_ptr<ParsedClass> &parsedClass = it.second;

        if (parsedClass->isFromMainFile())
            classes.push_back(parsedClass->generate());
    }

    Mustache::Data enums(Mustache::Data::List());
    for (auto &it : this->enums) {
        const std::unique_ptr<ParsedEnum> &parsedEnum = it.second;

        if (parsedEnum->shouldGenerate)
            enums.push_back(parsedEnum->generate());
    }

    Mustache::Data data;
    data.set("classes", classes);
    data.set("enums", enums);
    return data;
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedTranslationUnit::dump(unsigned depth) const {
    printf("%-*sTranslationUnit '%s'\n", depth * 2, "", this->name.c_str());

    for (auto &it : this->classes) {
        const std::unique_ptr<ParsedClass> &parsedClass = it.second;

        if (parsedClass->isFromMainFile())
            parsedClass->dump(depth + 1);
    }

    for (auto &it : this->enums) {
        const std::unique_ptr<ParsedEnum> &parsedEnum = it.second;

        if (parsedEnum->shouldGenerate)
            parsedEnum->dump(depth + 1);
    }
}

/** Print usage information.
 * @param argv0         Program name. */
static void usage(const char *argv0) {
    printf("Usage: %s [options...] <source> <output>\n", argv0);
    printf("\n");
    printf("Options:\n");
    printf("  -h            Display this help\n");
    printf("  -d            Dump parsed information, do not generate code\n");
    printf("  -D<define>    Preprocessor definition (as would be passed to clang)\n");
    printf("  -I<path>      Preprocessor include path (as would be passed to clang)\n");
    printf("  -s            Generate standalone code, which does not include the source file\n");
    printf("  -e            Ignore parse errors, generate empty output if any occur\n");
}

/** Main function of the object compiler.
 * @param argc          Argument count.
 * @param argv          Argument array.
 * @return              EXIT_SUCCESS or EXIT_FAILURE. */
int main(int argc, char **argv) {
    std::vector<const char *> clangArgs;
    bool dump = false;
    bool standalone = false;
    bool ignoreErrors = false;

    /* Parse arguments. */
    int opt;
    while ((opt = getopt(argc, argv, "hdD:I:se")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                return EXIT_SUCCESS;
            case 'd':
                dump = true;
                break;
            case 'D':
                clangArgs.push_back("-D");
                clangArgs.push_back(optarg);
                break;
            case 'I':
                clangArgs.push_back("-I");
                clangArgs.push_back(optarg);
                break;
            case 's':
                standalone = true;
                break;
            case 'e':
                ignoreErrors = true;
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    if (argc - optind != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *sourceFile = argv[optind];
    const char *outputFile = argv[optind + 1];

    /* Open the output file. This must be done first for standalone mode, so
     * that the generated file included by the source file exists. The wrapper
     * ensures that it is deleted if we fail. */
    struct OutputStream : std::ofstream {
        explicit OutputStream(const char *_file) :
            std::ofstream(_file, std::ofstream::out | std::ofstream::trunc),
            file(_file)
        {}

        ~OutputStream() {
            if (is_open()) {
                close();
                std::remove(this->file);
            }
        }

        const char *file;
    };

    OutputStream outputStream(outputFile);
    if (!outputStream) {
        fprintf(stderr, "%s: Failed to open '%s': %s\n", argv[0], outputFile, strerror(errno));
        return EXIT_FAILURE;
    }

    /* Source code is C++14, and define a macro to indicate we are the object
     * compiler. */
    clangArgs.push_back("-x");
    clangArgs.push_back("c++");
    clangArgs.push_back("-std=c++14");
    clangArgs.push_back("-DORION_OBJGEN=1");
    #if ORION_PLATFORM_WIN32
        clangArgs.push_back("-fms-compatibility-version=19");
    #endif

    /* Create an index with diagnostic output disabled. */
    CXIndex index = clang_createIndex(1, 0);

    /* Parse the source file. */
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        sourceFile,
        &clangArgs[0],
        clangArgs.size(),
        nullptr,
        0,
        CXTranslationUnit_Incomplete | CXTranslationUnit_SkipFunctionBodies);
    if (!unit) {
        fprintf(stderr, "%s: Error creating translation unit\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Check for errors. */
    unsigned numDiags = clang_getNumDiagnostics(unit);
    bool hadError = false;
    for (unsigned i = 0; i < numDiags; i++) {
        CXDiagnostic diag = clang_getDiagnostic(unit, i);

        if (clang_getDiagnosticSeverity(diag) >= CXDiagnostic_Error) {
            hadError = true;

            CXString diagString = clang_formatDiagnostic(
                diag,
                clang_defaultDiagnosticDisplayOptions());
            fprintf(stderr, "%s\n", clang_getCString(diagString));
            clang_disposeString(diagString);
        }

        clang_disposeDiagnostic(diag);
    }

    /* Begin output generation. */
    Mustache codeTemplate(g_objgenTemplate);
    Mustache::Data codeData;

    /* The ignore errors flag exists because in the case of a compilation error
     * during the real build, we want the error to be reported by the actual
     * compiler because those errors are usually more informative and with nicer
     * formatting, etc. When this flag is set, we generate an output file that
     * only includes the offending source file, and return success so that the
     * build will proceed and error when the compiler tries to compile our
     * output. Note this only applies to clang errors, we still fail for our
     * own errors. */
    if (hadError) {
        if (!ignoreErrors)
            return EXIT_FAILURE;
        fprintf(stderr, "%s: warning: Failed to generate, continuing upon request\n", outputFile);
    } else {
        /* Iterate over the AST. */
        CXCursor cursor = clang_getTranslationUnitCursor(unit);
        ParsedTranslationUnit parsedUnit(cursor);
        ParsedDecl::visitChildren(cursor, &parsedUnit);

        if (g_parseErrorOccurred)
            return EXIT_FAILURE;

        if (dump) {
            parsedUnit.dump();
            return EXIT_SUCCESS;
        }

        /* Generate the output data. */
        codeData = parsedUnit.generate();
    }

    if (!standalone) {
        /* For now resolve the source file path to an absolute path, and use
         * that as the include. It's not ideal as things will break if the
         * build tree is moved around, so if this becomes an issue in future
         * we could instead try to calculate a relative path between the output
         * directory and the source file. */
        Path fullPath;
        if (!Filesystem::getFullPath(Path(sourceFile, Path::kUnnormalizedPlatform), fullPath)) {
            fprintf(stderr, "%s: Failed to get absolute path of '%s'\n", argv[0], sourceFile);
            return EXIT_FAILURE;
        }

        codeData.set("include", fullPath.toPlatform());
    }

    codeTemplate.render(codeData, outputStream);

    /* We have succeeded, don't delete on exit. */
    outputStream.close();

    return EXIT_SUCCESS;
}
