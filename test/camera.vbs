' Requires a webcam and associated software/drivers installed.
' This script uses the Windows Imaging Component (WIC) which is available on Windows Vista and later.
' It also requires a reference to the "Microsoft Shell Controls And Automation" library.

' Check if WIC is available (Windows Vista or later)
On Error Resume Next
Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
Set colOperatingSystems = objWMIService.ExecQuery("Select * from Win32_OperatingSystem")
For Each objOperatingSystem in colOperatingSystems
    If InStr(1, objOperatingSystem.Caption, "Windows 95", vbTextCompare) > 0 Or _
       InStr(1, objOperatingSystem.Caption, "Windows 98", vbTextCompare) > 0 Or _
       InStr(1, objOperatingSystem.Caption, "Windows ME", vbTextCompare) > 0 Or _
       InStr(1, objOperatingSystem.Caption, "Windows NT", vbTextCompare) > 0 Or _
       InStr(1, objOperatingSystem.Caption, "Windows 2000", vbTextCompare) > 0 Or _
       InStr(1, objOperatingSystem.Caption, "Windows XP", vbTextCompare) > 0 Then
       MsgBox "This script requires Windows Vista or later.", vbCritical
       WScript.Quit
    End If
Next

On Error Goto 0

' Create Shell.Application object
Set objShell = CreateObject("Shell.Application")

' Get the Camera folder. This is the key part that makes this work on modern Windows.
Set objFolder = objShell.Namespace(&H14&) ' &H14& is the constant for "My Computer"

' Find the camera device. This might need adjustment depending on how the camera is exposed.
' This example assumes the camera is a direct child of "This PC".
Set objFolderItem = Nothing
For Each item In objFolder.Items
    If InStr(1, item.Name, "Camera", vbTextCompare) > 0 Then ' Adjust "Camera" if needed
        Set objFolderItem = item
        Exit For
    End If
Next

If objFolderItem Is Nothing Then
    MsgBox "Camera not found.", vbCritical
    WScript.Quit
End If

' Execute the "Take a photo" command. This relies on the camera's driver/software.
' This is the most reliable method on modern Windows, as it uses the built-in functionality.
On Error Resume Next ' In case the command isn't available
objFolderItem.InvokeVerbEx "TakePicture" ' Or "TakePhoto" - try both
If Err.Number <> 0 Then
    MsgBox "Could not take a picture. Error: " & Err.Description, vbCritical
    WScript.Quit
End If
On Error Goto 0

WScript.Echo "Picture taken (hopefully!). Check your Pictures library or the Camera Roll folder."

Set objFolderItem = Nothing
Set objFolder = Nothing
Set objShell = Nothing
Set objWMIService= Nothing
Set colOperatingSystems= Nothing
Set objOperatingSystem = Nothing

WScript.Quit