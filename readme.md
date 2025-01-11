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

It is a Remote Access Trojan made by me for Windows environment. Its staged nature allows for covert control of compromised machines, and uses a custom communication protocal which can connect two endpoints hidden behind NAT networks to communicate over internet with an intermediate server preventing direct connections to the attacker, thus hindering traceback efforts.
>~~No one can find you :) thank me later~~

It is undetectable by any standard Antivirus. This Malware is written in C++ which allows it to execute with great access to the memory and hence providing the potential for efficient execution, low resource usage, and access to low-level system functions.

## Requirements

### Target PC :
* Windows 10 / 11
* Internet Connection
### Attacker PC :
* Windows 10 / 11
* Internet Connection
>sweet right  :)

## Payloads

####   **1) Reverse_shell** 
>This custom reverse shell is coaded from scratch and uses just bare minimun to work, this helps it have a lesser footprint and be stealthy, use it well <3

####   **2) Enumerate target [Under development]** 
>A very powerful tool to get a very very detailed information of the target Os, User, Hardware as well as approx geographic coordinates, ~~Empowering you to go say hi personally.~~.

####   **3) A very Advanced Keylogger [Under development]** 
>At this point you know how it goes, made in c++.. from scratch... BUT this Keylogger also catures the mouse click and position, also captures the active window and also takes screenshots on set conditions ~~even in incognito.~~

####   **4) Webcam Capture [Under development]** 
>You better smile.

####   **5) Block Keyboard and mouse [Upcomming]** 
>Cause why not

####   **6) Screen Capture [Upcomming]** 
>yeah

## Installation

### Attacker:
```markdown
yeah just run the .exe
```
### Target:
```markdown

	for now just make the target run initial.cmd
in future most probably shellcode embedded in an .exe
```
## The End
So people have fun stay safe, If you have further ideas for payloads go on I am all Ears.

