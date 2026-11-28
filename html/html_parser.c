#include "vex.h"
#include "html/html.h"

#include <string.h>

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: HTMLParser (graphvex/web/html/html_parser.c)
 * ============================================================================
 * Zero-allocation HTML5 tokenizer and lightweight DOM tree builder. Scans
 * doctype, comments, start/end tags, and text runs into WebElement nodes
 * owned by a WebDocument's JSHeap realm arena.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: HTMLTokenizer / HTMLParser (html/html_parser.c)
 * ============================================================================
 * Forward HTML5 tokenizer and stack-based DOM tree builder.
 *
 * --CONSTANTS----------------------------
 * private  const u32   HTML_STACK_CAP     64 open-element stack depth
 *
 * --CONSTRUCTORS-------------------------
 * public   HTMLToken_zero()               zeroed EOF token
 * public   HTMLTokenizer_zero()           zeroed tokenizer
 * public   HTMLTokenizer_2(src, len)      initializes tokenizer at byte 0
 *
 * --FUNCTIONS----------------------------
 * public   HTMLTokenizer_next(self)       scans next doctype/tag/text/comment
 * public   HTMLParser_parse(...)          builds WebElement DOM tree in doc
 * private  isAsciiSpace(c)                checks ASCII HTML whitespace
 * private  isTagNameChar(c)               checks valid tag identifier char
 *
 * --GETTERS------------------------------
 * public   HTMLToken_kind(self)           HTMLTokenKind discriminant
 * public   HTMLToken_start(self)          payload byte offset
 * public   HTMLToken_length(self)         payload byte length
 * public   HTMLToken_selfClosing(self)    self-closing "/>" flag
 * ============================================================================
 */

static constexpr u32 HTML_STACK_CAP = 64u;

;;SETTER
static fn HTMLToken_setKind(HTMLToken *self, u32 kind)
{
    (*self).kind = kind;
}

;;SETTER
static fn HTMLToken_setStart(HTMLToken *self, bytes start)
{
    (*self).start = start;
}

;;SETTER
static fn HTMLToken_setLength(HTMLToken *self, bytes length)
{
    (*self).length = length;
}

;;SETTER
static fn HTMLToken_setSelfClosing(HTMLToken *self, bool selfClosing)
{
    (*self).selfClosing = selfClosing;
}

;;GETTER
static string HTMLTokenizer_source(const HTMLTokenizer *self)
{
    return (*self).source;
}

;;SETTER
static fn HTMLTokenizer_setSource(HTMLTokenizer *self, string source)
{
    (*self).source = source;
}

;;GETTER
static bytes HTMLTokenizer_length(const HTMLTokenizer *self)
{
    return (*self).length;
}

;;SETTER
static fn HTMLTokenizer_setLength(HTMLTokenizer *self, bytes length)
{
    (*self).length = length;
}

;;GETTER
static bytes HTMLTokenizer_cursor(const HTMLTokenizer *self)
{
    return (*self).cursor;
}

;;SETTER
static fn HTMLTokenizer_setCursor(HTMLTokenizer *self, bytes cursor)
{
    (*self).cursor = cursor;
}

;;CONSTRUCTOR
/**
 * Returns a zeroed HTMLToken.
 * @return zeroed HTMLToken
 */
HTMLToken HTMLToken_zero()
{
    HTMLToken t;
    memset(&t, 0, sizeof(t));

    return t;
}

;;CONSTRUCTOR
/**
 * Returns a zeroed HTMLTokenizer.
 * @return zeroed HTMLTokenizer
 */
HTMLTokenizer HTMLTokenizer_zero()
{
    HTMLTokenizer ht;
    memset(&ht, 0, sizeof(ht));

    return ht;
}

;;CONSTRUCTOR
/**
 * Constructs an HTMLTokenizer over source[0..length].
 * @param source borrowed UTF-8 HTML source
 * @param length byte length of source
 * @return initialized HTMLTokenizer
 */
