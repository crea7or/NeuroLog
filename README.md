# NeuroLog
 NeuroLog is a web log analyzer that summarize requests from the same network subnets to detect robots activity

1) Databases with current network subnets (netblocks) should be downloaded before first use.
The exact links for each of the datasets are as follows:
ftp.arin.net/pub/stats/arin/delegated-arin-extended-latest
ftp.ripe.net/ripe/stats/delegated-ripencc-latest
ftp.afrinic.net/pub/stats/afrinic/delegated-afrinic-latest
ftp.apnic.net/pub/stats/apnic/delegated-apnic-latest
ftp.lacnic.net/pub/stats/lacnic/delegated-lacnic-latest
(rename them to *.txt if they not)

2) Log files format that NeuroLog can parse:
1.2.3.4 - [11/Feb/2017:00:00:07 -0400] "GET /github/ HTTP/1.1" 200 7520 "https://www.github.com" "Mozilla/5.0 ..."

![screenshot](/neurolog.png?raw=true "screenshot")

3) Little help:
"NIC Files diretory" - folder where you saved databases with current network subnets.

"Log files directory" - where you log files.

"Log files mask" - mask for files, DOS notation ( * - any symbol etc.)

"Minimum hits to report" - subnets with less hits will not be reported.

"Minimum downloaded" - subnets with less downlaods (in bytes) will not be reported.

"Output directory" - where to place report and reparsed for efficiency databases with network subnets.

4) Hit start and NeuroLog should open html page with report when it will finish.
