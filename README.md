# File Panes
Multi-Paned File Explorer

The goal of this project is to create a multi-paned file explorer using the windows shell apis to maintain maximum compatibility users know and love with the default file explorer.


## v0.1.0 - in progress
- [x] make a better app icon
- [x] add button tool tips
- [x] go to folder on enter in address bar
- [x] add horizontal split button
- [x] add vertical split button
- [x] add back button
- [x] add up directiory button
- [x] add refresh button
- [x] add remove pane button
- [x] save current state and read last known state on start


# TODO
- [ ] add tool tip to folder explorer items
- [ ] save multiple layout configurations
- [ ] add item view options: list, details, icons 
- [ ] add keyboard shortcuts for explorer pane ctl-a, delete, ctl-x, ctl-c, ctl-v
- [ ] add paste/copy/delete/new folder buttons
- [ ] new pane after split should be same directory of pane being split
- [ ] hide buttons if they don't fit in pane width
- [ ] add drop down to expose more buttons ( kins of like a ribbon? )
- [ ] save network login information for future use and integrate with the explorer browser control. Is this possible?
- [ ] implement clean update strategy based on github releases?
- [ ] add preview pane
- [ ] add console pane
- [ ] add search
- [ ] do more to clean up rendering to remove jitter
   - double buffering on main window
   - double buffering on folder browser
- [ ] add favorites/quick access
- [ ] improve address bar
- [ ] add address bar history
- [ ] add custom actions ( menu or toolbar ?)
- [ ] customize explorer details columns
- [ ] add batch regx based rename with preview
- [ ] integrate one drive
- [ ] integrate iCloud drive
- [ ] make exploring 7z archives transparent like zip files
- [ ] move opening a folder location in tree out of main thread so when it takes a long time like HDD spin up or network access doesn't block the main thread.


