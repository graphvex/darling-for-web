#ifndef GRAPHVEX_WEB_DOM_H
#define GRAPHVEX_WEB_DOM_H

#include "vex.h"
#include "vexjs.h"

/**
 * Built-in HTML5 element tag identifiers for data-driven WebElement nodes.
 */
typedef enum WebTag {
    WEB_TAG_UNKNOWN = 0,
    WEB_TAG_TEXT = 1,
    WEB_TAG_DOCUMENT = 2,
    WEB_TAG_HTML = 3,
    WEB_TAG_HEAD = 4,
    WEB_TAG_TITLE = 5,
    WEB_TAG_BODY = 6,
    WEB_TAG_DIV = 7,
    WEB_TAG_SPAN = 8,
    WEB_TAG_P = 9,
    WEB_TAG_H1 = 10,
    WEB_TAG_H2 = 11,
    WEB_TAG_H3 = 12,
    WEB_TAG_A = 13,
    WEB_TAG_STYLE = 14,
    WEB_TAG_SCRIPT = 15,
} WebTag;

typedef struct WebElement WebElement;
typedef struct WebDocument WebDocument;

;;CLASSNAME("WebElement")
/**
 * Data-driven HTML5 DOM node embedding JSCell as its first field so vexjs
 * JSHeap traces JS-to-DOM cycles directly inside one realm MemoryArena.
 *
 * JSCell header           - intrusive GC header (must be first field)
 * u32 tag                 - WebTag classification
 * WebElement *parent      - borrowed parent element
 * WebElement *firstChild  - first child element in tree order
 * WebElement *nextSibling - next sibling element in tree order
 * string textData         - arena-owned text slice (for WEB_TAG_TEXT)
 * bytes textLength        - byte length of textData
 * string idAttr           - arena-owned id attribute (optional)
 * bytes idLength          - byte length of idAttr
 */
struct WebElement {
    JSCell header;
    u32 tag;
    WebElement *parent;
    WebElement *firstChild;
    WebElement *nextSibling;
    string textData;
    bytes textLength;
    string idAttr;
    bytes idLength;
};

;;CLASSNAME("WebDocument")
/**
 * Per-tab DOM document owning a vexjs JSHeap realm arena and a rooted root
 * WebElement tree. Destroying the document frees the entire realm in O(1).
 *
 * JSHeap heap      - owned per-realm JSHeap (MemoryArena + GC)
 * WebElement *root - rooted WEB_TAG_DOCUMENT root node
 * u32 nodeCount    - total allocated WebElement count
 */
struct WebDocument {
    JSHeap heap;
    WebElement *root;
    u32 nodeCount;
};

/**
 * Returns a zeroed WebDocument.
 * @return zeroed WebDocument
 */
WebDocument WebDocument_zero();

/**
 * Constructs a WebDocument with an unbounded per-realm JSHeap.
 * @return initialized WebDocument
 */
WebDocument WebDocument_0();

/**
 * Constructs a WebDocument with a page-budgeted per-realm JSHeap.
 * @param maxPages hard page ceiling (0 for unbounded)
 * @return initialized WebDocument
 */
WebDocument WebDocument_1(bytes maxPages);

#define WebDocument(...) CONSTRUCTOR_DISPATCH(WebDocument, __VA_ARGS__)

;;DECONSTRUCT
/**
 * Destroys the document's realm JSHeap in O(1) and zeroes the struct.
 * @param self document pointer
 */
fn WebDocument_destroy(WebDocument *self);

;;GETTER
WebElement *WebDocument_root(const WebDocument *self);

;;GETTER
u32 WebDocument_nodeCount(const WebDocument *self);

;;GETTER
JSHeap *WebDocument_heap(WebDocument *self);

/**
 * Allocates a new WebElement inside doc's realm JSHeap.
 * @param doc owning WebDocument
 * @param tag WebTag classification
 * @return allocated WebElement, or nullptr on budget failure
 */
WebElement *WebDocument_newElement(WebDocument *doc, u32 tag);

/**
 * Allocates a WEB_TAG_TEXT node copying text[0..len] into doc's realm arena.
 * @param doc  owning WebDocument
 * @param text UTF-8 text slice
 * @param len  byte length of text
 * @return allocated text WebElement, or nullptr on failure
 */
WebElement *WebDocument_newText(WebDocument *doc, string text, bytes len);

/**
 * Appends child to parent's child list and updates parent pointer.
 * @param parent parent WebElement
 * @param child  child WebElement
 * @return true on success
 */
bool WebElement_appendChild(WebElement *parent, WebElement *child);

/**
 * Finds the first descendant element matching tag (depth-first).
 * @param root subtree root
 * @param tag  WebTag to search for
 * @return matching WebElement, or nullptr if not found
 */
WebElement *WebElement_findTag(WebElement *root, u32 tag);

/**
 * Maps an ASCII tag name slice (case-insensitive) to a WebTag enum.
 * @param name   tag name slice
 * @param length byte length of name
 * @return WebTag value (WEB_TAG_UNKNOWN if unrecognized)
 */
u32 WebTag_fromName(string name, bytes length);

;;GETTER
u32 WebElement_tag(const WebElement *self);

;;GETTER
WebElement *WebElement_parent(const WebElement *self);

;;GETTER
WebElement *WebElement_firstChild(const WebElement *self);

;;GETTER
WebElement *WebElement_nextSibling(const WebElement *self);

;;GETTER
string WebElement_textData(const WebElement *self);

;;GETTER
bytes WebElement_textLength(const WebElement *self);

#endif
