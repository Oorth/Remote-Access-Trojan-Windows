# Da RAT
> By **Oorth**
<pre align="center">
      :::::::::      :::           :::::::::      ::: :::::::::::
     :+:    :+:   :+: :+:         :+:    :+:   :+: :+:   :+:
    +:+    +:+  +:+   +:+        +:+    +:+  +:+   +:+  +:+
   +#+    +:+ +#++:++#++:       +#++:++#:  +#++:++#++: +#+
  +#+    +#+ +#+     +#+       +#+    +#+ +#+     +#+ +#+
 #+#    #+# #+#     #+#       #+#    #+# #+#     #+# #+#
#########  ###     ###       ###    ### ###     ### ###
</pre>

## !! Disclaimer !!
This Remote Access Trojan (RAT) is intended solely for ethical security research and educational purposes. Its use is restricted to systems you own or have explicit authorization to test. Deploying this RAT on unauthorized systems is illegal and carries severe legal and ethical consequences.

Furthermore, due to its remote access capabilities, improper use or configuration can create security vulnerabilities, potentially exposing systems to further compromise. I am not responsible for any misuse or damage caused by this tool.
>If you break the law, that's your problem, not mine. But if you manage to take down a major corporation, I expect a cut of the profits.

## Overview

It is a Remote Access Trojan made by me for Windows environment. Its staged and fileless (The actual malware never touches the disk and gets executed from the memory) nature allows for covert control of compromised machines, and uses a custom communication protocal which can connect two endpoints hidden behind NAT networks to communicate over internet with an intermediate server preventing direct connections to the attacker, thus hindering traceback efforts.
>~~No one can find you :) thank me later~~

It is undetectable by any standard Antivirus. This Malware is written in C++ which allows it to execute with great access to the memory and hence providing the potential for efficient execution, low resource usage, and access to low-level system functions.

## Working
So this RAT is staged in nature, it consists of 3 main stages and different sub-stages depending on the payload.
```markdown
 	1) initial.cmd
 	2) module_downloader.vbs
  	3) target_script.exe
```
> ik target_script.exe is not a script, I am just too ~~lazy~~ to rename it

1) The initial.cmd is a single line command which when executed will download the stage 2 ie. module_downloader.vbs ( as initial is a single line command it provides flexibility designing the delivery mechanism )

2) The module_downloader creates a home directory for the malware in the temp directory with a random name and downloads target_script along with some dependencies and adds it to exclusions (will ditch this method soon as the downloaded files are not malicious.. )
> ik I said its fileless and it is, all the malicious code never touches the disk. We are going to talk about it

3) Now the target_script connects with the C2 server and the game begins :)
4) Now the attacker can use the console.exe to get a revershell and access to different payloads..

```markdown
The RAT also benifits from having a modular code, the RAT uses various dlls which share code with different modules,
this makes the individual malicious modules lightweight, also providing an option to reflectively load those dlls
which makes the RAT more stealthy...
```
## Evasion

### The RAT uses various methods to evade Static as well as Dynamic analysis such as
#### Static analysis Evasion ->
```markdown
>   1) Dynamic function linking
>   2) Reflective dll loading			[https://github.com/Oorth/Custom_MemoryModule]
>   3) Obsfuscation techniques
```
#### Dynamic analysis Evasion ->
```markdown
>   1) Debugger Evasion				[https://github.com/Oorth/Debugger_Evasion]
>   2) Api Hooking				[ Under development ]
```

## Payloads

####   **1) Reverse_shell** 
>This custom reverse shell is coded from scratch and using pipes and sockets, this helps it have a lesser footprint and be stealthy, use it well <3

####   **2) Enumerate target			[Under development]** 
>A very powerful tool to get a very very detailed information of the target Os, User, Hardware as well as approx geographic coordinates, ~~Empowering you to go say hi personally.~~.

####   **3) A very Advanced Keylogger**		[https://github.com/Oorth/Keylogger]
>Made in c++ this keylogger uses a C2 server and captures keystrokes, mouse clicks and co-ordinates, also captures the active window ~~even in incognito.~~

## Requirements

### Target PC :
* Windows 10 / 11
* Internet Connection
### Attacker PC :
* Windows 10 / 11
* Internet Connection
>sweet right  :)


## Usage

### Attacker:
```markdown
 run the console.exe
```
### Target:
```markdown
	for now just make the target run initial.cmd
	but in future I will develope a delivery mechanism, stay tuned
```
## The End
So people have fun stay safe, If you have further ideas for payloads go on I am all ears.

