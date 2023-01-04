@echo OFF
for /r %%f in (..\res\generic\*.png) do (
	convert.exe %%f PNG32:%%f
)

PAUSE
