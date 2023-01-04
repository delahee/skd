@ECHO OFF
curl "https://docs.google.com/spreadsheets/d/1tdpLU1qeyzy5W4uu8FTG81ya8lZUVNYoA0hEkmxBNzY/export?gid=0&format=tsv" > ../res/data/tiles.tsv
curl "https://docs.google.com/spreadsheets/d/1tdpLU1qeyzy5W4uu8FTG81ya8lZUVNYoA0hEkmxBNzY/export?gid=2114408089&format=tsv" > ../res/data/props.tsv
curl "https://docs.google.com/spreadsheets/d/1tdpLU1qeyzy5W4uu8FTG81ya8lZUVNYoA0hEkmxBNzY/export?gid=1028737175&format=tsv" > ../res/data/chars.tsv

curl "https://docs.google.com/spreadsheets/d/1tdpLU1qeyzy5W4uu8FTG81ya8lZUVNYoA0hEkmxBNzY/export?gid=1735745857&format=tsv" > ../res/data/tile_data.tsv