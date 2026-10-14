# Gummi on macOS (Apple Silicon) — build & packaging notes

This tree contains everything needed to build a self-contained, relocatable
`Gummi.app` and DMG for Apple Silicon Macs. This document lists what is
contained in the repository and what must exist on the build/target machine.

## What this port adds (all in the repo)

Source changes:

- `src/constants.h` — the build-time `GUMMI_DATA` / `GUMMI_LIBS` /
  `GUMMI_LOCALES` defines are now compile-time *defaults*; runtime-resolved
  paths live in `gummi_data_dir` / `gummi_libs_dir` / `gummi_locales_dir`.
- `src/utils.c` / `src/utils.h` — new `utils_setup_paths()`: on macOS it
  appends `/Library/TeX/texbin` (MacTeX) and the usual Homebrew locations to
  `PATH` (GUI apps launched via LaunchServices get a bare default PATH), and,
  when running from inside an `.app` bundle, relocates all resource paths into
  the bundle and primes `XDG_DATA_DIRS`, `GDK_PIXBUF_MODULE_FILE` and
  `ENCHANT_MODULE_PATH` for the bundled GTK runtime data.
- `src/main.c` — calls `utils_setup_paths()` first thing.
- `src/biblio.c` — biber support: if a `<jobname>.bcf` file exists (biblatex
  with the biber backend) and `biber` is on PATH, "compile bibliography" runs
  biber instead of bibtex, with success determined by its exit status.
- `src/gui/gui-menu.c` — the Document→Wordcount helper now copies the work
  file into the system temp dir with a sane name (previous version contained
  two memory/path bugs).
- `src/Makefile.am` — removed `-Wl,-export-dynamic`, which the macOS linker
  does not support (regenerate `Makefile.in` with automake if it was checked
  in from an older build).

Build & packaging scripts:

- `dev/macos-bundle.sh` — assembles `dist/Gummi.app`, bundles the full
  Homebrew dylib closure (rewritten to `@executable_path` references),
  gdk-pixbuf loaders + cache, GSettings schemas, gtksourceview language
  specs, Adwaita/hicolor icon themes, enchant providers, translations, and a
  thin-arm64 `biber` shim (see below); signs everything ad-hoc (mandatory on
  arm64 after `install_name_tool`) and produces `gummi-v<version>.dmg`.
- `dev/fix-mactex-biber.sh` — optional system-wide repair for MacTeX's biber
  (needs sudo; see below).

## External prerequisites (NOT in the repo)

Build machine:

1. Xcode Command Line Tools (`clang`, `lipo`, `install_name_tool`,
   `codesign`, `hdiutil`).
2. Homebrew (arm64, `/opt/homebrew` — the bundle script assumes this prefix)
   with:
   `brew install gtk+3 gtksourceview3 gtkspell3 poppler enchant synctex
   hicolor-icon-theme adwaita-icon-theme`
   plus, if the generated `configure`/`Makefile.in` need regeneration:
   `automake autoconf intltool pkg-config gettext`.
3. MacTeX (TeX Live) installed at `/Library/TeX/texbin` — needed at
   *bundle time* to extract the biber shim and at *run time* for the actual
   typesetters.

Target machine: macOS 11+ on Apple Silicon, MacTeX installed.

## The MacTeX/biber on Xcode 27 problem (documented behavior)

MacTeX's `/Library/TeX/texbin/biber` is a universal PAR-packed binary whose
self-extraction calls `/usr/bin/lipo -extract_family ...`. Xcode 27's lipo
removed `-extract_family`, so biber fails with:

    biber: extracting arm64 binary with lipo failed (wstatus=256)

The Gummi.app bundle works around this: `dev/macos-bundle.sh` extracts a
thin arm64 slice (`lipo -thin arm64`) into `Contents/Resources/texbin/biber`,
which skips the extraction step entirely, and gummi prepends that directory
to PATH when running from the bundle. biber therefore works inside the app
even on a machine where the system biber is broken.

To repair biber system-wide (for use from the terminal and other apps), run
once as admin:

    sudo sh dev/fix-mactex-biber.sh

(to undo: `sudo cp /Library/TeX/texbin/biber.universal /Library/TeX/texbin/biber`)

## Building

    ./configure            # or ./autogen.sh && ./configure
    make
    dev/macos-bundle.sh    # produces dist/Gummi.app and gummi-v<ver>.dmg

Running `src/gummi` from the build tree still uses the compile-time install
paths (use `--prefix` at configure time as usual); the bundle code paths only
activate when the executable is inside an `.app` layout that actually
contains its resources.

## Notes & limitations

- The DMG is ad-hoc signed, not notarized. First launch on another machine
  requires right-click → Open (Gatekeeper), or
  `xattr -dr com.apple.quarantine Gummi.app` after copying.
- `dist/` staging, the `.dmg`, `po/*.gmo`, `build_log.txt`, `configure~`,
  `compile`, `.dirstamp` files etc. are build artifacts — do not commit them
  (add to `.gitignore`).
- The icon theme copy is optional: if `adwaita-icon-theme` is not installed,
  the bundle script simply skips it and GTK falls back to its built-in theme.
