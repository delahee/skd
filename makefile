
tool_menu:

pylib:
	#pip install pywin
	#pip install win32event
	pip install fastlz
	pip install wget
	pip install requests
	#pip install elevate
	pip install command-runner
	
	
setup_msdf_cli:
	git submodule init
	git submodule update --recursive
	cd tools/msdf-bmfont-cli; \
	npm install
	
run:
	py bat/ProjectManager.py

build: