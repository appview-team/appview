# AppView Rules

## Scenarios we intend to support

### Scenario: Rules deployed from a container (i.e. Cribl Edge running in a container)

Start Cribl Edge Container (as defined in Cribl Edge documentation):
```
docker run -it --rm -e CRIBL_EDGE=1  -p 9420:9420 -v /<path_to_code>/appview:/opt/appview  -v /var/run/appview:/var/run/appview  -v /var/run/docker.sock:/var/run/docker.sock  -v /:/hostfs:ro  --privileged  --name cribl-edge cribl/cribl:latest bash
/opt/cribl/bin/cribl start
```
Tests:
```
sudo touch /etc/ld.so.preload # for safety
sudo chmod ga+w /etc/ld.so.preload # for safety
<start edge container>
<run top on the host>
<start a container>
<run top in that container>
appview rules --add top --sourceid A --rootdir /hostfs --unixpath /var/run/appview
appview rules --rootdir /hostfs
### Does the rules file contain an entry for top?
appview ps --rootdir /hostfs
### Are two top processes viewed by attach?
<run top on the host>
<start a new container>
<run top in the new container>
appview ps --rootdir /hostfs
### Are four top processes viewed (2 by attach, 2 by preload)?
### Is data flowing into edge from 3 processes (2 on host, 1 in new container)?
appview rules --remove top --sourceid A --rootdir /hostfs
appview rules --rootdir /hostfs
### Is the rules file empty?
appview ps --rootdir /hostfs
### Are 0 top processes viewed?
<run top on the host>
appview ps --rootdir /hostfs
### Are 0 top processes viewed?
### A unix sock path is supported on the rules add command line. it will place the unix path in the rules file where the config from Edge is placed. 
sudo appview rules --add top --unixpath /var/run/appview
at the end of the rules file we will see this:
source:
  unixSocketPath: /var/run/appview
  authToken: ""
the result is that /var/run/appview is mounted in new containers.
```

### Scenario: Rules deployed from a host (i.e. Cribl Edge running on the host)

Start Cribl Edge (as defined in Cribl documentation):
```
<switch the user to root>
curl https://cdn.cribl.io/dl/4.1.3/cribl-4.1.3-15457782-linux-x64.tgz -o ~/Downloads/cribl.tgz
cd /opt/
tar xvzf ~/Downloads/cribl.tgz
mv /opt/cribl/ /opt/cribl-edge
export CRIBL_HOME=/opt/cribl-edge # note: $CRIBL_HOME is set only in the cribl process (and cli children)
cd /opt/cribl-edge/bin
./cribl mode-edge
chown root:root /opt/cribl-edge/bin/cribl
./cribl start
```

Tests:
```
sudo touch /etc/ld.so.preload # for safety
sudo chmod ga+w /etc/ld.so.preload # for safety
<start edge on host>
<run top on the host>
<start a container>
<run top in that container>
appview rules --add top --sourceid A --unixpath /var/run/appview
appview rules
### Does the rules file contain an entry for top?
appview ps
### Are two top processes viewed by attach?
<run top on the host>
<start a new container>
<run top in the new container>
appview ps
### Are four top processes viewed (2 by attach, 2 by preload)?
### Is data flowing into edge from three processes (2 on host, 1 in new container)?
appview rules --remove top --sourceid A
appview rules
### Is the rules file empty?
appview ps
### Are 0 top processes viewed?
<run top on the host>
appview ps
### Are 0 top processes viewed?
```

### Where files will be created

Host processes:
- libappview: should end up in `/usr/lib/appview/<ver>/` on the host
- appview: should end up in `/usr/lib/appview/<ver>/` on the host
- appview_rules: should end up in `/usr/lib/appview/appview_rules` on the host
- unix socket: 
  - edge running in container: will be in `/var/run/appview/` on the host by default (edge documentation describes that `/var/run/appview` is mounted from the host into the container). 
  - edge running on host: will be in `$CRIBL_HOME/state/` by default

Existing container processes:
- libappview: installed into /usr/lib/appview/<ver>/ in all existing containers (/etc/ld.so.preload points to this)
- appview: _not required_
- appview_rules: `/usr/lib/appview` should be mounted into all existing containers into `/usr/lib/appview/`
- unix socket: the dirpath defined in `appview_rules` should be mounted into all existing containers (`$CRIBL_HOME/state/` note that the env var will be resolved in the appview_rules file)

New container processes:
- libappview: extracted into `/opt/appview` in all new containers
- appview: `/usr/lib/appview` should be mounted into all new containers into `/usr/lib/appview/`
- appview_rules: `/usr/lib/appview` should be mounted into all new containers
- unix socket: the dirpath defined in `appview_rules` should be mounted into all new containers (default `/var/run/appview/`)
