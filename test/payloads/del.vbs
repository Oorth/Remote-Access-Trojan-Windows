Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objShell = CreateObject("Shell.Application")
Set objWShell = CreateObject("WScript.Shell")

' Declare variables
Dim objFile, strLine, arrParts, strMainDir, strFilePath

' Get the script's directory
Dim strScriptPath
strScriptPath = WScript.ScriptFullName
strScriptPath = Left(strScriptPath, InStrRev(strScriptPath, "\"))
strFilePath = strScriptPath & "details.txt"

Set objFile = objFSO.OpenTextFile(strFilePath, 1) ' 1 = ForReading

' Read the file line by line
Do Until objFile.AtEndOfStream
    strLine = objFile.ReadLine

    ' Check if the line contains "Main Directory:"
    If InStr(1, strLine, "Main Directory:", vbTextCompare) > 0 Then
        arrParts = Split(strLine, ": ")
        strMainDir = Trim(arrParts(1))
        Exit Do ' Exit the loop once the directory is found
    End If
Loop
objFile.Close

strTempDir = objFSO.GetSpecialFolder(2).Path
strNewFilePath = strTempDir & "\del_temp.vbs"
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.CreateTextFile(strNewFilePath, True)

'\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

objFile.WriteLine "Set objFSO = CreateObject(""Scripting.FileSystemObject"")"
objFile.WriteLine "Dim strFolderToDelete"
objFile.WriteLine "strFolderToDelete = WScript.Arguments(0)"
objFile.WriteLine "WScript.Sleep 1000"
objFile.WriteLine "objFSO.DeleteFolder strFolderToDelete" ' Force deletion

objFile.Close

'\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


' Now, execute the newly created script and pass the data
Dim strCommand
strCommand = "wscript.exe """ & strNewFilePath & """ """ & strMainDir & """"
objWShell.Run strCommand, 0, False ' 0 = Hide window, True = Wait for completion

WScript.Quit
' ' Schedule the deletion of the script after a delay
' strScriptPath = WScript.ScriptFullName
' objShell.ShellExecute "cmd.exe", "/C del """ & strScriptPath & """", "", "", 0

Set objFile = Nothing
Set objFSO = Nothing
Set objShell = Nothing
