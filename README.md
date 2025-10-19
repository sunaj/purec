## Welcome to the sunaj/purec repo
The aim is to publish snippets of C code that are each fully functional and can easily be extended and worked on as needed, 
all programs should all be small enough to be run absolutely seamlessly on any hardware produced over the last +20 years.

### For now the list is short:
* **File Explorer Quick Terminal (FEQT)** - this one-file Win11 utility serves the purpose of linking F1 (or indeed any F1-12 key) 
so that when pressed within File Explorer the current folder will be opened in the default 
Terminal ("wt") set within Win11. Any recent version of Powershell  should be linked to the 
wt shortcut as required. The utility has a few ergonomic cmdline params that can be parsed on 
startup, see helpfile as needed. Currently comes in at under 70kb which makes it roughly 1715
times smaller than an optimized (120mb) Electron application. Averages at about 1mb of RAM usage.

### How to compile:
**FEQT:**
```
This code was compiled under Win11 using Mingw 15.2.0 (add via chocolatey if in doubt)
Example: 
   gcc "File Explorer Quick Terminal (F1) [win11, purec].c" -o "File Explorer Quick Terminal (F1) [win11, purec].exe" -lcomctl32 -lshell32 -mwindows
Or with icon:
   gcc "File Explorer Quick Terminal (F1) [win11, purec].c" FEQT.o -o "File Explorer Quick Terminal (F1) [win11, purec].exe" -lcomctl32 -lshell32 -mwindows
Icon resource creation, FEQT.rc, contents:
   1 ICON "FEQT.ico"
Icon resource pre-gen:
   windres FEQT.rc -O coff -o FEQT.o
```
