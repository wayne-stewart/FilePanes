# File Panes
Multi-Paned File Explorer

The goal of this project is to create a multi-paned file explorer using the windows shell apis to maintain maximum compatibility users know and love with the default file explorer.


# Primary TODO
- add back button
- add up directiory button
- go to folder on enter in address bar
- add refresh button
- add horizontal split button
- add vertical split button
- add remove pane button
- save current state and read last known state on start
- save multiple layout configurations
- add item view options: list, details, icons 
- make a better app icon
- add paste/copy/delete/new folder buttons

# Secondary TODO
- save network login information for future use and integrate with the explorer browser control. Is this possible?
- implement clean update strategy based on github releases?
- add preview pane
- add console pane
- add search
- do more to clean up rendering to remove jitter
   - double buffering on main window
   - double buffering on folder browser
- add favorites/quick access
- improve address bar
- add address bar history
- add custom actions ( menu or toolbar ?)
- customize explorer details columns
- add batch regx based rename with preview
- integrate one drive
- integrate iCloud drive