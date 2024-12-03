
set ws = CreateObject("wscript.shell")

dim i

ws.run("chrome")
wscript.sleep(248)

ws.sendkeys("^l")
wscript.sleep(248)
ws.sendkeys("https://ecm.smtech.in/ecm/login.aspx")
wscript.sleep(248)
ws.sendkeys("{ENTER}")
wscript.sleep(1000)
ws.sendkeys("UR Name")
wscript.sleep(248)
ws.sendkeys("{TAB}")
wscript.sleep(248)
ws.sendkeys("Your Pass")
wscript.sleep(248)
ws.sendkeys("{TAB}")
wscript.sleep(248)
ws.sendkeys("{ENTER}")
wscript.sleep(3000)
ws.sendkeys("{ENTER}")