HTMLTokenizer HTMLTokenizer_2(string source, bytes length)
{
    HTMLTokenizer ht = HTMLTokenizer_zero();
    HTMLTokenizer_setSource(&ht, source);
    HTMLTokenizer_setLength(&ht, length);
    HTMLTokenizer_setCursor(&ht, 0);

    return ht;
}

;;GETTER
/**
 * Reads the HTMLTokenKind discriminant.
 * @param self token pointer
 * @return HTMLTokenKind
 */
u32 HTMLToken_kind(const HTMLToken *self)
{
    if(!self)
        return HTML_TOK_EOF;

    return (*self).kind;
}

;;GETTER
/**
 * Reads the token start byte offset.
 * @param self token pointer
 * @return start offset
 */
bytes HTMLToken_start(const HTMLToken *self)
{
    if(!self)
        return 0;

    return (*self).start;
}

;;GETTER
/**
 * Reads the token byte length.
 * @param self token pointer
 * @return byte length
 */
bytes HTMLToken_length(const HTMLToken *self)
{
    if(!self)
        return 0;

    return (*self).length;
}

;;GETTER
/**
 * Reads whether a start tag ended with "/>".
 * @param self token pointer
 * @return true when self-closing
 */
bool HTMLToken_selfClosing(const HTMLToken *self)
{
    if(!self)
        return false;

    return (*self).selfClosing;
}

/**
 * Checks whether c is ASCII HTML whitespace.
 * @param c byte to inspect
 * @return true for space, tab, CR, or LF
 */
