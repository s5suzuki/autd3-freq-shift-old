Remove-Item *.jou
Remove-Item *.log
Remove-Item *.zip
Remove-Item *.prm
Remove-Item *.str
Remove-Item *.mcs
Remove-Item .\autd3-freq-shift-fpga.cache -Recurse -ErrorAction SilentlyContinue
Remove-Item .\autd3-freq-shift-fpga.gen -Recurse -ErrorAction SilentlyContinue
Remove-Item .\autd3-freq-shift-fpga.hw -Recurse -ErrorAction SilentlyContinue
Remove-Item .\autd3-freq-shift-fpga.runs -Recurse -ErrorAction SilentlyContinue
Remove-Item .\autd3-freq-shift-fpga.sim -Recurse -ErrorAction SilentlyContinue
Remove-Item .\.Xil -Recurse -ErrorAction SilentlyContinue
