#!/bin/sh
# macos-bundle.sh - build a self-contained, relocatable Gummi.app and
# package it into a distributable DMG for Apple Silicon (arm64).
#
# Prerequisites: gummi already built in src/ against the Homebrew
# (arm64, /opt/homebrew) GTK 3 stack, MacTeX installed on the target.
#
# Usage: dev/macos-bundle.sh [dist-dir]
set -e

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BREW=/opt/homebrew
VERSION=$(sed -n 's/.*C_PACKAGE_VERSION "\([^"]*\)".*/\1/p' "$ROOT/src/constants.h" | head -1)
DIST=${1:-"$ROOT/dist"}
APP="$DIST/Gummi.app"
RES="$APP/Contents/Resources"
MACOS="$APP/Contents/MacOS"
LIBDIR="$RES/lib"
DMG="$ROOT/gummi-v$VERSION.dmg"

GLIB_COMPILE_SCHEMAS=$(command -v glib-compile-schemas || echo "$BREW/bin/glib-compile-schemas")

echo "==> Building Gummi.app (version $VERSION) in $DIST"

### 1. App skeleton

rm -rf "$APP"
mkdir -p "$MACOS" "$RES/share/gummi/ui" "$RES/share/gummi/icons" \
         "$RES/share/gummi/misc" "$RES/share/gummi/snippets" \
         "$LIBDIR/gummi"

cp "$ROOT/src/gummi" "$MACOS/gummi"

cat > "$APP/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>gummi</string>
    <key>CFBundleIdentifier</key>
    <string>app.gummi.gummi</string>
    <key>CFBundleName</key>
    <string>Gummi</string>
    <key>CFBundleDisplayName</key>
    <string>Gummi</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>$VERSION</string>
    <key>CFBundleVersion</key>
    <string>$VERSION</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>
    <key>NSSupportsAutomaticGraphicsSwitching</key>
    <true/>
    <key>NSRequiresAquaSystemAppearance</key>
    <true/>
    <key>CFBundleDocumentTypes</key>
    <array>
        <dict>
            <key>CFBundleTypeName</key>
            <string>LaTeX document</string>
            <key>CFBundleTypeRole</key>
            <string>Editor</string>
            <key>CFBundleTypeExtensions</key>
            <array>
                <string>tex</string>
            </array>
        </dict>
    </array>
</dict>
</plist>
EOF

### 2. Gummi runtime data (glade UI, icons, snippets, helper scripts)

