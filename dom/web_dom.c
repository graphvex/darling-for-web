#include "vex.h"
#include "dom/dom.h"

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: WebDOM (graphvex/web/dom/web_dom.c)
 * ============================================================================
 * Data-driven DOM nodes (WebElement) and per-tab WebDocument container.
 * Every WebElement embeds JSCell as its first field inside the document's
 * JSHeap realm MemoryArena so DOM-to-JS and JS-to-DOM cycles are collected
 * cleanly and closing a tab tears down the entire DOM in O(1).
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: WebElement / WebDocument (dom/web_dom.c)
 * ============================================================================
 * GC-traced HTML5 DOM tree backed by vexjs JSHeap.
 *
 * --CONSTANTS----------------------------
 * private  const u64 RE_TYPE_WEB_ELEMENT  501u Relational Engine type id
 * private  const u64 RE_TYPE_WEB_TEXT     502u Relational Engine type id
 *
 * --CONSTRUCTORS-------------------------
 * public   WebDocument_zero()             zeroed document
 * public   WebDocument_0()                unbounded realm document
 * public   WebDocument_1(maxPages)        page-budgeted realm document
 *
 * --FUNCTIONS----------------------------
 * public   WebDocument_destroy(self)      O(1) realm arena teardown
 * public   WebDocument_newElement(...)    allocates WebElement in JSHeap
 * public   WebDocument_newText(...)       allocates WEB_TAG_TEXT node
 * public   WebElement_appendChild(...)    links child into parent list
 * public   WebElement_findTag(...)        depth-first tag lookup
 * public   WebTag_fromName(...)           case-insensitive tag classifier
 * private  visitWebElement(cell, heap)    GC trace callback for children
 *
 * --GETTERS------------------------------
 * public   WebDocument_root(self)         document root element
 * public   WebDocument_nodeCount(self)    allocated node count
 * public   WebDocument_heap(self)         underlying JSHeap pointer
 * public   WebElement_tag(self)           WebTag enum
 * public   WebElement_parent(self)        parent element
 * public   WebElement_firstChild(self)    first child element
 * public   WebElement_nextSibling(self)   next sibling element
 * public   WebElement_textData(self)      UTF-8 text payload
 * public   WebElement_textLength(self)    UTF-8 text byte length
 * ============================================================================
 */

static constexpr u64 RE_TYPE_WEB_ELEMENT = 501u;
static constexpr u64 RE_TYPE_WEB_TEXT = 502u;

;;SETTER
static fn WebElement_setTag(WebElement *self, u32 tag)
{
    (*self).tag = tag;
}

;;SETTER
static fn WebElement_setParent(WebElement *self, WebElement *parent)
{
    (*self).parent = parent;
}

;;SETTER
static fn WebElement_setFirstChild(WebElement *self, WebElement *child)
{
    (*self).firstChild = child;
}

;;SETTER
static fn WebElement_setNextSibling(WebElement *self, WebElement *sibling)
{
    (*self).nextSibling = sibling;
}

;;SETTER
static fn WebElement_setTextData(WebElement *self, string text)
{
    (*self).textData = text;
}

;;SETTER
static fn WebElement_setTextLength(WebElement *self, bytes len)
{
    (*self).textLength = len;
}

;;SETTER
static fn WebDocument_setHeap(WebDocument *self, JSHeap heap)
{
    (*self).heap = heap;
}

;;SETTER
static fn WebDocument_setRoot(WebDocument *self, WebElement *root)
{
    (*self).root = root;
}

;;SETTER
static fn WebDocument_setNodeCount(WebDocument *self, u32 count)
{
    (*self).nodeCount = count;
}

/**
 * Traces outgoing child and sibling edges on a WebElement during JSHeap mark.
 * @param cell WebElement cast as JSCell
 * @param heap active JSHeap
 */
static fn visitWebElement(JSCell *cell, JSHeap *heap)
{
    WebElement *el = (WebElement*) cell;
    WebElement *child = WebElement_firstChild(el);
    while(child)
    {
        JSHeap_markCell(heap, (JSCell*) child);
        child = WebElement_nextSibling(child);
    }
}

;;CONSTRUCTOR
/**
 * Returns a zeroed WebDocument.
 * @return zeroed WebDocument
 */
WebDocument WebDocument_zero()
{
    WebDocument doc;
    memset(&doc, 0, sizeof(doc));

    return doc;
}

;;CONSTRUCTOR
/**
 * Constructs a WebDocument with a page-budgeted realm JSHeap and rooted
 * WEB_TAG_DOCUMENT root element.
 * @param maxPages hard page ceiling (0 for unbounded)
 * @return initialized WebDocument
 */
