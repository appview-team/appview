---
title: Cribl Edge on a Host
---

# Running AppView With Cribl Edge on a Host

Host, in this context, means a standalone Linux host or Virtual Machine.

This topic walks you through a setup procedure, and then through the two basic AppView techniques:

1. **AppView by PID** – Instrumenting one process on one host.
1. **AppView by Rule** – Instrumenting multiple processes on an entire Edge Fleet.

We assume that you have an Edge Leader running in Cribl.Cloud, and that you want to add a new Edge Node to its Fleet, and to appview processes running on the new Edge Node.

You can easily modify these instructions to add more than one Edge Node and then use AppView by Rules to appview processes on the entire Fleet.

## Setting Up a New Edge Node

In Cribl.Cloud:

1. Click **Manage Edge**.
2. Select the Fleet where you’ll be adding a Linux host – `default_fleet` is fine.
3. From the **Add/Update Edge Node** drop-down, select **Linux** > **Add** to open the **Add Linux Node** modal.
4. Change the value of **User** from `cribl` to `root`.
5. Click **Copy script** and dismiss the modal.
    - Note: Several parameters provided on this modal can alter the contents of the script. Defaults are fine for this example.
6. Note the value of **Edge Nodes** at upper left.

On the Linux host we want to observe with Edge:

1. Open a shell. 
2. Paste the script into the shell. 
3. Edit the script so that it runs as root, by adding `sudo` to the bash command at the end.
    - For example, `curl 'https://...%2Fcribl' | bash - ` ... should be edited as follows:
    - `curl 'https://... %2Fcribl' | sudo bash - `
1. Execute the script, and give it a minute or so to complete.

Return to Cribl.Cloud UI to verify that the new Node is present:

1.  The value of **Edge Nodes** should have increased by 1.
2. Select **List View** (and filtering by host if needed) – the new Edge Node should appear in the list.  

At this point, Edge is installed on a Linux container in its default location (`/opt/cribl`), is running, and is connected to the Edge Leader.  

## Scoping by PID

Still in Edge’s **List View** tab:

1. Click the GUID for the host we’ve just added.
2. Click the count of **running processes** at upper left to open a list view of processes.

On the Linux host: 

- In a shell, start a process you want to appview.  For this example, let’s start `top`.  

Back in Edge: 

Within seconds, `top` should appear in the process list. (If you don’t see it, try filtering by command.) At this point `top` is running but is not yet instrumented by AppView.

1. Select the `top` command's row to open the **Process: top** drawer.
2. In the **AppView** tab, select the AppView **Configuration** we want to use for this process. 
    - Select `A sensible AppView configuration ...` .  
3. Leave the default **Source** as `in_appview`.
    - If you use a different AppView Source, configure that Source to set **General Settings** > **Optional Settings** > **UNIX socket permissions** to `777`.
4. Click **Start monitoring**.  
    - After a minute or so, you should see green checkmarks in all the Status columns.

Now you’ll want to confirm that Edge is receiving AppView data for the viewed `top` process.

On the Edge Leader, navigate to **More** > **Sources** and select **AppView** to open the Source page. 

In the row where `ID` is `in_appview`: 
- The Source should be enabled.
- Socket should be set to `$CRIBL_HOME/state/appview.sock`.  
- In the Status column, click **Live**, and you should now see events flowing.

## Scoping by Rule

On the Linux host: 

- In your shell, start a process you want to appview.  Again, let’s  run `top`.  

On the Edge Leader:

- Navigate to **More** > **Sources** and select **AppView** to open the Source page. 
- Select the row where `ID` is `in_appview` .

In the AppView Rules tab, under Rules, click **Add Rule** and complete the Rule as follows:

- **Process name**: `top`
- **Process argument**: Skip this, because **Process name** and **Process argument** are mutually exclusive.
- **AppView config**: Select `A sensible AppView configuration ...` .  

Next:

1. Click **Save**.
2. Click **Commit** and provide a commit message. 
3. Click **Commit and Deploy**.

Wait for the changes to be deployed to the Edge Node – probably around 30 seconds. 

- Click the **Live Data tab**, and you should now see events flowing. Next, we'll show that any process matching your Rule gets viewed.

Back on the Linux host:

1. In your shell, start **another** `top` process.
2. Click the **Live Data tab**.
   
You should now see events flowing from two `top` processes – they’ll have different PIDs.

Return to Edge’s **List View** tab – the **AppView** column should indicate that both `top` processes are being viewed.

This gets to what’s so powerful about Rules: You can start processes **after** setting things up, and they get viewed.

Optionally, you can try the other two configurations and see how the viewed data changes. For the config that has payloads enabled, `curl` and `wget` are good choices.

If you want to do more, consider creating Routes and Pipelines in Cribl Edge, to send AppView data to your favorite Destination(s).