cp "$ROOT"/data/ui/*.glade            "$RES/share/gummi/ui/"
cp "$ROOT/data/icons/gummi.png"       "$RES/share/gummi/icons/"
cp "$ROOT/data/icons/icon.png"        "$RES/share/gummi/icons/"
cp "$ROOT/data/misc/default.tex"      "$RES/share/gummi/misc/"
cp "$ROOT/data/snippets/snippets.cfg"  "$RES/share/gummi/snippets/"
cp "$ROOT/data/snippets/snippets.lang" "$RES/share/gummi/snippets/"
cp "$ROOT/lib/latex_dvi.sh"           "$LIBDIR/gummi/"
chmod +x "$LIBDIR/gummi/latex_dvi.sh"

cp "$ROOT/data/icons/gummi.png"       "$RES/AppIcon.png"

# translations
for mo in "$ROOT"/po/*.gmo; do
    [ -e "$mo" ] || continue
    loc=$(basename "$mo" .gmo)
    mkdir -p "$RES/share/locale/$loc/LC_MESSAGES"
    cp "$mo" "$RES/share/locale/$loc/LC_MESSAGES/gummi.mo"
done

### 3. GTK runtime data (loaders, schemas, language specs)

echo "==> Bundling GTK runtime data"

GDKVER=2.10.0
mkdir -p "$LIBDIR/gdk-pixbuf-2.0/$GDKVER/loaders"
if ls "$BREW/lib/gdk-pixbuf-2.0/$GDKVER/loaders/"*.so >/dev/null 2>&1; then
    cp "$BREW/lib/gdk-pixbuf-2.0/$GDKVER/loaders/"*.so \
       "$LIBDIR/gdk-pixbuf-2.0/$GDKVER/loaders/"
elif ls "$BREW/lib/gdk-pixbuf-2.0/$GDKVER/loaders/"*.dylib >/dev/null 2>&1; then
    cp "$BREW/lib/gdk-pixbuf-2.0/$GDKVER/loaders/"*.dylib \
       "$LIBDIR/gdk-pixbuf-2.0/$GDKVER/loaders/"
fi

# loaders.cache with rewritten (bundle-relative) module paths
if [ -f "$BREW/lib/gdk-pixbuf-2.0/$GDKVER/loaders.cache" ]; then
    sed "s|$BREW/lib/|@executable_path/../Resources/lib/|g" \
        "$BREW/lib/gdk-pixbuf-2.0/$GDKVER/loaders.cache" \
        > "$LIBDIR/gdk-pixbuf-2.0/$GDKVER/loaders.cache"
fi

# gsettings schemas (file chooser, dialog settings etc.)
mkdir -p "$RES/share/glib-2.0/schemas"
cp "$BREW/share/glib-2.0/schemas/"*.gschema.xml \
   "$RES/share/glib-2.0/schemas/" 2>/dev/null || true
"$GLIB_COMPILE_SCHEMAS" "$RES/share/glib-2.0/schemas" 2>/dev/null || true

# gtksourceview language definitions & styles
if [ -d "$BREW/share/gtksourceview-3.0" ]; then
    cp -RL "$BREW/share/gtksourceview-3.0" "$RES/share/"
fi

# shared mime info database, when available (icon/pixbuf format lookup)
if [ -d "$BREW/share/mime/packages" ]; then
    mkdir -p "$RES/share/mime"
    cp -RL "$BREW/share/mime/packages" "$RES/share/mime/" 2>/dev/null || true
else
    echo "    (no shared-mime-info installed; skipping mime database)"
fi

# icon themes
for theme in hicolor Adwaita; do
    if [ -d "$BREW/share/icons/$theme" ]; then
        mkdir -p "$RES/share/icons"
        cp -RL "$BREW/share/icons/$theme" "$RES/share/icons/"
    fi
done

# enchant spell backend providers (applespell needs no dictionaries)
if ls "$BREW/lib/enchant-2/"*.so >/dev/null 2>&1; then
    mkdir -p "$LIBDIR/enchant-2"
    cp "$BREW/lib/enchant-2/"*.so "$LIBDIR/enchant-2/"
fi

# TeX tool shims: MacTeX 2025's universal biber fails to self-extract with
# Xcode 27's lipo (the removed "-extract_family" option); ship a thin arm64
# slice that skips the extraction entirely. gummi prepends this directory
# to PATH when running from the bundle (utils_setup_paths).
if [ -f /Library/TeX/texbin/biber ]; then
    echo "==> Adding thin-arm64 biber shim (MacTeX/XCode27 lipo workaround)"
    mkdir -p "$RES/texbin"
    if /usr/bin/lipo /Library/TeX/texbin/biber -thin arm64 \
            -output "$RES/texbin/biber" 2>/dev/null; then
        chmod u+x "$RES/texbin/biber"
        codesign --force --sign - "$RES/texbin/biber"
        "$RES/texbin/biber" --version
    else
        rmdir "$RES/texbin" 2>/dev/null || true
        echo "    (lipo failed; skipping biber shim)"
    fi
fi

### 4. Dylib closure: repeat until every non-system reference is bundled

echo "==> Bundling dylib dependencies"

resolve_real() {
    # resolve a Mach-O load path to a real filesystem path
    case "$1" in
        @rpath/*)
            rname=${1#@rpath/}
            if [ -f "$LIBDIR/$rname" ]; then
                echo "$LIBDIR/$rname"
            else
                find "$BREW/lib" "$BREW/opt" -name "$rname" \
                    \( -type f -o -type l \) 2>/dev/null | head -1
            fi
            ;;
        *) echo "$1" ;;
    esac
}

pass=0
while :; do
    pass=$((pass + 1))
    [ $pass -gt 20 ] && { echo "dependency resolution did not converge"; break; }
    left=0
    find "$LIBDIR" -type f \( -name '*.dylib' -o -name '*.so' \) \
         > /tmp/gummi-bundle-files.$$
    echo "$MACOS/gummi" >> /tmp/gummi-bundle-files.$$
    while read -r f; do
        deps=$(otool -L "$f" 2>/dev/null | tail -n +2 | awk '{print $1}' |
               grep -E '^(/opt/homebrew|/usr/local|@rpath)' || true)
        [ -z "$deps" ] && continue
        echo "$deps" | while read -r dep; do
            real=$(resolve_real "$dep")
            [ -f "$real" ] || continue
            base=$(basename "$real")
            dest="$LIBDIR/$base"
            case "$real" in
                "$dest"*) : ;;  # already bundled; just rewrite below
                *)
                    if [ ! -f "$dest" ]; then
                        cp "$real" "$dest"
                        chmod u+w "$dest"
                    fi
                    ;;
            esac
            install_name_tool -change "$dep" \
                "@executable_path/../Resources/lib/$base" "$f" 2>/dev/null || true
        done
    done < /tmp/gummi-bundle-files.$$
    rm -f /tmp/gummi-bundle-files.$$
    echo "    pass $pass complete"
    # any remaining brew-local references left anywhere in the bundle?
    # (fresh scan, so files copied *during* this pass are counted too.
    #  a file whose line 2 is its own id — common for dylibs, but some
    #  .so modules are really dylibs — must skip that line: ids are fixed
    #  by the -id pass below.)
    find "$LIBDIR" -type f \( -name '*.dylib' -o -name '*.so' \) | while read -r f; do
        stem=$(basename "$f"); stem=${stem%.*}
        idline=$(otool -L "$f" 2>/dev/null | sed -n 2p | awk '{print $1}')
        case "$idline" in
            *"$stem"*) skip=+3 ;;
            *)         skip=+2 ;;
        esac
        otool -L "$f" 2>/dev/null | tail -n $skip | awk '{print $1}' |
        grep -qE '^(/opt/homebrew|/usr/local|@rpath)' && echo "$f" || true
    done > /tmp/gummi-bundle-left.$$
    while read -r lf; do
        echo "    left: $lf:"
        lstem=$(basename "$lf"); lstem=${lstem%.*}
        lidline=$(otool -L "$lf" 2>/dev/null | sed -n 2p | awk '{print $1}')
        case "$lidline" in
            *"$lstem"*) lskip=+3 ;;
            *)           lskip=+2 ;;
        esac
        otool -L "$lf" 2>/dev/null | tail -n $lskip |
            grep -E '/opt/homebrew|/usr/local|@rpath' | head -2 |
            sed 's/^/      /'
    done < /tmp/gummi-bundle-left.$$ | head -12
    otool -L "$MACOS/gummi" 2>/dev/null | tail -n +2 | awk '{print $1}' |
        grep -qE '^(/opt/homebrew|/usr/local|@rpath)' && echo x >> /tmp/gummi-bundle-left.$$ || true
    [ -s /tmp/gummi-bundle-left.$$ ] && left=1 || true
    rm -f /tmp/gummi-bundle-left.$$
    [ "$left" -eq 0 ] && break
done

# bundle-relative install ids for all bundled libraries
find "$LIBDIR" -type f \( -name '*.dylib' -o -name '*.so' \) | while read -r f; do
    rel=${f#"$RES/"}
    install_name_tool -id "@executable_path/../Resources/$rel" "$f" \
        2>/dev/null || true
done

### 5. Ad-hoc code signing (mandatory on arm64 after install_name_tool)

echo "==> Signing"
# sign each library/module individually (a single multi-file codesign call
# stops at the first problematic file and leaves the rest unsigned)
find "$LIBDIR" -type f \( -name '*.dylib' -o -name '*.so' \) | while read -r f; do
    codesign --force --sign - "$f" >/dev/null 2>&1 || \
        echo "    WARNING: could not sign $f"
done
# sign the bundle as a whole last: this seals the main executable as the
# bundle's main binary (signing it standalone first produces a broken seal)
codesign --force --sign - "$APP"

### 6. DMG packaging

echo "==> Creating DMG"
rm -f "$DMG"
rm -rf "$DIST/dmg-root"
mkdir -p "$DIST/dmg-root"
cp -R "$APP" "$DIST/dmg-root/"
ln -s /Applications "$DIST/dmg-root/Applications"
hdiutil create -volname "Gummi $VERSION" -srcfolder "$DIST/dmg-root" \
    -ov -format UDZO "$DMG"
rm -rf "$DIST/dmg-root"

echo "==> Done: $DMG"
