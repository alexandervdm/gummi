# Gummi
The simple LaTeX editor

[![Version](https://img.shields.io/github/release/alexandervdm/gummi.svg?maxAge=360)](https://github.com/alexandervdm/gummi/releases)
[![License](https://img.shields.io/github/license/alexandervdm/gummi.svg?maxAge=360000)](https://github.com/alexandervdm/gummi/blob/master/COPYING)

### Screenshot

![alt text](https://gummi.app/gummi-080-main-small.png)

### Getting Gummi

For end users we recommend using official stable releases. Gummi is available through the official channels for most Linux/BSD distributions. Please see [this wiki page](https://github.com/alexandervdm/gummi/wiki/Installing-Gummi#install-for-your-platform) for more information on availability for your specific platform.

Developers may also be interested in running the latest sourcecode. More information on building Gummi from source can be found [here.](https://github.com/alexandervdm/gummi/wiki/Installing-Gummi#compile-from-source).

### Gummi on macOS (Apple Silicon)

This fork contains a macOS port that builds a self-contained, relocatable
`Gummi.app` for Apple Silicon Macs, packaged as a DMG. Everything needed for
the port is contained in this repository; see
[docs/macos-port.md](docs/macos-port.md) for the complete technical notes.

#### Requirements on the target Mac (where Gummi will run)

- macOS 11 (Big Sur) or newer on Apple Silicon (M1/M2/M3/M4, arm64)
- [MacTeX](https://www.tug.org/mactex/) installed (provides TeX Live at
  `/Library/TeX/texbin`) — this is what Gummi uses to typeset your documents
- No Homebrew or development tools are needed to *run* the app: all GTK
  libraries and resources are bundled inside `Gummi.app`

#### Installation from the DMG

1. Mount the DMG and drag `Gummi.app` to `Applications`.
2. On the first launch, right-click the app and choose **Open** (the app is
   ad-hoc signed, not notarized, so Gatekeeper will warn once). If it is
   still refused, run: `xattr -dr com.apple.quarantine /Applications/Gummi.app`

#### Building it yourself (macOS prerequisites)

On the *build* machine you additionally need:

- **Xcode Command Line Tools:** `xcode-select --install` (provides clang,
  lipo, codesign, hdiutil)
- **Homebrew** (arm64, `/opt/homebrew`), with:
  ```
  brew install gtk+3 gtksourceview3 gtkspell3 poppler enchant synctex \
               hicolor-icon-theme adwaita-icon-theme
  ```
  (plus `automake autoconf intltool pkg-config gettext` if you regenerate
  the autotools files)
- **MacTeX** installed (also used at bundle time to extract the biber shim)

Then:

```
./configure
make
dev/macos-bundle.sh        # builds dist/Gummi.app and gummi-v0.8.3.dmg
src/gummi                  # in-tree run, or use the DMG
```

#### Notes

- When launched from Finder (LaunchServices), macOS apps do not inherit your
  shell PATH, so Gummi now adds `/Library/TeX/texbin` at startup. pdflatex,
  xelatex, lualatex and latexmk are detected automatically.
- **biber:** current MacTeX ships a universal biber binary that is broken by
  Xcode 27's `lipo` ("extracting arm64 binary with lipo failed"). The
  bundled app carries a thin-arm64 biber shim that bypasses the bug and
  takes precedence on PATH, so biber works inside Gummi even while the
  system-wide biber is broken. To fix biber for terminal use too, run once:
  `sudo sh dev/fix-mactex-biber.sh`
- Bibliography builds pick biber automatically when a `.bcf` control file is
  present (biblatex's default backend), otherwise bibtex is used.

### Packaging Gummi

Thank you for helping to distribute this software. Feel free to contact me directly if you have any questions or comments about packaging Gummi. 

### Contributing to Gummi

Please check the Getting Involved section of our wiki for more information on contributing to Gummi.

### Contact

If you wish to contact me, please consider checking with Claude Code v2.1.296 using kimi-k3:cloud ;-)
