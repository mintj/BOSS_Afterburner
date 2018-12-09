# Setup of your BOSS environment

{% hint style="danger" %}
**Warning:** In it's current version, this tutorial assumes you use a `bash` terminal.
{% endhint %}

I advise you to set up your environment in the same way I did. There are two main directories that you will be using: \(1\) the _workarea_, which contains your run scripts for **BOSS**, and \(2\) the _BOSS Afterburner_ repository, which contains your analysis code.

I placed the _workarea_ folder **within** the BOSS Afterburner repository \(`BOSS_Afterburner/boss`\) so that it is available as an example, but another common procedure is to put it in the `ihepbatch` folder \(see [Organisation of the IHEP server](ihep-server.md)\). The BOSS Afterburner is best placed in the `besfs` folder. Data generated with **BOSS** through the _workarea_ scripts will then be written to the `BOSS_Afterburner/data` subdirectory and analysis plots to the `BOSS_Afterburner/plots` folder.

## \(1\) Set up your _workarea_

{% hint style="warning" %}
**@todo** Test the procedure described below!

**@todo** Rewrite for TC shell.
{% endhint %}

You will be running your **BOSS** analyses from this folder. It contains a _Configuration Management Tool_ folder \([`cmthome-*`](https://github.com/redeboer/BOSS_Afterburner/tree/master/boss/cmthome-7.0.4)\), which is used to set up path variables for **BOSS**, and a `workarea-*` folder where you develop your own **BOSS** packages \(including the `jobOptions*.txt` files\). Here, the \* stands for the version of BOSS you are using.

### **Step 1:** Go to your workarea folder

For the sake of making this tutorial work in a general setting, we will first define a `bash` variable here:

```text
BOSSWORKAREA="/besfs/users/$USER/BOSS_Afterburner/boss"
```

You can change what is between the quotation marks `"` by whatever folder you prefer, for instance by `/ihepbatch/bes/$USER`. As you see, we'll use the `BOSS_Afterburner/boss` folder in this tutorial.

Now, move into that directory.

```text
cd "$BOSSWORKAREA"
```

At this stage, you'll have to decide which version of BOSS you have to use. At the time of writing, **version 7.0.4** is the latest stable version, though it could be that for your analysis you have to use data sets that were reconstructed with older versions of **BOSS**. Here, I'll just use `7.0.4`, but you can replace this number with whatever version you need.

Again, we'll therefore define it as a variable here.

```text
BOSSVERSION=7.0.4
```

### **Step 2: Import environment scripts**

We'll first have to obtain some scripts that can set up the necessary references to **BOSS**. This is done by copying the `cmthome-*` folder from BOSS Software directory to the `ihepbatch` folder where you currently are:

```text
cp -Rf /afs/ihep.ac.cn/bes3/offline/Boss/cmthome/cmthome-$BOSSVERSION cmthome
```

and navigate into that copy:

```text
cd cmthome*
```

### **Step 3: Modify `requirements`**

You'll now have to modify the file called `requirements` so that it handles your username properly. We'll use the `vi` editor here, but you can use whatever editor you prefer:

```text
vi requirements
```

The file contains the following lines:

```bash
#macro WorkArea "/ihepbatch/bes/maqm/workarea"

#path_remove CMTPATH "${WorkArea}"
#path_prepend CMTPATH "${WorkArea}"
```

Uncomment them \(remove the hash `#`\) and what is between the first quotation marks `"..."` with your the path to your workarea. In the case of having the workarea within the BOSS Afterburner, it looks like this:

```bash
macro WorkArea "/besfs/users/$USER/BOSS_Afterburner/boss/workarea"

path_remove CMTPATH "${WorkArea}"
path_prepend CMTPATH "${WorkArea}"
```

{% hint style="info" %}
Note that `$CMTPATH` will actually be set to a subfolder called `workarea` **within** what we defined as the _workarea_. We will create this subfolder later.
{% endhint %}

### **Step 4: Set references to BOSS**

Now you can use the scripts in `cmthome` to set all references to **BOSS** at once, using:

```bash
source setupCMT.sh  # starts connection to the CMT
cmt config          # initiates configuration
source setup.sh     # sets path variables
```

Just to be sure, you can check whether the path variables have been set correctly:

```text
echo $CMTPATH
```

If everything went well, it should print something like:

```text
/besfs/bes/$USER/BOSS_Afterburner/boss/workarea:/afs/ihep.ac.cn/bes3/offline/Boss/7.0.4:/afs/ihep.ac.cn/bes3/offline/ExternalLib/SLC6/ExternalLib/gaudi/GAUDI_v23r9:/afs/ihep.ac.cn/bes3/offline/ExternalLib/SLC6/ExternalLib/LCGCMT/LCGCMT_65a
```

### **Step 5: Modify your `bashrc`**

Adapt your `bash` profile \(`.bash_profile`\) and _run commands_ file \(`.bashrc`\) so that **BOSS** is loaded automatically every time you log into the server. The easy solution is simply copy-pasting and running these commands:

```bash
echo -e "if test -f .bashrc; then\n\tsource .bashrc\nfi" >> ~/.bash_profile
echo "BOSSVERSION=7.0.4" >> ~/.bashrc
echo "BOSSWORKAREA=\"/besfs/users/$USER/BOSS_Afterburner/boss\"" >> ~/.bashrc
echo "CMTHOME=\"/afs/ihep.ac.cn/bes3/offline/Boss/cmthome/cmthome-$BOSSVERSION\"" >> ~/.bashrc
echo "IHEPBATCH=\"/ihepbatch/bes/$USER\"" >> ~/.bashrc
echo "source $BOSSWORKAREA/cmthome/setupCMT.sh" >> ~/.bashrc
echo "source $BOSSWORKAREA/cmthome/setup.sh" >> ~/.bashrc
echo "source $BOSSWORKAREA/workarea/TestRelease/TestRelease-00-00-86/cmt/setup.sh" >> ~/.bashrc
echo "export PATH=$PATH:/afs/ihep.ac.cn/soft/common/sysgroup/hep_job/bin/" >> ~/.bashrc
```

However, to avoid becoming a [copy ninja](https://pics.me.me/kakashi-went-from-the-copy-ninja-to-the-copy-paste-14969048.png), you'd better modify these files yourself. First, add these lines to your bash profile \(`vi ~/bash_profile`\):

```bash
if test -f .bashrc; then
    source .bashrc
fi
```

These lines force the server to load your `.bashrc` run commands file when you log in. In that file, you should add the following lines:

```bash
BOSSVERSION=7.0.4
BOSSWORKAREA="/besfs/users/$USER/BOSS_Afterburner/boss"
CMTHOME="/afs/ihep.ac.cn/bes3/offline/Boss/cmthome/cmthome-$BOSSVERSION"
IHEPBATCH="/ihepbatch/bes/$USER"
source "$BOSSWORKAREA/cmthome/setupCMT.sh"
source "$BOSSWORKAREA/cmthome/setup.sh"
source "$BOSSWORKAREA/workarea/TestRelease/TestRelease-00-00-86/cmt/setup.sh"
export PATH=$PATH:/afs/ihep.ac.cn/soft/common/sysgroup/hep_job/bin/
```

Notice that the commands we used in **Step 4** are used here again. There is also a reference to the `workarea` \(see **Step 7**\). The last line allows you to submit **BOSS** jobs to the queue \(`hep_sub`\) — for now, don't worry what this means.

You can now either log in again to the server or use `source ~/.bashrc` to reload the run commands.

### **Step 6: Test it `boss.exe`**

To test whether everything went correctly, you can try to run **BOSS**:

```text
boss.exe
```

It should result in a \(trivial\) error message like this:

```text
               BOSS version: 7.0.4
************** BESIII Collaboration **************

the jobOptions file is : jobOptions.txt
ERROR! the jobOptions file is empty!
```

### **Step 7: Create your** _**workarea**_

It is convention to place your **BOSS** packages in a _workarea_ folder next to the `cmthome` folder we have been using so far. Let's create it:

```text
cd /ihepbatch/bes/$USER
mkdir workarea-7.0.4
```

We'll get back to this folder when we [set up a BOSS package](../#set-up-a-boss-package).

## \(2\) Set up the BOSS Afterburner

This is the simple part. Go to the _BES file system_ folder:

```text
cd /besfs/users/$USER
```

[Clone](https://help.github.com/articles/cloning-a-repository/) the _BOSS Afterburner_ repository:

```text
git clone https://github.com/redeboer/BOSS_Afterburner
```

The _BOSS Aafterburner_ will be used to analyse output from **BOSS**. We get back to this in the section [Data analysis](data-analysis.md).
