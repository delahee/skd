

all: create read_sync 
	./convert1ToMSDFWithCharset.bat "Aero Matics Bold.ttf" charset_efigs.txt
	./convert1ToMSDFWithCharset.bat 'Aero Matics Display Light.ttf' charset_efigs.txt
	./convert1ToMSDFWithCharset.bat 'Aero Matics Light.ttf' charset_efigs.txt
	./convert1ToMSDFWithCharset.bat Marksman.ttf charset_efigs.txt
	./convert1ToMSDFWithCharset.bat 'Aero Matics Display Bold.ttf'  charset_efigs.txt
	./convert1ToMSDFWithCharset.bat 'Aero Matics Display Regular.ttf' charset_efigs.txt
	./convert1ToMSDFWithCharset.bat 'Aero Matics Regular.ttf' charset_efigs.txt
	make write_sync

deps:
	npm install opentype.js
	npm install arabic-persian-reshaper
	npm install js2xmlparser
	npm install map-limit
	npm install cli-progress
	npm install maxrects-packer
	npm install jimp
	npm install handlebars
	npm install commander
	npm install commander
	
create :
	mkdir -p msdf
	
FORCE:

read_sync:
	rsync -ar --del ../font/*.* .

write_sync:
	rsync -ar --del msdf/* ../res/fonts