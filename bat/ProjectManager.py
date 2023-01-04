import platform
from datetime import datetime
import shutil
from multiprocessing import Pool
import time
import glob
import subprocess
import sys
import tkinter as tk
from tkinter import ttk
import configparser
import os
import string

import requests
import fastlz
import wget

import threading
from queue import Queue
from tkinter import messagebox as mb

from command_runner.elevate import elevate

local_config = configparser.ConfigParser()
repo_config = configparser.ConfigParser()

onexit = []
root = tk.Tk()
log = None
compileProc = None
scriptPath = os.path.dirname(os.path.realpath('__file__'))
if not scriptPath.endswith("bat"):
	scriptPath += "\\bat"
editSize = 120
task = None
compiling = False
gathering = False
syncing = False
resQueue = Queue()
projectRoot = None

tools = None

def rsync(src,dst):
	if os.stat(src).st_mtime - os.stat(dst).st_mtime > 1:
		shutil.copy2(src, dst)

def cls():
	if log is not None:
		log.configure(state="normal")
		log.delete('1.0', tk.END)
		log.configure(state="disabled")

def trace(msg):
	if log is not None:
		log.configure(state="normal")
		log.insert("end", msg + "\n")
		log.see("end")
		log.configure(state="disabled")
	else:
		print(" log is none? ")
	return msg

def pathToCyg(path):
	if( platform.system() == "Windows"):
		path = os.path.normpath(path)
		path = path.replace("\\","/")
		path = path.replace(":", "/")
		path = "/cygdrive/"+path
		return path
	else:
		return path
	
def test_lz():
	lit = "tototatatititomatiloupi"
	buf = bytes(fastlz.compress(lit))
	f = open("tes_lz.plz","wb")
	f.write(buf)
	f.flush()
	f.close()
	print("success " + str(len(buf)))
	
	test_read_lz()
	
def test_read_lz():
	f = open("tes_lz.plz", "rb")
	buf = f.read()
	buf = fastlz.decompress(buf)
	print(buf)

def lz_compress(inpath, outpath):
	fin = open(inpath, "rb")
	buf = fin.read();
	fin.close()
	bufc = fastlz.compress(buf)
	f = open(outpath, "wb")
	f.write(bytearray(bufc))
	f.flush()
	f.close()


def make_pvr(inpath,outpath):
	cur_sec = "settings"
	cur_entry = "pvr_tex_tool"
	pvr_tool = local_config.get(cur_sec, cur_entry, fallback='D:/PowerVR_Tools/PVRTexTool/CLI/Windows_x86_64/PVRTexToolCLI.exe')
	return subprocess.call([pvr_tool,"-i",inpath, "-o",outpath,"-f","b8g8r8a8"])

def test_pvr():
	make_pvr("res/monsters.png","res/monsters.pvr")

def make_one_pvr_lz(src):
	dst = os.path.splitext(src)[0] + ".pvr.plz"
	dstPvr = os.path.splitext(src)[0] + ".pvr"
	if not os.path.exists(dst):
		retcode = make_pvr(src, dstPvr)
		if retcode == 0:
			retcode = lz_compress(dstPvr, dst)
			os.remove(dstPvr)
		else:
			print("retcode:" + str(retcode))
	else:
		srcTime = os.path.getmtime(src)
		dstTime = os.path.getmtime(dst)
		if dstTime < srcTime:
			retcode = make_pvr(src, dstPvr)
			if retcode == 0:
				retcode = lz_compress(dstPvr, dst)
				os.remove(dstPvr)
			else:
				print("retcode:" + str(retcode))

def make_one_atlas(src):
	cur_sec = "settings"
	cur_entry = "texturepacker"
	tpacker_tool = local_config.get(cur_sec, cur_entry, fallback='C:/Program Files/CodeAndWeb/TexturePacker/bin/TexturePacker.exe')
	return subprocess.call([tpacker_tool, src])

def make_all_atlas():
	print("make_all_atlas")
	try:
		gl = glob.iglob("dt_art/export/**.tps", recursive=True)
		Pool().map(make_one_atlas, gl)
	except Exception as e:
		print("err:"+str(e))
	print("finished")
				
