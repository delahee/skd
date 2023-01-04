REM Drag n drop a ttf file here to generate its msdf

for %%x in (%*) do (
   node msdf-bmfont-cli/cli.js -o "msdf/%%~nx.png" -s 32 -t msdf -b 2 -p 4 --smart-size --pot %%x
)