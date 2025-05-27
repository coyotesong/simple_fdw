# Changelog

## 0.0.3 - renamed project, added 'simplefdw' api, added documentation

- renamed project to reflect changed focus
- added documentation for structures used in implemented FDW API, including most transitive dependencies
- extracted preliminary 'simplefdw' SPI - the idea is that the FDW implementation will call the provided SPI for the actual work
- write minimal implementation of 'simplefdw'
- write minimal dockerfile so I can test the extension
- verified I can read row metadata(attribute name, type, etc.) in IterateForeignScan
- Note: this is a hodgepodge since I've pulled in chucks from FileFdw but I wanted to push before cleanup for someone working on a similar project.

## 0.0.2 - set new FDW functions to NULL

- added FDW functions introduced through 17 to blackhole_fdw_handler. Set them all to NULL - did not add empty implementation.

## 0.0.1 - initial import

- imported from [https://bitbucket.org/adunstan/blackhole_fdw/](https://bitbucket.org/adunstan/blackhole_fdw/)
- note: this was last updated in 2013!
- reformatted to replace tabs with spaces
