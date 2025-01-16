'assignes our wscript shell to our 'ws' object
set ws = CreateObject("wscript.shell")

'run box
ws.run("cmd")
wscript.sleep(248)
'inject keystrokes
ws.sendkeys("Hello World!")

