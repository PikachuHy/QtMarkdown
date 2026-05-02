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
```

Tests use the [doctest](https://github.com/doctest/doctest) framework. Some test files define their own `main` via `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`; others use `DOCTEST_CONFIG_IMPLEMENT` with a manual `doctest::Context`. Each test binary is self-contained. Test files use `#define private public` / `#define protected public` to access internal members directly.

## Architecture

QtMarkdown has three core library layers with a strict dependency chain: **Parser → Render → Editor**.

```
QtMarkdownParser  (no Qt GUI dependency)
    ↑
QtMarkdownRender  (depends on Qt::Gui, MicroTeX)
    ↑
QtMarkdownEditorCore  (platform-agnostic editing logic)
    ↑
QtWidgetMarkdownEditor / QtQuickMarkdownEditor  (platform-specific shells)
```

### 1. Parser (`src/parser/` → `QtMarkdownParser`)

Parses Markdown text into an AST using a two-pass approach:

- **Tokenization**: `parseLine()` in `Parser.cpp` splits a line into `Token` objects (type + offset/length into the original text). Special characters `#`, `*`, `~`, `` ` ``, `$`, `[]`, `()`, `!`, `>` become typed tokens; everything else is `text`.
- **Block parsing**: Block parsers are `std::function<ParseResult(const LineList&, int)>` registered in a static vector in `Parser::parse()`, tried in order until one succeeds. `ParagraphParser` is last as the fallback. Each block parser internally registers its own static vector of `LineParserFn` for inline parsing. `ParseResult` = `{ bool success; int offset; unique_ptr<Node> node }` — if `success` is true, the parser consumed `offset` lines and produced an AST `node`; if false, the next parser is tried.
- **Inline parsing**: `LineParserFn` = `std::function<ParseResult(const TokenList&, int)>`. Inline parsers live in `src/parser/parsers/` (`ImageParser`, `LinkParser`, `InlineCodeParser`, `InlineLatexParser`, `SemanticTextParser`).
- **AST nodes**: `Node` base class with `NodeType` enum (in `Node.h`). `Container` nodes hold children via `NodePtrList` = `std::vector<std::unique_ptr<Node>>` (defined in `Node.h`; note `mddef.h` still has a stale `QList<parser::Node*>` alias). Leaf nodes: `Text`, `Image`, `Link`, `InlineCode`, etc., in `src/parser/nodes/`. The classic **Visitor pattern** is used: `NodeVisitor` (in `Visitor.h`) declares a `visit()` overload for every concrete node type; each node implements `accept(NodeVisitor* v) { v->visit(this); }`.
- **List nodes**: `ListNode` and `ListItemNode` (in `nodes/ListNode.h`) extend `Container`; concrete list classes inline `accept()` to avoid diamond inheritance.
- **PieceTable** (`PieceTable.h`): `PieceTableItem` struct backs `Text` nodes for incremental edits. Each `Text` holds a list of `PieceTableItem` entries referencing spans (offset+length) in either `Document::m_originalBuffer` or `Document::m_addBuffer`. When text is inserted, new content goes into the add buffer and a new `PieceTableItem` points to it — the original text is never modified.

Key types from `mddef.h`: `String` = `QString`, `SizeType` = `qsizetype`, `sptr<T>` = `std::shared_ptr<T>`, `DocPtr` = raw `parser::Document*`. Block/inline parser function signatures are in `ParserDetail.h`.

Each module layers additional type aliases in its own `mddef.h`:
- `src/render/mddef.h`: `Brush`, `Color`, `Font`, `InstructionPtr`, `InstructionPtrList`
- `src/editor/mddef.h`: `KeyEvent`, `MouseEvent`, `Painter`, `Timer`, `Point`

### 2. Render (`src/render/` → `QtMarkdownRender`)

Converts an AST node into a draw list for painting. `Render::render()` is a static method — the class is stateless, just a factory for `Block` objects.

- `RenderPrivate` implements `NodeVisitor`, producing a `Block` per AST node. Internally uses a `LayoutPass` to break blocks into lines and a `PaintPass` to generate instructions.
- **Block → LogicalLine → VisualLine → Cell**: a block contains logical lines; each logical line may wrap into multiple visual lines; each visual line holds cells (text runs, images, etc.). **Destruction order matters**: `Instruction` objects (which hold raw `Cell*` pointers) must be destroyed before `LogicalLine` objects (which own the `Cell`s via `VisualLine::vector<unique_ptr<Cell>>`).
- **Instruction** list: Each `Block` accumulates `Instruction` objects (a draw list). Concrete instructions: `TextInstruction`, `StaticTextInstruction`, `ImageInstruction`, `FillRectInstruction`, `EllipseInstruction`, `LatexInstruction`, `StaticImageInstruction`. Painting is just executing this list with a `QPainter`.
- `StringUtil::split()` segments text into Chinese/English/Emoji runs for correct line-breaking.
- LaTeX math: MicroTeX submodule (`3rd/MicroTeX`), linked via `microtex` + `microtex-qt`. `graphic_qt.h` is the Qt graphics backend.
- `RenderSetting` (defined in `Render.h`) holds all visual config: fonts, margins (`docMargin`, `codeMargin`, `listMargin`, `checkboxMargin`, `quoteMargin`), heading sizes (`headerFontSize`), `maxWidth`, `lineSpacing`, etc.

### 3. Editor (`src/editor/` → `QtMarkdownEditorCore`, `QtWidgetMarkdownEditor`, `QtQuickMarkdownEditor`)

- **`Document`**: Wraps a `parser::Document` (composition via `unique_ptr<parser::Document>`, NOT inheritance). Maintains rendered `BlockList`, owns the `CommandStack` for undo/redo, and provides cursor navigation methods. When text changes at a cursor position, the affected `Text` node's `PieceTableItem` list is updated, then only the changed block is re-rendered.
- **`CursorCoord`**: Position model = `(blockNo, lineNo, offset)` — which block, which logical line, and character offset within that line. This is the fundamental addressing scheme for all cursor operations.
- **`Cursor`**: Holds a `CursorCoord` + screen-space `Point` position and height. `SelectionRange` stores a `caret` and `anchor` cursor pair.
- **`Editor`**: Central controller. Handles keyboard/mouse events, cursor movement, text insertion/deletion. Delegates text mutations to `Document`.
- **`Command`** pattern: `InsertTextCommand`, `RemoveTextCommand`, `InsertReturnCommand` for undo/redo. `CommandStack` uses a `vector<unique_ptr<Command>>` + `int m_top` index (single list with position marker, not two separate stacks). The actual document mutations are implemented by internal `NodeVisitor` subclasses in `Command.cpp` (`DocumentOperationVisitor`, `InsertReturnVisitor`, `RemoveTextVisitor`, `InsertTextVisitor`) that are `friend` classes of `Document`.
- Three library tiers: `QtMarkdownEditorCore` (platform-agnostic logic), `QtWidgetMarkdownEditor` (QWidget), `QtQuickMarkdownEditor` (QML).

### Third-party dependencies (`3rd/`)

Git submodules: `MicroTeX` (LaTeX math), `magic_enum` (enum reflection), `pscm` (Scheme interpreter).

### Examples (`example/`)

`QtWidgetMarkdownEditorExample`, `QtQuickMarkdownEditorExample`, and `QtMarkdownParserExample` (parser-only demo).

## Key Conventions

- The `DEBUG` macro (from `debug.h`) outputs `[debug]` prefixed messages with function name, file, and line number. Use it instead of raw `qDebug()` for diagnostic output.
- AST nodes are owned by parent `Container` via `std::unique_ptr` (using the `NodePtrList` alias in `Node.h`). Use `std::make_unique` for allocation. `Text*` pointers in semantic nodes (`ItalicText`, `BoldText`, etc.) are non-owning. Note: `mddef.h` has a stale `NodePtrList = QList<parser::Node*>` alias in the `md` namespace — `Node.h` redefines it properly in `md::parser`.
- The `ASSERT()` macro (from `debug.h`) calls `Backtrace::backtrace()` before `qt_assert` in debug builds.
- `BUILD_STATIC` CMake option controls static vs shared library builds.
- The `.clang-format` file at the repo root defines code style.
- CI uses the `cmake-build-*` pattern in `.gitignore`; `build` is also gitignored.
- Export macro `QTMARKDOWNSHARED_EXPORT` is defined in `src/QtMarkdown_global.h` — used on all public classes.
