#ifndef GRAPHVEX_WEB_HTML_H
#define GRAPHVEX_WEB_HTML_H

#include "vex.h"
#include "dom/dom.h"

/**
 * Token kinds emitted by the HTML5 forward tokenizer.
 */
typedef enum HTMLTokenKind {
    HTML_TOK_EOF = 0,
    HTML_TOK_DOCTYPE = 1,
    HTML_TOK_START_TAG = 2,
    HTML_TOK_END_TAG = 3,
    HTML_TOK_TEXT = 4,
    HTML_TOK_COMMENT = 5,
} HTMLTokenKind;

;;CLASSNAME("HTMLToken")
/**
 * Single lexical token referencing a byte slice inside an HTML5 document.
 *
 * u32 kind         - HTMLTokenKind discriminant
 * bytes start      - byte offset of tag name or text payload
 * bytes length     - byte length of tag name or text payload
 * bool selfClosing - true when start tag ended with "/>"
 */
typedef struct HTMLToken {
    u32 kind;
    bytes start;
    bytes length;
    bool selfClosing;
} HTMLToken;

;;CLASSNAME("HTMLTokenizer")
/**
 * Zero-allocation forward HTML5 tokenizer over UTF-8 markup.
 *
 * string source - borrowed UTF-8 HTML source
 * bytes length  - total byte length of source
 * bytes cursor  - current byte offset inside source
 */
typedef struct HTMLTokenizer {
    string source;
    bytes length;
    bytes cursor;
} HTMLTokenizer;

/**
 * Returns a zeroed HTMLToken.
 * @return zeroed HTMLToken
 */
HTMLToken HTMLToken_zero();

/**
 * Returns a zeroed HTMLTokenizer.
 * @return zeroed HTMLTokenizer
 */
HTMLTokenizer HTMLTokenizer_zero();

/**
 * Constructs an HTMLTokenizer over source[0..length].
 * @param source borrowed UTF-8 HTML buffer
 * @param length byte length of source
 * @return initialized HTMLTokenizer
 */
HTMLTokenizer HTMLTokenizer_2(string source, bytes length);

#define HTMLTokenizer(...) CONSTRUCTOR_DISPATCH(HTMLTokenizer, __VA_ARGS__)

/**
 * Scans and returns the next HTMLToken.
 * @param self active HTMLTokenizer
 * @return next HTMLToken (HTML_TOK_EOF at end)
 */
HTMLToken HTMLTokenizer_next(HTMLTokenizer *self);

/**
 * Parses HTML5 markup[0..length] into doc's rooted DOM tree.
 * @param doc    initialized WebDocument
 * @param source UTF-8 HTML5 source text
 * @param length byte length of source
 * @return true on successful parse
 */
bool HTMLParser_parse(WebDocument *doc, string source, bytes length);

;;GETTER
u32 HTMLToken_kind(const HTMLToken *self);

;;GETTER
bytes HTMLToken_start(const HTMLToken *self);

;;GETTER
bytes HTMLToken_length(const HTMLToken *self);

;;GETTER
bool HTMLToken_selfClosing(const HTMLToken *self);

#endif
