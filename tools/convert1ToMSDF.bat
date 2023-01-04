REM Drag n drop a ttf file here to generate its msdf

node msdf-bmfont-cli/cli.js -o "msdf/%1.png" -s 32 -t msdf -b 2 -p 4 --smart-size --pot %1