def make_all_pvr_lz():
	print("make_all_pvr")
	try:
		gl = glob.iglob("res/**/**.png", recursive=True)
		Pool().map(make_one_pvr_lz, gl)
	except Exception as e:
		print("err:"+str(e))
	print("finished")
	
def make_fmod():
	print("make_fmod")
	cur_sec = "settings"
	cur_entry = "fmodstudio"
	fmodstudio = local_config.get(cur_sec, cur_entry, fallback='C:/Program Files/FMOD SoundSystem/FMOD Studio 2.01.06/fmodstudio.exe')

	os.chdir(projectRoot)
	subprocess.call([fmodstudio,"-build", "dt_fmod_session/DT_Audio.fspro"])
	shutil.copytree(projectRoot + "/dt_fmod_session/Build/Desktop", projectRoot + "/res/fmod", dirs_exist_ok=True)

def mkdir_force(dst):
	if not os.path.exists(dst):
		os.mkdir(dst)
		
def make_gifs():
	print("make_gifs")
	cur_sec = "settings"
	cur_entry = "gifs_builder"
	os.chdir(projectRoot)
	
	
	libraries = ["assets","assetsNew","base","base_bg",
				 "character_herk","character_hyji","character_ikarus","character_phi",
				 "character_scout",
				 "log",
				 "logo",
				 "monsters",
				 "ui",
				 ];
	lib = "assets"
	srcDir = "dt_art/export/" + lib
	src = "alpha_attackBack*.png"
	dst = "dt_art/export/gif/"+lib+"/alpha_attackBack.gif"
	
	"convert -delay 20 -loop 0 *.jpg myimage.gif"
	mkdir_force("dt_art/gif")
	#mkdir_force("gfx/gif/assets")
	#os.system("tools\\convert.exe -delay 6.66 -loop 0 -alpha remove -background black gfx/export/assets/alpha_attackBack*.png gfx/export/gif/assets/alpha_attackBack.gif")
	#print("done")
	
	#libraries=  ["assets"]
	for lib in libraries:
		src = "dt_art/export/" + lib
		mkdir_force("dt_art/gif/"+lib)
		anims=[]
		for file in glob.glob("dt_art/export/" + lib+"/*.png"):
			#print("process "+file)
			ofile = file
			#todo insert date check here
			file = file.replace(".png", "");
			end = file[len(file)-1];
			#print(end)
			if( ( ord(end) < ord('0')) or ( ord(end) > ord('9')) ):
				#print("skipped "+file)
				continue;
			file = file.rstrip(string.digits);
			anims.append(file);
			print("added " + file)
		anims = list(set(anims))
		print(anims)
		for a in anims:
			#print(a)
			fname = os.path.basename(a)
			a = a + "*.png"
			d = "dt_art/gif/"+lib+"/"+fname+".gif"
			#print(a)
			#print(d)
			cvt="tools\\convert.exe"
			cvt += " -delay 6.66 "
			cvt += " -loop 0 "
			cvt += " -alpha remove "
			cvt += " -background black "
			cvt += a #a looks like blabla*.png
			cvt += " "
			cvt += d #d looks like blabla.gif
			os.system(cvt)
		print("done "+lib)
	print("done ALL" )

