/*
 * Copyright (C) 2015 Alex Smith
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
 * @brief               Object compiler main function.
 */

#include <clang-c/Index.h>

#include <list>
#include <memory>
#include <string>
#include <vector>

#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/** Details common to all parsed declarations. */
struct ParsedDecl {
    CXCursor cursor;                /**< Cursor for the declaration. */
    ParsedDecl *parent;             /**< Parent declaration. */
    std::string name;               /**< Name of the declaration. */
    bool isAnnotated;               /**< Whether the declaration is annotated. */
public:
    ParsedDecl(CXCursor _cursor, ParsedDecl *_parent);
    virtual ~ParsedDecl() {}

    /** Dump this declaration.
     * @param depth         Indentation depth. */
    virtual void dump(unsigned depth) = 0;

    static void visitChildren(CXCursor cursor, ParsedDecl *decl);
protected:
    bool handleAnnotation(CXCursor cursor, const char *type);

    /** Called when a child is reached on this declaration.
     * @param cursor        Child cursor.
     * @param kind          Kind of the cursor. */
    virtual void handleChild(CXCursor cursor, CXCursorKind kind) = 0;
private:
    static CXChildVisitResult astCallback(CXCursor cursor, CXCursor parent, CXClientData data);
};

/** Details of a parsed property. */
struct ParsedProperty : ParsedDecl {
public:
    ParsedProperty(CXCursor cursor, ParsedDecl *parent);

    void dump(unsigned depth) override;
protected:
    void handleChild(CXCursor cursor, CXCursorKind kind) override;
};

/** Details of a parsed class. */
struct ParsedClass : ParsedDecl {
    bool isObject;                  /**< Whether the class derives from Object. */

    /** List of child properties. */
    std::list<std::unique_ptr<ParsedProperty>> properties;
public:
    ParsedClass(CXCursor cursor, ParsedDecl *parent);

    bool isValid() const;

    void dump(unsigned depth) override;
protected:
    void handleChild(CXCursor cursor, CXCursorKind kind) override;
};

/** Details of a parsed translation unit. */
struct ParsedTranslationUnit : ParsedDecl {
    /** List of child classes. */
    std::list<std::unique_ptr<ParsedClass>> classes;
public:
    explicit ParsedTranslationUnit(CXCursor cursor);