WebDocument WebDocument_1(bytes maxPages)
{
    WebDocument doc = WebDocument_zero();
    JSHeap heap = JSHeap(maxPages);
    if(!JSHeap_arena(&heap))
        return doc;

    WebDocument_setHeap(&doc, heap);
    WebDocument_setNodeCount(&doc, 0u);

    WebElement *root = WebDocument_newElement(&doc, WEB_TAG_DOCUMENT);
    if(!root || !JSHeap_pushRoot(WebDocument_heap(&doc), (JSCell*) root))
    {
        JSHeap_destroy(WebDocument_heap(&doc));
        return WebDocument_zero();
    }

    WebDocument_setRoot(&doc, root);
    return doc;
}

;;CONSTRUCTOR
/**
 * Constructs an unbounded WebDocument.
 * @return initialized WebDocument
 */
WebDocument WebDocument_0()
{
    return WebDocument_1(0);
}

;;DECONSTRUCT
/**
 * Destroys the document's realm JSHeap in O(1) and zeroes the WebDocument.
 * @param self document pointer
 */
fn WebDocument_destroy(WebDocument *self)
{
    if(!self)
        return;

    JSHeap_destroy(WebDocument_heap(self));
    memset(self, 0, sizeof(WebDocument));
}

;;GETTER
/**
 * Reads the rooted WEB_TAG_DOCUMENT root element.
 * @param self document pointer
 * @return root WebElement
 */
WebElement *WebDocument_root(const WebDocument *self)
{
    if(!self)
        return nullptr;

    return (*self).root;
}

;;GETTER
/**
 * Reads the total allocated WebElement count in this document.
 * @param self document pointer
 * @return nodeCount
 */
u32 WebDocument_nodeCount(const WebDocument *self)
{
    if(!self)
        return 0u;

    return (*self).nodeCount;
}

;;GETTER
/**
 * Reads the mutable JSHeap pointer owned by this document.
 * @param self document pointer
 * @return JSHeap pointer
 */
JSHeap *WebDocument_heap(WebDocument *self)
{
    if(!self)
        return nullptr;

    return &(*self).heap;
}

/**
 * Allocates a zeroed WebElement inside doc's realm JSHeap.
 * @param doc owning WebDocument
 * @param tag WebTag classification
 * @return allocated WebElement or nullptr
 */
WebElement *WebDocument_newElement(WebDocument *doc, u32 tag)
{
    if(!doc)
        return nullptr;

    JSCell *cell = JSHeap_allocCell(WebDocument_heap(doc), JS_CELL_DOM_NODE, RE_TYPE_WEB_ELEMENT, sizeof(WebElement), visitWebElement);
    if(!cell)
        return nullptr;

    WebElement *el = (WebElement*) cell;
    WebElement_setTag(el, tag);
    WebDocument_setNodeCount(doc, WebDocument_nodeCount(doc) + 1u);

    return el;
}

/**
 * Allocates a WEB_TAG_TEXT node copying text[0..len] into the realm arena.
 * @param doc  owning WebDocument
 * @param text UTF-8 text slice
 * @param len  byte length
 * @return allocated text WebElement or nullptr
 */
WebElement *WebDocument_newText(WebDocument *doc, string text, bytes len)
{
    if(!doc || !text || len < 0)
        return nullptr;

    WebElement *el = WebDocument_newElement(doc, WEB_TAG_TEXT);
    if(!el)
        return nullptr;

    MemoryArena *arena = JSHeap_arena(WebDocument_heap(doc));
    utf8 *copy = (utf8*) MemoryArena_alloc(arena, RE_TYPE_WEB_TEXT, len + 1);
    if(!copy)
        return nullptr;

    if(len > 0)
        memcpy(copy, text, (size_t) len);

    copy[len] = '\0';
    WebElement_setTextData(el, copy);
    WebElement_setTextLength(el, len);

    return el;
}

/**
 * Appends child to the end of parent's child linked list.
 * @param parent parent element
 * @param child  child element
 * @return true on success
 */
bool WebElement_appendChild(WebElement *parent, WebElement *child)
{
    if(!parent || !child || parent == child)
        return false;

    WebElement_setParent(child, parent);
    WebElement_setNextSibling(child, nullptr);

    WebElement *first = WebElement_firstChild(parent);
    if(!first)
    {
        WebElement_setFirstChild(parent, child);
        return true;
    }

    WebElement *tail = first;
    while(WebElement_nextSibling(tail))
        tail = WebElement_nextSibling(tail);

    WebElement_setNextSibling(tail, child);
    return true;
}

