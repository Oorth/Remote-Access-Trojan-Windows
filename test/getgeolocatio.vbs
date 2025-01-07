
Dim http, response, ipAddressToLookup

' Set the IP address directly here (for testing purposes, you can change it to any IP)
ipAddressToLookup = "49.36.102.230" ' Example: Google Public DNS

' Create HTTP object to send a GET request
Set http = CreateObject("MSXML2.ServerXMLHTTP.6.0")
http.open "GET", "http://ipinfo.io/" & ipAddressToLookup & "/json", False
http.setRequestHeader "Content-Type", "application/json"
http.send

WScript.Echo http.responseText
Set http = Nothing

WScript.Quit
