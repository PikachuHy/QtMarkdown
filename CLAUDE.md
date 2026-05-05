# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (adjust CMAKE_PREFIX_PATH to your Qt installation)
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-qt> -DQT=ON

# Build
cmake --build build

# Build with tests enabled
cmake -B build -DCMAKE_PREFIX_PATH=<path-to-qt> -DQT=ON -DBUILD_TEST=ON
cmake --build build

# Run all tests
ctest --test-dir build

# Run a single test
ctest --test-dir build -R test_parser
# Or run the test binary directly:
./build/test/test_parser
# Editor tests need Qt offscreen:
QT_QPA_PLATFORM=offscreen ./build/test/test_editor
```

Tests use the [doctest](https://github.com/doctest/doctest) framework. Some test files define their own `main` via `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`; others use `DOCTEST_CONFIG_IMPLEMENT` with a manual `doctest::Context`. Each test binary is self-contained. Test files use `#define private public` / `#define protected public` to access internal members directly.

## Architecture

QtMarkdown has four layers with a strict dependency chain: **Parser → Render → EditorCore → Platform Shells**.

```
QtMarkdownParser  (zero Qt dependency, pure C++26)
    ↑
QtMarkdownRender  (depends on Qt::Gui, MicroTeX)
    ↑
QtMarkdownEditorCore  (platform-agnostic editing logic)
    ↑
QtMarkdownPlatform → QtWidgetMarkdownEditor / QtQuickMarkdownEditor
```

### 1. Parser (`src/parser/` → `QtMarkdownParser`)

Parses Markdown text into an AST using a two-pass approach:

- **Tokenization**: `parseLine()` in `Parser.cpp` splits a line into `Token` objects (type + offset/length into the original text). Special characters `#`, `*`, `~`, `` ` ``, `$`, `[]`, `()`, `!`, `>` become typed tokens; everything else is `text`.
- **Block parsing**: Block parsers are `std::function<ParseResult(const LineList&, int)>` registered in a static vector in `ParserPrivate::parse()`, tried in order until one succeeds. `parseParagraph` is last as the fallback. Each block parser internally registers its own static vector of `LineParserFn` for inline parsing. `ParseResult` = `{ bool success; int offset; unique_ptr<Node> node }` — if `success` is true, the parser consumed `offset` lines and produced an AST `node`; if false, the next parser is tried.
- **Inline parsing**: `LineParserFn` = `std::function<ParseResult(const TokenList&, int)>`. Inline parsers live in `src/parser/parsers/` (`ImageParser`, `LinkParser`, `InlineCodeParser`, `InlineLatexParser`, `SemanticTextParser`).
- **AST nodes**: `Node` base class with `NodeType` enum (in `Node.h`). `Container` nodes hold children via `NodePtrList` = `std::vector<std::unique_ptr<Node>>`. Leaf nodes: `Text`, `Image`, `Link`, `InlineCode`, etc., in `src/parser/nodes/`. The classic **Visitor pattern** is used: `NodeVisitor` (in `Visitor.h`) declares a `visit()` overload for every concrete node type; each node implements `accept(NodeVisitor* v) { v->visit(this); }`.
- **List nodes**: `ListNode` and `ListItemNode` (in `nodes/ListNode.h`) extend `Container`; concrete list classes inline `accept()` to avoid diamond inheritance.
- **PieceTable** (`PieceTable.h`): `PieceTableItem` struct backs `Text` nodes for incremental edits. Each `Text` holds a list of `PieceTableItem` entries referencing spans (offset+length) in either `Document::m_originalBuffer` or `Document::m_addBuffer`. When text is inserted, new content goes into the add buffer and a new `PieceTableItem` points to it — the original text is never modified.
- **Node virtual methods**: `Node` declares three virtual methods used by the editor for cursor-to-markdown-position mapping:
  - `contentLength(doc)` — rendered content length (excludes markup delimiters)
  - `serializedLength(doc)` — markdown length (includes `**`, `*`, `#` prefixes, etc.)
  - `calcMarkdownOffset(doc, contentPos, mdPos)` — maps a content position to a markdown position
  - `clone()` — deep copy, used for snapshot-based undo
  Each of the 22+ concrete node types overrides these as needed. Types without editable content (Hr, Lf, Table) use the default `return 0` / `return false` implementations.
