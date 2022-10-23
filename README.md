# **ISA VUT FIT 2022/23**

### Name of the project: Tunelování datových přenosů přes DNS dotazy
### Author: Dalibor Králik (xkrali20) 
### Date: 23.10.2022<br/><br/>
## Description
### Project restrictions:
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; This project doesn't support IPv6.

## How to run the application
1. &nbsp; Open terminal in the root folder of the project and type command<br/> 
&nbsp; `$make` 
<br/><br/>
2. &nbsp; `dns_receiver {BASE_HOST} {DST_DIRPATH}` 
<br/>&nbsp; In the first terminal in the root folder and type command<br/><br/>
 &nbsp; `$sudo ./dns_receiver example.com ./data`<br/>
<br/> &nbsp; Where example.com is {BASE_HOST} and ./data is {DST_PATH} 
<br/> -> **{BASE_HOST}** serves to set up base domain for receiving DNS packets
<br/> -> **{DST_DIRPATH}** serves to set up path for the folder/file with received data <br/><br/>
3. &nbsp; `dns_sender [-u UPSTREM_DNS_IP] {BASE_HOST} {DST_FILEPATH} [SRC_FILEPATH]`<br/> &nbsp; Open second terminal in the root folder and type command<br/><br/>
&nbsp; `dns_sender -u 127.0.0.1 example.com output/data.txt ./data.txt`<br/>
<br/> -> **-u** serves to set up force remote DNS server connection
<br/> -> **{BASE_HOST}** serves to set up base domain for sending DNS packets
<br/> -> **{DST_FILEPATH}** serves to set up path for the file with received data
<br/> -> **[SRD_FILEPATH]** File with input data. If not used as a parameter, then data is read from STDIN.

<br /><br />
## List of uploaded files
* receiver/dns_receiver_events.c -> source file of DNSreceiver application
* receiver/dns_receiver_events.h -> header file of DNSreceiver application
* sender/dns_sender_events.c -> source file of DNSsender application
* sender/dns_sender_events.h -> header file of DNSsender application
* base32.c -> source file of base32 hashing library
* base32.h -> header file of base32 hashing library
* LICENSE
* Makefile
* README.md
    
