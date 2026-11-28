#include "vex.h"
#include "web.h"

#include <string.h>

;;DEFINITION
/**
 * ============================================================================
 * DEFINITION: WebTest (graphvex/web/test/web_test.c)
 * ============================================================================
 * Headless automated test suite for graphvex/web V1. Verifies Web_arena
 * lifecycle, HTML5 tokenization and DOM tree construction, and JSHeap GC
 * tracing over JSCell-embedded WebElement nodes.
 * ============================================================================
 */

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: WebTest (test/web_test.c)
 * ============================================================================
 * Single-file automated test harness for graphvex/web.
 *
 * --FUNCTIONS----------------------------
 * private  testWebArenaLifecycle()        checks Web_arena alloc/shutdown
 * private  testHtmlDomAndGc()             parses HTML5 and runs JSHeap_collect
 * public   main()                         runs all test cases
 * ============================================================================
 */

;;TEST
/**
 * Verifies Web_arena initialization and shutdown through re's MemoryPool.
 * @return true on clean allocation and teardown
 */
static bool testWebArenaLifecycle()
{
    MemoryArena *arena = Web_arena();
    if(!arena)
        return false;

    any block = MemoryArena_alloc(arena, 1u, 64);
    if(!block)
        return false;

    MemoryArena_free(arena, block);
    Web_arenaShutdown();

    return true;
}

;;TEST
/**
 * Parses a standard HTML5 document into a WebDocument, verifies the DOM tree
 * structure and text payloads, allocates an unreachable orphan WebElement, and
 * confirms JSHeap_collect sweeps only the orphan while keeping the rooted DOM.
 * @return true when DOM structure and GC invariants hold
 */
static bool testHtmlDomAndGc()
{
    WebDocument doc = WebDocument();
    if(!WebDocument_root(&doc))
        return false;

    constexpr utf8 html[] =
        "<!doctype html>"
        "<html>"
        "  <head><title>Elrio</title></head>"
        "  <body>"
        "    <h1>Hello Elrio</h1>"
        "    <p>True HTML5 and CSS.</p>"
        "  </body>"
        "</html>";

    if(!HTMLParser_parse(&doc, html, (bytes) (sizeof(html) - 1u)))
    {
        WebDocument_destroy(&doc);
        return false;
    }

    WebElement *root = WebDocument_root(&doc);
    WebElement *h1 = WebElement_findTag(root, WEB_TAG_H1);
    WebElement *p = WebElement_findTag(root, WEB_TAG_P);
    if(!h1 || !p)
    {
        WebDocument_destroy(&doc);
        return false;
    }

    WebElement *h1Text = WebElement_firstChild(h1);
    WebElement *pText = WebElement_firstChild(p);
    if(!h1Text || !pText)
    {
        WebDocument_destroy(&doc);
        return false;
    }

    constexpr utf8 expH1[] = "Hello Elrio";
    constexpr utf8 expP[] = "True HTML5 and CSS.";
    if(WebElement_textLength(h1Text) != (bytes) (sizeof(expH1) - 1u)
        || memcmp(WebElement_textData(h1Text), expH1, sizeof(expH1) - 1u) != 0)
    {
        WebDocument_destroy(&doc);
        return false;
    }

    if(WebElement_textLength(pText) != (bytes) (sizeof(expP) - 1u)
        || memcmp(WebElement_textData(pText), expP, sizeof(expP) - 1u) != 0)
    {
        WebDocument_destroy(&doc);
        return false;
    }

    WebElement *orphan = WebDocument_newElement(&doc, WEB_TAG_DIV);
    if(!orphan)
    {
        WebDocument_destroy(&doc);
        return false;
    }

    u32 swept = JSHeap_collect(WebDocument_heap(&doc));
    bool ok = (swept == 1u) && (WebElement_findTag(root, WEB_TAG_H1) == h1);

    WebDocument_destroy(&doc);
    return ok;
}

/**
 * Runs all graphvex/web V1 automated tests.
 * @return 0 on pass, non-zero on failure
 */
i32 main()
{
    if(!testWebArenaLifecycle())
    {
        fprint(stderr, "web_test: testWebArenaLifecycle failed");
        return 1;
    }

    if(!testHtmlDomAndGc())
    {
        fprint(stderr, "web_test: testHtmlDomAndGc failed");
        return 2;
    }

    print("web_test: PASS (Web_arena, HTML5 parser, WebElement DOM, JSCell GC)");
    return 0;
}