# noinspection PyAttributeOutsideInit
class App(tk.Frame):
	tabControl = None
	winLog = None
	winTab = None
	editorTab = None
	genericTab = None
	ggdriveTab = None
	settingsTab = None
	gg_builddest = ""
	winBuildTag = "Release"
	
	curMakeTool = ""
	
	def __init__(self, master=None):
		tk.Frame.__init__(self, master)
		
		self.createWidgets()
		self.grid()
	
	def make_tool(self):
		print("curMakeTool :" +self.curMakeTool)
		
		os.chdir(projectRoot)
		def beforeBuild():
			pass
		def afterBuild():
			print("after build");
			if( self.curMakeTool != ""):
				src = os.path.normpath(projectRoot+"/dev/bin/x64/Release/TestApp.exe").replace("\\\\","/")
				dest = src.replace("TestApp.exe", self.curMakeTool +".exe" )
				
				if (not os.path.exists(src)):
					trace("No valid TestAndTool.exe to copy")
				try:
					print("aliasing exe")
					shutil.copyfile(src, dest);
				except:
					print("error occured");
				#tooldest = self.getDestWinPath() + "/" + os.path.basename(dest)


				tooldest = projectRoot+"/tools/"+self.curMakeTool +".exe"

				print("copying exe to tool dir")
				#copy exe
				shutil.copyfile(dest, tooldest );

				dll_cmd = os.path.normpath(projectRoot + "/dev/bin/x64/Release/*.dll").replace("\\\\","/")
				#dll_cmd = pathToCyg(dll_cmd)
				print("sincing dll from "+dll_cmd)

				dll_dst = os.path.normpath(projectRoot+"/tools/")
				#dll_dst = pathToCyg(dll_dst)
                
				print("to "+dll_dst)
				#copy dll
				#proc = subprocess.Popen(["rsync", "-ar", "--del", "--force", dll_cmd, dll_dst])
				#rsync(dll_cmd, dll_dst)
				for file in glob.glob(dll_cmd):
					rsync(file, dll_dst)

				trace("Tool created at "+tooldest)
			else:
				print("no name")
		beforeBuild();
		afterBuild();
		

	def createEditorTab(self,frame):
		for t in tools:
			if(len(t) == 0):
				sep = ttk.Separator(frame)
				sep.grid(sticky=tk.W,column=0,columnspan=4, ipadx=100)
			#sep.pack(fill='x')
			else:
				tool = tk.Button(frame)
				line = t
				lbd = lambda line=line: line[1](line)
				tool.config(text=t[0], command=lbd)
				tool.grid(sticky=tk.W)

		
	def createGenericTab(self,frame):
		self.testLz = testLz = tk.Button(frame)
		testLz.config(text="[DEBUG] test lz", command=test_lz)
		testLz.grid(sticky=tk.W)
		
		self.testPvr = testPvr = tk.Button(frame)
		testPvr.config(text="[DEBUG] test pvr", command=test_pvr)
		testPvr.grid(sticky=tk.W)
		
		self.btBuildAtlas = btBuildAtlas = tk.Button(frame)
		btBuildAtlas.config(text="make atlases", command=make_all_atlas)
		btBuildAtlas.grid(sticky=tk.W)
		
		self.btBuildPvrLz = btBuildPvrLz = tk.Button(frame)
		btBuildPvrLz.config(text="make .pvr.plz", command=make_all_pvr_lz)
		btBuildPvrLz.grid(sticky=tk.W)
		
		self.btBuildFMOF = btBuildFMOF = tk.Button(frame)
		btBuildFMOF.config(text="make fmod bank", command=make_fmod)
		btBuildFMOF.grid(sticky=tk.W)
		
		self.btBuildGIFS = btBuildGIFS = tk.Button(frame)
		btBuildGIFS.config(text="make gifs", command=make_gifs)
		btBuildGIFS.grid(sticky=tk.W)
	
	def createGGDriveTab(self, frame):
		cur_sec = "gg"
		cur_entry = "buildDir"
		gg_drive_builddir = tk.Label(frame, text="google drive build dir:")
		gg_drive_builddir.grid(sticky=tk.W)

		def get():
			return local_config.get(cur_sec, cur_entry, fallback='D:\Drive')
		self.gg_builddest = gg_builddest = get()
		msbString = tk.StringVar(root)
		msbString.set(gg_builddest)
		def local(sv):
			local_config.set(cur_sec, cur_entry, sv.get())
		msbString.trace("w", lambda name, index, mode, sv=msbString:local(sv))
		gg_build_field = tk.Entry(frame, textvariable=msbString, width=editSize)
		gg_build_field.grid(sticky=tk.W, padx=0)
		
		self.btExplore = btExplore = tk.Button(frame)
		btExplore.config(text="Explore", command=lambda:os.startfile(os.path.normpath(get())))
		btExplore.grid(sticky=tk.W)
	
	def createSettingTab(self, frame):
		cur_sec = "settings"
		entry_pvr = "pvr_tex_tool"
		pvr_tool_label = tk.Label(frame, text="PVRTex CLI (ex: PVRTexToolCLI.exe) :");
		self.pvr_tool_exe = pvr_tool_exe = local_config.get(cur_sec, entry_pvr, fallback='D:/PowerVR_Tools/PVRTexTool/CLI/Windows_x86_64/PVRTexToolCLI.exe')
		pvr_tool_str = tk.StringVar(root)
		pvr_tool_str.set( pvr_tool_exe )
		pvr_tool_str.trace("w", lambda name, index, mode, sv= pvr_tool_str: local_config.set(cur_sec, entry_pvr, sv.get()))
		pvr_tool_field = tk.Entry(frame, textvariable=pvr_tool_str, width=editSize)
		pvr_tool_field.grid(sticky=tk.W, padx=0)

		entry_fmod = "fmodstudio"
		fmod_lable = tk.Label(frame, text="FMOD Studio executable (ex: fmodstudio.exe) :");
		self.fmod_exe = fmod_exe = local_config.get(cur_sec, entry_fmod, fallback='C:/Program Files/FMOD SoundSystem/FMOD Studio 2.01.06/fmodstudio.exe')
		fmod_tool_str = tk.StringVar(root)
		fmod_tool_str.set( fmod_exe )
		fmod_tool_str.trace("w", lambda name, index, mode, sv= fmod_tool_str: local_config.set(cur_sec, entry_fmod, sv.get()))
		fmod_tool_field = tk.Entry(frame, textvariable=fmod_tool_str, width=editSize)
		fmod_tool_field.grid(sticky=tk.W, padx=0)

		entry_tPacker = "texturepacker"
		fmod_lable = tk.Label(frame, text="Texture Packer executable (ex: TexturePacker.exe) :");
		self.tPacker_exe = tPacker_exe = local_config.get(cur_sec, entry_tPacker, fallback='C:/Program Files/CodeAndWeb/TexturePacker/bin/TexturePacker.exe')
		tPacker_str = tk.StringVar(root)
		tPacker_str.set( tPacker_exe )
		tPacker_str.trace("w", lambda name, index, mode, sv= tPacker_str: local_config.set(cur_sec, entry_tPacker, sv.get()))
		tPacker_field = tk.Entry(frame, textvariable=tPacker_str, width=editSize)
		tPacker_field.grid(sticky=tk.W, padx=0)
		
	def getDestWinPath(self):
		vt = repo_config.get("win", "vertag", fallback='alpha_0')
		final_dest = os.path.realpath(self.gg_builddest) + "/win64/" + self.winBuildTag + "/" + vt
		return final_dest
		
	def onWinToGGSyncPressed(self):
		global syncing
		cls()
		if (syncing):
			cls()
			trace("already syncing")
			return
		
		syncing = True
		
		now = datetime.now()
		date_tag = now.strftime("_%Y_%m_%d__%H_%M_%S")
		vt = repo_config.get("win", "vertag", fallback='alpha_0')
		final_dest = os.path.realpath(self.gg_builddest) + "/win64/" + self.winBuildTag + "/" + vt
		dst = os.path.normpath(final_dest)
		src = os.path.normpath(projectRoot + "/build/win64/*")
		
		print(trace("syncing " + src + " to " + final_dest))
		
		# win64/release/date/monbuild
		if not os.path.isdir(dst):
			os.makedirs(dst)
		
		print("sending to " + dst)
		
		# delete unused file in build
		#src = pathToCyg(src)
		#dst = pathToCyg(dst)
		print("normalized for rsync src:"+src+" dst:"+dst)
		
		gg_log = open(scriptPath + "/gg.txt", "w", encoding='utf-8')
		gg_log.write("opening gg sync process\\r\\n")
		gg_log.flush()
		
		trace("syncing...")
		trace("src:"+src)
		trace("dst:"+dst)

		#proc = subprocess.Popen(["rsync", "-ar", "--del", "--force", src, dst],stderr=gg_log,stdout=gg_log)
		for file in glob.glob(src):
			rsync(file, dst)

		def oncomplete():
			global syncing
			proc.poll()
			if (proc.returncode is None):
				self.after(100, oncomplete)
			else:
				gg_log.flush()
				gg_log.close()
				syncing = False
				if (proc.returncode == 0):
					trace("sync done ")
				else:
					trace("sync failed "+str(proc.returncode))
					
					ggErr =open(scriptPath + "/gg.txt", "r", encoding='utf-8')
					trace( ggErr.read() )
		
		#self.after(100, oncomplete)
		trace("sync done ")
	
	def createWidgets(self):
		cur_sec = "win"
		
		tabControl = ttk.Notebook(self)
		self.editorTab = editorTab = tk.Frame(tabControl)
		self.genericTab = genericTab = tk.Frame(tabControl)
		self.winTab = winTab = tk.Frame(tabControl)
		self.ggdriveTab = ggdriveTab = tk.Frame(tabControl)
		self.settingsTab = settingsTab = tk.Frame(tabControl)
		tabControl.add(editorTab, text='Editor')
		tabControl.add(genericTab, text='Generic')
		tabControl.add(ggdriveTab, text='GDrive')
		tabControl.add(winTab, text='Win64')
		tabControl.add(settingsTab, text='settings')
		self.createEditorTab(editorTab)
		self.createGenericTab(genericTab)
		self.createGGDriveTab(ggdriveTab)
		self.createSettingTab(settingsTab)
		tabControl.grid(sticky=tk.W)
		
		sz = winTab.grid_size()
		
		###########
		# msbuild
		cur_entry = "vertag"
		winVerTag = tk.Label(winTab, text="Version tag:")
		winVerTag.grid(sticky=tk.W)
		vt_var = tk.StringVar()
		vt_var.set(repo_config.get(cur_sec, cur_entry, fallback='alpha_0'))
		vt_var.trace("w", lambda name, index, mode, sv=vt_var: repo_config.set(cur_sec, cur_entry, vt_var.get()))
		ms_vt_field = tk.Entry(winTab, textvariable=vt_var, width=editSize)
		ms_vt_field.grid(sticky=tk.W, padx=0)
		
		msBuildDir = tk.Label(winTab, text="MsBuild dir:")
		msBuildDir.grid(sticky=tk.W)
		msBuildWinDest = local_config.get("win", 'msbuild', fallback='Z:\\VSS_2019\\MSBuild\\Current\\Bin\\MSBuild.exe')
		msbString = tk.StringVar(root)
		msbString.set(msBuildWinDest)
		msbString.trace("w", lambda name, index, mode, sv=msbString: local_config.set("win", 'msbuild', sv.get()))
		msbuildField = tk.Entry(winTab, textvariable=msbString, width=editSize)
		msbuildField.grid(sticky=tk.W, padx=0)
		
		###########
		# build dests
		buildDestLabel = tk.Label(winTab, text="Win build dest dir:")
		buildDestLabel.grid(sticky=tk.W)
		
		myWinDest = local_config.get("win", 'dest', fallback='')
		myString = tk.StringVar(root)
		myString.set(myWinDest)
		buildDestField = tk.Entry(winTab, textvariable=myString, width=editSize)
		buildDestField.grid(sticky=tk.W, padx=0)
		onexit.append(lambda: local_config.set("win", "dest", myString.get()))
		
		#self.btMakeWinDebugRes  = btMakeWinDebugRes = tk.Button(winTab)
		#btMakeWinDebugRes.config(text="Make win64 debug res", command=self.onMakeDebugResPressed)
		#btMakeWinDebugRes.grid(sticky=tk.W)
		
		self.btBuildWinBuild = btBuildWinBuild = tk.Button(winTab)
		btBuildWinBuild.config(text="Compile win64 & Gather", command=self.onMakePressed)
		btBuildWinBuild.grid(sticky=tk.W)
		
		self.btGatherWin64 = btGatherWin64 = tk.Button(winTab)
		btGatherWin64.config(text="Gather win64 build", command=self.onGatherPressed)
		btGatherWin64.grid(sticky=tk.W)
		
		self.btSyncToGGWin64 = btSyncToGGWin64 = tk.Button(winTab)
		btSyncToGGWin64.config(text="Sync win64 build to GDrive", command=self.onWinToGGSyncPressed)
		btSyncToGGWin64.grid(sticky=tk.W)
		
		#####
		tool_name = tk.Label(winTab, text="tool name")
		self.btMakeTool = btMakeTool = tk.Button(winTab)
		btMakeTool.config(text="make tool", command=self.make_tool)
		msbString = tk.StringVar(root)
		msbString.set(self.curMakeTool)
		
		def assignCMT(msb):
			self.curMakeTool = msb.get()
		
		msbString.trace("w", lambda name, index, mode, sv=msbString: assignCMT(msbString))
		tool_name_field = tk.Entry(winTab, textvariable=msbString, width=editSize)
		tool_name_field.grid(sticky=tk.W)
		#####
		btMakeTool.grid(sticky=tk.W)
		
		global log
		self.winLog = log = tk.Text(winTab)
		log.configure(state="disabled", height=20, width=int(editSize * 0.8))
		log.grid(sticky=tk.W)
		
		
	
	# self.btPremake = tk.Button(self)
	# self.btPremake.config(text="Premake");
	# self.btPremake.grid(column=0, sticky=tk.W);
	
	def doCompileWin64(self):
		# retarget base path
		cwd = os.getcwd()
		if cwd.endswith("bat"):
			os.chdir("..")
		cwd = os.getcwd()
		trace("running in " + cwd)
		trace("cleaning")
		try:
			os.chdir("dev/bin/x64/" + self.winBuildTag)
			gl = glob.glob("*.exe")
			for f in gl:
				os.remove(f)
		except:
			pass
		os.chdir(cwd)
		trace("compiling in " + cwd)
		msBuildExe = local_config.get("win", 'msbuild', fallback='')
		if (msBuildExe == ''):
			cls()
			trace("ERROR getting msbuild path")
			return
		buildLog = open(scriptPath + "/buildLog.txt", "w", encoding='utf-8')
		buildLog.write("opening compile process\\r\\n")
		buildLog.flush()
		os.chdir("dev")
		
		def compile():
			global compileProc
			compileProc = subprocess.Popen(
				[msBuildExe, "Project/VisualStudio/DT.sln", "/p:Configuration=" + self.winBuildTag, "-m:4"],
				bufsize=1024, stderr=buildLog, stdout=buildLog)
			compileProc = subprocess.Popen(
				[msBuildExe, "Project/VisualStudio/Test.sln", "/p:Configuration=" + self.winBuildTag, "-m:4"],
				bufsize=1024, stderr=buildLog, stdout=buildLog)
			trace("compile in progress...")
		
		compile()
	
	def doAssembleWin64(self, after):
		os.chdir(projectRoot)
		trace("launching compile thread")
		threading.Thread(target=self.doCompileWin64).start()
		
		def waitToGather():
			global compileProc
			if (compileProc is not None):
				compileProc.poll()
				if (compileProc.returncode is not None):
					trace("compile finished")
					global compiling
					compiling = False
					if (compileProc.returncode == 0):
						global gathering
						gathering = True
						self.doGatherWin64( after )
					else:
						trace("error in building, see BuildLog.txt");
						path = scriptPath + "/BuildLog.txt";
						print("trying to print "+path);
						f = open(path,encoding="utf-8");
						try:
							trace(f.read())
						except:
							pass
						f.close()
				else:
					root.after(300, waitToGather)
			else:
				root.after(300, waitToGather)
		
		root.after(300, waitToGather)
	
	def doGatherWin64(self, after):
		os.chdir(projectRoot)
		print("gathering in " + projectRoot)
		gatherLog = open(scriptPath + "/gatherLog.txt", "w", encoding='utf-8')
		gatherLog.write("opening gather process\\r\\n")
		gatherLog.flush()
		
		dst = "build/win64"
		if not os.path.exists(dst):
			os.mkdir(dst)
		trace("gathering to " + dst)
		
		# rsync exe
		exeCmd = [["dev/bin/x64/Release/DTApp.exe", dst]]
		dllCmd = [["dev/bin/x64/Release/*.dll", dst]]
		resCmd = [["res/", dst + "/res/"],
				  ["res_platform/win/", dst + "/res/"]]
		trace("sync in progress")
		subs = []
		
		def syncProc(cmds):
			for src, dst in cmds:
				try:
					trace("syncinc " + src)
					sub = subprocess.Popen(["rsync", "-ar", "--force", src, dst])
					subs.append(sub)
				except:
					msg = sys.exc_info()[0];
					trace(msg)
					gatherLog.write("failure detected:" + msg)
		
		syncProc(exeCmd)
		syncProc(dllCmd)
		
		if (not os.path.exists(dst + "/res")):
			os.mkdir(dst + "/res")
		syncProc(resCmd)
		
		# rsync lib/dll
		# rsync generic ressources
		# rsync win ressources
		# rsync redists
		fdst = os.path.normpath(projectRoot + "/" + dst)
		
		def finished():
			trace("gathered at " + str(datetime.now()) + " to " + fdst)
			gatherLog.flush()
			gatherLog.close()
			global gathering
			gathering = False
			after()
		
		def checkFinished():
			for s in subs:
				s.poll()
				if s.returncode is None:
					root.after(100, checkFinished)
					return
			finished()
		
		root.after(100, checkFinished)
		
	
	def onMakeDebugResPressed(self,onEnd=None):
		os.chdir( projectRoot )
		
		cls()
		trace("syncing debug res")
		procs=[]
		
		src="res/*"
		dst="dev/res/"
		
		procs.append(subprocess.Popen(["rsync", "-ar", "--force", src, dst]))
		
		src="res_platform/win/*"
		dst="dev/res/"
		
		procs.append(subprocess.Popen(["rsync", "-ar", "--force", src, dst]))
		
		def checkFinished():
			for proc in procs:
				proc.poll()
				if proc.returncode is None:
					root.after(100, checkFinished)
					return
			trace("synced")
			if onEnd is not None:
				onEnd()
		root.after(100, checkFinished)

		
	def onMakePressed(self):
		global compiling
		if (compiling):
			trace("Error : compile in progress")
			return
		compiling = True
		cls()
		self.doAssembleWin64( self.nope )
	
	def nope(self):
		pass
	
	def onGatherPressed(self):
		global compiling
		global gathering
		if (compiling):
			trace("Error : compile in progress")
			return
		if (gathering):
			trace("Error : gather in progress")
			return
		gathering = True
		cls()
		self.doGatherWin64( self.nope )


