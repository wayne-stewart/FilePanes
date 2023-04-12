# File Panes
Multi-Paned File Explorer

The goal of this project is to create a multi-paned file explorer using the windows shell apis to maintain maximum compatibility users know and love with the default file explorer.

## v0.2 in progress
- [x] handle delete key
- [x] handle ctl-a to select all
- [x] handle ctl-x to cut
- [x] handle ctl-c to copy
- [x] handle ctl-v to paste
- [x] copy/paste now uses dynamically allocated memory to avoid artificial limitations
- [x] show default windows explorer status bar in each explorer pane
- [ ] folder browser updates
- [ ] - auto detect drive changes
- [ ] - add favorites/quick access
- [x] - default should just show drive letters and home
- [x] - integrate one drive
- [ ] show network login for remote shares
- [ ] new pane after split should be same directory of pane being split


## v0.1
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
- [ ] save multiple layout configurations
- [ ] add paste/copy/delete/new folder buttons
- [ ] hide buttons if they don't fit in pane width
- [ ] add drop down to expose more buttons ( kind of like a ribbon? )
- [ ] implement clean update strategy based on github releases?
- [ ] add preview pane
- [ ] add console pane
- [ ] add search
- [ ] click path parts in address bar to naviate
- [ ] add address bar history
- [ ] add custom actions ( menu or toolbar ?)
- [ ] customize explorer details columns
- [ ] add batch regx based rename with preview
- [ ] integrate iCloud drive
- [ ] make exploring 7z archives transparent like zip files
- [ ] move opening a folder location in tree out of main thread so when it takes a long time like HDD spin up or network access doesn't block the main thread.
- [ ] add auto updater based on github releases

