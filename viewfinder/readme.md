# Getting cursors showcase

## Prerequisite steps
1. Put `xmc_to_png.scm` into `~/.config/GIMP/version.subversion/scripts`.
2. For getting only real ones files and not symlinks we could use `find`.
   To skip hashes that starts with a number we do `[A-Za-z]` as first symbol.
   For getting only names without a path we use custom output`-printf "%f\n"`.
   Do not accept names which are hashes via simple `sed` script: `sed '/[0-9a-f]\{16\}/d'`.
   ```
   find "WarCraftIII/cursors" -maxdepth 1 -type f -name "[a-zA-Z]*" -printf "%f.png\n" | sed '/[0-9a-f]\{16\}/d' | sort > viewfinder/cursors_named_list_for_view.txt
   ```
3. Reorder them as needed (as a current output grid would be 6 cursors in a row, group them by 6 in each).

4. Then execute in terminal:
   ```bash
   for cursor in $(cat viewfinder/cursors_named_list_for_view.txt | sed 's/\.png$//') ; do gimp --no-interface --no-data --no-fonts --batch "(xmc_to_png \"WarCraftIII/cursors/${cursor}\" \"viewfinder/${cursor}.png\")" --batch '(gimp-quit 0)' ; done
   ```
   for getting cursors images in `png` format (yea, 'tis very slow).

## Requisite steps
1. 
    ```bash
   magick montage $(cat viewfinder/cursors_named_list_for_view.txt | sed 's/^\(.\)/viewfinder\/\1/') -adjoin -background none -bordercolor black -tile 6x6 -frame 2 -geometry '68x68+0+0>' viewfinder/found.png
   ```
   for getting pretty cursors example.