- **ParseContext** (`ParseContext.h`): `thread_local ParseContext` struct tells the `Text(offset, length)` constructor which PieceTable buffer to reference (`original` or `add`). Set by `ParserPrivate::parse()` via RAII `ParseContextGuard`. This lets the editor re-parse add-buffer content without modifying every inline parser.

Key types from `mddef.h`: `String` = `md::String` (custom UTF-8 wrapper around `std::string`, defined in `MdString.h`), `SizeType` = `int64_t`, `sptr<T>` = `std::shared_ptr<T>`, `Char` = `char`. Block/inline parser function signatures are in `ParserDetail.h`.

Each module layers additional type aliases:
- `src/render/mddef.h`: `InstructionPtr`, `InstructionPtrList`, plus re-exports from `editor/core/Types.h` (`Color`, `Point`, `Size`, `Rect`, `Font`)
- `src/editor/core/Types.h`: `Point`, `Size`, `Rect`, `Color`, `Margins`, `FontDescription`, `ImageData`
- `src/editor/core/Event.h`: `KeyEvent`, `MouseEvent`
- `src/editor/core/AbstractPainter.h`: abstract `Painter` interface
- `src/editor/core/Timer.h`: `Timer` abstraction

### 2. Render (`src/render/` → `QtMarkdownRender`)

Converts an AST node into a draw list for painting. `Render::render()` is a static method — the class is stateless, just a factory for `Block` objects.

- `RenderPrivate` implements `NodeVisitor`, producing a `Block` per AST node. Internally uses a `LayoutPass` to break blocks into lines and a `PaintPass` to generate instructions.
- **Block → LogicalLine → VisualLine → Cell**: a block contains logical lines; each logical line may wrap into multiple visual lines; each visual line holds cells (text runs, images, etc.). **Destruction order matters**: `Instruction` objects (which hold raw `Cell*` pointers) must be destroyed before `LogicalLine` objects (which own the `Cell`s via `VisualLine::vector<unique_ptr<Cell>>`).
- **Instruction** list: Each `Block` accumulates `Instruction` objects (a draw list). Concrete instructions: `TextInstruction`, `StaticTextInstruction`, `ImageInstruction`, `FillRectInstruction`, `EllipseInstruction`, `LatexInstruction`, `StaticImageInstruction`. Painting is just executing this list with a `QPainter`.
- `StringUtil::split()` segments text into Chinese/English/Emoji runs for correct line-breaking.
- LaTeX math: MicroTeX submodule (`3rd/MicroTeX`), linked via `microtex` + `microtex-qt`. `graphic_qt.h` is the Qt graphics backend.
- `RenderSetting` (defined in `Render.h`) holds all visual config: fonts, margins (`docMargin`, `codeMargin`, `listMargin`, `checkboxMargin`, `quoteMargin`), heading sizes (`headerFontSize`), `maxWidth`, `lineSpacing`, etc.

### 3. Editor (`src/editor/` → `QtMarkdownEditorCore`, `QtWidgetMarkdownEditor`, `QtQuickMarkdownEditor`)

- **`Document`**: Wraps a `parser::Document` (composition via `unique_ptr<parser::Document>`, NOT inheritance). Maintains rendered `BlockList`, owns the `CommandStack` for undo/redo, and provides cursor navigation methods. Key methods for text-based editing:
  - `serializeBlock(blockNo)` — serializes a block's AST back to markdown via `MarkdownSerializer`
  - `cursorToMarkdownPosition(coord)` — maps cursor to a position in the serialized markdown, using `Node::calcMarkdownOffset()`
  - `replaceBlocksFromText(start, end, md, offset, len)` — re-parses edited markdown and replaces the AST subtree
  - `findCursorFromContentPosition(blockNo, contentPos)` — maps back from content position to `CursorCoord` after re-parse
