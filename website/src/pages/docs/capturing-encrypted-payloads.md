---
title: Capturing Encrypted Payloads
---

<span id="capturing-encrypted-payloads"></span>

# Capturing Encrypted Payloads

AppView has the unique capability of capturing application payloads without any need for the user to provide any keys or configuration.

## HTTPS

```
appview run --payloads curl https://www.gnu.org  # Make a HTTPS request to a website with AppView loaded
appview flows                                    # Inspect the network flows. Look for one with inbound data.
appview flows <id>                               # Look at that flow in more detail
appview flows <id> --in                          # Look at the decrypted inbound payload (the web server's response)
```

