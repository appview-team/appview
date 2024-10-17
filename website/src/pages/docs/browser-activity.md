---
title: Inspecting Browser Activity
---

## Inspecting Browser Activity

In this blog entry, we will use AppView to learn about a browser's network and file activity during a web browsing session.

### Using Firefox and Chrome

Both of these browsers use a sandbox. It is a security feature, intended to isolate the browser processes. Neither of the sandboxes detect the AppView library. The intercept behavior utilized by the library does have undesirable effects on the browser, so we recommend an AppView of the browser without the use of the sandbox.The Chrome sandbox can be disabled from the command line.  The Firefox sandbox is disabled by use of 2 environment variables. 

The detection mechanisms employed by the library are intended to notify about potential security concerns in any given process (we should create a blog describing detection as a precursor to this?). There is behavior used by both browsers that are notified as potential security issues. For example, Chrome OS replaces most standard libc functions by modifying the GOT. The library detects this as a potential issue. It is a viable potential issue in many cases. The library is performing as expected. At the same time, this causes a large number of notifications to be generated. Therefore, these specific notifications can be disabled. Moreover, FIrefox emits a log warning when message queue functions are utilized. The library uses message queue to create a communication channel with an external appview CLI process. If we choose to disable these warnings we disable the use of message queues as an IPC by the library. One other behavior of note from Firefox relates to increased use of physical resources. Firefox processes increase limits on the number of pending signals, for example.  The limit is set to 50,000 and 200,000 max. These seem like large values. Makes sense that library detection would notify about this behavior. These notifications can be disabled if desired.  

### Managing AppView Notifications

AppView notifications can be disabled by setting the following environment variables:

`APPVIEW_IPC_ENABLE=false`

Disables any IPC with a CLI and the use of mq\_open.

`APPVIEW_NOTIFY_FUNCS=false`

Firefox increases resource usage of several resources and detection catches this and creates security events. This doesn't hurt anything. Just a notification. If we don't want to notify on this, it can be disabled.

`APPVIEW_NOTIFY_LIBS=false`

Chrome replaces many libc functions with its' own version. This activity creates a lot of security events. Assuming we don't want to get overwhelmed by these events, they can be disabled..

### AppView and Firefox

AppView Firefox without the Mozilla sandbox:

```
$ export MOZ_DISABLE_CONTENT_SANDBOX=1
$ export MOZ_DISABLE_SOCKET_PROCESS_SANDBOX=1
```

Both should be defined in order to disable the sandbox.

Starting the session:
```
$ appview firefox
```
  
If setting the sandbox environment variables for all processes in the current shell, as defined above, is undesirable, the following command line can be used to create the same behavior for one Firefox session:

```
$ MOZ_DISABLE_CONTENT_SANDBOX=1 MOZ_DISABLE_SOCKET_PROCESS_SANDBOX=1 appview firefox
```

### AppView and Chrome

AppView Chrome without the sandbox:

```
$ appview /opt/google/chrome/chrome --no-sandbox
```

### Create a Report Describing Browser Activity

Appview is able to create a report describing behavior of a viewed process. 
  
```
$ appview report
```

The report produces a list of network connections made, and files accessed, as follows:

```
================
Network Activity
================
IP             	PORT	URL                                                             	DURATION	BYTES SENT	BYTES RECEIVED
44.238.192.228 	443 	shavar.services.mozilla.com                                     	324     	1968      	3366
142.250.114.94 	80  	o.pki.goog                                                      	178750  	16245     	26097
172.66.41.9    	443 	-                                                               	43120   	13149     	140548
159.203.79.158 	443 	sentry.immergo.tv                                               	60915   	4281      	12014
23.105.12.142  	443 	ssbsync.smartadserver.com                                       	24996   	3307      	6007
68.67.160.76   	443 	-                                                               	32710   	24562     	34489
8.2.110.161    	443 	cm-x.mgid.com                                                   	29267   	7601      	27068
147.28.146.89  	443 	-                                                               	25809   	1576      	3599
140.82.113.6   	443 	api.github.com                                                  	36942   	2377      	5815
216.239.32.178 	443 	www.google-analytics.com                                        	33677   	4111      	6436
104.18.20.226  	80  	ocsp.globalsign.com                                             	27618   	910       	3803
34.120.237.76  	443 	img-getpocket.cdn.mozilla.net                                   	58250   	12990     	147956
169.150.236.105	443 	cdn.pbxai.com                                                   	34291   	1450      	7043
3.225.218.10   	443 	ups.analytics.yahoo.com                                         	28732   	2662      	9487
174.137.133.32 	443 	sync.adkernel.com                                               	19202   	13342     	26617
34.36.165.17   	443 	tiles-cdn.prod.ads.prod.webservices.mozgcp.net                  	57813   	3336      	41930
104.18.35.167  	443 	cdn-ima.33across.com                                            	32787   	1330      	19912
142.250.115.147	443 	www.google.com                                                  	30866   	3000      	7481
34.117.228.201 	443 	tpsc-ue1.doubleverify.com                                       	76520   	41649     	54451
35.81.64.53    	443 	-                                                               	20225   	1391      	7110
54.230.202.114 	443 	assets.revcontent.com                                           	573     	1191      	57778
140.82.114.3   	443 	github.com                                                      	32989   	2014      	96887
104.18.17.212  	443 	-                                                               	34523   	923       	3738
51.222.39.184  	443 	onetag-sys.com                                                  	28980   	4111      	14086
34.232.40.247  	443 	-                                                               	31200   	3181      	22487
52.8.114.4     	443 	tlx.3lift.com                                                   	33226   	24778     	15820
18.160.102.45  	443 	snippet.univtec.com                                             	43231   	4763      	569467
151.101.130.217	443 	-                                                               	30578   	977       	5027
18.210.249.120 	443 	ad.360yield.com                                                 	28703   	2091      	7885
35.244.181.201 	443 	-                                                               	24372   	2394      	16376
35.190.72.216  	443 	-                                                               	26814   	1801      	7347
74.125.126.154 	443 	-                                                               	174     	872       	4993
3.213.208.246  	443 	ce.lijit.com                                                    	18671   	1926      	5727
34.149.97.1    	443 	firefox-api-proxy.cdn.mozilla.net                               	116544  	2318      	15936
100.25.172.61  	443 	-                                                               	17349   	2301      	13625
104.18.30.13   	443 	www.newsnetmedia.com                                            	36026   	13025     	822493
104.18.16.212  	443 	clientcontent.franklyinc.com                                    	97881   	4002      	15344
142.250.113.95 	443 	fonts.googleapis.com                                            	483     	1251      	6549
198.8.71.131   	443 	p.rfihub.com                                                    	41069   	7709      	47378
69.173.156.149 	443 	pixel-eu.rubiconproject.com                                     	33601   	5842      	9349
18.160.102.129 	443 	-                                                               	24190   	1438      	10133
104.26.9.169   	443 	-                                                               	11366   	2482      	586
...
# truncated for brevity
```