##end class
######################################

def onExit():
	for task in onexit:
		task()
	
	with open(scriptPath + "/config.ini", "w") as outfile:
		local_config.write(outfile)
	with open(scriptPath + "/repo_config.ini", "w") as outfile:
		repo_config.write(outfile)
	print("config written")


try:
	local_config.read_file(open(scriptPath + '/config.ini'))
except:
	print("no local config")

try:
	repo_config.read_file(open(scriptPath + '/repo_config.ini'))
except:
	print("no repo config")

for conf in [local_config, repo_config]:
	for sec in ["win", "gg", "settings"]:
		if not sec in conf.sections():
			conf.add_section(sec)

# configure project root
cwd = os.getcwd()
if cwd.endswith("bat"):
	os.chdir("..")
	cwd = os.getcwd()
projectRoot = os.path.realpath(cwd)

def onMenuBt( line ):
	print("trying to exec yo ?" + str(line))
	os.chdir(projectRoot + "/")
	subprocess.call([projectRoot+"/tools/"+line[2]] + line[3])
	os.chdir(projectRoot )

def onDevRes( line ):
	def onEnd():
		mb.showinfo("Finished","Resources succesfully synced")
	myApp.onMakeDebugResPressed(onEnd)
    
def onRunDebugBuild( line ):
	print("trying to exec debug build")
	os.chdir(projectRoot + "/")
	subprocess.call([projectRoot+"/dev/bin/x64/debug/TestApp.exe"])
	os.chdir(projectRoot )
	
