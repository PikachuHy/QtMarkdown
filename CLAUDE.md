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

Tests use the [doctest](https://github.com/doctest/doctest) framework. The main entry point is defined via `#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` in each test file, so each test binary is self-contained.

## Architecture

QtMarkdown is a Qt-based Markdown engine with four layers, each a separate CMake library target:

### 1. Parser (`src/parser/` → `QtMarkdownParser`)

Parses Markdown text into an AST using a two-pass approach:

- **Tokenization**: `parseLine()` in `Parser.cpp` splits a line into `Token` objects (type + offset/length into the original text). Special characters `#`, `*`, `~`, `` ` ``, `$`, `[]`, `()`, `!`, `>` become typed tokens; everything else is `text`.
- **Block parsing**: Each block parser is in `src/parser/parsers/` (e.g., `HeaderParser.cpp`, `CodeBlockParser.cpp`, `OrderedListParser.cpp`, `UnorderedListParser.cpp`, `CheckboxListParser.cpp`, `QuoteBlockParser.cpp`, `LatexBlockParser.cpp`, `ParagraphParser.cpp`). `ParagraphParser` is the fallback/default.
- **Inline parsing**: Each inline parser is also in `src/parser/parsers/` (`ImageParser.cpp`, `LinkParser.cpp`, `InlineCodeParser.cpp`, `InlineLatexParser.cpp`, `SemanticTextParser.cpp`).
- **AST nodes**: `Node` base class with `NodeType` enum (defined in `Node.h`). `Container` nodes hold children (`NodePtrList` = `std::vector<std::unique_ptr<Node>>`). Leaf nodes: `Text`, `Image`, `Link`, `InlineCode`, etc. Node classes are in `src/parser/nodes/`. The `Visitable<T>` / `Visitor<T>` CRTP pattern enables double-dispatch.
- **List nodes**: `ListNode` and `ListItemNode` (in `nodes/ListNode.h`) extend `Container` and serve as base classes for `CheckboxList`/`UnorderedList`/`OrderedList` and their item types. The concrete list classes inline the `accept()` method to avoid diamond inheritance.
- **PieceTable** (`PieceTable.h`): The `PieceTableItem` struct backs `Text` nodes for efficient edits. Each `Text` holds a list of `PieceTableItem` entries that reference spans (offset+length) in either the original buffer or the add buffer.

Key types from `mddef.h`: `String` = `QString`, `SizeType` = `qsizetype`, `sptr<T>` = `std::shared_ptr<T>`, `DocPtr` = raw `parser::Document*`. All parser functions are declared in `ParserDetail.h`.

### 2. Render (`src/render/` → `QtMarkdownRender`)

Converts an AST node into a draw list for painting:

- `RenderPrivate` is a `MultipleVisitor` implementing `visit()` for every node type. Each `visit` method produces a `Block`.
- **Block** → **LogicalLine** → **VisualLine** → **Cell**: a block contains logical lines; each logical line may wrap into multiple visual lines; each visual line holds cells (text runs, images, etc.).
- **Instruction** list: Each `Block` accumulates `Instruction` objects (a draw list). Concrete instructions: `TextInstruction`, `StaticTextInstruction`, `ImageInstruction`, `FillRectInstruction`, `EllipseInstruction`, `LatexInstruction`, `StaticImageInstruction`. Rendering is just executing this instruction list with a `QPainter`.
- `StringUtil::split()` segments text into Chinese/English/Emoji runs for proper line-breaking.
- LaTeX math rendering uses the MicroTeX submodule (`3rd/MicroTeX`).
- `RenderSetting` holds all visual configuration (fonts, margins, spacing, sizes).

### 3. Editor (`src/editor/` → `QtMarkdownEditorCore`, `QtWidgetMarkdownEditor`, `QtQuickMarkdownEditor`)

- **`Editor`**: Central controller. Handles keyboard/mouse events, cursor movement, text insertion/deletion. Delegates input to `Document` and renders via the render layer.
- **`Document`** (extends `parser::Document`): Maintains rendered `BlockList`, owns the `CommandStack` for undo/redo, and provides cursor movement operations.
  - When text is inserted/removed at a cursor position, the affected `Text` node's `PieceTableItem` list is updated, then only the changed block is re-rendered.
- **`Command`** pattern: `InsertTextCommand`, `RemoveTextCommand`, `InsertReturnCommand` for undo/redo.
- Three library tiers: `QtMarkdownEditorCore` (platform-agnostic logic), `QtWidgetMarkdownEditor` (QWidget-based), `QtQuickMarkdownEditor` (QML-based).

### 4. Third-party (`3rd/`)

Git submodules: `MicroTeX` (LaTeX math), `magic_enum` (enum reflection), `pscm` (Scheme interpreter).

## Key Conventions

- AST nodes are owned by parent `Container` via `std::unique_ptr` (`NodePtrList` = `std::vector<std::unique_ptr<Node>>`). Use `std::make_unique` for allocation. `Text*` pointers in semantic nodes (ItalicText, BoldText, etc.) are non-owning.
- The `ASSERT()` macro (from `debug.h`) calls `Backtrace::backtrace()` before `qt_assert` in debug builds.
- `BUILD_STATIC` CMake option controls static vs shared library builds.
- The `.clang-format` file at the repo root defines code style.
- CI uses the `cmake-build-*` pattern in `.gitignore`; `build` is also gitignored.
- Export macro `QTMARKDOWNSHARED_EXPORT` is defined in `src/QtMarkdown_global.h` — used on all public classes.