    void dump(unsigned depth = 0) override;
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

/** Split a string into tokens.
 * @param str           String to split.
 * @param tokens        Vector to fill with tokens (existing content left intact).
 * @param delimiters    Delimiter characters (defaults to " ").
 * @param maxTokens     Maximum number of tokens (-1 for no limit, the default).
 *                      After this limit is reached the remainder of the string
 *                      is added to the last token. */
static void tokenize(
    const std::string &str,
    std::vector<std::string> &tokens,
    const char *delimiters = " ",
    int maxTokens = -1)
{
    size_t last = 0;
    size_t pos = 0;
    int numTokens = 0;

    while (pos != std::string::npos) {
        if (maxTokens > 0 && numTokens == maxTokens - 1) {
            tokens.emplace_back(str, last);
            break;
        } else {
            pos = str.find_first_of(delimiters, last);
            tokens.emplace_back(str, last, pos - last);
            last = pos + 1;
            numTokens++;
        }
    }
}

/** Initialise the declaration.
 * @param _cursor       Cursor to initialise from.
 * @param _parent       Parent declaration. */
ParsedDecl::ParsedDecl(CXCursor _cursor, ParsedDecl *_parent) :
    cursor(_cursor),
    parent(_parent),
    isAnnotated(false)
{
    CXString name = clang_getCursorSpelling(cursor);
    this->name = clang_getCString(name);
    clang_disposeString(name);
}

/** Handle an annotation attribute.
 * @param cursor        Current cursor position.
 * @param type          Expected annotation type string.
 * @return              Whether this was the expected annotation type. */
bool ParsedDecl::handleAnnotation(CXCursor cursor, const char *type) {
    CXString str = clang_getCursorSpelling(cursor);
    std::string annotation(clang_getCString(str));
    clang_disposeString(str);

    std::vector<std::string> tokens;
    tokenize(annotation, tokens, ":", 3);

    if (tokens[0].compare("orion")) {
        /* Don't raise an error for annotations that aren't marked as being
         * for us, could be annotations for other reasons. */
        return false;
    } else if (tokens.size() != 3) {
        parseError(cursor, "malformed annotation");
        return false;
    } else if (tokens[1].compare(type)) {
        parseError(cursor, "unexpected '%s' annotation", tokens[1].c_str());
        return false;
    }

    // TODO: Parse the annotation arguments.

    this->isAnnotated = true;
    return true;
}

/** AST visitor callback function.
 * @param cursor        Current cursor.
 * @param parent        Parent cursor.
 * @param data          Next declaration pointer.
 * @return              Child visitor status code. */
CXChildVisitResult ParsedDecl::astCallback(CXCursor cursor, CXCursor parent, CXClientData data) {
    CXCursorKind kind = clang_getCursorKind(cursor);
    ParsedDecl *currentDecl = reinterpret_cast<ParsedDecl *>(data);
    currentDecl->handleChild(cursor, kind);
    return CXChildVisit_Continue;
}

/** Visit child nodes.
 * @param cursor        Current cursor position.
 * @param decl          Declaration to visit with. */
void ParsedDecl::visitChildren(CXCursor cursor, ParsedDecl *decl) {
    clang_visitChildren(cursor, astCallback, decl);
}

/** Initialise the property.
 * @param cursor        Cursor to initialise from.
 * @param _parent       Parent declaration. */
ParsedProperty::ParsedProperty(CXCursor cursor, ParsedDecl *parent) :
    ParsedDecl(cursor, parent)
{}

/** Called when a child is reached on this declaration.
 * @param cursor        Child cursor.
 * @param kind          Kind of the cursor. */
void ParsedProperty::handleChild(CXCursor cursor, CXCursorKind kind) {
    switch (kind) {
        case CXCursor_AnnotateAttr:
            if (handleAnnotation(cursor, "property")) {
                ParsedClass *parent = static_cast<ParsedClass *>(this->parent);
                if (!parent->isObject) {
                    parseError(
                        cursor,
                        "'property' attribute on field '%s' in non-Object class '%s'",
                        this->name.c_str(), this->parent->name.c_str());
                }
            }

            break;
        default:
            break;
    }
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedProperty::dump(unsigned depth) {
    printf("%-*sProperty '%s'\n", depth * 2, "", this->name.c_str());
}

/** Initialise the class.
 * @param cursor        Cursor to initialise from.
 * @param _parent       Parent declaration. */
ParsedClass::ParsedClass(CXCursor cursor, ParsedDecl *parent) :
    ParsedDecl(cursor, parent),
    isObject(false)
{}

/**
 * Check validity of the class.
 *
 * Checks the whether the class is a valid meta-class. The return value
 * indicates whether this class should have code generated for it. If there
 * are any code errors then the global parse error flag will be set.
 *
 * @return              Whether the class is valid.
 */
bool ParsedClass::isValid() const {
    if (this->isAnnotated && this->isObject) {
        return true;
    } else if (this->isObject) {
        parseError(
            this->cursor,
            "Object-derived class '%s' missing 'class' annotation; CLASS() macro missing?",
            this->name.c_str());
    }

    return false;
}

/** Called when a child is reached on this declaration.
 * @param cursor        Child cursor.
 * @param kind          Kind of the cursor. */
void ParsedClass::handleChild(CXCursor cursor, CXCursorKind kind) {
    switch (kind) {
        case CXCursor_CXXBaseSpecifier:
        {
            /* Check if this class is derived from Object. */
            CXType type = clang_getCursorType(cursor);
            CXString str = clang_getTypeSpelling(type);
            std::string typeName(clang_getCString(str));
            clang_disposeString(str);

            if (!typeName.compare("Object"))
                this->isObject = true;

            break;
        }

        case CXCursor_VarDecl:
        {
            /* Static class variable fall under VarDecl. The class annotation
             * is applied to the metaClass member, so if we have that variable,
             * then descend onto children keeping the same current declaration
             * so we see the annotation below. */
            CXString str = clang_getCursorSpelling(cursor);
            std::string typeName(clang_getCString(str));
            clang_disposeString(str);

            if (!typeName.compare("metaClass"))
                visitChildren(cursor, this);

            break;
        }

        case CXCursor_AnnotateAttr:
            if (handleAnnotation(cursor, "class")) {
                if (!this->isObject) {
                    parseError(
                        cursor,
                        "'class' attribute on non-Object class '%s'",
                        this->name.c_str());
                }
            }

            break;

        case CXCursor_FieldDecl:
        {
            /* FieldDecl is an instance variable. Look for properties. */
            std::unique_ptr<ParsedProperty> parsedProperty(new ParsedProperty(cursor, this));
            visitChildren(cursor, parsedProperty.get());

            if (parsedProperty->isAnnotated)
                this->properties.emplace_back(std::move(parsedProperty));

            break;
        }

        default:
            break;
    }
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedClass::dump(unsigned depth) {
    printf("%-*sClass '%s'\n", depth * 2, "", this->name.c_str());

    for (const std::unique_ptr<ParsedProperty> &parsedProperty : this->properties)
        parsedProperty->dump(depth + 1);
}

/** Initialise the translation unit.
 * @param cursor        Cursor to initialise from. */
ParsedTranslationUnit::ParsedTranslationUnit(CXCursor cursor) :
    ParsedDecl(cursor, nullptr)
{}

/** Called when a child is reached on this declaration.
 * @param cursor        Child cursor.
 * @param kind          Kind of the cursor. */
void ParsedTranslationUnit::handleChild(CXCursor cursor, CXCursorKind kind) {
    switch (kind) {
        case CXCursor_ClassDecl:
        case CXCursor_StructDecl:
            /* Ignore class declarations outside the main source file. */
            if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor))) {
                std::unique_ptr<ParsedClass> parsedClass(new ParsedClass(cursor, this));
                visitChildren(cursor, parsedClass.get());

                if (parsedClass->isValid())
                    this->classes.emplace_back(std::move(parsedClass));
            }

            break;

        default:
            break;
    }
}

/** Dump this declaration.
 * @param depth         Indentation depth. */
void ParsedTranslationUnit::dump(unsigned depth) {
    printf("%-*sTranslationUnit '%s'\n", depth * 2, "", this->name.c_str());

    for (const std::unique_ptr<ParsedClass> &parsedClass : this->classes)
        parsedClass->dump(depth + 1);
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
}

/** Main function of the object compiler.
 * @param argc          Argument count.
 * @param argv          Argument array.
 * @return              EXIT_SUCCESS or EXIT_FAILURE. */
int main(int argc, char **argv) {
    std::vector<const char *> clangArgs;
    bool dump = false;

    /* Parse arguments. */
    int opt;
    while ((opt = getopt(argc, argv, "hdD:I:")) != -1) {
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

    /* Source code is C++11, and define a macro to indicate we are the object
     * compiler. */
    clangArgs.push_back("-x");
    clangArgs.push_back("c++");
    clangArgs.push_back("-std=c++14");
    clangArgs.push_back("-DORION_OBJGEN=1");

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

    if (hadError)
        return EXIT_FAILURE;

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

    return EXIT_SUCCESS;
}