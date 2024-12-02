#Builds resources for the rat


return -join ((65..90) + (97..122) + (48..57) | Get-Random -Count 10 | %{[char]$_})

