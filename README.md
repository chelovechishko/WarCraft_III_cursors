# WarCraft III cursors (Undead main) theme

## 1 Description
This is WarCraft themed cursors.

Misc additions/deletions/modifications included.
Like hot spot fixes, animation duration changes, renaming, relinking, recoloring (fel_orc_hand), rotation of existing image and so creating new cursor, shrinkages.

### 1.1 Code description
A small program for getting bitmaps and hashes of core X11 cursors and additional mozilla's bitmaps.

## 2 Installation
Inside `~/.local/share/cursors/default/index.theme` (create if necessary) write:
```
[Icon Theme]
Inherits = WarCraftIII
```
Unpack cursors into `~/.local/share/cursors/`. So structure should be like: `~/.local/share/cursors/WarCraftIII/cursors/left_ptr`

Note, that `XCURSOR_PATH` should include something like: `${XDG_DATA_HOME}/cursors` in environment settings of yours. E. g. `XCURSOR_PATH=${HOME}/.local/share/cursors`... in output of `printenv`.

That should be enough.

## 3 Compilation of code
Compile it via `gcc -o cursors_dump -lX11 -lXcursor main.c`

- Add `-DDUMP_MOZ_CURSORS` for dumping moz cursors bitmaps from `nsGtkCursors.h` file.

- Add `-DDUMP_X11_CURSORS` for dumping X11 core cursors bitmaps from `cursor.bdf` file.

## 4 History
* 2002, July 5 — **WarCraft III** released

* 2003, July 16 — **mizrahav** [uploaded](http://www.wincustomize.com/explore/cursorfx/575/) `Warcraft 3 Undead` cursors

* 20xx — [GrynayS](https://www.deviantart.com/grynays/about#about) (aka [KuduK](https://web.archive.org/web/20210308160620/https://www.pling.com/u/kuduk/)) converted the theme into X11 Mouse theme and refined it with GIMP.

* 2020, June 5 — [sorely](https://www.opencode.net/sorely/warcraft-3-cursors) further changed and updated the theme.

## 5 Some other links
https://freedesktop.org/wiki/Specifications/cursor-spec/

https://htmlpreview.github.io/?https://github.com/mdomlop/retrosmart-x11-cursors/blob/master/cursortest.html

https://www.w3schools.com/csSref/pr_class_cursor.asp

https://echoecho.com/csscursors.htm

https://github.com/trunkmaster/nextspace/wiki/Mouse-Cursors

https://www.x.org/releases/X11R7.7/doc/man/man3/Xcursor.3.xhtml

___

P. S. i don't own [World of] WarCraft [III] icons, nor other authors work, all rights belong to owners.