- **Editing flow** (text-based, NOT direct AST manipulation):
  1. Save a `clone()` snapshot of the affected AST block(s) for undo
  2. Serialize the block to markdown text via `MarkdownSerializer`
  3. Compute the markdown edit position via `cursorToMarkdownPosition()`
  4. Edit the markdown text (insert/delete characters)
  5. Re-parse with `Parser::parse(text, PieceTableItem::add, offset)`
  6. Replace the old AST subtree with `replaceBlocksFromText()`
  7. Compute the new cursor position from content position
- **`CursorCoord`**: Position model = `(blockNo, lineNo, offset)` — which block, which logical line, and character offset within that line. This is the fundamental addressing scheme for all cursor operations.
- **`Cursor`**: Holds a `CursorCoord` + screen-space `Point` position and height. `SelectionRange` stores a `caret` and `anchor` cursor pair.
- **`Editor`**: Central controller. Handles keyboard/mouse events, cursor movement, text insertion/deletion. Delegates text mutations to `Document`.
- **`Command`** pattern: `InsertTextCommand`, `RemoveTextCommand`, `InsertReturnCommand`, `RemoveTextRangeCommand`, `UpgradeToHeaderCommand` for undo/redo. `CommandStack` uses a `vector<unique_ptr<Command>>` + `int m_top` index (single list with position marker, not two separate stacks). Each command saves a `clone()` snapshot of affected AST blocks before executing; undo restores the snapshot. No more per-command undo state tracking (text_delete, block_merge, header_degrade, etc.).
- **`MarkdownSerializer`**: Implements `NodeVisitor` to serialize an AST subtree back to markdown text. Used by the editing flow to convert AST → text before applying edits.
- Three library tiers: `QtMarkdownEditorCore` (platform-agnostic logic), `QtWidgetMarkdownEditor` (QWidget), `QtQuickMarkdownEditor` (QML).

### 4. Platform (`src/platform/qt/` → `QtMarkdownPlatform`)

Thin Qt adapter layer implementing the abstract interfaces from `editor/core/`:
- `QtAdapters.h`, `QtFontMetricsProvider.h`, `QtImageProvider.h`, `QtLatexPlatform.h`
- `QtWidgetMarkdownEditor` and `QtQuickMarkdownEditor` — platform-specific editor shells
- The core parser/render/editor layers have zero Qt dependency; all Qt types are contained here

### Third-party dependencies (`3rd/`)

Git submodules: `MicroTeX` (LaTeX math rendering), `magic_enum` (enum reflection utility).

### Examples (`example/`)

`QtWidgetMarkdownEditorExample`, `QtQuickMarkdownEditorExample`, and `QtMarkdownParserExample` (parser-only demo).

## Key Conventions

- The `DEBUG` macro (from `debug.h`) outputs `[debug]` prefixed messages with function name, file, and line number. Use it instead of raw `qDebug()` for diagnostic output.
- AST nodes are owned by parent `Container` via `std::unique_ptr` (using the `NodePtrList` alias in `Node.h`). Use `std::make_unique` for allocation. `Text*` pointers in semantic nodes (`ItalicText`, `BoldText`, etc.) are non-owning.
- The `ASSERT()` macro (from `debug.h`) calls `Backtrace::backtrace()` before `qt_assert` in debug builds.
- `BUILD_STATIC` CMake option controls static vs shared library builds.
- The `.clang-format` file at the repo root defines code style.
- CI uses the `cmake-build-*` pattern in `.gitignore`; `build` is also gitignored.
- Export macro `QTMARKDOWNSHARED_EXPORT` is defined in `src/QtMarkdown_global.h` — used on all public classes.