def onSyncText( line ):
	def onEnd():
		mb.showinfo("Finished","Texts fetched in /res")
	#thanks https://curl.trillworks.com/#python
	
	list = [
		[(('gid', '0'),('format', 'tsv')),  			"13EQd7vZ3sQ2Tpn9fD-aY0yCx3_PrS8hJxJPcmU-g0wE","/res/txt/ui.tsv"],
		[(('gid', '1625938547'),('format', 'tsv')),  	"13EQd7vZ3sQ2Tpn9fD-aY0yCx3_PrS8hJxJPcmU-g0wE","/res/txt/quips.tsv"],
		[(('gid', '1962917645'),('format', 'tsv')),  	"13EQd7vZ3sQ2Tpn9fD-aY0yCx3_PrS8hJxJPcmU-g0wE","/res/txt/tuto.tsv"],
	]
	for ac in list:
		params = ac[0]
		response = requests.get('https://docs.google.com/spreadsheets/d/'+ac[1]+'/export', params=params)
		with open(projectRoot + ac[2], "wb") as outfile:
			outfile.write( response.content )
		response.close();
	onEnd()
	
def onSyncGGData( line ):
	#thanks https://curl.trillworks.com/#python
	
	list = [
		[(('gid', '1539909901'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Classes.tsv"],
		[(('gid', '1907103503'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Actions.tsv"],
		[(('gid', '848435456'),('format', 'tsv')),   "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Weapons.tsv"],
		[(('gid', '2117902398'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Entities.tsv"],
		[(('gid', '1372632286'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Kernels.tsv"],
		[(('gid', '1466269767'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Chipsets.tsv"],
		[(('gid', '1250190897'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Items.tsv"],
		[(('gid', '307715291'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Biomes.tsv"],
        [(('gid', '1250190897'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/AttackSpec.tsv"],
        [(('gid', '28538680'),('format', 'tsv')),  "1-gen7FdLUyRLwHt_v9pwyNhxf7mwbjPZs7dDkhIdJAQ","/res/data/Values.tsv"],
	]

	def onEnd():
		mb.showinfo("Finished","Data fetched in /res/data")
		pass

	for ac in list:
		params = ac[0]
		print("reading ac "+str(ac[0])+" => "+str(ac[2]))
		response = requests.get('https://docs.google.com/spreadsheets/d/'+ac[1]+'/export', params=params)
		with open(projectRoot + ac[2], "wb") as outfile:
			outfile.write( response.content )
		response.close();
	onEnd()

tools = [
	["Fetch GG Texts", onSyncText, ""],
	["Fetch GG Data", onSyncGGData, ""],
	#["Sync Dev Res", onDevRes,""],
	[],
	[],
	["Menu Editor", onMenuBt, "editor.exe", ["--tool", "menu"]],
	["Room Editor", onMenuBt, "editor.exe", ["--tool", "room"]],
	["Entity Editor", onMenuBt, "editor.exe", ["--tool", "entity"]],
	["BG Editor", onMenuBt, "editor.exe", ["--tool", "bg"]],
    ["Anim Editor", onMenuBt, "editor.exe", ["--tool", "anim"]],
	[],
	[],
	["Run Proto", onMenuBt, "proto", []],
	["Run Debug Build", onRunDebugBuild, ""]
]

def main():
	print("Project Man running")
	print(scriptPath)
	print("project root set to: " + projectRoot)

	# create the application
	myApp = App(root)

	#
	# here are method calls to the window manager class
	#
	myApp.master.title("Dt Project manager")
	myApp.master.maxsize(1280, 720)
	myApp.master.minsize(800, 600)

	# start the program
	myApp.mainloop()
	onExit()

if __name__ == '__main__':
	print("elevating")
	res = elevate(main)
	if( not res ):
		print("res:" + res)