/**
 * Searches root and its descendants depth-first for the first element matching tag.
 * @param root subtree root
 * @param tag  WebTag enum value
 * @return matching WebElement or nullptr
 */
WebElement *WebElement_findTag(WebElement *root, u32 tag)
{
    if(!root)
        return nullptr;

    if(WebElement_tag(root) == tag)
        return root;

    WebElement *child = WebElement_firstChild(root);
    while(child)
    {
        WebElement *hit = WebElement_findTag(child, tag);
        if(hit)
            return hit;

        child = WebElement_nextSibling(child);
    }

    return nullptr;
}

/**
 * Case-insensitive ASCII comparison between a slice and a literal string.
 * @param slice  source slice
 * @param len    slice length
 * @param target lowercase literal target
 * @param tLen   target length
 * @return true when equal ignoring ASCII case
 */
static bool asciiEqualsIgnoreCase(string slice, bytes len, string target, bytes tLen)
{
    if(len != tLen)
        return false;

    for(bytes i = 0; i < len; i++)
    {
        utf8 a = slice[i];
        if(a >= 'A' && a <= 'Z')
            a = (utf8) (a + ('a' - 'A'));

        if(a != target[i])
            return false;
    }

    return true;
}

/**
 * Maps an HTML tag name slice to its WebTag enum value.
 * @param name   UTF-8 tag name slice
 * @param length byte length
 * @return WebTag enum value
 */
u32 WebTag_fromName(string name, bytes length)
{
    if(!name || length <= 0)
        return WEB_TAG_UNKNOWN;

    if(asciiEqualsIgnoreCase(name, length, "html", 4))
        return WEB_TAG_HTML;

    if(asciiEqualsIgnoreCase(name, length, "head", 4))
        return WEB_TAG_HEAD;

    if(asciiEqualsIgnoreCase(name, length, "title", 5))
        return WEB_TAG_TITLE;

    if(asciiEqualsIgnoreCase(name, length, "body", 4))
        return WEB_TAG_BODY;

    if(asciiEqualsIgnoreCase(name, length, "div", 3))
        return WEB_TAG_DIV;

    if(asciiEqualsIgnoreCase(name, length, "span", 4))
        return WEB_TAG_SPAN;

    if(asciiEqualsIgnoreCase(name, length, "p", 1))
        return WEB_TAG_P;

    if(asciiEqualsIgnoreCase(name, length, "h1", 2))
        return WEB_TAG_H1;

    if(asciiEqualsIgnoreCase(name, length, "h2", 2))
        return WEB_TAG_H2;

    if(asciiEqualsIgnoreCase(name, length, "h3", 2))
        return WEB_TAG_H3;

    if(asciiEqualsIgnoreCase(name, length, "a", 1))
        return WEB_TAG_A;

    if(asciiEqualsIgnoreCase(name, length, "style", 5))
        return WEB_TAG_STYLE;

    if(asciiEqualsIgnoreCase(name, length, "script", 6))
        return WEB_TAG_SCRIPT;

    return WEB_TAG_UNKNOWN;
}

;;GETTER
/**
 * Reads the WebTag enum of an element.
 * @param self element pointer
 * @return WebTag value
 */
u32 WebElement_tag(const WebElement *self)
{
    if(!self)
        return WEB_TAG_UNKNOWN;

    return (*self).tag;
}

;;GETTER
/**
 * Reads the parent element pointer.
 * @param self element pointer
 * @return parent WebElement or nullptr
 */
WebElement *WebElement_parent(const WebElement *self)
{
    if(!self)
        return nullptr;

    return (*self).parent;
}

;;GETTER
/**
 * Reads the first child element pointer.
 * @param self element pointer
 * @return firstChild WebElement or nullptr
 */
WebElement *WebElement_firstChild(const WebElement *self)
{
    if(!self)
        return nullptr;

    return (*self).firstChild;
}

;;GETTER
/**
 * Reads the next sibling element pointer.
 * @param self element pointer
 * @return nextSibling WebElement or nullptr
 */
WebElement *WebElement_nextSibling(const WebElement *self)
{
    if(!self)
        return nullptr;

    return (*self).nextSibling;
}

;;GETTER
/**
 * Reads the UTF-8 text payload of a WEB_TAG_TEXT node.
 * @param self element pointer
 * @return UTF-8 string or nullptr
 */
string WebElement_textData(const WebElement *self)
{
    if(!self)
        return nullptr;

    return (*self).textData;
}

;;GETTER
/**
 * Reads the byte length of textData.
 * @param self element pointer
 * @return byte length
 */
bytes WebElement_textLength(const WebElement *self)
{
    if(!self)
        return 0;

    return (*self).textLength;
}
