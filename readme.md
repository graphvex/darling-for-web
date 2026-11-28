# graphvex/web (darling-for-web)

Standard HTML5, CSS, and DOM engine for `elrio`, layered cleanly inside `graphvex` on top of `re` and `vexjs`.

## Ownership & Layering

- **`nio/`** -- single per-repo `MemoryArena` (`Web_arena()`) drawn from `re`.
- **`dom/`** -- data-driven `WebElement` (`WebTag` enum) embedding `JSCell` as its first field so `vexjs` `JSHeap` collects JS-to-DOM reference cycles in one pass.
- **`html/`** -- zero-copy HTML5 tokenizer and tree builder producing `WebDocument` trees inside the realm `JSHeap`.
- **`test/`** -- automated headless verification (`web_test.c`) for HTML5 tokenization, DOM tree building, and cyclic `JSCell` GC over `WebElement` nodes.

## Next Milestones

1. **CSS parser & cascade** (`css/`): rule/selector parsing, specificity, and computed styles (`display`, `color`, `padding`, `margin`).
2. **Layout box tree** (`layout/`): Block, Inline (using `graphvex` shared text shaping), and Flex formatting contexts emitting `collage` `ElementNode`s.
3. **DOM JS bindings**: expose `document.getElementById`, `textContent`, and `addEventListener` to `vexjs` inside `sandcastles` `WEB_CONTENT` workers.