static bool isAsciiSpace(utf8 c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

/**
 * Checks whether c can appear in an HTML tag name.
 * @param c byte to inspect
 * @return true when alphanumeric, '-' or '_'
 */
static bool isTagNameChar(utf8 c)
{
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return true;

    if(c >= '0' && c <= '9')
        return true;

    return c == '-' || c == '_';
}

/**
 * Scans and returns the next HTMLToken from self.
 * @param self active HTMLTokenizer
 * @return scanned HTMLToken
 */
HTMLToken HTMLTokenizer_next(HTMLTokenizer *self)
{
    HTMLToken tok = HTMLToken_zero();
    if(!self || !HTMLTokenizer_source(self))
        return tok;

    string src = HTMLTokenizer_source(self);
    bytes len = HTMLTokenizer_length(self);
    bytes pos = HTMLTokenizer_cursor(self);

    if(pos >= len)
    {
        HTMLToken_setKind(&tok, HTML_TOK_EOF);
        return tok;
    }

    if(src[pos] != '<')
    {
        bytes start = pos;
        while(pos < len && src[pos] != '<')
            pos++;

        HTMLTokenizer_setCursor(self, pos);
        HTMLToken_setKind(&tok, HTML_TOK_TEXT);
        HTMLToken_setStart(&tok, start);
        HTMLToken_setLength(&tok, pos - start);
        return tok;
    }

    pos++;
    if(pos >= len)
    {
        HTMLTokenizer_setCursor(self, pos);
        HTMLToken_setKind(&tok, HTML_TOK_EOF);
        return tok;
    }

    if(src[pos] == '!')
    {
        if((pos + 2) < len && src[pos + 1] == '-' && src[pos + 2] == '-')
        {
            pos += 3;
            bytes start = pos;
            while((pos + 2) < len && !(src[pos] == '-' && src[pos + 1] == '-' && src[pos + 2] == '>'))
                pos++;

            bytes tokLen = pos - start;
            if((pos + 2) < len)
                pos += 3;
            else
                pos = len;

            HTMLTokenizer_setCursor(self, pos);
            HTMLToken_setKind(&tok, HTML_TOK_COMMENT);
            HTMLToken_setStart(&tok, start);
            HTMLToken_setLength(&tok, tokLen);
            return tok;
        }

        pos++;
        bytes start = pos;
        while(pos < len && src[pos] != '>')
            pos++;

        bytes tokLen = pos - start;
        if(pos < len)
            pos++;

        HTMLTokenizer_setCursor(self, pos);
        HTMLToken_setKind(&tok, HTML_TOK_DOCTYPE);
        HTMLToken_setStart(&tok, start);
        HTMLToken_setLength(&tok, tokLen);
        return tok;
    }

    bool isEnd = false;
    if(src[pos] == '/')
    {
        isEnd = true;
        pos++;
    }

    bytes nameStart = pos;
    while(pos < len && isTagNameChar(src[pos]))
        pos++;

    bytes nameLen = pos - nameStart;
    bool selfClosing = false;

    while(pos < len && src[pos] != '>')
    {
        if(src[pos] == '/' && (pos + 1) < len && src[pos + 1] == '>')
            selfClosing = true;

        pos++;
    }

    if(pos < len)
        pos++;

    HTMLTokenizer_setCursor(self, pos);
    HTMLToken_setKind(&tok, isEnd ? HTML_TOK_END_TAG : HTML_TOK_START_TAG);
    HTMLToken_setStart(&tok, nameStart);
    HTMLToken_setLength(&tok, nameLen);
    HTMLToken_setSelfClosing(&tok, selfClosing);

    return tok;
}

/**
 * Checks whether text[0..len] contains only ASCII whitespace.
 * @param text slice start
 * @param len  slice length
 * @return true when all bytes are whitespace
 */
static bool isAllWhitespace(string text, bytes len)
{
    for(bytes i = 0; i < len; i++)
    {
        if(!isAsciiSpace(text[i]))
            return false;
    }

    return true;
}

/**
 * Parses HTML5 markup[0..length] into doc's rooted WebElement DOM tree.
 * @param doc    initialized WebDocument
 * @param source UTF-8 HTML5 source text
 * @param length byte length of source
 * @return true when parsing completed
 */
bool HTMLParser_parse(WebDocument *doc, string source, bytes length)
{
    if(!doc || !WebDocument_root(doc) || !source)
        return false;

    WebElement *stack[HTML_STACK_CAP];
    u32 depth = 1u;
    stack[0] = WebDocument_root(doc);

    HTMLTokenizer tok = HTMLTokenizer(source, length);

    while(true)
    {
        HTMLToken t = HTMLTokenizer_next(&tok);
        u32 kind = HTMLToken_kind(&t);

        if(kind == HTML_TOK_EOF)
            break;

        if(kind == HTML_TOK_DOCTYPE || kind == HTML_TOK_COMMENT)
            continue;

        if(kind == HTML_TOK_TEXT)
        {
            bytes tStart = HTMLToken_start(&t);
            bytes tLen = HTMLToken_length(&t);
            if(isAllWhitespace(source + tStart, tLen))
                continue;

            WebElement *textNode = WebDocument_newText(doc, source + tStart, tLen);
            if(!textNode)
                return false;

            if(!WebElement_appendChild(stack[depth - 1u], textNode))
                return false;

            continue;
        }

        if(kind == HTML_TOK_START_TAG)
        {
            u32 tag = WebTag_fromName(source + HTMLToken_start(&t), HTMLToken_length(&t));
            WebElement *el = WebDocument_newElement(doc, tag);
            if(!el)
                return false;

            if(!WebElement_appendChild(stack[depth - 1u], el))
                return false;

            if(!HTMLToken_selfClosing(&t) && depth < HTML_STACK_CAP)
            {
                stack[depth] = el;
                depth++;
            }

            continue;
        }

        if(kind == HTML_TOK_END_TAG)
        {
            u32 tag = WebTag_fromName(source + HTMLToken_start(&t), HTMLToken_length(&t));
            for(u32 i = depth; i > 1u; i--)
            {
                if(WebElement_tag(stack[i - 1u]) == tag)
                {
                    depth = i - 1u;
                    break;
                }
            }
        }
    }

    return true;
}
