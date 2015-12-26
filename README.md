# srtmerfter
This is a srt subtitle file tool.It can shift time and merge two srt files.

## Usage 
* Sort Lines : srtmerfter file1 file2 ......
* Shift Time : srtmerfter -s +/-1,300  file.srt
* Merge Files: srtmerfter -m file1.srt file2.srt

## Tips
This program is using fgets() to read srt file lines, And the Mac file EOL is CR, Which cause it cannot process the Mac file format correctly.
So better convert file format to unix or dos before use it.
