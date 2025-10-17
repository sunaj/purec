File Explorer Quick Terminal:

This one-file Win11 utility serves the purpose of linking F1 (or indeed any F1-12 key) 
so that when pressed within File Explorer the current folder will be opened in the default 
Terminal ("wt") set within Win11. Any recent version of Powershell  should be linked to the 
wt shortcut as required. The utility has a few ergonomic cmdline params that can be parsed on 
startup, see below.

Adding the exe to run on startup will probably be wanted, I suggest using the "shell:startup" 
address bar link in File Explorer and adding a shortcut to this as an easy one-step solution.

Command-line parameters:

--fhotkey<N>
  Sets the hotkey to the specified F-key. Replace <N> with a number from 1 to 12.
  Example: --fhotkey2 sets the hotkey to F2.
  If not specified, the default hotkey is F1.

--notrayicon
  Hides the application icon from the system tray.
  
