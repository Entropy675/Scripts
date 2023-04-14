set a = createobject("wscript.shell")

Set args = Wscript.Arguments

wscript.sleep(3000)
a.run "AutoHotkey.exe mousemovement.ahk"